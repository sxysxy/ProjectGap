#include "stdafx.h"
#include "RGSSBitmap.h"
#include "RGSSRuntimePlugin.h"
#include "RGSSGraphics.h"

/*
    RGSS Bitmap修改，高性能Bitmap(底层由d3d⑨ texture实现)

    对于Bitmap类，做出了如下修改：
        *添加实例变量@bitmap_data,储存底层的数据的指针
        *添加实例变量@__disposed，相应的，重新实现了dispose,disposed?方法
        *添加方法__get_texture，只是用来检测贴图是否加载成功（如果失败了会返回nil的）
        
*/

namespace RGSS {
    namespace Bitmap {

        VALUE klass;  //Bitmap的魔改
        struct BitmapData {  //Bitmap数据包
            SDL_Texture *texture;
            int width, height;
        };          //@bitmap_data会存入这个结构体的指针

        RColor RGSSColor2RColor(VALUE color) {
            typedef unsigned char u8;
            return RColor((u8)rb_float_noflonum_value(rb_funcall2(color, rb_intern("red"), 0, nullptr)),
                (u8)rb_float_noflonum_value(rb_funcall2(color, rb_intern("green"), 0, nullptr)),
                (u8)rb_float_noflonum_value(rb_funcall2(color, rb_intern("blue"), 0, nullptr)),
                (u8)rb_float_noflonum_value(rb_funcall2(color, rb_intern("alpha"), 0, nullptr)));
        }
        RRect RGSSRect2RRect(VALUE rect) {
            return RRect(FIX2INT(rb_funcall2(rect, rb_intern("x"), 0, nullptr)),
                FIX2INT(rb_funcall2(rect, rb_intern("y"), 0, nullptr)),
                FIX2INT(rb_funcall2(rect, rb_intern("width"), 0, nullptr)),
                FIX2INT(rb_funcall2(rect, rb_intern("height"), 0, nullptr)));
        }

