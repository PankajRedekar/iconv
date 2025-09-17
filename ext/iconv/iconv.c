#include "ruby.h"
#include <iconv.h>
#include <errno.h>
#include <string.h>

typedef struct {
    iconv_t cd;
} iconv_data;

static void iconv_dfree(void *p) {
    iconv_data *d = (iconv_data *)p;
    if (d->cd != (iconv_t)-1) {
        iconv_close(d->cd);
    }
    xfree(d);
}

static const rb_data_type_t iconv_data_type = {
    "Iconv",
    {0, iconv_dfree, 0,},
    0, 0, RUBY_TYPED_FREE_IMMEDIATELY,
};

static VALUE iconv_alloc(VALUE klass) {
    iconv_data *d;
    return TypedData_Make_Struct(klass, iconv_data, &iconv_data_type, d);
}

static VALUE iconv_initialize(VALUE self, VALUE to, VALUE from) {
    iconv_data *d;
    TypedData_Get_Struct(self, iconv_data, &iconv_data_type, d);

    d->cd = iconv_open(StringValueCStr(to), StringValueCStr(from));
    if (d->cd == (iconv_t)-1) {
        rb_sys_fail("iconv_open");
    }
    return self;
}

static VALUE iconv_convert(VALUE self, VALUE str) {
    iconv_data *d;
    TypedData_Get_Struct(self, iconv_data, &iconv_data_type, d);

    char *inbuf = RSTRING_PTR(str);
    size_t inbytesleft = RSTRING_LEN(str);

    size_t outlen = inbytesleft * 4 + 16;
    VALUE out = rb_str_new(NULL, outlen);
    char *outbuf = RSTRING_PTR(out);
    size_t outbytesleft = outlen;

    size_t res = iconv(d->cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    if (res == (size_t)-1) {
        rb_sys_fail("iconv");
    }

    rb_str_set_len(out, outlen - outbytesleft);
    return out;
}

void Init_iconv(void) {
    VALUE cIconv = rb_define_class("Iconv", rb_cObject);
    rb_define_alloc_func(cIconv, iconv_alloc);
    rb_define_method(cIconv, "initialize", iconv_initialize, 2);
    rb_define_method(cIconv, "convert", iconv_convert, 1);
}
