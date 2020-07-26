/*
 * gosql.h
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
#ifndef GOSQL_H
#define GOSQL_H 1

typedef struct {
    unsigned int id;
} gosql_t;

void rbgosql_init(void);

typedef struct {
    char *ptr;
    int len;
    VALUE obj;
} rbgosql_str_result_t;

int rbgosql_rarray_len(VALUE ary);
VALUE rbgosql_rarray_aref(VALUE ary, int idx);
VALUE rbgosql_make_error(_GoString_ msg);

// Convert Go string to ruby object
VALUE rbgosql_string_to_obj(_GoString_ s);
// Convert ruby object to Go string
rbgosql_str_result_t rbgosql_obj_to_string(VALUE s, int strict);

VALUE rbgosql_uint2num(unsigned int i);

#endif
