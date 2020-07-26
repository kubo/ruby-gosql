package main

// #cgo linux LDFLAGS: -Wl,-unresolved-symbols=ignore-all
// #cgo pkg-config: ruby
// #include <ruby.h>
// #include "gosql.h"
import "C"
import (
	"sync/atomic"
	"database/sql"
)

// * `obj_map` holds Go objects referenced by ruby objects in order to
//   prevent them from being GC'ed.
// * A ruby object hold a key of the map. The key is removed from
//   the map when the ruby object is freed by ruby's GC. After that
//   the value is freed by Go's GC.
// * Ruby runs only one native thread at a time. So no need to use
//   mutex.
var obj_map = make(map[uint32]interface{})
var last_obj_id uint32

func obj_map_add(obj interface{}) C.VALUE {
	var id uint32
	id_is_used := true
	for id_is_used {
		id = atomic.AddUint32(&last_obj_id, 1)
		_, id_is_used = obj_map[id]
	}
	obj_map[id] = obj
	return C.rbgosql_uint2num(C.uint(id))
}

func obj_map_get_db(obj *C.gosql_t) *sql.DB {
	// TODO: check errors
	return obj_map[uint32(obj.id)].(*sql.DB)

}

func obj_map_get_result(obj *C.gosql_t) sql.Result {
	// TODO: check errors
	return obj_map[uint32(obj.id)].(sql.Result)
}

func make_error(err error) C.VALUE {
	return C.rbgosql_make_error(err.Error())
}

func obj_to_string(obj C.VALUE, strict bool) (string, C.VALUE) {
	var c_strict C.int
	if strict {
		c_strict = 1
	} else {
		c_strict = 0
	}
	s := C.rbgosql_obj_to_string(obj, c_strict)
	if s.len < 0 {
		return "", s.obj
	}
	return C.GoStringN(s.ptr, s.len), C.Qnil
}

//export Init_gosql
func Init_gosql() {
	C.rbgosql_init()
}

//export rbgosql_drivers
func rbgosql_drivers(module C.VALUE) C.VALUE {
	drivers := sql.Drivers()
	ary := C.rb_ary_new_capa(C.long(len(drivers)))
	for _, driver := range drivers {
		C.rb_ary_push(ary, C.rbgosql_string_to_obj(driver));
	}
	return ary
}

//export rbgosql_open
func rbgosql_open(driver_name C.VALUE, data_source_name C.VALUE) C.VALUE {
	driver, errobj := obj_to_string(driver_name, true)
	if errobj != C.Qnil {
		return errobj;
	}
	data_source, errobj := obj_to_string(data_source_name, true)
	if errobj != C.Qnil {
		return errobj;
	}
	db, err := sql.Open(driver, data_source)
	if err != nil {
		return make_error(err)
	}
	return obj_map_add(db)
}

//export rbgosql_db_exec
func rbgosql_db_exec(self *C.gosql_t, sql_stmt C.VALUE, args C.VALUE) C.VALUE {
	sql, errobj := obj_to_string(sql_stmt, true)
	if errobj != C.Qnil {
		return errobj
	}
	argc := C.rbgosql_rarray_len(args);
	params := make([]interface{}, 0, argc)
	for i := C.int(0); i < argc; i++ {
		s, err := obj_to_string(C.rbgosql_rarray_aref(args, i), false)
		if err != C.Qnil {
			return err
		}
		params = append(params, s)
	}
	result, err := obj_map_get_db(self).Exec(sql, params...);
	if err != nil {
		return make_error(err)
	}
	return obj_map_add(result)
}

//export rbgosql_result_last_insert_id
func rbgosql_result_last_insert_id(self *C.gosql_t) C.VALUE {
	last_insert_id, err := obj_map_get_result(self).LastInsertId()
	if err != nil {
		return make_error(err)
	}
	return C.rb_ll2inum(C.longlong(last_insert_id))
}

//export rbgosql_result_rows_affected
func rbgosql_result_rows_affected(self *C.gosql_t) C.VALUE {
	rows_affected, err := obj_map_get_result(self).RowsAffected()
	if err != nil {
		return make_error(err)
	}
	return C.rb_ll2inum(C.longlong(rows_affected))
}

//export rbgosql_release_obj
func rbgosql_release_obj(obj *C.gosql_t) {
	delete(obj_map, uint32(obj.id))
}

func main() {}
