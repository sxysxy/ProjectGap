#pragma once

#include "stdafx.h"
#include <climits>
#include <Common\include\PluginData.h>

#define RUBYCALL __cdecl

namespace Ruby {
    typedef unsigned long VALUE;
    typedef VALUE ID;
  
    constexpr VALUE Qfalse = 0;
    constexpr VALUE Qtrue = 2;
    constexpr VALUE Qnil = 4;
    constexpr VALUE Qundef = 6;

    static inline bool RTEST(VALUE v) { return (v & ~Qnil) != 0; }
    static inline bool NIL_P(VALUE p) { return p == Qnil; }

    constexpr VALUE RUBY_CLASS_NONE = 0x00;
    constexpr VALUE RUBY_CLASS_OBJECT = 0x01;
    constexpr VALUE RUBY_CLASS_CLASS = 0x02;
    constexpr VALUE RUBY_CLASS_MODULE = 0x03;
    constexpr VALUE RUBY_CLASS_FLOAT = 0x04;
    constexpr VALUE RUBY_CLASS_STRING = 0x05;
    constexpr VALUE RUBY_CLASS_REGEXP = 0x06;
    constexpr VALUE RUBY_CLASS_ARRAY = 0x07;
    constexpr VALUE RUBY_CLASS_HASH = 0x08;
    constexpr VALUE RUBY_CLASS_BIGNUM = 0x0a;
    constexpr VALUE RUBY_CLASS_FILE = 0x0b;
    constexpr VALUE RUBY_CLASS_NIL    = 0x11;
    constexpr VALUE RUBY_CLASS_TRUE   = 0x12;
    constexpr VALUE RUBY_CLASS_FALSE  = 0x13;
    constexpr VALUE RUBY_CLASS_SYMBOL = 0x14;
    constexpr VALUE RUBY_CLASS_FIXNUM = 0x15;
    constexpr VALUE RUBY_CLASS_UNDEF  = 0x1b;
    constexpr VALUE RUBY_CLASS_NODE   = 0x1c;
    constexpr VALUE RUBY_CLASS_ICLASS = 0x1d;
    constexpr VALUE RUBY_CLASS_ZOMBIE = 0x1e;
    constexpr VALUE RUBY_CLASS_MASK = 0x1f;

    constexpr VALUE FL_USHIFT = 12;
    template<int n>
    struct FL_USER {
        constexpr static VALUE value = ((VALUE)1) << (FL_USHIFT + n);
    };

    //Basic
    struct RBasic {
        VALUE flags;
        VALUE klass;
    };
    static inline struct RBasic *RBASIC(VALUE obj) {
        return (struct RBasic*)obj;
    }

    //String
    constexpr int RSTRING_EMBED_LEN_MAX = (sizeof(VALUE) * 3) / sizeof(char) - 1;
    struct RString {
        struct RBasic basic;
        union {
            struct {
                long len;
                char *ptr;
                union {
                    long capa;
                    VALUE shared;
                } aux;
            }heap;
            char ary[RSTRING_EMBED_LEN_MAX + 1];
        }as;
    };
    static inline struct RString *RSTRING(VALUE obj) {
       return (struct RString*)obj;
    }
    constexpr VALUE RSTRING_NOEMBED = FL_USER<1>::value;
    constexpr VALUE RSTRING_EMBED_LEN_MASK = FL_USER<2>::value | FL_USER<3>::value | FL_USER<4>::value |
                                                         FL_USER<5>::value | FL_USER<6>::value;
    constexpr VALUE RSTRING_EMBED_LEN_SHIFT = FL_USHIFT + 2;

