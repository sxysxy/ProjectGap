#pragma once
#include "stdafx.h"
#include "RubySupport.h"
#include "RGSSRuntimePlugin.h"
using namespace Ruby;

//Bitamp类的修改。

namespace RGSS {
    RColor RGSSColor2RColor(VALUE color);
    RRect RGSSRect2RRect(VALUE rect);

    namespace Bitmap {

        //
        struct BitmapData {  //Bitmap数据包
            SDL_Texture *texture;
            int width, height;
            bool dirty;
            RColor *pixels;
        };          //@bitmap_data会存入这个结构体的指针

        extern VALUE klass;
        void InitBitmap();
        
        inline BitmapData *GetData(VALUE self) {
            VALUE p = rb_funcall2(self, rb_intern("bitmap_data"), 0, nullptr);
            //VALUE p = rb_iv_get(self, "@bitmap_data");
            return p == Qnil ? nullptr : (BitmapData *)FIX2INT(p);
        }
        inline SDL_Texture *GetTexture(VALUE self) {
            BitmapData *p = GetData(self);
            return p ? p->texture : nullptr;
        }

        VALUE __cdecl dispose(VALUE self);
        void __cdecl stretch_blt_opacity_base(BitmapData *dest_data, const RRect &dest_rect, SDL_Texture *src, const RRect &src_rect, int opacity);
    }
}