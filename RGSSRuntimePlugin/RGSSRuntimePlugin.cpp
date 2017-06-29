// RGSSRuntimePlugin.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "RGSSRuntimePlugin.h"
#include "RubySupport.h"

PluginData gPluginData;

extern "C" {
    RGSSRUNTIMEPLUGIN_API void InitPlugin(PluginData *data) {
        Ruby::InitRuntime(data->hRGSSCore);
        Ruby::rb_cObject = Ruby::rb_eval_string_protect(u8"Object", nullptr);
        memcpy(&gPluginData, data, sizeof(PluginData));
    }

    RGSSRUNTIMEPLUGIN_API void ApplyPlugin() {
        puts("233");        
    }
}