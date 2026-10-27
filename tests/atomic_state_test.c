/* Fault injection wraps Windows I/O, while exercising the production writer. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
static int failure;
static BOOL WINAPI test_write(HANDLE h,LPCVOID p,DWORD n,LPDWORD wrote,LPOVERLAPPED o) {
    if (failure==1 || failure==5) {
        WriteFile(h,p,n/2,wrote,o);
        if(failure==5) ExitProcess(77); /* killed before flush/rename */
        return FALSE;
    }
    return WriteFile(h,p,n,wrote,o);
}
static BOOL WINAPI test_flush(HANDLE h) { return failure==2?FALSE:FlushFileBuffers(h); }
static BOOL WINAPI test_close(HANDLE h) { BOOL ok=CloseHandle(h);return failure==3?FALSE:ok; }
static BOOL WINAPI test_move(LPCWSTR a,LPCWSTR b,DWORD f) {
    return failure==4?FALSE:MoveFileExW(a,b,f);
}
#define WriteFile test_write
#define FlushFileBuffers test_flush
#define CloseHandle test_close
#define MoveFileExW test_move
#include "atomic_state_file.c"
#undef WriteFile
#undef FlushFileBuffers
#undef CloseHandle
#undef MoveFileExW
static const char path[]="atomic-last-good.state";
static void check(const char *expected) {
    char data[100]={0}; FILE *f=fopen(path,"rb"); assert(f);
    size_t n=fread(data,1,sizeof(data),f); fclose(f);
    assert(n==strlen(expected) && !memcmp(data,expected,n));
}
int main(int argc,char **argv) {
    if(argc>1) { failure=5; snes_atomic_state_write(path,"interrupted",11);return 99; }
    assert(snes_atomic_state_write(path,"original",8));
    for(failure=1;failure<=4;++failure) {
        assert(!snes_atomic_state_write(path,"replacement",11)); check("original");
    }
    failure=0;
    HANDLE locked=CreateFileA(path,GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
    assert(locked!=INVALID_HANDLE_VALUE);
    assert(!snes_atomic_state_write(path,"replacement",11));
    CloseHandle(locked); check("original");
    char exe[1024],cmd[1200]; assert(GetModuleFileNameA(NULL,exe,sizeof(exe)));
    sprintf(cmd,"\"%s\" interrupt",exe);
    STARTUPINFOA si={sizeof(si)}; PROCESS_INFORMATION pi;
    assert(CreateProcessA(NULL,cmd,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi));
    assert(WaitForSingleObject(pi.hProcess,10000)==WAIT_OBJECT_0);
    DWORD exit_code; assert(GetExitCodeProcess(pi.hProcess,&exit_code) && exit_code==77);
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);check("original");
    assert(snes_atomic_state_write(path,"replacement",11));check("replacement");
    assert(!snes_atomic_state_write("missing-parent/slot.state","bad",3));
    assert(snes_atomic_state_write("unicode-\xc3\xa1.state","ok",2));
    assert(GetFileAttributesW(L"unicode-\x00e1.state")!=INVALID_FILE_ATTRIBUTES);
    DeleteFileW(L"unicode-\x00e1.state");
    puts("PASS: partial write, flush/close/rename failure, locked destination, process death, recovery");
    return 0;
}
