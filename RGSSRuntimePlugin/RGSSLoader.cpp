#include "stdafx.h"
#include "RGSSLoader.h"

namespace RGSS {
    namespace RGSSLoader {
        VALUE klass;

        VALUE window_handle(VALUE self) {
            return INT2FIX((long)gPluginData.GraphicsInformation.hWnd);
        }
        VALUE debug_break(VALUE self) {
            __asm {
                int 0x03
            }
            return Qnil;
        }
        VALUE rgssdll_handle(VALUE self) {
            return INT2FIX((long)gPluginData.hRGSSCore);
        }

        void InitRGSSLoader() {
            klass = rb_define_module("RGSSLoader");

            rb_define_module_function(klass, "window_handle", window_handle, 0);
            rb_define_module_function(klass, "debug_break", debug_break, 0);
            rb_define_module_function(klass, "rgssdll_handle", rgssdll_handle, 0);
        }
    }
}