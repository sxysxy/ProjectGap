#include "stdafx.h"
#include "RGSSGraphics.h"
#include "RubySupport.h"
#include "RGSSRuntimePlugin.h"

namespace RGSS {
    namespace Graphics {
        SDL_Renderer *renderer;
        VALUE klass;
        tagGraphicsData GraphicsData;

        static VALUE __cdecl update(VALUE self) {
            SDL_SetRenderTarget(renderer, nullptr);
            if (!GraphicsData.freeze) {
                SDL_RenderPresent(renderer);
                GraphicsData.frame_count++;
                if (GraphicsData.frame_count == 60 && GraphicsData.show_fps) {
                    //用于显示fps的代码
                }
            }
            return self;
        }
        static VALUE __cdecl clear(VALUE self) {
            SDL_SetRenderTarget(renderer, nullptr);
            SDL_RenderClear(renderer);
            return self;
        }
        static VALUE __cdecl frame_count(VALUE self) {
            return INT2FIX(GraphicsData.frame_count);
        }
        void InitGraphics() {
          //  puts("Plugin InitGraphics");

            renderer = gPluginData.GraphicsInformation.renderer;
            klass = rb_eval_cstring("Graphics");
            rb_define_module_function(klass, "update", update, 0);
            rb_define_module_function(klass, "clear", clear, 0);
            rb_define_module_function(klass, "frame_count", frame_count, 0);
        }
    }
}