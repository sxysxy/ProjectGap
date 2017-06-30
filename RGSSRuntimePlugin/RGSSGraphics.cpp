#include "stdafx.h"
#include "RGSSGraphics.h"
#include "RubySupport.h"
#include "RGSSRuntimePlugin.h"

namespace RGSS {
    namespace Graphics {
        SDL_Renderer *renderer;
        VALUE klass;

        static VALUE __cdecl update(VALUE self) {
            SDL_SetRenderTarget(renderer, nullptr);
            SDL_RenderPresent(renderer);
            return self;
        }
        static VALUE __cdecl clear(VALUE self) {
            SDL_SetRenderTarget(renderer, nullptr);
            SDL_RenderClear(renderer);
            return self;
        }

        void InitGraphics() {
          //  puts("Plugin InitGraphics");

            renderer = gPluginData.GraphicsInformation.renderer;
            klass = rb_eval_cstring("Graphics");
            rb_define_module_function(klass, "update", update, 0);
            rb_define_module_function(klass, "clear", clear, 0);
        }
    }
}