    static inline char *RSTRING_PTR(VALUE str) {
        return (!(RBASIC(str)->flags & RSTRING_NOEMBED)) ? RSTRING(str)->as.ary : RSTRING(str)->as.heap.ptr;
    }
    static inline long RSTRING_LEN(VALUE str) {
        return (!(RBASIC(str)->flags & RSTRING_NOEMBED)) ? \
            (long)((RBASIC(str)->flags >> RSTRING_EMBED_LEN_SHIFT) & (RSTRING_EMBED_LEN_MASK >> RSTRING_EMBED_LEN_SHIFT)) : \
                RSTRING(str)->as.heap.len;
    }
        
    //Array
    constexpr int RARRAY_EMBED_LEN_MAX = 3;
    struct RArray {
        struct RBasic basic;
        union {
            struct {
                long len;
                union {
                    long capa;
                    VALUE shared;
                }aux;
                VALUE *ptr;
            }heap;
            VALUE ary[RARRAY_EMBED_LEN_MAX];
        }as;
    };
    static inline struct RArray *RARRAY(VALUE obj){
        return (struct RArray*)obj;
    }
    constexpr VALUE RARRAY_EMBED_FLAG = FL_USER<1>::value;
    constexpr VALUE RARRAY_EMBED_LEN_MASK = FL_USER<3>::value | FL_USER<4>::value;
    constexpr VALUE RARRAY_EMBED_LEN_SHIFT = FL_USHIFT+3;
    static inline VALUE *RARRAY_PTR(VALUE ary) {
        return (RBASIC(ary)->flags & RARRAY_EMBED_FLAG) ? RARRAY(ary)->as.ary : RARRAY(ary)->as.heap.ptr;
    }
    static inline long RARRAY_LEN(VALUE ary) {
       return (RBASIC(ary)->flags & RARRAY_EMBED_FLAG) ? \
            (long)((RBASIC(ary)->flags >> RARRAY_EMBED_LEN_SHIFT) & (RARRAY_EMBED_LEN_MASK >> RARRAY_EMBED_LEN_SHIFT)) : \
            RARRAY(ary)->as.heap.len;
    }

    //RData:
    struct RData {
        struct RBasic basic;
        void (*dmark)(void *);
        void (*dfree)(void *);
        void *data;
    };
    static inline struct RData *RDATA(VALUE obj) {
        return (struct RData *)obj;
    }
    static inline void *RDATA_PTR(VALUE obj) {
        return RDATA(obj)->data;
    }
    
    //Fixnum
    constexpr VALUE RUBY_FIXNUM_FLAG = 0x01;
    static inline long FIX2LONG(VALUE x) {
        return (long)(((long)x)>>1);
    }
    static inline unsigned long FIX2ULONG(VALUE x) {
        return ((x>>1) & LONG_MAX);
    }
    static inline int FIX2INT(VALUE x) {
        return (int)FIX2LONG(x);
    }
    static inline unsigned int FIX2UINT(VALUE x) {
        return (unsigned int)FIX2ULONG(x);
    }
    static inline VALUE INT2FIX(int x) {
        return ((VALUE)(((long)(x)) << 1 | RUBY_FIXNUM_FLAG));
    }
    constexpr VALUE RUBY_SYMBOL_FLAG = 0x0e;
    static inline VALUE ID2SYM(ID id) {
        return (((VALUE)(id) << 8) | RUBY_SYMBOL_FLAG);
    }
    constexpr unsigned long FIXNUM_MAX = (unsigned long)(LONG_MAX >> 1);
    constexpr long FIXNUM_MIN = ((long)LONG_MIN >> (int)1);
    static inline bool FIXNUM_P(VALUE x) {
        return (((long)x) & RUBY_FIXNUM_FLAG) != 0;
    }
    static inline bool POSFIXABLE(unsigned long x) {
        return x <= FIXNUM_MAX;
    }
    static inline bool NEGFIXABLE(long x) {
        return x >= FIXNUM_MIN;
    }
    static inline bool FIXABLE(long x) {
        return (NEGFIXABLE(x) && (x <= 0 || POSFIXABLE(x)));
    }

