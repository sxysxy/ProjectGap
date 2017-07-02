#include "stdafx.h"
#include "RGSSBitmap.h"
#include "RGSSRuntimePlugin.h"
#include "RGSSGraphics.h"

/*
    RGSS Bitmap�޸ģ�������Bitmap(�ײ���d3d�� textureʵ��)

    ����Bitmap�࣬�����������޸ģ�
        *���ʵ������@bitmap_data,����ײ�����ݵ�ָ��
        *���ʵ������@__disposed����Ӧ�ģ�����ʵ����dispose,disposed?����
        *��ӷ���__get_texture��ֻ�����������ͼ�Ƿ���سɹ������ʧ���˻᷵��nil�ģ�
        
*/

namespace RGSS {

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


    namespace Bitmap {

        VALUE klass;  //Bitmap��ħ��
        struct BitmapData {  //Bitmap���ݰ�
            SDL_Texture *texture;
            int width, height;
        };          //@bitmap_data���������ṹ���ָ��
        static BitmapData *__cdecl initialize(VALUE self) {  //��ʼ�����õĲ���
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
            //׼�����ļ��м���λͼ��λͼ���ܴ����ڼ��ܵ�����
            //�ȳ��Լ��ܵ�������:
            void *pdata; int len;
            SDL_RWops *rwops = nullptr;
            rgss_load_rgssad_file(RSTRING_PTR(path), &pdata, &len);
            bool inrgssad = pdata != nullptr; 
            if (inrgssad) { //���ܵ����д��ڣ�
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
        static VALUE __cdecl get_texture(VALUE self) { //ע������Ǹ�ruby�����õ�
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
        static VALUE __cdecl stretch_blt_opacity(VALUE self, VALUE dest_rect, VALUE src_bmp, VALUE src_rect, VALUE opacity) {
            SDL_SetRenderTarget(Graphics::renderer, GetTexture(self));
            SDL_Texture *tex = GetTexture(src_bmp);
            SDL_SetTextureAlphaMod(tex, opacity);
            SDL_RenderCopyEx(Graphics::renderer, tex, &RGSSRect2RRect(src_rect), &RGSSRect2RRect(dest_rect),
                                0, nullptr, SDL_FLIP_NONE);
            SDL_SetTextureAlphaMod(tex, 255);
            return Qnil;
        }
        static VALUE __cdecl stretch_blt(VALUE self, VALUE dest_rect, VALUE src_bmp, VALUE src_rect) {
            return stretch_blt_opacity(self, dest_rect, src_bmp, src_rect, 255);
        }
        static VALUE __cdecl blt_opacity(VALUE self, VALUE x, VALUE y, VALUE src_bmp, VALUE src_rect, VALUE opacity) {
            char code[256];
            sprintf(code, "Rect.new(%d,%d,%d,%d)", FIX2INT(x), FIX2INT(y), 
                    FIX2INT(rb_funcall2(src_rect, rb_intern("width"), 0, nullptr)),
                    FIX2INT(rb_funcall2(src_rect, rb_intern("height"), 0, nullptr)));
            return stretch_blt_opacity(self, rb_eval_cstring(code), src_bmp, src_rect, opacity);
        }
        static VALUE __cdecl blt(VALUE self, VALUE x, VALUE y, VALUE src_bmp, VALUE src_rect) {
            return blt_opacity(self, x, y, src_bmp, src_rect, 255);
        }
        static VALUE __cdecl clear(VALUE self) {
            SDL_SetRenderDrawColor(Graphics::renderer, 0, 0, 0, 0);             //
            SDL_RenderClear(Graphics::renderer);
            return Qnil;
        }
        static VALUE __cdecl clear_rect1(VALUE self, VALUE rect) {
            SDL_SetRenderDrawBlendMode(Graphics::renderer, SDL_BLENDMODE_NONE);  //����ģʽ
            fill_rect(&RGSSRect2RRect(rect), RColor(0));
            SDL_SetRenderDrawBlendMode(Graphics::renderer, SDL_BLENDMODE_BLEND); //�л��غϳ�ģʽ 
            return Qnil;
        }
        static VALUE __cdecl clear_rect2(VALUE self, VALUE x, VALUE y, VALUE w, VALUE h) {
            SDL_SetRenderDrawBlendMode(Graphics::renderer, SDL_BLENDMODE_NONE);  //����ģʽ
            fill_rect(&RRect(FIX2INT(x), FIX2INT(y), FIX2INT(w), FIX2INT(h)), RColor(0));
            SDL_SetRenderDrawBlendMode(Graphics::renderer, SDL_BLENDMODE_BLEND); //�л��غϳ�ģʽ 
            return Qnil;
        }
        static VALUE  __cdecl get_pixel(VALUE self, VALUE x, VALUE y) {
            SDL_SetRenderTarget(Graphics::renderer, GetTexture(self));
            RColor t = 0;
            SDL_RenderReadPixels(Graphics::renderer, &RRect(FIX2INT(x), FIX2INT(y), 1, 1), 0, &t.color, 1);
            char buf[100];
            sprintf(buf, "Color.new(%d,%d,%d,%d)", t.rgba.r, t.rgba.g, t.rgba.b, t.rgba.a);
            return rb_eval_cstring(buf);
        }
        static VALUE __cdecl set_pixel(VALUE self, VALUE x, VALUE y, VALUE color) {
            fill_rect(&RRect(FIX2INT(x), FIX2INT(y), 1, 1), RGSSColor2RColor(color));
            return Qnil;
        }
        static VALUE __cdecl hue_change(VALUE self, VALUE d) {
            return self;
        }

        void InitBitmap() {
            klass = rb_eval_string_protect(u8"Bitmap", nullptr);

            //������չ�ű�
            LoadLibScript("Bitmap.rb");

            //initialize������ʵ��.
            rb_define_method(klass, "__init_wh", initialize_wh, 2);
            rb_define_method(klass, "__init_path", initialize_path, 1);

            //��ȡtextureָ�룬��ruby����ģ�����INT2FIX��󷵻ؽ����
            rb_define_method(klass, "__get_texture", get_texture, 0);

            //disposeϵ��
            rb_define_method(klass, "dispose", dispose, 0);
            rb_define_method(klass, "disposed?", check_disposed, 0);

            //��ȡ�ߴ�
            rb_define_method(klass, "width", width, 0);
            rb_define_method(klass, "height", height, 0);

            //fill_rect
            rb_define_method(klass, "__fill_rect_2args", fill_rect2, 2);
            rb_define_method(klass, "__fill_rect_5args", fill_rect1, 5);

            //clear
            rb_define_method(klass, "clear", clear, 0);
            rb_define_method(klass, "__clear_rect_1args", clear_rect1, 1);
            rb_define_method(klass, "__clear_rect_4args", clear_rect2, 4);

            //pixel
            rb_define_method(klass, "set_pixel", set_pixel, 3);
            rb_define_method(klass, "get_pixel", get_pixel, 2);

            //blt
            rb_define_method(klass, "__blt_4args", blt, 4);
            rb_define_method(klass, "__blt_5args", blt_opacity, 5);
            rb_define_method(klass, "__stretch_blt_3args", stretch_blt, 3);
            rb_define_method(klass, "__stretch_blt_4args", stretch_blt_opacity, 4);

            //test_method, only for test Bitmap functions, in rgss please use Sprite to make Bitmap shown on the screen.
            rb_define_method(klass, "show_on_screen", show_on_screen, 2);
        }
        
    }
}