#include "stdafx.h"
#include "RGSSGraphics.h"
#include "RubySupport.h"
#include "RGSSRuntimePlugin.h"
#include <algorithm>
#include <functional>
#include "RGSSBitmap.h"

namespace RGSS {
    namespace Graphics {
        SDL_Renderer *renderer;
        VALUE klass;
        tagGraphicsData GraphicsData;
        unsigned gTaskOrder;

        static VALUE __cdecl update(VALUE self) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            for (auto it = GraphicsData.tasks.begin(); it != GraphicsData.tasks.end(); ){
                if ((*it)->IsValid()) {
                    (*it)->render();
                    ++it;
                }else {
                    auto p = it;
                    delete (*it);
                    ++it;
                    GraphicsData.tasks.erase(p);
                }
            }
            
            SDL_SetRenderTarget(renderer, nullptr);
            if (!GraphicsData.freeze) {
                SDL_RenderPresent(renderer);
               
            }else {
                //用于执行渐变的代码
            }
            GraphicsData.frame_count++;
            if (GraphicsData.frame_count == 60 && GraphicsData.show_fps) {
                //用于显示fps的代码
            }
            return Qnil;
        }
        static VALUE __cdecl clear(VALUE self) {
            SDL_SetRenderTarget(renderer, nullptr);
            SDL_RenderClear(renderer);
            return self;
        }
        static VALUE __cdecl frame_count(VALUE self) {
            return INT2FIX(GraphicsData.frame_count);
        }
        static VALUE __cdecl frame_rate(VALUE self) {
            return INT2FIX(60);
        }
        static VALUE __cdecl get_brightness(VALUE self) {
            return INT2FIX(GraphicsData.brightness);
        }
        static VALUE __cdecl set_brightness(VALUE self, VALUE v) {
            GraphicsData.brightness = FIX2INT(v);
            return v;
        }
        static VALUE __cdecl frame_reset(VALUE self) {
            return Qnil;
        }
        static VALUE __cdecl freeze(VALUE self) {
            GraphicsData.freeze = true;
            return Qnil;
        }
        static VALUE __cdecl transition0(VALUE self) {
            GraphicsData.freeze = false;
            return Qnil;
        }
        static VALUE __cdecl transition1(VALUE self) {
            GraphicsData.freeze = false;
            return Qnil;
        }
        static VALUE __cdecl transition2(VALUE self) {
            GraphicsData.freeze = false;
            return Qnil;
        }
        static VALUE __cdecl transition3(VALUE self) {
            GraphicsData.freeze = false;
            return Qnil;
        }
        static VALUE __cdecl snap_to_bitmap(VALUE self) {
            VALUE bmp = rb_eval_cstring("Bitmap.new(Graphics.width, Graphics.height)");
            Bitmap::BitmapData *data = Bitmap::GetData(bmp);
            SDL_SetRenderTarget(Graphics::renderer, nullptr);
            SDL_RenderReadPixels(Graphics::renderer, nullptr, 0, data->pixels, 4*data->width);
            SDL_UpdateTexture(data->texture, nullptr, data->pixels, 4*data->width);
            data->dirty = false;
            return bmp;
        }
        void InitGraphics() {
          //  puts("Plugin InitGraphics");

            LoadLibScript("Graphics.rb");
            gTaskOrder = 0;

            renderer = gPluginData.GraphicsInformation.renderer;
            klass = rb_eval_cstring("Graphics");
            rb_define_module_function(klass, "update", update, 0);
            rb_define_module_function(klass, "clear", clear, 0);
            rb_define_module_function(klass, "frame_count", frame_count, 0);
            rb_define_module_function(klass, "brightness", get_brightness, 0);
            rb_define_module_function(klass, "brightness=", set_brightness, 1);
            rb_define_module_function(klass, "frame_reset", frame_reset, 0);
            rb_define_module_function(klass, "freeze", freeze, 0);
            rb_define_module_function(klass, "__transition_noarg", transition0, 0);
            rb_define_module_function(klass, "__transition_1arg", transition1, 1);
            rb_define_module_function(klass, "__transition_2args", transition2, 2);
            rb_define_module_function(klass, "__transition_3args", transition3, 3);
            rb_define_module_function(klass, "snap_to_bitmap", snap_to_bitmap, 0);
        }

        VALUE rgss_check_sprite_dispose_protect(VALUE sprite) {
            return rb_funcall2(sprite, rb_intern("disposed?"), 0, nullptr);
        }
    }
}