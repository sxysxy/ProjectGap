#pragma once

#include "stdafx.h"

#define RUBYCALL __cdcel

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
    constexpr VALUE RUBY_CLASS_ARRAY = 0x07;
    constexpr VALUE RUBY_CLASS_HASH = 0x08;
    constexpr VALUE RUBY_CLASS_BIGNUM = 0x0a;
    constexpr VALUE RUBY_CLASS_FILE = 0x0b;
    constexpr VALUE RUBY_CLASS_SYMBOL = 0x14;
    constexpr VALUE RUBY_CLASS_FIXNUM = 0x15;
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
}// end of namespace 