// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� RGSSRUNTIMEPLUGIN_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// RGSSRUNTIMEPLUGIN_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
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