/*
 * TMNT IV Turtles in Time — desktop host shim.
 *
 * Identity and hooks, nothing else. The host itself is the framework's
 * (snesrecomp/runner/src/desktop/host_main.h): the pre-boot launcher, ROM
 * resolution and digest checks, config.ini and keybinds.ini, the window and
 * the SDL / OpenGL presenters, audio, gamepads, the in-game save-state
 * browser and rewind filmstrip, the OSD, the pacing clock, crash handlers and
 * the post-mortem report, mod packages, Generate & rebuild, and the netplay
 * barrier when the project is built with it. A fix there reaches this
 * project on a submodule pull; nothing in this file needs to change for it.
 *
 * This file used to BE the host -- a ~700-line copy made at scaffold time --
 * so every project scaffolded before a fix kept the bug. Two ports of the
 * same game, a week apart, had different hosts.
 *
 * Nothing here ever ships a ROM. The player supplies one; the host makes
 * that easy (launcher, then positional argument, then a copy beside the
 * executable, then the rom.cfg cache, then a file picker) and checks what it
 * is handed against the digests in rom_identity.txt.
 *
 * Grow this file only with what is specific to THIS title: a custom
 * presenter (prepare_frame / draw_frame), an SPC player, a pacing rule
 * (keep_pacing_debt), a Mods provider. See SnesDesktopHostGame for the
 * complete list of hooks and what each is for.
 */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "host_main.h"
#include "game_rtl.h"
#include "common_rtl.h"
#include <string.h>
#include "snesrecomp_rom_identity.h"  /* generated from rom_identity.txt */

#ifndef __ANDROID__
#define SDL_MAIN_HANDLED 1
#endif
#include "desktop/sdl_compat.h"
#ifdef __ANDROID__
/* SDLActivity loads libmain.so and dlsyms its SDL_main. */
#define main SDL_main
#endif

#ifndef SNES_GAME_VERSION
#define SNES_GAME_VERSION "dev"
#endif

static const SnesDesktopHostGame kGameHost = {
    .display_name        = "TMNT IV Turtles in Time",
    .window_title        = "TMNT IV Turtles in Time",
    .region              = SNESRECOMP_ROM_REGION,
    .rom_file            = SNESRECOMP_ROM_FILE,
    .expected_sha256_hex = SNESRECOMP_ROM_EXPECTED_SHA256,
    .expected_crc32_hex  = SNESRECOMP_ROM_EXPECTED_CRC32,
    .game_id             = SNESRECOMP_ROM_GAME_ID,
    .build_version       = SNES_GAME_VERSION,
    .game_info           = &kGameInfo,
    .num_players         = 2,
    /* Battery-backed SRAM shows the launcher's SAVES panel. Leave NULL for a
     * title without one. The path is exe-relative. */
    .sram_path           = NULL, /* USA cartridge has no battery SRAM. */
};

#ifndef __ANDROID__
#undef main   /* desktop: keep plain main() even if SDL_main.h remapped it */
#endif
#ifdef TMNT_RETAIL_WINDOWS
static int retail_run(int argc, char **argv) { return snesrecomp_desktop_main(&kGameHost, argc, argv); }
#endif
int main(int argc, char **argv)
{
#ifdef _WIN32
    wchar_t root[1024]; char utf8[3072];
    DWORD n = GetEnvironmentVariableW(L"LOCALAPPDATA", root, 1024);
    if (!n || n > 950) goto save_path_error;
    wcscat_s(root, 1024, L"\\TMNTIVRecomp");
    if (!CreateDirectoryW(root, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) goto save_path_error;
    wcscat_s(root, 1024, L"\\saves");
    if (!CreateDirectoryW(root, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) goto save_path_error;
    if (!WideCharToMultiByte(CP_UTF8, 0, root, -1, utf8, sizeof(utf8), NULL, NULL) ||
        strlen(utf8) >= 1024) goto save_path_error;
    RtlSetSaveRoot(utf8);
    RtlSetQuickSaveRoot(utf8);
#endif
#ifdef TMNT_RETAIL_WINDOWS
    extern int tmnt_windows_main(int, char **, int (*)(int, char **));
    return tmnt_windows_main(argc, argv, retail_run);
#else
    return snesrecomp_desktop_main(&kGameHost, argc, argv);
#endif
#ifdef _WIN32
save_path_error:
    MessageBoxW(NULL, L"Não foi possível preparar a pasta de saves em LocalAppData.\nCannot prepare the save folder in LocalAppData.",
                L"TMNT IV", MB_OK | MB_ICONERROR);
    return 1;
#endif
}
