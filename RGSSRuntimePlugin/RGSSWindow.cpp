#include "stdafx.h"
#include "RGSSWindow.h"
#include "RGSSGraphics.h"
#include "RGSSBitmap.h"
#include "RGSSSprite.h"
#include "RubySupport.h"
#include <algorithm>
using namespace Ruby;

namespace RGSS {
    namespace Window {
        VALUE klass;

        static VALUE initialize(VALUE self) {
            rb_iv_set(self, "@openness", INT2FIX(0));
            rb_iv_set(self, "@active", Qtrue);
            rb_iv_set(self, "@visible", Qtrue);
            rb_iv_set(self, "@arrows_visible", Qtrue);
            rb_iv_set(self, "@pause", Qfalse);
            rb_iv_set(self, "@ox", INT2FIX(0));
            rb_iv_set(self, "@oy", INT2FIX(0));
            rb_iv_set(self, "@z", INT2FIX(100));
            rb_iv_set(self, "@padding", INT2FIX(12));
            rb_iv_set(self, "@padding_bottom", INT2FIX(12));
            rb_iv_set(self, "@opacity", INT2FIX(255));
            rb_iv_set(self, "@back_opacity", INT2FIX(255));
            rb_iv_set(self, "@contents_opacity", INT2FIX(255));
            rb_iv_set(self, "@tone", rb_eval_cstring("Tone.new"));
            rb_iv_set(self, "@cursor_rect", rb_eval_cstring("Rect.new"));
            rb_iv_set(self, "@contents", rb_eval_cstring("Bitmap.new(1, 1)"));
            rb_iv_set(self, "@__disposed", Qfalse);

            rb_iv_set(self, "@back_sprite", rb_eval_cstring("Sprite.new"));
            rb_iv_set(self, "@frame_sprite", rb_eval_cstring("Sprite.new"));
            rb_iv_set(self, "@contents_sprite", rb_eval_cstring("Sprite.new"));
            rb_iv_set(self, "@cursor_sprite", rb_eval_cstring("Sprite.new"));
            rb_iv_set(self, "@arrows_sprite", rb_eval_cstring("Sprite.new"));
            rb_iv_set(self, "@pause_sprite", rb_eval_cstring("Sprite.new"));
            return self;
        }


        static void update_back(VALUE self) {
            VALUE sprite = rb_funcall2(self, rb_intern("back_sprite"), 0, nullptr);
            int pd = FIX2INT(rb_funcall2(self, rb_intern("padding"), 0, nullptr));
            //int pdb = FIX2INT(rb_funcall2(self, rb_intern("padding_bottom"), 0, nullptr));
            //int w = FIX2INT(rb_funcall2(self, rb_intern("width"), 0, nullptr)) - pd;
            //int h = FIX2INT(rb_funcall2(self, rb_intern("height"), 0, nullptr)) - (pd + pdb) / 2;
           // printf("%d\n", FIX2INT(rb_funcall2(self, rb_intern("y"), 0, nullptr)) + pd / 2);
            rb_iv_set(sprite, "@x", INT2FIX(FIX2INT(rb_funcall2(self, rb_intern("x"), 0, nullptr)) + pd / 2));
            rb_iv_set(sprite, "@y", INT2FIX(FIX2INT(rb_funcall2(self, rb_intern("y"), 0, nullptr)) + pd / 2));
            rb_iv_set(sprite, "@z", rb_funcall2(self, rb_intern("z"), 0, nullptr));

            //opacity:
            float opacity = FIX2INT(rb_funcall2(self, rb_intern("opacity"), 0, nullptr)) / 255.0f;
            float back_opacity = FIX2INT(rb_funcall2(self, rb_intern("back_opacity"), 0, nullptr)) / 255.0f;
            rb_iv_set(sprite, "@opacity", INT2FIX(int(255 * opacity*back_opacity)));
            rb_iv_set(sprite, "@visible", rb_funcall2(self, rb_intern("open?"), 0, nullptr));
        }
        static void update_frame(VALUE self) {
            VALUE sprite = rb_funcall2(self, rb_intern("frame_sprite"), 0, nullptr);
            int pd = FIX2INT(rb_funcall2(self, rb_intern("padding"), 0, nullptr));
            // int pdb = FIX2INT(rb_funcall2(self, rb_intern("padding_bottom"), 0, nullptr));
            //int w = FIX2INT(rb_funcall2(self, rb_intern("width"), 0, nullptr));
            //int h = FIX2INT(rb_funcall2(self, rb_intern("height"), 0, nullptr));
            rb_iv_set(sprite, "@x", rb_funcall2(self, rb_intern("x"), 0, nullptr));
            rb_iv_set(sprite, "@y", rb_funcall2(self, rb_intern("y"), 0, nullptr));
            rb_iv_set(sprite, "@z", rb_funcall2(self, rb_intern("z"), 0, nullptr));
        }

