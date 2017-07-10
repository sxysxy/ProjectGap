#pragma once
#include "RubySupport.h"
#include "RGSSRuntimePlugin.h"
using namespace Ruby;

namespace RGSS {
    namespace RGSSLoader {

        extern VALUE klass;
        void InitRGSSLoader();
    }
}