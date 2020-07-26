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