        static void update_contents(VALUE self) {
            VALUE sprite = rb_funcall2(self, rb_intern("contents_sprite"), 0, nullptr);
            int pd = FIX2INT(rb_funcall2(self, rb_intern("padding"), 0, nullptr));
            // int pdb = FIX2INT(rb_funcall2(self, rb_intern("padding_bottom"), 0, nullptr));
            // int w = FIX2INT(rb_funcall2(self, rb_intern("width"), 0, nullptr));
            // int h = FIX2INT(rb_funcall2(self, rb_intern("height"), 0, nullptr));
            rb_iv_set(sprite, "@x", INT2FIX(FIX2INT(rb_funcall2(self, rb_intern("x"), 0, nullptr)) + pd ));
            rb_iv_set(sprite, "@y", INT2FIX(FIX2INT(rb_funcall2(self, rb_intern("y"), 0, nullptr)) + pd ));;
            rb_iv_set(sprite, "@z", rb_funcall2(self, rb_intern("z"), 0, nullptr));
            rb_iv_set(sprite, "@bitmap", rb_funcall2(self, rb_intern("contents"), 0, nullptr));

            //opacity:
            float opacity = FIX2INT(rb_funcall2(self, rb_intern("opacity"), 0, nullptr))/255.0f;
            float contents_opacity = FIX2INT(rb_funcall2(self, rb_intern("contents_opacity"), 0, nullptr))/255.0f;
            rb_iv_set(sprite, "@opacity", INT2FIX(int(255*opacity*contents_opacity)));
            rb_iv_set(sprite, "@visible", rb_funcall2(self, rb_intern("open?"), 0, nullptr));
        }

        //static void refresh_cursor(VALUE self);
        static void update_cursor(VALUE self) {
            /*
            VALUE sprite = rb_funcall2(self, rb_intern("cursor_sprite"), 0, nullptr);
            int pd = FIX2INT(rb_funcall2(self, rb_intern("padding"), 0, nullptr)) >> 1;
            RRect cursor_rect = RGSSRect2RRect(rb_funcall2(self, rb_intern("cursor_rect"), 0, nullptr));
            if (!cursor_rect.w || !cursor_rect.h)return;

            int x = cursor_rect.x + pd;
            int y = cursor_rect.y + pd;
            int w = cursor_rect.w;
            int h = cursor_rect.h;
            const int m = 4;
            int x2 = Rmax(x, pd);
            int y2 = Rmax(y, pd);
           // int ox = x - x2;
           // int oy = y - y2;
           // int w2 = Rmin(w, FIX2INT(rb_funcall2(self, rb_intern("width"), 0, nullptr)) - pd - x2);
           // int h2 = Rmin(h, FIX2INT(rb_funcall2(self, rb_intern("height"), 0, nullptr)) - pd - y2);

            rb_iv_set(sprite, "@x", INT2FIX(pd + x2 + FIX2INT(rb_funcall2(self, rb_intern("x"), 0, nullptr))));
            rb_iv_set(sprite, "@y", INT2FIX(pd + y2 + FIX2INT(rb_funcall2(self, rb_intern("y"), 0, nullptr))));
            */
          
           // refresh_cursor(self);
            VALUE sprite = rb_funcall2(self, rb_intern("cursor_sprite"), 0, nullptr);
            int pd = FIX2INT(rb_funcall2(self, rb_intern("padding"), 0, nullptr)) >> 1;
            RRect cursor_rect = RGSSRect2RRect(rb_funcall2(self, rb_intern("cursor_rect"), 0, nullptr));
            if (!cursor_rect.w || !cursor_rect.h)return;

            int x = cursor_rect.x + pd;
            int y = cursor_rect.y + pd;
            int w = cursor_rect.w;
            int h = cursor_rect.h;
            const int m = 4;
            int x2 = Rmax(x, pd);
            int y2 = Rmax(y, pd);
            rb_iv_set(sprite, "@x", INT2FIX(pd + x2 + FIX2INT(rb_funcall2(self, rb_intern("x"), 0, nullptr))));
            rb_iv_set(sprite, "@y", INT2FIX(pd + y2 + FIX2INT(rb_funcall2(self, rb_intern("y"), 0, nullptr))));

            int count = FIX2INT(rb_funcall2(self, rb_intern("frame_count"), 0, nullptr)%40);
            int cursor_opacity = FIX2INT(rb_funcall2(self, rb_intern("contents_opacity"), 0, nullptr)); 
            if (rb_funcall2(self, rb_intern("active"), 0, nullptr) == Qtrue) {
                if(count < 20)
                    cursor_opacity -= (count << 2);
                else
                    cursor_opacity -= ((40-count) << 2);
            }
           
            rb_iv_set(sprite, "@opacity", INT2FIX(cursor_opacity));
            rb_iv_set(sprite, "@visible", rb_funcall2(self, rb_intern("open?"), 0, nullptr));
        }

