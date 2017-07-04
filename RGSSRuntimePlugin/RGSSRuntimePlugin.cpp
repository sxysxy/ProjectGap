// RGSSRuntimePlugin.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "RGSSRuntimePlugin.h"
#include "RubySupport.h"
#include "RGSSBitmap.h"
#include "RGSSGraphics.h"
#include "RGSSSprite.h"

PluginData gPluginData;

extern "C" {
    RGSSRUNTIMEPLUGIN_API void InitPlugin(PluginData *data) {
        Ruby::InitRuntime(data->hRGSSCore);
        Ruby::rb_cObject = Ruby::rb_eval_string_protect(u8"Object", nullptr);
        memcpy(&gPluginData, data, sizeof(PluginData));

        //get_rtp
        HKEY hKey = nullptr;
        wchar_t szBuffer1[MAX_PATH];
        DWORD a = 0, b = MAX_PATH-1;
        RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Enterbrain\\RGSS3\\RTP", 0, KEY_READ, &hKey);
        RegQueryValueExW(hKey, gPluginData.szRTPName, nullptr, &a, (LPBYTE)szBuffer1, &b);
        for (int i = 0; szBuffer1[i]; i++) 
            if(szBuffer1[i] == L'\\')szBuffer1[i] = L'/';

        //��Unicodeת������Ϸ�õ�UTF8����
        int len = WideCharToMultiByte(CP_UTF8, 0, szBuffer1, -1, nullptr, 0, nullptr, nullptr);
        WideCharToMultiByte(CP_UTF8, 0, szBuffer1, -1, gPluginData.RTPPath, len, nullptr, nullptr);

        RegCloseKey(hKey);
        
    }

    RGSSRUNTIMEPLUGIN_API void ApplyPlugin() {
        RGSS::Graphics::InitGraphics();     //1
        RGSS::Bitmap::InitBitmap();         //2
        RGSS::Sprite::InitSprite();         //3
    }
}