// RGSSRuntimePlugin.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "RGSSRuntimePlugin.h"
#include "RubySupport.h"

extern "C" {
    RGSSRUNTIMEPLUGIN_API void InitPlugin(PluginData *data) {
        Ruby::InitRuntime(data->hRGSSCore);
    }
}