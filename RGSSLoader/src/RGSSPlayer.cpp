#include "stdafx.h"
#include "RGSSPlayer.h"

static void __cdecl MessageBoxError(HWND hWnd, const wchar_t *title, const wchar_t *format, ...) {
    static wchar_t lpwError[RGSSPlayer::MAX_LEN];
    va_list ap;
    va_start(ap, format);
    vswprintf_s(lpwError, format, ap);
    va_end(ap);
    MessageBoxW(hWnd, lpwError, title, MB_ICONERROR);
}

void RGSSPlayer::GetAppPath() {
    int len = GetModuleFileNameW(hInstance, szAppPath, MAX_PATH);
    if (!len) {
        MessageBoxError(0, L"¥ÌŒÛ", L"º”‘ÿRGSSPlayer¬∑æ∂ ß∞‹£¨≥Ã–Ú÷’÷π£°");
        ExitProcess(0);
    }
    for (--len; len > 0; len--) {
        if (szAppPath[len] == L'\\' || szAppPath[len] == L'/') {
            szAppPath[len] = 0;
            break;
        }
    }
    SetCurrentDirectoryW(szAppPath);
}

void RGSSPlayer::LoadIniConfig() {
    int len = GetModuleFileNameW(hInstance, szIniPath, MAX_PATH);
    szIniPath[len-1] = L'i';
    szIniPath[len-2] = L'n';
    szIniPath[len-3] = L'i';
    if (GetFileAttributesW(szIniPath) != INVALID_FILE_ATTRIBUTES) {
        GetPrivateProfileStringW(L"Game", L"Title", lpDefaultTitle, szTitle, MAX_PATH, szIniPath);
        GetPrivateProfileStringW(L"Game", L"Library", lpDefaultLibrary, szLibrary, MAX_PATH, szIniPath);
        GetPrivateProfileStringW(L"Game", L"Scripts", lpDefaultScripts, szScripts, MAX_PATH, szIniPath);
    }else{
        wcscpy(szTitle, lpDefaultTitle);
        wcscpy(szLibrary, lpDefaultLibrary);
        wcscpy(szScripts, lpDefaultScripts);
    }
}

void RGSSPlayer::GetRGSSADPath() {
    int len = ::GetModuleFileNameW(hInstance, szRGSSADPath, MAX_PATH);
    for (--len; len > 0; len--) {
        if (szRGSSADPath[len] == L'.') {
            szRGSSADPath[len + 1] = L'r';
            szRGSSADPath[len + 2] = L'g';
            szRGSSADPath[len + 3] = L's';
            szRGSSADPath[len + 4] = L's';
            szRGSSADPath[len + 5] = L'3';
            szRGSSADPath[len + 6] = L'a';
            szRGSSADPath[len + 7] = 0;
            break;
        }
    }
    
    if (GetFileAttributesW(szRGSSADPath) != INVALID_FILE_ATTRIBUTES) 
        lpRGSSAD = szRGSSADPath;
    else 
        lpRGSSAD = 0;
}

void RGSSPlayer::CreatPlayerWindow() {
    int _x = GetSystemMetrics(SM_CXSCREEN)/2 - nWidth/2;
    int _y = GetSystemMetrics(SM_CYSCREEN)/2 - nHeight/2;
    SDL_Window *window = SDL_CreateWindow("", _x, _y, nWidth, nHeight, SDL_WINDOW_SHOWN);
    if (!window) {
        MessageBoxError(0, szTitle, L"¥¥Ω®¥∞ø⁄ ß∞‹£¨≥Ã–Ú÷’÷π£°");
        ExitProcess(0);
    }
    SDL_SysWMinfo info;
    RtlZeroMemory(&info, sizeof(info));
    SDL_GetWindowWMInfo(window, &info);
    hWnd = info.info.win.window;
    SetWindowText(hWnd, szTitle);
    if (lpArgv) {
        for (int i = 0; i < nArgc; i++) {
            if (!lstrcmpW(lpArgv[i], L"console")) {
                if (AllocConsole()) {
                    SetConsoleTitle(L"RGSS Console");
                    freopen("conout$", "w", stdout);
                    freopen("conerr$", "w", stderr);
                    freopen("conin$", "r", stdin);
                }
                break;
            }
        }
    }
}

void RGSSPlayer::LoadRGSS() {
    hRGSSCore = LoadLibraryW(szLibrary);
    if (!hRGSSCore) {
        MessageBoxError(hWnd, szTitle, L"º”‘ÿRGSS∫À–ƒø‚ ß∞‹£¨¬∑æ∂£∫ %s", szLibrary);
        ExitProcess(0);
    }
}

void RGSSPlayer::InitPlayer() {
    hInstance = GetModuleHandle(nullptr);
    GetAppPath();
    LoadIniConfig();
    GetRGSSADPath();
    lpArgv = CommandLineToArgvW(GetCommandLineW(), &nArgc);
    CreatPlayerWindow();
    LoadRGSS();
}