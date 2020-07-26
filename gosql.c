/*
 * gosql.c
 * URL: https://github.com/kubo/ruby-gosql
 * ------------------------------------------------------
 * Copyright 2020 Kubo Takehiro <kubo@jiubao.org>. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice, this list of
 *       conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright notice, this list
 *       of conditions and the following disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 * or implied, of the authors.
 *
 */
#include <ruby.h>
#include <ruby/encoding.h>
#include "libgosql.h"

#define SafeUtf8StringValue(v) do { \
    SafeStringValue(v); \
    v = rb_str_export_to_enc(v, rb_utf8_encoding()); \
} while (0)

static VALUE cDB;
static VALUE cResult;
static VALUE cError;

static rb_data_type_t db_data_type = {
    "GoSql.DB",
    {
        NULL, (void (*)(void*))rbgosql_release_obj,
    },
};

static rb_data_type_t result_data_type = {
    "GoSql.Result",
    {
        NULL, (void (*)(void*))rbgosql_release_obj,
    },
};

static VALUE make_obj(VALUE obj, VALUE klass, const rb_data_type_t *data_type)
{
    gosql_t *gosql;
    size_t id = NUM2SIZET(obj);

    obj = TypedData_Make_Struct(klass, gosql_t, data_type, gosql);
    gosql->id = id;
    return obj;
}

static void check_err(VALUE obj)
{
    if (rb_obj_is_kind_of(obj, rb_eStandardError)) {
        rb_exc_raise(obj);
    }
}

static VALUE rbgosql_open_c(VALUE module, VALUE driver_name, VALUE data_soruce_name)
{
    VALUE obj;

    obj = rbgosql_open(driver_name, data_soruce_name);
    check_err(obj);
    return make_obj(obj, cDB, &db_data_type);
}

static VALUE rbgosql_db_exec_c(int argc, VALUE *argv, VALUE self)
{
    gosql_t *gosql;
    VALUE sql, args;
    VALUE obj;

    TypedData_Get_Struct(self, gosql_t, &db_data_type, gosql);
    rb_scan_args(argc, argv, "1*", &sql, &args);
    obj = rbgosql_db_exec(gosql, sql, args);
    check_err(obj);
    return make_obj(obj, cResult, &result_data_type);
}

static VALUE rbgosql_result_last_insert_id_c(VALUE self)
{
    gosql_t *gosql;
    VALUE obj;

    TypedData_Get_Struct(self, gosql_t, &result_data_type, gosql);
    obj = rbgosql_result_last_insert_id(gosql);
    check_err(obj);
    return obj;
}

static VALUE rbgosql_result_rows_affected_c(VALUE self)
{
    gosql_t *gosql;
    VALUE obj;

    TypedData_Get_Struct(self, gosql_t, &result_data_type, gosql);
    obj = rbgosql_result_rows_affected(gosql);
    check_err(obj);
    return obj;
}

void rbgosql_init(void)
{
    VALUE mGoSql = rb_define_module("GoSql");

    rb_define_module_function(mGoSql, "drivers", rbgosql_drivers, 0);
    rb_define_module_function(mGoSql, "open", rbgosql_open_c, 2);

    cDB = rb_define_class_under(mGoSql, "DB", rb_cObject);
    rb_define_method(cDB, "exec", rbgosql_db_exec_c, -1);

    cResult = rb_define_class_under(mGoSql, "Result", rb_cObject);
    rb_define_method(cResult, "last_insert_id", rbgosql_result_last_insert_id_c, 0);
    rb_define_method(cResult, "rows_affected", rbgosql_result_rows_affected_c, 0);

    cError = rb_define_class_under(mGoSql, "Error", rb_eStandardError);
}

int rbgosql_rarray_len(VALUE ary)
{
    return RARRAY_LEN(ary);
}

VALUE rbgosql_rarray_aref(VALUE ary, int idx)
{
    return RARRAY_AREF(ary, idx);
}

VALUE rbgosql_make_error(GoString msg)
{
    return rb_exc_new(cError, msg.p, msg.n);
}

VALUE rbgosql_string_to_obj(GoString s)
{
    return rb_utf8_str_new(s.p, s.n);
}

static VALUE obj_to_s_utf8(VALUE s)
{
    s = rb_obj_as_string(s);
    s = rb_str_export_to_enc(s, rb_utf8_encoding());
    return s;
}

static VALUE obj_to_str_utf8(VALUE s)
{
    s = rb_str_to_str(s);
    s = rb_str_export_to_enc(s, rb_utf8_encoding());
    return s;
}

rbgosql_str_result_t rbgosql_obj_to_string(VALUE s, int strict)
{
    rbgosql_str_result_t rv;
    int state;

    rv.obj = rb_protect(strict ? obj_to_str_utf8 : obj_to_s_utf8, s, &state);
    if (state) {
        rv.ptr = NULL;
        rv.len = -1;
        rv.obj = rb_errinfo();
        rb_set_errinfo(Qnil);
    } else {
        rv.ptr = RSTRING_PTR(rv.obj);
        rv.len = RSTRING_LEN(rv.obj);
    }
    return rv;
}

VALUE rbgosql_uint2num(unsigned int i)
{
    return RB_UINT2NUM(i);
}
