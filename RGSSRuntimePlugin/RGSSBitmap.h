#pragma once
#include "stdafx.h"
#include "RubySupport.h"
#include "RGSSRuntimePlugin.h"
using namespace Ruby;

//BitampÀàµÄÐÞ¸Ä¡£

namespace RGSS {
    namespace Bitmap {

        //
        
        extern VALUE klass;
        void InitBitmap();
        RColor RGSSColor2RColor(VALUE color);
        RRect RGSSRect2RRect(VALUE rect);
    }
}