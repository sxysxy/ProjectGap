#include "stdafx.h"
#include "RGSSSprite.h"
#include "RGSSBitmap.h"

namespace RGSS {
    namespace Sprite {
        VALUE klass;
        
        static VALUE initialize(VALUE self) {  
            Graphics::CreateRenderTask(self, [self]() {
                VALUE bitmap = rb_funcall2(self, rb_intern("bitmap"), 0, nullptr);
                if(bitmap == Qnil)return;
              
                bool visible = rb_funcall2(self, rb_intern("visible"), 0, nullptr) == Qtrue;
                int x = FIX2INT(rb_funcall2(self, rb_intern("x"), 0, nullptr));
                int y = FIX2INT(rb_funcall2(self, rb_intern("y"), 0, nullptr));
                int ox = FIX2INT(rb_funcall2(self, rb_intern("ox"), 0, nullptr));
                int oy = FIX2INT(rb_funcall2(self, rb_intern("oy"), 0, nullptr));
                double zoom_x = rb_float_noflonum_value(rb_funcall2(self, rb_intern("zoom_x"), 0, nullptr));
                double zoom_y = rb_float_noflonum_value(rb_funcall2(self, rb_intern("zoom_y"), 0, nullptr));
                int angle = FIX2INT(rb_funcall2(self, rb_intern("angle"), 0, nullptr));
                bool hmirror = rb_funcall2(self, rb_intern("mirror"), 0, nullptr) == Qtrue;
                bool vmirror = rb_funcall2(self, rb_intern("vmirror"), 0, nullptr) == Qtrue;
                int opacity = rb_funcall2(self, rb_intern("opacity"), 0, nullptr);

                Bitmap::BitmapData *data = Bitmap::GetData(bitmap);
                if(!data || !data->texture)return;
                
                RRect srcR(0, 0, data->width, data->height);
                RRect destR(x, y, int(zoom_x*data->width), int(zoom_y*data->height));
                SDL_Point center = SDL_Point{ox, oy};
                SDL_RendererFlip flip = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL*hmirror | SDL_FLIP_VERTICAL*vmirror);
                SDL_SetRenderTarget(Graphics::renderer, nullptr);
                SDL_SetRenderDrawBlendMode(Graphics::renderer, SDL_BLENDMODE_BLEND);
                Uint8 old_opacity;
                SDL_GetTextureAlphaMod(data->texture, &old_opacity);
                SDL_SetTextureAlphaMod(data->texture, opacity);
                SDL_RenderCopyEx(Graphics::renderer, data->texture, &srcR, &destR, -angle, &center, flip);
                SDL_SetTextureAlphaMod(data->texture, old_opacity);
            });
            return self;
        }
        static VALUE update(VALUE self) {
            return Qnil;
        }
        static VALUE xxx(VALUE self) {
            MessageBoxW(0, L"233", L"2333", 0);
            return Qnil;
        }
       
        void InitSprite() {
            klass = rb_eval_cstring("Sprite");
            LoadLibScript("Sprite.rb");

            rb_define_method(klass, "__init", initialize, 0);
            rb_define_method(klass, "update", update, 0);

            //Set Hook
           // SetHook(rgss_sprite_new, xxx);
           // SetHook(gPluginData.hRGSSCore + 0xa450/4, initialize);
           // SetHook(gPluginData.hRGSSCore + 0xa510/4, initialize);
           // SetHook(rgss_sprite_setbitmap, xxx);
        }
    }
}