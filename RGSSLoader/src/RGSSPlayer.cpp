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
        MessageBoxError(0, L"����", L"����RGSSPlayer·��ʧ�ܣ�������ֹ��");
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
    window = SDL_CreateWindow("", _x, _y, nWidth, nHeight, SDL_WINDOW_SHOWN);
    if (!window) {
        MessageBoxError(0, szTitle, L"��������ʧ�ܣ�������ֹ��");
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
        MessageBoxError(hWnd, szTitle, L"����RGSS���Ŀ�ʧ�ܣ�·���� %s��������ֹ��", szLibrary);
        ExitProcess(0);
    }
    
#define __get_ptr(fn) lpfn##fn = (fn)GetProcAddress(hRGSSCore, #fn);  \
       if (!lpfn##fn) {  \
           MessageBoxError(hWnd, szTitle, L"����RGSS���Ŀ⺯�� %s ʧ�ܣ�������ֹ��", L#fn);  \
           ExitProcess(0); \
       }

    __get_ptr(RGSSEval);
    __get_ptr(RGSSGameMain);
    __get_ptr(RGSSInitialize3);
    __get_ptr(RGSSSetupFonts);
    __get_ptr(RGSSSetupRTP);
    
#undef __get_ptr

    static wchar_t szRtpName[1024];
    if (!lpfnRGSSSetupRTP(szIniPath, szRtpName, 1024)) {
        MessageBoxError(hWnd, szTitle, L"�Ҳ��� RGSS-RTP : %s", szRtpName);
        ExitProcess(0);
    }

    lpfnRGSSInitialize3(hRGSSCore);

    for (int i = 0; lpArgv && i < nArgc; i++) {
        if (!lstrcmpW(lpArgv[i], L"btest")) {
            lpRGSSAD = 0;
            lpfnRGSSEval(u8"$TEST = true");
            lpfnRGSSEval(u8"$BTEST = true");
        }else {
            if (!lstrcmpW(lpArgv[i], L"test")) {
                lpRGSSAD = 0;
                lpfnRGSSEval(u8"$TEST = true");
            }else
                lpfnRGSSEval(u8"$TEST = false");
            lpfnRGSSEval(u8"$BTEST = false");
        }
    }

}

void RGSSPlayer::InitD3DContext() {
    renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
                                            //Ӳ�����٣�������Ⱦ��ͼ
    if (!renderer) {
        MessageBoxError(hWnd, szTitle, L"��ʼ��D3D����Ⱦ��ʧ�ܣ�������ֹ��");
        ExitProcess(0);
    }
    SDL_RenderClear(renderer);
}

void RGSSPlayer::MakePreRubyScripts() {
    
    
}

void RGSSPlayer::InitPlayer() {
    hInstance = GetModuleHandle(nullptr);
    GetAppPath();
    LoadIniConfig();
    GetRGSSADPath();
    lpArgv = CommandLineToArgvW(GetCommandLineW(), &nArgc);
    CreatPlayerWindow();
    LoadRGSS();
    InitD3DContext();
    MakePreRubyScripts();
}

void RGSSPlayer::DestroyPlayer() {
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    if (hRGSSCore) {
        FreeLibrary(hRGSSCore);
        hRGSSCore = nullptr;
    }
}

void RGSSPlayer::MainLoop() {
    lpfnRGSSGameMain(hWnd, szScripts, (lpRGSSAD ? reinterpret_cast<wchar_t **>(lpRGSSAD):&lpRGSSAD));
}