        static BitmapData *__cdecl initialize(VALUE self) {  //初始化共用的部分
            BitmapData *p = (BitmapData *)malloc(sizeof(BitmapData));
            RtlZeroMemory(p, sizeof(BitmapData));
            rb_iv_set(self, "@bitmap_data", INT2FIX((long)p));
            rb_iv_set(self, "@__disposed", Qfalse);
            return p;
        }
        static VALUE __cdecl initialize_wh(VALUE self, VALUE w, VALUE h) {
            BitmapData *p = initialize(self);
            p->width = FIX2INT(w), p->height = FIX2INT(h);
            p->texture = SDL_CreateTexture(gPluginData.GraphicsInformation.renderer, SDL_PIXELFORMAT_RGBA8888, 
                SDL_TEXTUREACCESS_TARGET, p->width, p->height);
            SDL_SetRenderTarget(Graphics::renderer, p->texture);
            SDL_RenderClear(Graphics::renderer);
            SDL_SetRenderTarget(Graphics::renderer, nullptr);
            return self;
        }
        static VALUE __cdecl initialize_path(VALUE self, VALUE path) {
            BitmapData *p = initialize(self);
            //准备从文件中加载位图，位图可能存在于加密档案。
            //先尝试加密档案里面:
            void *pdata; int len;
            SDL_RWops *rwops = nullptr;
            rgss_load_rgssad_file(RSTRING_PTR(path), &pdata, &len);
            bool inrgssad = pdata != nullptr; 
            if (inrgssad) { //加密档案中存在：
                rwops = SDL_RWFromMem(pdata, len);
            }else {
                rwops = SDL_RWFromFile(RSTRING_PTR(path), "r");
            }
            p->texture = IMG_LoadTexture_RW(Graphics::renderer, rwops, 0);
            //if(inrgssad)free(pdata);      //free?
            SDL_QueryTexture(p->texture, nullptr, nullptr, &p->width, &p->height);
            return p->texture?Qtrue:Qfalse;
        }
        inline BitmapData *GetData(VALUE self) {
            VALUE p = rb_iv_get(self, "@bitmap_data");
            return p == Qnil ? nullptr : (BitmapData *)FIX2INT(p);
        }
        inline SDL_Texture *GetTexture(VALUE self) {
            BitmapData *p = GetData(self);
            return p?p->texture : nullptr;
        }
        static VALUE __cdecl check_disposed(VALUE self) {
            return rb_iv_get(self, "@__disposed") != Qfalse;
        }
        static VALUE __cdecl dispose(VALUE self) {
            if(check_disposed(self))return Qnil;
            BitmapData *p = GetData(self);
            SDL_DestroyTexture(p->texture);
            free(p);
            rb_iv_set(self, "@__disposed", Qtrue);
            return Qnil;
        }
        static VALUE __cdecl get_texture(VALUE self) { //注：这个是给ruby层面用的
            BitmapData *p = GetData(self);
            if(p)return INT2FIX((int)p->texture);
            else return Qnil;
        }
        static VALUE __cdecl rect(VALUE self) {
            char buf[120];
            BitmapData *p = GetData(self); 
            int w = p?p->width:0, h = p?p->height:0;
            sprintf(buf, "Rect.new(0,0,%d,%d)", w, h);
            return rb_eval_cstring(buf);
        }
        static void fill_rect(const RRect *rect, const RColor c) {
            SDL_SetRenderDrawColor(Graphics::renderer, c.rgba.r, c.rgba.g, c.rgba.b, c.rgba.a);
            SDL_RenderFillRect(Graphics::renderer, rect);
            SDL_SetRenderDrawColor(Graphics::renderer, 0, 0, 0, 0);
        }
        static VALUE __cdecl fill_rect1(VALUE self, VALUE x, VALUE y, VALUE w, VALUE h, VALUE color) {
            SDL_SetRenderTarget(Graphics::renderer, GetTexture(self));
            RRect rect(FIX2INT(x), FIX2INT(y), FIX2INT(w), FIX2INT(h));
            RColor c = RGSSColor2RColor(color);
            fill_rect(&rect, c);
            return Qnil;
        }
        static VALUE __cdecl fill_rect2(VALUE self, VALUE rect, VALUE color) {
            SDL_SetRenderTarget(Graphics::renderer, GetTexture(self));
            fill_rect(&RGSSRect2RRect(rect), RGSSColor2RColor(color));
            return Qnil;
        }
        static VALUE __cdecl width(VALUE self) {
            BitmapData *p = GetData(self);
            return p ? INT2FIX(p->width) : INT2FIX(0);
        }
        static VALUE __cdecl height(VALUE self) {
            BitmapData *p = GetData(self);
            return p ? INT2FIX(p->height) : INT2FIX(0);
        }
        static VALUE __cdecl show_on_screen(VALUE self, VALUE x, VALUE y) {
            SDL_SetRenderTarget(Graphics::renderer, nullptr);
            RRect drect(FIX2INT(x), FIX2INT(y), FIX2INT(width(self)), FIX2INT(height(self)));
            RRect srect(0, 0, FIX2INT(width(self)), FIX2INT(height(self)));
            SDL_RenderCopy(Graphics::renderer, GetTexture(self), &srect, &drect);
            return Qnil;
        }
        void InitBitmap() {
            klass = rb_eval_string_protect(u8"Bitmap", nullptr);

            //加载扩展脚本
            LoadLibScript("Bitmap.rb");

            //initialize的两个实现.
            rb_define_method(klass, "__init_wh", initialize_wh, 2);
            rb_define_method(klass, "__init_path", initialize_path, 1);

            //获取texture指针，在ruby层面的，（会INT2FIX最后返回结果）
            rb_define_method(klass, "__get_texture", get_texture, 0);

            //dispose系列
            rb_define_method(klass, "dispose", dispose, 0);
            rb_define_method(klass, "disposed?", check_disposed, 0);

            //获取尺寸
            rb_define_method(klass, "width", width, 0);
            rb_define_method(klass, "height", height, 0);

            //fill_rect
            rb_define_method(klass, "__fill_rect_2args", fill_rect2, 2);
            rb_define_method(klass, "__fill_rect_5args", fill_rect1, 5);

            //test_method
            rb_define_method(klass, "show_on_screen", show_on_screen, 2);
        }
        
    }
}