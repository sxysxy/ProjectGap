// RGSSRuntimePlugin.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "RGSSRuntimePlugin.h"
#include "RubySupport.h"

extern "C" {
    RGSSRUNTIMEPLUGIN_API void InitPlugin(PluginData *data) {
        Ruby::InitRuntime(data->hRGSSCore);
    }
}