    //type 
    constexpr VALUE RUBY_IMM_MASK = 0x03;  //immediate value mask
    static inline bool IMM_P(VALUE x) {
        return (x&RUBY_IMM_MASK) != 0;
    }
    constexpr VALUE RUBY_SPECIAL_SHIFT = 8;
    static inline bool SYMBOL_P(VALUE x) {
        return ((x&~(~(VALUE)0 << RUBY_SPECIAL_SHIFT)) == RUBY_SYMBOL_FLAG);
    }
    static inline VALUE BUILTIN_TYPE(VALUE x) {
        return RBASIC(x)->flags & RUBY_CLASS_MASK;
    }
    static inline VALUE rb_type(VALUE obj) {
        if (IMM_P(obj)) {
            if(FIXNUM_P(obj))return RUBY_CLASS_FIXNUM;
            if(obj == Qtrue)return RUBY_CLASS_TRUE;
            if(SYMBOL_P(obj))return RUBY_CLASS_SYMBOL;
            if(obj == Qundef)return RUBY_CLASS_UNDEF;
        }else if (!RTEST(obj)) {
            if(obj == Qnil)return RUBY_CLASS_NIL;
            if(obj == Qfalse)return RUBY_CLASS_FALSE;
        }
        return BUILTIN_TYPE(obj);
    }

    //Functions
    constexpr int addr_rb_funcall2 = 199424;
    constexpr int addr_rb_define_class = 386480;
    constexpr int addr_rb_const_defined = 425680;
    constexpr int addr_rb_const_get = 428048;
    constexpr int addr_rb_intern = 295376;
    constexpr int addr_rb_intern2 = 345376;
    constexpr int addr_rb_intern3 = 344080;
    constexpr int addr_rb_fiber_start = 532332;
    constexpr int addr_rb_define_module = 387072;
    constexpr int addr_rb_define_module_function = 389200;
    constexpr int addr_rb_define_global_const = 426224;
    constexpr int addr_rb_define_global_function = 426224;
    constexpr int addr_rb_eval_string_protect = 197152;
    constexpr int addr_rb_id2name = 653040;
    constexpr int addr_rb_id2str = 295536;
    constexpr int addr_rb_scan_args = 389616;
    constexpr int addr_rb_class_new_instance = 212016;
    constexpr int addr_rb_define_method = 388576;
    constexpr int addr_rb_define_singleton_method = 153456;
    constexpr int addr_rb_str_new = 0x36290;
    constexpr int addr_rb_define_const = 0x68070;
    constexpr int addr_rb_str_new2 = 0x36340;
    constexpr int addr_rb_string_value = 0x37A40;
    constexpr int addr_rb_string_value_ptr = 0x37A70;
    constexpr int addr_rb_obj_is_kind_of = 0x332D0;
    constexpr int addr_rb_raise = 0x26210;
    constexpr int addr_rb_obj_classname = 0x68420;
    constexpr int addr_rb_ary_push = 0x8D2A0;
    constexpr int addr_rb_ary_aref = 0x89C40;
    constexpr int addr_rb_ary_aset = 0x8D870;
    constexpr int addr_rb_ary_new = 0x88DC0;

    typedef VALUE(RUBYCALL* RubyFunc)(...);
    typedef void(RUBYCALL* RubyDataFunc)(void*);

    typedef VALUE(*pfn_rb_const_get)(VALUE, ID);
    typedef void(*pfn_rb_define_const)(VALUE, const char*, VALUE);
    typedef void(*pfn_rb_define_global_const)(const char*, VALUE);

    typedef VALUE(*pfn_rb_iv_get)(VALUE, const char*);
    typedef VALUE(*pfn_rb_iv_set)(VALUE, const char*, VALUE);

    typedef void*(*pfn_ruby_xmalloc)(size_t);
    typedef void(*pfn_ruby_xfree)(void*);

    typedef void(*pfn_rb_gc_mark)(VALUE);

