// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 RGSSRUNTIMEPLUGIN_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// RGSSRUNTIMEPLUGIN_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#pragma once

#ifdef RGSSRUNTIMEPLUGIN_EXPORTS
#define RGSSRUNTIMEPLUGIN_API __declspec(dllexport)
#else
#define RGSSRUNTIMEPLUGIN_API __declspec(dllimport)
#endif

#include <Common\include\PluginData.h>

extern PluginData gPluginData;
struct RRect : public SDL_Rect {
    RRect(int _x, int _y, int _w, int _h) {
        x = _x, y = _y, w = _w, h = _h;
    }
};

struct RColor {
    union {
        struct {
            unsigned char r,g,b,a;
        }rgba;
        struct {
            unsigned char h,s,l,a;
        }hsla;
        unsigned int color;
    };
    RColor(const unsigned int c = 0){color = c;}
    RColor(unsigned char v1, unsigned char v2, unsigned char v3, unsigned char v4) {
        rgba.r = v1, rgba.g = v2, rgba.b = v3, rgba.a = v4;
    }
};