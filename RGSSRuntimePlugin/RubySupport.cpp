#include "stdafx.h"
#include "RubySupport.h"

namespace Ruby {
    pfn_rb_funcall2 rb_funcall2;
    pfn_rb_define_class rb_define_class;
    pfn_rb_const_defined rb_const_defined;
    pfn_rb_const_get rb_const_get;
    pfn_rb_intern rb_intern;
    pfn_rb_define_module rb_define_module;
    pfn_rb_define_module_function rb_define_module_function;
    pfn_rb_define_global_const rb_define_global_const;
    pfn_rb_define_global_function rb_define_global_function;
    pfn_rb_eval_string_protect rb_eval_string_protect;;
    pfn_rb_id2name rb_id2name;
    pfn_rb_scan_args rb_scan_args;
    pfn_rb_class_new_instance rb_class_new_instance;
    pfn_rb_define_method rb_define_method;
    pfn_rb_str_new  rb_str_new;
    pfn_rb_str_new2  rb_str_new2;
    pfn_rb_define_const rb_define_const;
    pfn_rb_string_value rb_string_value;
    pfn_rb_string_value_ptr rb_string_value_ptr;
    pfn_rb_obj_is_kind_of rb_obj_is_kind_of;
    pfn_rb_raise rb_raise;
    pfn_rb_obj_classname rb_obj_classname;
    pfn_rb_ary_aref rb_ary_aref;
    pfn_rb_ary_aset rb_ary_aset;
    pfn_rb_ary_new rb_ary_new;
    pfn_rb_ary_push rb_ary_push;
    VALUE rb_cObject;

    void InitRuntime(HMODULE hRGSSCore) {
#define __set_ptr(fn) fn = (pfn_##fn)((DWORD)addr_##fn+(DWORD)hRGSSCore);
    __set_ptr(rb_funcall2)
    __set_ptr(rb_define_class)
    __set_ptr(rb_intern)
    __set_ptr(rb_define_module)
    __set_ptr(rb_define_module_function)
    __set_ptr(rb_define_global_const)
    __set_ptr(rb_eval_string_protect)
    __set_ptr(rb_ary_new)
    __set_ptr(rb_str_new)
    __set_ptr(rb_define_method)
    __set_ptr(rb_scan_args)
    __set_ptr(rb_class_new_instance)
#undef __set_ptr
       // rb_cObject = rb_eval_string_protect("Object", nullptr);
    }
}