        static void refresh_back(VALUE self) {
            VALUE sprite = rb_funcall2(self, rb_intern("back_sprite"), 0, nullptr);
            int pd = FIX2INT(rb_funcall2(self, rb_intern("padding"), 0, nullptr));
            int pdb = FIX2INT(rb_funcall2(self, rb_intern("padding_bottom"), 0, nullptr));
            int w = FIX2INT(rb_funcall2(self, rb_intern("width"), 0, nullptr))-pd;
            int h = FIX2INT(rb_funcall2(self, rb_intern("height"), 0, nullptr))-(pd+pdb)/2;
            rb_iv_set(sprite, "@x", INT2FIX(FIX2INT(rb_funcall2(self, rb_intern("x"), 0, nullptr))+pd/2));
            rb_iv_set(sprite, "@y", INT2FIX(FIX2INT(rb_funcall2(self, rb_intern("y"), 0, nullptr))+pd/2));
            rb_iv_set(sprite, "@z", rb_funcall2(self, rb_intern("z"), 0, nullptr));

            //--------------------------------------------
            VALUE t[2];
            t[0] = INT2FIX(w); t[1] = INT2FIX(h);
            VALUE bitmap = rb_funcall2(Bitmap::klass, rb_intern("new"), 2, t);
            VALUE old_bmp = rb_funcall2(sprite, rb_intern("bitmap"), 0, nullptr);
            if (old_bmp != Qnil) {
                Bitmap::dispose(old_bmp);
            }
            rb_iv_set(sprite, "@bitmap", bitmap);
            
            SDL_Texture *skin = Bitmap::GetTexture(rb_funcall2(self, rb_intern("windowskin"), 0, nullptr));
            Bitmap::BitmapData *contents_data = Bitmap::GetData(bitmap);

            if (w > 0 && h > 0 && skin != nullptr) {
                const int p = 64;
                Bitmap::stretch_blt_opacity_base(contents_data, 
                    RRect(0, 0, w, h),
                    skin,
                    RRect(0, 0, p, p),
                    255);
                for (int y = 0; y < h; y += p)for (int x = 0; x < w; x += p) {
                    Bitmap::stretch_blt_opacity_base(contents_data,
                        RRect(x, y, p, p),
                        skin,
                        RRect(0, p, p, p),
                        255);
                }
            }
        }
        static void refresh_frame(VALUE self) {
            VALUE sprite = rb_funcall2(self, rb_intern("frame_sprite"), 0, nullptr);
            int pd = FIX2INT(rb_funcall2(self, rb_intern("padding"), 0, nullptr));
           // int pdb = FIX2INT(rb_funcall2(self, rb_intern("padding_bottom"), 0, nullptr));
            int w = FIX2INT(rb_funcall2(self, rb_intern("width"), 0, nullptr));
            int h = FIX2INT(rb_funcall2(self, rb_intern("height"), 0, nullptr));
            rb_iv_set(sprite, "@x", rb_funcall2(self, rb_intern("x"), 0, nullptr));
            rb_iv_set(sprite, "@y", rb_funcall2(self, rb_intern("y"), 0, nullptr));
            rb_iv_set(sprite, "@z", rb_funcall2(self, rb_intern("z"), 0, nullptr));

            //-----------------------------
            VALUE t[2];
            t[0] = INT2FIX(w); t[1] = INT2FIX(h);
            VALUE bitmap = rb_funcall2(Bitmap::klass, rb_intern("new"), 2, t);
            VALUE old_bmp = rb_funcall2(sprite, rb_intern("bitmap"), 0, nullptr);
            if (old_bmp != Qnil) {
                Bitmap::dispose(old_bmp);
            }
            rb_iv_set(sprite, "@bitmap", bitmap);

            SDL_Texture *skin = Bitmap::GetTexture(rb_funcall2(self, rb_intern("windowskin"), 0, nullptr));
            Bitmap::BitmapData *contents_data = Bitmap::GetData(bitmap);

            const int m = pd;
            if (w > 0 && h > 0 && skin) {
                const int p = 64;
                const int q = 64;
                
                //bitmap.blt(skin, p + m, 0 + 0, p - m * 2, m, m, 0, w - m * 2, m);
                Bitmap::stretch_blt_opacity_base(contents_data, 
                    RRect(m, 0, w - m * 2, m),
                    skin,
                    RRect(p + m, 0 + 0, p - m * 2, m), 255);

                //bitmap.blt(skin, p + m, 0 + q - m, p - m * 2, m, m, h - m, w - m * 2, m);
                Bitmap::stretch_blt_opacity_base(contents_data,
                    RRect(m, h - m, w - m * 2, m),
                    skin,
                    RRect(p + m, 0 + q - m, p - m * 2, m), 255);

                //bitmap.blt(skin, p + 0, 0 + m, m, p - m * 2, 0, m, m, h - m * 2);
                Bitmap::stretch_blt_opacity_base(contents_data,
                    RRect(0, m, m, h - m * 2),
                    skin,
                    RRect(p + 0, 0 + m, m, p - m * 2), 255);

                //bitmap.blt(skin, p + q - m, 0 + m, m, p - m * 2, w - m, m, m, h - m * 2);
                Bitmap::stretch_blt_opacity_base(contents_data,
                    RRect(w - m, m, m, h - m * 2),
                    skin,
                    RRect(p + q - m, 0 + m, m, p - m * 2), 255);

                //bitmap.blt(skin, p + 0, 0 + 0, m, m, 0, 0, m, m);
                Bitmap::stretch_blt_opacity_base(contents_data,
                    RRect(0, 0, m, m),
                    skin,
                    RRect(p + 0, 0 + 0, m, m), 255);

                //bitmap.blt(skin, p + q - m, 0 + 0, m, m, w - m, 0, m, m);
                Bitmap::stretch_blt_opacity_base(contents_data,
                    RRect(w - m, 0, m, m),
                    skin,
                    RRect(p + q - m, 0 + 0, m, m), 255);

                //bitmap.blt(skin, p + 0, 0 + q - m, m, m, 0, h - m, m, m);
                Bitmap::stretch_blt_opacity_base(contents_data,
                    RRect(0, h - m, m, m),
                    skin,
                    RRect(p + 0, 0 + q - m, m, m), 255);

                //bitmap.blt(skin, p + q - m, 0 + q - m, m, m, w - m, h - m, m, m);
                Bitmap::stretch_blt_opacity_base(contents_data,
                    RRect(w - m, h - m, m, m),
                    skin,
                    RRect(p + q - m, 0 + q - m, m, m), 255);
                
            }
        }

