#include "stdafx.h"
#include "RGSSSprite.h"

namespace RGSS {
    namespace Sprite {
        VALUE klass;
        
        static VALUE initialize(VALUE self) {

            return self;
        }
        static VALUE initialize_viewport(VALUE self, VALUE viewport) {

            return self;
        }

        void InitSprite() {
            klass = rb_eval_cstring("Sprite");
            LoadLibScript("Sprite.rb");

            rb_define_method(klass, "__init", initialize, 0);
            rb_define_method(klass, "__init_viewport", initialize, 1);

        }
    }
}