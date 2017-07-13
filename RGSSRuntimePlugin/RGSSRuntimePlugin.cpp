// RGSSRuntimePlugin.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "RGSSRuntimePlugin.h"
#include "RubySupport.h"
#include "RGSSBitmap.h"
#include "RGSSGraphics.h"
#include "RGSSSprite.h"
#include "RGSSLoader.h"
#include "RGSSWindow.h"

PluginData gPluginData;

extern "C" {
    RGSSRUNTIMEPLUGIN_API void InitPlugin(PluginData *data) {
        Ruby::InitRuntime(data->hRGSSCore);
        memcpy(&gPluginData, data, sizeof(PluginData));

        //get_rtp
        HKEY hKey = nullptr;
        wchar_t szBuffer1[MAX_PATH];
        DWORD a = 0, b = MAX_PATH-1;
        RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Enterbrain\\RGSS3\\RTP", 0, KEY_READ, &hKey);
        RegQueryValueExW(hKey, gPluginData.szRTPName, nullptr, &a, (LPBYTE)szBuffer1, &b);
        for (int i = 0; szBuffer1[i]; i++) 
            if(szBuffer1[i] == L'\\')szBuffer1[i] = L'/';

        //把Unicode转换成游戏用的UTF8编码
        int len = WideCharToMultiByte(CP_UTF8, 0, szBuffer1, -1, nullptr, 0, nullptr, nullptr);
        WideCharToMultiByte(CP_UTF8, 0, szBuffer1, -1, gPluginData.RTPPath, len, nullptr, nullptr);

        RegCloseKey(hKey);
    }

    RGSSRUNTIMEPLUGIN_API void ApplyPlugin() {
        RGSS::RGSSLoader::InitRGSSLoader(); //0
        RGSS::Graphics::InitGraphics();     //1
        RGSS::Bitmap::InitBitmap();         //2
        RGSS::Sprite::InitSprite();         //3
        RGSS::Window::InitWindow();         //4
    }
}

thread_local WIN32_FIND_DATAA FileFindData;
thread_local char FileFindPath[MAX_PATH];
char *GetFullFileName(const char *filename) {
    sprintf(FileFindPath, "%s*", filename);
    HANDLE h;
    bool rtp = false;
    char *pname;
    if ((h = FindFirstFileA(FileFindPath, &FileFindData)) != INVALID_HANDLE_VALUE) 
        pname = FileFindData.cFileName;
    else { //Find in RTP
        sprintf(FileFindPath, "%s/%s*", gPluginData.RTPPath, filename);
        if ((h = FindFirstFileA(FileFindPath, &FileFindData)) != INVALID_HANDLE_VALUE) {
            pname = FileFindData.cFileName;
            rtp = true;
        }else
            return nullptr;
    }
    int px;
    for(px = 0; FileFindPath[px] != '.' && FileFindPath[px] != '*'; px++);
    if (FileFindPath[px] == '.') {
        if(rtp)sprintf(FileFindPath, "%s/%s", gPluginData.RTPPath, filename);
        else strcpy(FileFindPath, filename);
    }
    else if (FileFindPath[px] == '*') {
        int len = strlen(FileFindPath);
        for (--len; ~len; len--) {
            if ((FileFindPath[len] == '/' || FileFindPath[len] == '\\')) {
                FileFindPath[len+1] = 0;
                break;
            }
        }
        strcat(FileFindPath, FileFindData.cFileName);
    }
    return FileFindPath;
}