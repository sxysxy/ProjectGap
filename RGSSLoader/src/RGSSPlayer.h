#pragma once
#include "stdafx.h"

class RGSSPlayer {
public:
    constexpr static int nWidth = 544;
    constexpr static int nHeight = 416;
    constexpr static int MAX_LEN = 1024;
    constexpr static wchar_t *lpDefaultLibrary = L"RGSS300.dll";
    constexpr static wchar_t *lpDefaultTitle = L"Untitled";
    constexpr static wchar_t *lpDefaultScripts = L"Data\\Scripts.rvdata2";

    HINSTANCE hInstance;
    HWND hWnd;
    wchar_t szAppPath[MAX_PATH], szIniPath[MAX_PATH], szRGSSADPath[MAX_PATH];
    wchar_t szLibrary[MAX_PATH], szTitle[MAX_PATH], szScripts[MAX_PATH];
    wchar_t *lpRGSSAD;

    int nArgc;
    LPWSTR *lpArgv;

    HANDLE hRGSSCore;

private:
    void GetAppPath();
    void LoadIniConfig();
    void GetRGSSADPath();
    void CreatPlayerWindow();
    void LoadRGSS();
public: 
    void InitPlayer();

};