    typedef void(*pfn_rb_define_method)(VALUE classmod, char *name, VALUE(*)(), int argc);
    typedef VALUE(*pfn_rb_define_class)(const char*, VALUE);
    typedef VALUE(*pfn_rb_define_module)(const char*);
    typedef VALUE(*pfn_rb_define_class_under)(VALUE, const char*, VALUE);
    typedef VALUE(*pfn_rb_define_module_under)(VALUE, const char*);

    typedef	void(*pfn_rb_define_class_method)(VALUE, const char*, RubyFunc, int);
    typedef	void(*pfn_rb_define_module_function)(VALUE, const char*, RubyFunc, int);
    typedef	void(*pfn_rb_define_global_function)(const char*, RubyFunc, int);

    typedef void(*pfn_rb_define_alloc_func)(VALUE, VALUE(*rb_alloc_func_t)(VALUE));
    typedef void(*pfn_rb_undef_alloc_func)(VALUE);
    typedef VALUE(*pfn_rb_data_object_alloc)(VALUE, void*, RubyDataFunc, RubyDataFunc);
    typedef void(*pfn_rb_undef_method)(VALUE, const char*);
    typedef void(*pfn_rb_define_alias)(VALUE, const char*, const char*);

    typedef ID(*pfn_rb_intern)(const char*);
    typedef const char*(*pfn_rb_id2name)(ID);

    typedef VALUE(*pfn_rb_funcall)(VALUE, ID, int, ...);
    typedef VALUE(*pfn_rb_funcall2)(VALUE, ID, int, const VALUE*);
    typedef VALUE(*pfn_rb_funcall3)(VALUE, ID, int, const VALUE*);
    typedef int(*pfn_rb_scan_args)(int, const VALUE*, const char*, ...);
    typedef VALUE(*pfn_rb_call_super)(int, const VALUE*);
    typedef int(*pfn_rb_respond_to)(VALUE, ID);

    typedef VALUE(*pfn_rb_eval_string)(const char*);
    typedef VALUE(*pfn_rb_eval_string_protect)(const char*, int*);

    typedef VALUE(*pfn_rb_protect)(VALUE(*)(VALUE), VALUE, int*);
    typedef void(*pfn_rb_raise)(VALUE, const char*, ...);
    typedef VALUE(*pfn_rb_errinfo)(void);

    typedef VALUE(*pfn_rb_obj_class)(VALUE);
    typedef VALUE(*pfn_rb_singleton_class)(VALUE);
    typedef VALUE(*pfn_rb_obj_is_instance_of)(VALUE, VALUE);
    typedef VALUE(*pfn_rb_obj_is_kind_of)(VALUE, VALUE);
    typedef const char*(*pfn_rb_class2name)(VALUE);
    typedef const char*(*pfn_rb_obj_classname)(VALUE);
    typedef int(*pfn_rb_type)(VALUE obj);
    typedef void(*pfn_rb_check_type)(VALUE, int);
    typedef VALUE(*pfn_rb_convert_type)(VALUE, int, const char *, const char *);

    typedef long(*pfn_rb_num2long)(VALUE);
    typedef unsigned long(*pfn_rb_num2ulong)(VALUE);
    typedef double(*pfn_rb_num2dbl)(VALUE);

    typedef VALUE(*pfn_rb_int2num)(long);
    typedef VALUE(*pfn_rb_uint2num)(unsigned long);

    typedef VALUE(*pfn_rb_str_new)(const char*, long);
    typedef VALUE(*pfn_rb_str_new2)(const char*);
    typedef VALUE(*pfn_rb_str_new3)(VALUE);
    typedef void(*pfn_rb_str_modify)(VALUE);
    typedef VALUE(*pfn_rb_str_cat)(VALUE, const char*, long);
    typedef VALUE(*pfn_rb_str_buf_new)(long);
    typedef VALUE(*pfn_rb_str_buf_append)(VALUE, VALUE);
    typedef VALUE(*pfn_rb_inspect)(VALUE);
    typedef VALUE(*pfn_rb_obj_as_string)(VALUE);

