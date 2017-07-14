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
                unsigned order;   //用于记录先后顺序(priority相同的情况下比较order)
                inline int priorityz() const{
                    return FIX2INT(rb_funcall2(sprite, rb_intern("z"), 0, nullptr));
                }
                inline int priorityy() const {
                    return FIX2INT(rb_funcall2(sprite, rb_intern("y"), 0, nullptr));
                }
                bool operator<(const RenderTask &t)const {
                    int a =  priorityz(), b = t.priorityz();
                    if(a < b)return true;
                    else if (a == b) {
                        int c = priorityy(), d = t.priorityy();
                        return (c < d || (c == d && order < t.order));
                    }else
                        return false;
                }
                bool IsValid() const{
                    if (rb_type(sprite) == RUBY_CLASS_NONE)return false;
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
            
            extern unsigned gTaskOrder;
            inline void CreateRenderTask(VALUE obj, const std::function<void(void)> &t) {
                GraphicsData.tasks.insert(new RenderTask{obj, t, gTaskOrder++});
            }

            extern SDL_Renderer *renderer;
            extern VALUE klass;
            extern tagGraphicsData GraphicsData;
            extern bool onForeground;

            void InitGraphics();
        }
    }
}