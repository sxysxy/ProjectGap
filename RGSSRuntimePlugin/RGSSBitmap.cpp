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
        float delta = float(maxv - minv);
        h = s = 0;
        l = (maxv + minv) / 2.0f;

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
        VALUE klass_font;

 
        static BitmapData *__cdecl initialize(VALUE self) {  //初始化共用的部分
            BitmapData *p = (BitmapData *)malloc(sizeof(BitmapData));
            RtlZeroMemory(p, sizeof(BitmapData));
            rb_iv_set(self, "@bitmap_data", INT2FIX((long)p));
            rb_iv_set(self, "@__disposed", Qfalse);
            p->dirty = true;  //没错，先设置为true;
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
            SDL_SetTextureBlendMode(p->texture, SDL_BLENDMODE_BLEND);
            p->pixels = (RColor *)malloc(p->width*p->height*4);
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
                char *filename = GetFullFileName(RSTRING_PTR(path));
                rwops = SDL_RWFromFile(filename, "rb");
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
            p->pixels = (RColor *)malloc(p->width*p->height * 4);
            SDL_SetTextureBlendMode(p->texture, SDL_BLENDMODE_BLEND);
            
            return p->texture?Qtrue:Qfalse;
        }
     
        VALUE __cdecl check_disposed(VALUE self) {
            return rb_iv_get(self, "@__disposed") != Qfalse;
        }
        VALUE __cdecl dispose(VALUE self) {
            if(check_disposed(self))return Qnil;
            BitmapData *p = GetData(self);
            free(p->pixels);
            SDL_DestroyTexture(p->texture);
            free(p);
            rb_iv_set(self, "@__disposed", Qtrue);
            rb_iv_set(self, "@bitmap_data", INT2FIX(0));
            return Qnil;
        }
        VALUE __cdecl get_texture(VALUE self) { //注：这个是给ruby层面用的
            BitmapData *p = GetData(self);
            if(p)return INT2FIX((int)p->texture);
            else return Qnil;
        }
        VALUE __cdecl rect(VALUE self) {
            char buf[120];
            BitmapData *p = GetData(self); 
            int w = p?p->width:0, h = p?p->height:0;
            sprintf(buf, "Rect.new(0,0,%d,%d)", w, h);
            return rb_eval_cstring(buf);
        }
        static void fill_rect(BitmapData *d, const RRect *rect, const RColor c) {
            SDL_SetRenderTarget(Graphics::renderer, d->texture);  
            SDL_SetRenderDrawColor(Graphics::renderer, c.rgba.r, c.rgba.g, c.rgba.b, c.rgba.a);
            SDL_RenderFillRect(Graphics::renderer, rect);
            SDL_SetRenderDrawColor(Graphics::renderer, 0, 0, 0, 0);
            d->dirty = true;
        }
        VALUE __cdecl fill_rect1(VALUE self, VALUE x, VALUE y, VALUE w, VALUE h, VALUE color) {
            RRect rect(FIX2INT(x), FIX2INT(y), FIX2INT(w), FIX2INT(h));
            RColor c = RGSSColor2RColor(color);
            fill_rect(GetData(self), &rect, c);
            return Qnil;
        }
        VALUE __cdecl fill_rect2(VALUE self, VALUE rect, VALUE color) {
            fill_rect(GetData(self), &RGSSRect2RRect(rect), RGSSColor2RColor(color));
            return Qnil;
        }
        VALUE __cdecl width(VALUE self) {
            BitmapData *p = GetData(self);
            return p ? INT2FIX(p->width) : INT2FIX(0);
        }
        VALUE __cdecl height(VALUE self) {
            BitmapData *p = GetData(self);
            return p ? INT2FIX(p->height) : INT2FIX(0);
        }
        VALUE __cdecl show_on_screen(VALUE self, VALUE x, VALUE y) {
            SDL_SetRenderTarget(Graphics::renderer, nullptr);
            SDL_SetRenderDrawBlendMode(Graphics::renderer, SDL_BLENDMODE_BLEND);
            RRect drect(FIX2INT(x), FIX2INT(y), FIX2INT(width(self)), FIX2INT(height(self)));
            RRect srect(0, 0, FIX2INT(width(self)), FIX2INT(height(self)));
            SDL_RenderCopy(Graphics::renderer, GetTexture(self), &srect, &drect);
            return Qnil;
        }
         
        void __cdecl stretch_blt_opacity_base(BitmapData *dest_data, const RRect &dest_rect,  SDL_Texture *src, const RRect &src_rect, int opacity) {
            SDL_SetRenderTarget(Graphics::renderer, dest_data->texture);
            SDL_Texture *tex = src;
            Uint8 alpha;
            SDL_GetTextureAlphaMod(tex, &alpha);
            SDL_SetTextureAlphaMod(tex, opacity);
            SDL_RenderCopyEx(Graphics::renderer, tex, &src_rect, &dest_rect,
                                0, nullptr, SDL_FLIP_NONE);
            SDL_SetTextureAlphaMod(tex, alpha);
            dest_data->dirty = true;
        }
        VALUE __cdecl stretch_blt_opacity(VALUE self, VALUE dest_rect, VALUE src_bmp, VALUE src_rect, VALUE opacity) {
            stretch_blt_opacity_base(GetData(self), RGSSRect2RRect(dest_rect), GetTexture(src_bmp), RGSSRect2RRect(src_rect),
                                        FIX2INT(opacity));
            return Qnil;
        }
        VALUE __cdecl stretch_blt(VALUE self, VALUE dest_rect, VALUE src_bmp, VALUE src_rect) {
            return stretch_blt_opacity(self, dest_rect, src_bmp, src_rect, 255);
        }
        VALUE __cdecl blt_opacity(VALUE self, VALUE x, VALUE y, VALUE src_bmp, VALUE src_rect, VALUE opacity) {
            RRect sr = RGSSRect2RRect(src_rect);
            RRect dr(FIX2INT(x), FIX2INT(y), sr.w, sr.h);
            stretch_blt_opacity_base(GetData(self), dr, GetTexture(src_bmp), sr, FIX2INT(opacity));
            return Qnil;
        }
        VALUE __cdecl blt(VALUE self, VALUE x, VALUE y, VALUE src_bmp, VALUE src_rect) {
            return blt_opacity(self, x, y, src_bmp, src_rect, 255);
        }
        VALUE __cdecl clear(VALUE self) {
            SDL_SetRenderDrawColor(Graphics::renderer, 0, 0, 0, 0);             //
            BitmapData *d = GetData(self);
            SDL_SetRenderTarget(Graphics::renderer, d->texture);
            SDL_RenderClear(Graphics::renderer);
            d->dirty = true;
            return Qnil;
        }
        VALUE __cdecl clear_rect1(VALUE self, VALUE rect) {
            SDL_SetRenderDrawBlendMode(Graphics::renderer, SDL_BLENDMODE_NONE);  //覆盖模式
            fill_rect(GetData(self), &RGSSRect2RRect(rect), RColor(0));
            SDL_SetRenderDrawBlendMode(Graphics::renderer, SDL_BLENDMODE_BLEND); //切换回合成模式 
            return Qnil;
        }
        VALUE __cdecl clear_rect2(VALUE self, VALUE x, VALUE y, VALUE w, VALUE h) {
            SDL_SetRenderDrawBlendMode(Graphics::renderer, SDL_BLENDMODE_NONE);  //覆盖模式
            fill_rect(GetData(self), &RRect(FIX2INT(x), FIX2INT(y), FIX2INT(w), FIX2INT(h)), RColor(0));
            SDL_SetRenderDrawBlendMode(Graphics::renderer, SDL_BLENDMODE_BLEND); //切换回合成模式 
            return Qnil;
        }
        inline static void update_pixels(BitmapData *d) {
            if (d->dirty) {
                SDL_SetRenderTarget(Graphics::renderer, d->texture);
                SDL_RenderReadPixels(Graphics::renderer, nullptr, SDL_PIXELFORMAT_RGBA8888, d->pixels, d->width * 4);
                d->dirty = false;
            }
        }
        static VALUE  __cdecl get_pixel(VALUE self, VALUE x, VALUE y) {
            BitmapData *d = GetData(self);
            update_pixels(d);
            RColor t = d->pixels[d->width*FIX2INT(y)+FIX2INT(x)];
            char buf[100];
            sprintf(buf, "Color.new(%d,%d,%d,%d)", t.rgba.r, t.rgba.g, t.rgba.b, t.rgba.a);
            return rb_eval_cstring(buf);
        }
        static VALUE __cdecl set_pixel(VALUE self, VALUE x, VALUE y, VALUE color) {
            fill_rect(GetData(self), &RRect(FIX2INT(x), FIX2INT(y), 1, 1), RGSSColor2RColor(color));
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
            data->dirty = true;
            return Qnil;
        }
        static VALUE get_all_pixels(VALUE self) {
            BitmapData *data = GetData(self);
            int w = data->width, h = data->height;
            update_pixels(data);

            VALUE kary = rb_eval_cstring("Array");
            VALUE size = INT2FIX(w*h);
            VALUE ary = rb_funcall2(kary, rb_intern("new"), 1, &size); 
            VALUE kcolor = rb_eval_cstring("Color");

            const int len = w*h;
            int length = w*h / 4;     //循环展开，4层
            VALUE tmp[4];
            VALUE col;
            const ID idnew = rb_intern("new"), idset = rb_intern("[]=");
            for (RColor *p = data->pixels; p < data->pixels + length; p += 4) { 

            #define loop_expand(ptr) tmp[0] = INT2FIX(ptr->rgba.r);tmp[1] = INT2FIX(ptr->rgba.g); \
                                     tmp[2] = INT2FIX(ptr->rgba.b);tmp[3] = INT2FIX(ptr->rgba.a); \
                                     col = rb_funcall2(kcolor, idnew, 4, tmp); \
                                     tmp[0] = INT2FIX(ptr-data->pixels), tmp[1] = col; \
                                     rb_funcall2(ary, idset, 2, tmp);

                  loop_expand(p)
                  loop_expand((p+1))
                  loop_expand((p+2))
                  loop_expand((p+3))
            }
            for (RColor *p = data->pixels + length; p < data->pixels + len; p++) {
                loop_expand(p);
                #undef loop_expand
            }

            return ary;
        }

        //Font
        struct FontData{
            int size;
            TTF_Font *font;
            unsigned style;
        };
        static inline FontData *GetFontData(VALUE obj) {
            VALUE v = rb_funcall2(obj, rb_intern("font_data"), 0, nullptr);
            //VALUE v = rb_iv_get(obj, "@font_data");
            return v == Qnil ? nullptr : (FontData*)FIX2INT(v);
        }
        static VALUE __cdecl set_bold(VALUE self) {
            FontData *d = GetFontData(self);
            VALUE bold = rb_funcall2(self, rb_intern("bold"), 0, nullptr);
            if (!d || !d->font)return bold;
            if (bold == Qtrue) {
                d->style |= TTF_STYLE_BOLD;
            }else {
                d->style &= (~TTF_STYLE_BOLD);
            }
            TTF_SetFontStyle(d->font, d->style);
            return bold;
        }
        static VALUE __cdecl set_italic(VALUE self) {
            FontData *d = GetFontData(self);
            VALUE italic = rb_funcall2(self, rb_intern("italic"), 0, nullptr);
            if (!d || !d->font)return italic;
            if (italic == Qtrue) {
                d->style |= TTF_STYLE_ITALIC;
            }
            else {
                d->style &= (~TTF_STYLE_ITALIC);
            }
            TTF_SetFontStyle(d->font, d->style);
            return italic;
        }
        static VALUE __cdecl set_underline(VALUE self) {
            FontData *d = GetFontData(self);
            VALUE underline = rb_funcall2(self, rb_intern("underline"), 0, nullptr);
            if (!d || !d->font)return underline;
            if (underline == Qtrue) {
                d->style |= TTF_STYLE_UNDERLINE;
            }
            else {
                d->style &= (~TTF_STYLE_UNDERLINE);
            }
            TTF_SetFontStyle(d->font, d->style);
            return underline;
        }
        static VALUE __cdecl set_strike_through(VALUE self) {
            FontData *d = GetFontData(self);
            VALUE strike_through = rb_funcall2(self, rb_intern("strike_through"), 0, nullptr);
            if (!d || !d->font)return strike_through;
            if (strike_through == Qtrue) {
                d->style |= TTF_STYLE_STRIKETHROUGH;
            }
            else {
                d->style &= (~TTF_STYLE_STRIKETHROUGH);
            }
            TTF_SetFontStyle(d->font, d->style);
            return strike_through;
        }
        static VALUE __cdecl apply_default(VALUE self, VALUE name) {
            FontData *d = new FontData;
            RtlZeroMemory(d, sizeof(FontData));
            d->size = 24;
           // d->style = TTF_STYLE_NORMAL; //0

            d->font = TTF_OpenFont(RSTRING_PTR(name), d->size-2);
            if(!d->font)puts(SDL_GetError());

            rb_iv_set(self, "@font_data", INT2FIX((long)d));
            return self;
        }
        static VALUE __cdecl set_size(VALUE self) {
            FontData *d = GetFontData(self);
            if(!d)return Qnil;
            d->size = FIX2INT(rb_funcall2(self, rb_intern("size"), 0, nullptr));
            TTF_CloseFont(d->font);
            d->font = TTF_OpenFont(RSTRING_PTR(rb_funcall2(self, rb_intern("font_name"), 0, nullptr)), d->size-2);
            return INT2FIX(d->size);
        }
        static VALUE __cdecl dispose_font(VALUE self) {
            FontData *d = GetFontData(self);
            if (d) {
                TTF_CloseFont(d->font);
                d->font = nullptr;
            }
            rb_iv_set(self, "@font_data", INT2FIX(0));
            return Qnil;
        }

        //draw_text系列
        static void __cdecl draw_text(SDL_Texture *tex, TTF_Font *font, const char *str, RColor color, int x, int y, int w, int h, int align) {
            if(!font)return;
            SDL_Surface *suf = TTF_RenderUTF8_Solid(font, str, SDL_Color{color.rgba.r, color.rgba.g, color.rgba.b, color.rgba.a});
            SDL_Texture *ftex = SDL_CreateTextureFromSurface(Graphics::renderer, suf);
            SDL_SetRenderTarget(Graphics::renderer, tex);
            int real_w, real_h;
            SDL_QueryTexture(ftex, nullptr, nullptr, &real_w, &real_h);
            int posx = x;
            if(align == 1)posx = (w-real_w)/2;
            else if(align == 2)posx = w-real_w;
            SDL_RenderCopy(Graphics::renderer, ftex, nullptr, &RRect(posx, y+((h-real_h)>>1), real_w, real_h));   
            SDL_DestroyTexture(ftex);
            SDL_FreeSurface(suf);
        }
        static VALUE __cdecl draw_text1(VALUE self, VALUE x, VALUE y, VALUE w, VALUE h, VALUE str, VALUE align) {
            BitmapData *d = GetData(self);
            str = rb_funcall2(str, rb_intern("to_s"), 0, nullptr);
            VALUE font = rb_funcall2(self, rb_intern("font"), 0, nullptr);
            draw_text(d->texture, GetFontData(font)->font,
                RSTRING_PTR(str), RGSSColor2RColor(rb_funcall2(font, rb_intern("color"), 0, nullptr)),
                FIX2INT(x), FIX2INT(y), FIX2INT(w), FIX2INT(h), FIX2INT(align));
            d->dirty = true;
            return Qnil;
        }
        static VALUE __cdecl draw_text2(VALUE self, VALUE x, VALUE y, VALUE w, VALUE h, VALUE str) {
            return draw_text1(self, x, y, w, h, str, 0);
        }
        static VALUE __cdecl draw_text3(VALUE self, VALUE rect, VALUE str, VALUE align) {
            RRect r = RGSSRect2RRect(rect);
            BitmapData *d = GetData(self);
            str = rb_funcall2(str, rb_intern("to_s"), 0, nullptr);
            VALUE font = rb_funcall2(self, rb_intern("font"), 0, nullptr);
            draw_text(d->texture, GetFontData(font)->font, 
                RSTRING_PTR(str), RGSSColor2RColor(rb_funcall2(font, rb_intern("color"), 0, nullptr)),
                r.x, r.y, r.w, r.h, FIX2INT(align));
            d->dirty = true;
            return Qnil;
        }
        static VALUE __cdecl draw_text4(VALUE self, VALUE rect, VALUE str) {
            return draw_text3(self, rect, str, 0);
        }
        static VALUE __cdecl text_size(VALUE self, VALUE str) {
            str = rb_funcall2(str, rb_intern("to_s"), 0, nullptr);
            VALUE font = rb_funcall2(self, rb_intern("font"), 0, nullptr);
            FontData *d = GetFontData(font);
            if(!d || !d->font)return Qnil;
            int w, h;
            TTF_SizeUTF8(d->font, RSTRING_PTR(str), &w, &h);
            char buf[100];
            sprintf(buf, "Rect.new(0,0,%d,%d)", w, h);
            return rb_eval_cstring(buf);
        }
        void InitBitmap() {
            klass = rb_eval_string_protect(u8"Bitmap", nullptr);
            klass_font = rb_eval_cstring("Font");

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
            
            //字体
            if (TTF_Init() < 0) 
                MessageBoxW(gPluginData.GraphicsInformation.hWnd, L"初始化True Type Font引擎失败，程序已终止！", L"错误", MB_ICONERROR);
            
            rb_define_method(klass_font, "__set_bold", set_bold, 0);
            rb_define_method(klass_font, "__set_italic", set_italic, 0);
            rb_define_method(klass_font, "__set_underline", set_underline, 0);
            rb_define_method(klass_font, "__set_strike_through", set_strike_through, 0);
            rb_define_method(klass_font, "__apply_default", apply_default, 1);
            rb_define_method(klass_font, "__set_size", set_size, 0);
            //rb_define_method(klass_font, "__set_outline", set_outline, 0);
            rb_define_method(klass_font, "dispose", dispose_font, 0);
            
            //draw_text
            rb_define_method(klass, "__draw_text_6args", draw_text1, 6);
            rb_define_method(klass, "__draw_text_5args", draw_text2, 5);
            rb_define_method(klass, "__draw_text_3args", draw_text3,3);
            rb_define_method(klass, "__draw_text_2args", draw_text4, 2);
            
            rb_define_method(klass, "text_size", text_size, 1);
            
        }
        
    }
}