        inline static void refresh_contents(VALUE self) {
            update_contents(self);
        }

        static VALUE refresh_cursor(VALUE self) {
            VALUE sprite = rb_funcall2(self, rb_intern("cursor_sprite"), 0, nullptr);
            int pd = FIX2INT(rb_funcall2(self, rb_intern("padding"), 0, nullptr))>>1;
            RRect cursor_rect = RGSSRect2RRect(rb_funcall2(self, rb_intern("cursor_rect"), 0, nullptr));
            if(!cursor_rect.w || !cursor_rect.h)return Qnil;
            
            int x = cursor_rect.x+pd;
            int y = cursor_rect.y+pd;
            int w = cursor_rect.w;
            int h = cursor_rect.h;
            const int m = 8;
            int x2 = Rmax(x, pd);
            int y2 = Rmax(y, pd);
            int ox = x - x2;
            int oy = y - y2;
            int w2 = Rmin(w, FIX2INT(rb_funcall2(self, rb_intern("width"), 0, nullptr))-pd-x2);
            int h2 = Rmin(h, FIX2INT(rb_funcall2(self, rb_intern("height"), 0, nullptr))-pd-y2);
            VALUE tmp[2];
            tmp[0] = INT2FIX(w2), tmp[1] = INT2FIX(h2);
            VALUE bitmap = rb_funcall2(Bitmap::klass, rb_intern("new"), 2, tmp);
            VALUE old_bmp = rb_funcall2(sprite, rb_intern("bitmap"), 0, nullptr);
            if (old_bmp != Qnil) {
                Bitmap::dispose(old_bmp);
            }
            rb_iv_set(sprite, "@bitmap", bitmap);
            //rb_iv_set(sprite, "@width", INT2FIX(w2));
            //rb_iv_set(sprite, "@height", INT2FIX(h2));
            
            rb_iv_set(sprite, "@x", INT2FIX(pd + x2 + FIX2INT(rb_funcall2(self, rb_intern("x"), 0, nullptr))));
            rb_iv_set(sprite, "@y", INT2FIX(pd + y2 + FIX2INT(rb_funcall2(self, rb_intern("y"), 0, nullptr))));
            SDL_Texture *skin = Bitmap::GetTexture(rb_funcall2(self, rb_intern("windowskin"), 0, nullptr));
            Bitmap::BitmapData *contents_data = Bitmap::GetData(bitmap);
            if (skin) {
                const int p = 64;
                const int q = 32;
               
                //bitmap.blt(skin, p + m, p + m, q - m * 2, q - m * 2, ox + m, oy + m, w - m * 2, h - m * 2);
                Bitmap::stretch_blt_opacity_base(contents_data, 
                    RRect(ox + m, oy + m, w - m * 2, h - m * 2),
                    skin,
                    RRect(p + m, p + m, q - m * 2, q - m * 2), 255);

                //bitmap.blt(skin, p + m, p + 0, q - m * 2, m, ox + m, oy + 0, w - m * 2, m);
                Bitmap::stretch_blt_opacity_base(contents_data,
                    RRect(ox + m, oy + 0, w - m * 2, m),
                    skin,
                    RRect(p + m, p + 0, q - m * 2, m), 255);

                //bitmap.blt(skin, p + m, p + q - m, q - m * 2, m, ox + m, oy + h - m, w - m * 2, m);
                Bitmap::stretch_blt_opacity_base(contents_data,
                    RRect(ox + m, oy + h - m, w - m * 2, m),
                    skin,
                    RRect(p + m, p + q - m, q - m * 2, m), 255);

                //bitmap.blt(skin, p + 0, p + m, m, q - m * 2, ox + 0, oy + m, m, h - m * 2);
                Bitmap::stretch_blt_opacity_base(contents_data,
                    RRect(ox + 0, oy + m, m, h - m * 2),
                    skin,
                    RRect(p + 0, p + m, m, q - m * 2), 255);

                //bitmap.blt(skin, p + q - m, p + m, m, q - m * 2, ox + w - m, oy + m, m, h - m * 2);
                Bitmap::stretch_blt_opacity_base(contents_data,
                    RRect(ox + w - m, oy + m, m, h - m * 2),
                    skin,
                    RRect(p + q - m, p + m, m, q - m * 2), 255);

                //bitmap.blt(skin, p + 0, p + 0, m, m, ox + 0, oy + 0, m, m);
                Bitmap::stretch_blt_opacity_base(contents_data,
                    RRect(ox + 0, oy + 0, m, m),
                    skin,
                    RRect(p + 0, p + 0, m, m), 255);

                //bitmap.blt(skin, p + q - m, p + 0, m, m, ox + w - m, oy + 0, m, m);
                Bitmap::stretch_blt_opacity_base(contents_data,
                    RRect(ox + w - m, oy + 0, m, m),
                    skin,
                    RRect(p + q - m, p + 0, m, m), 255);

                //bitmap.blt(skin, p + 0, p + q - m, m, m, ox + 0, oy + h - m, m, m);
                Bitmap::stretch_blt_opacity_base(contents_data,
                    RRect(ox + 0, oy + h - m, m, m),
                    skin,
                    RRect(p + 0, p + q - m, m, m), 255);

                //bitmap.blt(skin, p + q - m, p + q - m, m, m, ox + w - m, oy + h - m, m, m)
                Bitmap::stretch_blt_opacity_base(contents_data,
                    RRect(ox + w - m, oy + h - m, m, m),
                    skin,
                    RRect(p + q - m, p + q - m, m, m), 255);
            }

            return Qnil;
        }

        static VALUE update(VALUE self) {
            update_back(self);
            update_frame(self);
            update_contents(self);
            update_cursor(self);
          //  update_arrow(self);
           // update_pause(self);
            rb_iv_set(self, "@frame_count", INT2FIX(1+FIX2INT(rb_funcall2(self, rb_intern("frame_count"), 0, nullptr))));
            return Qnil;
        }

        static VALUE refresh_all(VALUE self) {
            refresh_back(self);
            refresh_frame(self);
            refresh_contents(self);
            refresh_cursor(self);
          // refresh_arrow(self);
          //  refresh_pause(self);
            return Qnil;
        }

        void InitWindow() {
            klass = rb_eval_cstring("Window");
            LoadLibScript("Window.rb");

            rb_define_method(klass, "__init", initialize, 0);
            rb_define_method(klass, "update", update, 0);
            rb_define_method(klass, "__refresh_all", refresh_all, 0);
            rb_define_method(klass, "__refresh_cursor", refresh_cursor, 0);
        }

    }
}