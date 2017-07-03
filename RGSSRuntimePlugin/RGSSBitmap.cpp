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
    template<typename T>
    inline T Rmax(const T &a, const T &b) {
        return a > b ? a : b;
    }
    template<typename T>
    inline T Rmin(const T &a, const T &b) {
        return a < b? a : b;
    }

    RColor RGSSColor2RColor(VALUE color) {   //to rgba 我们默认使用RGBA8888
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
    void RGB2HSL(int r, int g, int b, float &h, float &s, float &l) {
        h = 0, s = 0;
        int maxv = Rmax(r, Rmax(g, b)), minv = Rmin(r, Rmin(g, b));
        float delta = maxv - minv;
        h = s = 0;
        l = (maxv + minv) / 2.0;

        if (delta > 0) {
            if (r == maxv) {
                h = 60 * ((g - b) / delta + 6);
                if (h > 360)h -= 360;
            }
            else if (g == maxv) {
                h = 60 * ((b - r) / delta + 2);
            }
            else {
                h = 60 * ((r - g) / delta + 4);
            }
            s = delta / (255 - fabs(2 * l - 255));
        }
    }
    void HSL2RGB(float h, float s, float l, int &r, int &g, int &b) {
        float c = (255 - fabs(2 * l - 255))*s;

        float p = h / 60;
        int t = int(p / 2);
        p -= t * 2;

        float x = c*(1 - fabs(p - 1));
        float tm = l - c / 2;

        int m = int(tm);
        int cm = int(c + tm);
        int xm = int(x + tm);
        if (h < 60) {
            r = cm, g = xm, b = m;
        }
        else if (h < 120) {
            r = xm, g = cm, b = m;
        }
        else if (h < 180) {
            r = m, g = cm, b = xm;
        }
        else if (h < 240) {
            r = m, g = xm, b = cm;
        }
        else if (h < 300) {
            r = xm, g = m, b = cm;
        }
        else {
            r = cm, g = m, b = xm;
        }

    }

    namespace Bitmap {

        VALUE klass;  //Bitmap的魔改
        struct BitmapData {  //Bitmap数据包
            SDL_Texture *texture;
        //    SDL_Texture *origin;        //hue_change里面才会用到，未经hue_change处理的图...
            int width, height;
        };          //@bitmap_data会存入这个结构体的指针
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
                                                                        //我们默认使用RGBA8888
            p->texture = SDL_CreateTexture(gPluginData.GraphicsInformation.renderer, SDL_PIXELFORMAT_RGBA8888, 
                SDL_TEXTUREACCESS_TARGET, p->width, p->height);
            SDL_SetRenderTarget(Graphics::renderer, p->texture);
            SDL_RenderClear(Graphics::renderer);
            return self;
        }
        static VALUE __cdecl initialize_path(VALUE self, VALUE path) {
            BitmapData *p = initialize(self);
            //准备从文件中加载位图，位图可能存在于加密档案。
            //先尝试加密档案里面:
            void *pdata; int len;
            SDL_RWops *rwops = nullptr;
            bool inrgssad = rgss_load_rgssad_file(RSTRING_PTR(path), &pdata, &len);
            if (inrgssad) { //加密档案中存在：
                rwops = SDL_RWFromMem(pdata, len);
            }else {
                rwops = SDL_RWFromFile(RSTRING_PTR(path), "r");
            }
            SDL_Texture *t = IMG_LoadTexture_RW(Graphics::renderer, rwops, 0);
            SDL_QueryTexture(t, nullptr, nullptr, &p->width, &p->height);
            p->texture = SDL_CreateTexture(Graphics::renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
                            p->width, p->height);
            SDL_SetRenderTarget(Graphics::renderer, p->texture);
            SDL_RenderCopy(Graphics::renderer, t, NULL, NULL);
            SDL_DestroyTexture(t);
            SDL_FreeRW(rwops);
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
        static VALUE __cdecl stretch_blt_opacity(VALUE self, VALUE dest_rect, VALUE src_bmp, VALUE src_rect, VALUE opacity) {
            SDL_SetRenderTarget(Graphics::renderer, GetTexture(self));
            SDL_Texture *tex = GetTexture(src_bmp);
            SDL_SetTextureAlphaMod(tex, FIX2INT(opacity));
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
            SDL_SetRenderDrawBlendMode(Graphics::renderer, SDL_BLENDMODE_NONE);  //覆盖模式
            fill_rect(&RGSSRect2RRect(rect), RColor(0));
            SDL_SetRenderDrawBlendMode(Graphics::renderer, SDL_BLENDMODE_BLEND); //切换回合成模式 
            return Qnil;
        }
        static VALUE __cdecl clear_rect2(VALUE self, VALUE x, VALUE y, VALUE w, VALUE h) {
            SDL_SetRenderDrawBlendMode(Graphics::renderer, SDL_BLENDMODE_NONE);  //覆盖模式
            fill_rect(&RRect(FIX2INT(x), FIX2INT(y), FIX2INT(w), FIX2INT(h)), RColor(0));
            SDL_SetRenderDrawBlendMode(Graphics::renderer, SDL_BLENDMODE_BLEND); //切换回合成模式 
            return Qnil;
        }
        static VALUE  __cdecl get_pixel(VALUE self, VALUE x, VALUE y) {
            SDL_SetRenderTarget(Graphics::renderer, GetTexture(self));
            RColor t = 0;
            SDL_RenderReadPixels(Graphics::renderer, &RRect(FIX2INT(x), FIX2INT(y), 1, 1), 0, &t.color, 4);
            char buf[100];
            sprintf(buf, "Color.new(%d,%d,%d,%d)", t.rgba.r, t.rgba.g, t.rgba.b, t.rgba.a);
            return rb_eval_cstring(buf);
        }
        static VALUE __cdecl set_pixel(VALUE self, VALUE x, VALUE y, VALUE color) {
            fill_rect(&RRect(FIX2INT(x), FIX2INT(y), 1, 1), RGSSColor2RColor(color));
            return Qnil;
        }
        static VALUE __cdecl hue_change(VALUE self, VALUE hue) {
            BitmapData *data = GetData(self);
            SDL_SetRenderTarget(Graphics::renderer, data->texture);
            int w = data->width, h = data->height;
            
            int pitch = SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_ABGR8888)*w;  //w*4
            RColor *pixels = (RColor *)malloc(pitch*h);
            if (!pixels) {
                MessageBoxW(gPluginData.GraphicsInformation.hWnd, L"内存不足，程序已终止！", L"错误", MB_ICONERROR);
                ExitProcess(0);
            }
            
            if (SDL_RenderReadPixels(Graphics::renderer, nullptr, SDL_PIXELFORMAT_RGBA8888, pixels, pitch) < 0) 
                puts(SDL_GetError());
            
            if (!FIXNUM_P(hue)) 
                hue = rb_funcall2(hue, rb_intern("to_i"), 0, nullptr);
            int dhue = (FIX2INT(hue)%360+360)%360;
            

            const int len = w*h;
            int length = w*h/4;     //循环展开，4层
            for (RColor *p = pixels; p < pixels + length; p += 4) {
                float ch = 0, cs = 0, cl = 0;
                int r, g, b;            //为了和原rgss的hue_change效果相同，这里使用abgr格式读取
           #define loop_expand(ptr) RGB2HSL(ptr->abgr.r, ptr->abgr.g, ptr->abgr.b, ch, cs, cl); \
                ch = ch + dhue;  \
                if (ch >= 360)ch -= 360; \
                HSL2RGB(ch, cs, cl, r, g, b); \
                ptr->abgr.r = r, ptr->abgr.g = g, ptr->abgr.b = b; 
                
                loop_expand(p);
                loop_expand((p + 1));
                loop_expand((p + 2));
                loop_expand((p + 3));
            }
            for (RColor *p = pixels + length; p < pixels + len; p++) {
                float ch = 0, cs = 0, cl = 0;
                int r, g, b;
                loop_expand(p);
            #undef loop_expand
            }
                 
            if (SDL_UpdateTexture(data->texture, nullptr, pixels, pitch) < 0) 
                puts(SDL_GetError());
            free(pixels);
            
            return Qnil;
        }
        static VALUE get_all_pixels(VALUE self) {
            BitmapData *data = GetData(self);
            SDL_SetRenderTarget(Graphics::renderer, data->texture);
            int w = data->width, h = data->height;

            int pitch = SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_ABGR8888)*w;  //w*4
            RColor *pixels = (RColor *)malloc(pitch*h);
            SDL_RenderReadPixels(Graphics::renderer, nullptr, SDL_PIXELFORMAT_ABGR8888, pixels, pitch);

            VALUE kary = rb_eval_cstring("Array");
            VALUE size = INT2FIX(w*h);
            VALUE ary = rb_funcall2(kary, rb_intern("new"), 1, &size); 
            VALUE kcolor = rb_eval_cstring("Color");

            const int len = w*h;
            int length = w*h / 4;     //循环展开，4层
            VALUE tmp[4];
            VALUE col;
            const ID idnew = rb_intern("new"), idset = rb_intern("[]=");
            for (RColor *p = pixels; p < pixels + length; p += 4) { 

            #define loop_expand(ptr) tmp[0] = INT2FIX(ptr->rgba.r);tmp[1] = INT2FIX(ptr->rgba.g); \
                                     tmp[2] = INT2FIX(ptr->rgba.b);tmp[3] = INT2FIX(ptr->rgba.a); \
                                     col = rb_funcall2(kcolor, idnew, 4, tmp); \
                                     tmp[0] = INT2FIX(ptr-pixels), tmp[1] = col; \
                                     rb_funcall2(ary, idset, 2, tmp);

                  loop_expand(p)
                  loop_expand((p+1))
                  loop_expand((p+2))
                  loop_expand((p+3))
            }
            for (RColor *p = pixels + length; p < pixels + len; p++) {
                loop_expand(p);
                #undef loop_expand
            }

            free(pixels);
            return ary;
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

            //fill_rect  很快的w
            rb_define_method(klass, "__fill_rect_2args", fill_rect2, 2);
            rb_define_method(klass, "__fill_rect_5args", fill_rect1, 5);

            //clear 很快的哦
            rb_define_method(klass, "clear", clear, 0);
            rb_define_method(klass, "__clear_rect_1args", clear_rect1, 1);
            rb_define_method(klass, "__clear_rect_4args", clear_rect2, 4);

            //pixel  get_pixel是比较慢的
            rb_define_method(klass, "set_pixel", set_pixel, 3);
            rb_define_method(klass, "get_pixel", get_pixel, 2);

            //blt  很快的哦
            rb_define_method(klass, "__blt_4args", blt, 4);
            rb_define_method(klass, "__blt_5args", blt_opacity, 5);
            rb_define_method(klass, "__stretch_blt_3args", stretch_blt, 3);
            rb_define_method(klass, "__stretch_blt_4args", stretch_blt_opacity, 4);

            //hue change，这个函数速度很慢，没比原rgss的快多少
            rb_define_method(klass, "hue_change", hue_change, 1);

            //懒省事用的，直接能把bitmap弄屏幕上去显示，不需要搞个精灵
                        //（这个show_on_screen方法需要每帧都调用，就像Sprite#update一样)
            rb_define_method(klass, "show_on_screen", show_on_screen, 2);

            //get all pixels,returns a ruby Array
            rb_define_method(klass, "get_all_pixels", get_all_pixels, 0);
        }
        
    }
}