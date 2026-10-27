/* Windows retail ROM gate. Never strips copier headers or trusts filenames. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <stdlib.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdint.h>
#include "sha256.h"
#include "snesrecomp_rom_identity.h"

static const wchar_t canonical[] = L"Teenage Mutant Ninja Turtles IV - Turtles in Time (USA).sfc";
static const wchar_t title[] = L"TMNT IV - Turtles in Time PC Recomp";
static void error(const wchar_t *detail) {
    MessageBoxW(NULL, detail, title, MB_OK | MB_ICONERROR);
}
/* Hold the source against writes/deletion until its verified bytes are copied. */
static unsigned char *read_rom(const wchar_t *path) {
    HANDLE f = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    LARGE_INTEGER size;
    unsigned char *data = NULL;
    DWORD got;
    if (f == INVALID_HANDLE_VALUE) return NULL;
    if (GetFileSizeEx(f, &size) && size.QuadPart == 1048576) {
        data = malloc(1048576);
        if (data && (!ReadFile(f, data, 1048576, &got, NULL) || got != 1048576)) {
            free(data); data = NULL;
        }
    }
    CloseHandle(f);
    if (data) {
        unsigned char digest[32]; char hex[65];
        sha256_compute(data, 1048576, digest);
        for (int i = 0; i < 32; ++i) sprintf(hex + 2*i, "%02x", digest[i]);
        if (_stricmp(hex, SNESRECOMP_ROM_EXPECTED_SHA256)) { free(data); data = NULL; }
    }
    return data;
}
static int store_rom(const wchar_t *dir, const wchar_t *dest, const unsigned char *data) {
    wchar_t temp[MAX_PATH]; DWORD written; int ok = 0;
    if (!GetTempFileNameW(dir, L"rom", 0, temp)) return 0;
    HANDLE f = CreateFileW(temp, GENERIC_WRITE, 0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f != INVALID_HANDLE_VALUE) {
        ok = WriteFile(f, data, 1048576, &written, NULL) && written == 1048576 && FlushFileBuffers(f);
        CloseHandle(f);
    }
    if (ok) ok = MoveFileExW(temp, dest, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    if (!ok) DeleteFileW(temp);
    return ok;
}
/* --validate-rom and --import-rom are noninteractive release acceptance probes. */
int tmnt_windows_main(int argc, char **argv, int (*run)(int, char **)) {
    int wc; wchar_t **wa = CommandLineToArgvW(GetCommandLineW(), &wc);
    if (!wa) return 1;
    int validate = wc == 3 && !wcscmp(wa[1], L"--validate-rom");
    int import = wc == 3 && !wcscmp(wa[1], L"--import-rom");
    if (validate) { unsigned char *p = read_rom(wa[2]); int ok = p != NULL; free(p); LocalFree(wa); return ok ? 0 : 2; }
    wchar_t dir[MAX_PATH], dest[MAX_PATH], selected[MAX_PATH] = L"";
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, dir)) ||
        wcslen(dir) + 110 >= MAX_PATH) { LocalFree(wa); return 1; }
    wcscat_s(dir, MAX_PATH, L"\\TMNTIVRecomp");
    if (SHCreateDirectoryExW(NULL, dir, NULL) != ERROR_SUCCESS && GetFileAttributesW(dir) == INVALID_FILE_ATTRIBUTES) {
        error(L"Não foi possível criar a pasta de dados. / Cannot create the user data folder."); LocalFree(wa); return 1;
    }
    swprintf_s(dest, MAX_PATH, L"%s\\%s", dir, canonical);
    unsigned char *data = import ? NULL : read_rom(dest);
    if (data) free(data);
    else for (;;) {
        if (import) wcscpy_s(selected, MAX_PATH, wa[2]);
        else {
            OPENFILENAMEW picker = {0}; picker.lStructSize = sizeof(picker);
            picker.lpstrTitle = L"Selecione sua ROM USA original / Select your original USA ROM";
            picker.lpstrFilter = L"SNES ROM (*.sfc)\0*.sfc\0Todos os arquivos / All files\0*.*\0";
            picker.lpstrFile = selected; picker.nMaxFile = MAX_PATH;
            picker.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
            if (!GetOpenFileNameW(&picker)) { LocalFree(wa); return 0; }
        }
        data = read_rom(selected);
        if (!data) {
            if (import) { LocalFree(wa); return 2; }
            error(L"ROM incompatível, corrompida ou ilegível. Selecione a versão USA original sem cabeçalho (1048576 bytes).\n\nIncompatible, corrupt or unreadable ROM. Select the original USA version without a copier header (1048576 bytes).\n\nSHA-256: 5b82cdd6f2da56f43680d6a5021faebe2e06036d30602c1a7917aa414cf8b5f4");
            continue;
        }
        int ok = store_rom(dir, dest, data); free(data);
        if (!ok) { if (!import) error(L"Não foi possível salvar a ROM. / Cannot save the ROM."); LocalFree(wa); return 3; }
        break;
    }
    LocalFree(wa);
    if (import) return 0;
    /* The retail executable manifest opts the CRT into UTF-8 paths. */
    char rom[MAX_PATH*3], config[MAX_PATH*3]; wchar_t cfg[MAX_PATH];
    swprintf_s(cfg, MAX_PATH, L"%s\\config.ini", dir);
    if (GetFileAttributesW(cfg) == INVALID_FILE_ATTRIBUTES &&
        !WritePrivateProfileStringW(L"Graphics", L"Fullscreen", L"1", cfg)) {
        error(L"Não foi possível salvar a configuração. / Cannot save settings.");
        return 1;
    }
    if (!WideCharToMultiByte(CP_UTF8, 0, dest, -1, rom, sizeof(rom), NULL, NULL) ||
        !WideCharToMultiByte(CP_UTF8, 0, cfg, -1, config, sizeof(config), NULL, NULL)) return 1;
    SetCurrentDirectoryW(dir);
    char *args[] = {argv[0], "--launcher", "--config", config, "--rom", rom, NULL};
    return run(6, args);
}
