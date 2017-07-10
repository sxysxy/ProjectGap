#include "stdafx.h"
#include "RGSSWindow.h"
#include "RGSSGraphics.h"
#include "RGSSBitmap.h"
#include "RGSSSprite.h"
#include "RubySupport.h"

namespace RGSS {
    namespace Window {
        VALUE klass;

        void InitWindow() {
            klass = rb_eval_cstring("Window");

        }
    }
}