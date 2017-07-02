#pragma once
#include "stdafx.h"
#include "RubySupport.h"
#include "RGSSRuntimePlugin.h"
using namespace Ruby;

//BitampÀàµÄÐÞ¸Ä¡£

namespace RGSS {
    RColor RGSSColor2RColor(VALUE color);
    RRect RGSSRect2RRect(VALUE rect);

    namespace Bitmap {

        //
        
        extern VALUE klass;
        void InitBitmap();
   
    }
}