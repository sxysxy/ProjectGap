#pragma once

#include "stdafx.h"
#include "RGSSGraphics.h"
#include "RubySupport.h"
#include "RGSSRuntimePlugin.h"
using namespace Ruby;

namespace RGSS {
    namespace Graphics {
        extern "C" {
            //
            struct tagGraphicsData{
                bool freeze;
                int frame_count;
                bool show_fps;

                int brightness;
            };

            extern SDL_Renderer *renderer;
            extern VALUE klass;
            extern tagGraphicsData GraphicsData;

            void InitGraphics();
        }
    }
}