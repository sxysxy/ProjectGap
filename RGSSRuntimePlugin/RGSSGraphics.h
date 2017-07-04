#pragma once

#include "stdafx.h"
#include "RGSSGraphics.h"
#include "RubySupport.h"
#include "RGSSRuntimePlugin.h"
#include <vector>
#include <set>
#include <unordered_map>
#include <functional>
using namespace Ruby;

namespace RGSS {

    namespace Graphics {
        extern "C" {
            //
            typedef std::function<void(void)> RenderTaskCall;

            struct RenderTask {
                VALUE sprite;
                RenderTaskCall render;
                inline int priority() const{
                    return FIX2INT(rb_funcall2(sprite, rb_intern("z"), 0, nullptr));
                }
                bool operator<(const RenderTask &t)const {
                    return priority() < t.priority();
                }
                bool IsValid() const{
                    return rb_funcall2(sprite, rb_intern("disposed?"), 0, nullptr) != Qtrue;
                }
            };
            struct CompareTask {
                bool operator()(const RenderTask *a, const RenderTask *b) const{
                    return (*a) < (*b);
                }
            };
            struct tagGraphicsData{
                bool freeze;
                int frame_count;
                bool show_fps;

                int brightness;
                std::set < RenderTask *, CompareTask> tasks;
                
            };
            extern tagGraphicsData GraphicsData;
            
            inline void CreateRenderTask(VALUE obj, const std::function<void(void)> &t) {
                GraphicsData.tasks.insert(new RenderTask{obj, t});
            }

            extern SDL_Renderer *renderer;
            extern VALUE klass;
            extern tagGraphicsData GraphicsData;

            void InitGraphics();
        }
    }
}