#pragma once
#include "RubySupport.h"
#include "RGSSGraphics.h"
using namespace Ruby;

namespace RGSS {
    namespace Window {
        extern VALUE klass;

        void InitWindow();
    }
}