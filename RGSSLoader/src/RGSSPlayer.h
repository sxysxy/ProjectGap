#pragma once
#include "stdafx.h"
#include <Common\include\GraphicsInfo.h>

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
    SDL_Window *window;
    wchar_t szAppPath[MAX_PATH], szIniPath[MAX_PATH], szRGSSADPath[MAX_PATH];
    wchar_t szLibrary[MAX_PATH], szTitle[MAX_PATH], szScripts[MAX_PATH];
    wchar_t *lpRGSSAD;

    int nArgc;
    LPWSTR *lpArgv;

    HMODULE hRGSSCore;
    typedef bool (*RGSSSetupRTP)(const wchar_t *lpIniPath, wchar_t *lpErrorBuffer, int nBufferLength);
    typedef void (*RGSSSetupFonts)();
    typedef void (*RGSSInitialize3)(HANDLE hRGSSDll);
    typedef unsigned (*RGSSEval)(const char *lpScripts);
    typedef void (*RGSSGameMain)(HWND hWnd, const wchar_t *lpScriptsName, wchar_t **pRGSSADName);

    RGSSSetupRTP  lpfnRGSSSetupRTP;
    RGSSSetupFonts lpfnRGSSSetupFonts;
    RGSSInitialize3 lpfnRGSSInitialize3;
    RGSSEval lpfnRGSSEval;
    RGSSGameMain lpfnRGSSGameMain;

    SDL_Renderer *renderer;

private:
    void GetAppPath();
    void LoadIniConfig();
    void GetRGSSADPath();
    void CreatPlayerWindow();
    void LoadRGSS();
    void MakePreRubyScripts();
    void InitD3DContext();
public: 
    void InitPlayer();
    void MainLoop();
    void DestroyPlayer();
};