    typedef VALUE(*pfn_rb_ary_new)(void);
    typedef VALUE(*pfn_rb_ary_new2)(long);
    typedef VALUE(*pfn_rb_ary_new4)(long, const VALUE*);
    typedef void(*pfn_rb_ary_store)(VALUE, long, VALUE);
    typedef VALUE(*pfn_rb_ary_push)(VALUE, VALUE);
    typedef VALUE(*pfn_rb_ary_pop)(VALUE);
    typedef VALUE(*pfn_rb_ary_shift)(VALUE);
    typedef VALUE(*pfn_rb_ary_unshift)(VALUE, VALUE);
    typedef VALUE(*pfn_rb_ary_entry)(VALUE, long);
    typedef VALUE(*pfn_rb_ary_clear)(VALUE);

    typedef VALUE(*pfn_rb_float_new)(double);

    typedef int(*pfn_rb_block_given_p)(void);
    typedef VALUE(*pfn_rb_block_proc)(void);

    typedef VALUE(*pfn_rb_string_value)(volatile VALUE*);
    typedef char*(*pfn_rb_string_value_ptr)(volatile VALUE*);
    typedef char*(*pfn_rb_string_value_cstr)(volatile VALUE*);

    typedef char*(*pfn_rb_string_ptr)(VALUE str);
    typedef long(*pfn_rb_string_len)(VALUE str);
    typedef VALUE*(*pfn_rb_array_ptr)(VALUE ary);
    typedef long(*pfn_rb_array_len)(VALUE ary);
    typedef void*(*pfn_rb_userdata_ptr)(VALUE d);

    typedef VALUE(*pfn_rb_ary_aref)(int argc, VALUE *argv, VALUE ary);
    typedef VALUE(*pfn_rb_ary_aset)(int argc, VALUE *argv, VALUE ary);

    typedef	int(*pfn_rb_const_defined)(VALUE klass, ID id);
    typedef VALUE(*pfn_rb_class_new_instance)(int argc, VALUE *argv, VALUE klass);
    //export here;
    extern pfn_rb_funcall2 rb_funcall2;
    extern pfn_rb_define_class rb_define_class;
    extern pfn_rb_const_defined rb_const_defined;
    extern pfn_rb_const_get rb_const_get;
    extern pfn_rb_intern rb_intern;
    extern pfn_rb_define_module rb_define_module;
    extern pfn_rb_define_module_function rb_define_module_function;
    extern pfn_rb_define_global_const rb_define_global_const;
    extern pfn_rb_define_global_function rb_define_global_function;
    extern pfn_rb_eval_string_protect rb_eval_string_protect;;
    extern pfn_rb_id2name rb_id2name;
    extern pfn_rb_scan_args rb_scan_args;
    extern pfn_rb_class_new_instance rb_class_new_instance;
    extern pfn_rb_define_method rb_define_method;
    extern pfn_rb_str_new  rb_str_new;
    extern pfn_rb_str_new2  rb_str_new2;
    extern pfn_rb_define_const rb_define_const;
    extern pfn_rb_string_value rb_string_value;
    extern pfn_rb_string_value_ptr rb_string_value_ptr;
    extern pfn_rb_obj_is_kind_of rb_obj_is_kind_of;
    extern pfn_rb_raise rb_raise;
    extern pfn_rb_obj_classname rb_obj_classname;
    extern pfn_rb_ary_aref rb_ary_aref;
    extern pfn_rb_ary_aset rb_ary_aset;
    extern pfn_rb_ary_new rb_ary_new;
    extern pfn_rb_ary_push rb_ary_push;

    void InitRuntime(HMODULE hRGSSCore);
}// end of namespace 