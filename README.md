# ruby-gosql - Ruby library to use Go's [`database/sql`][]

Ruby-gosql is at extremely early stage.

It can:
* connect to any database supported by Go's [SQL database drivers][]
* execute a SQL statement without returning any rows
* pass non-null placeholder parameters as string

It cannot:
* execute a SQL statement that returns rows
* begin, commit or rollback a transaction
* pass null placeholder parameters
* pass placeholder parameters as any type except string

## Future Plan

No plans now. I started this project just to learn Go and how to wrap a
Go package into a C shared library. I'm halfway through learning Go, but
reached the goal of wrapping a Go package.

Feel free to take over this project if you are interested.

## How to compile

Check out the source code, install SQL drivers at least one, run
`ruby extconf.rb`, make `gosql.so` and install it.

```shell
$ git checkout https://github.com/kubo/ruby-gosql.git
$ cd ruby-gosql
$ go get github.com/mattn/go-sqlite3  # for example
$ go get github.com/lib/pq            # for example
$ ruby extconf.rb
$ make && make install
```

Note: The installed library may not work when a SQL driver depends
on a external C shared library. You may need to edit `Makefile`
created by `extconf.rb` by yourself to add dependency to the
external C shared library.

I have tested this on Linux. It won't work on other platforms.

## Example

```ruby
require "gosql"

GoSql.drivers # => ["sqlite3", "postgres"] when go-sqlite3 and pq are installed.

db = GoSql.open("sqlite3", "testdb.sqlite3")
db.exec("drop table if exists test_table")
db.exec("create table test_table (key integer, value text)")
db.exec("insert into test_table values (1, '1st')")
db.exec("insert into test_table values (?, ?)", 2, "2nd") # all parameters as set as strings.
result = db.exec("delete from test_table")
result.rows_affected # => 2
```

## Implemented methods

* `GoSql.drivers # => array of String` - [`func Drivers`](https://golang.org/pkg/database/sql/#Drivers)
* `GoSql.open(driver_name, data_source_name) # => GoSql::DB` - [`func Open`](https://golang.org/pkg/database/sql/#Open)
* `GoSql::DB#exec(sql, *placeholder_parameters) # => GoSql::Result` - [`func (*DB)Exec`](https://golang.org/pkg/database/sql/#DB.Exec)
* `GoSql::Result#last_insert_id # => Integer` - [`LastInsertId()` in the Result interface](https://golang.org/pkg/database/sql/#Result)
* `GoSql::Result#rows_affected # => Integer` - [`RowsAffected()` in the Result interface](https://golang.org/pkg/database/sql/#Result)

## Implementation Note

Disclaimer: The followings may be incorrect. I'm a newbie about Go.

### How `gosql.go` and `gosql.c` are combined into a C shared library

When C functions are called from Go and Go functions are called from C,

* Put C function prototypes of C functions used from Go into `gosql.h`.
  The header file is included by both `gosql.go` and `gosql.c`.
* Implement `gosql.go` and build it as a C-archive file by `go build -buildmode=c-archive`.
  * The key point is `#cgo LDFLAGS: -Wl,-unresolved-symbols=ignore-all`. (See [go#25832 comment][])
  * `libgosql.h` is created along with `libgosql.a`.
  * The header file includes C function prototypes of functions exported from Go.
* Implement `gosql.c`, which includes `libgosql.h` to use Go functions.
* Link `libgosql.a` and `gosql.c` into `gosql.so`.

The above steps are complicated in order to use C prototypes of Go functions.

When your code doesn't call Go functions from C or use manually created C
prototypes of Go functions, it is simple. Just include `gosql.c` in a
Go file and use `go build -buildmode=c-shared`. Note that the Go file
including `gosql.c` must not have `//export`.

### How Go objects are protected from Go's GC

* This module stores a map from integers to Go objects as a global
  variable in Go.
* The map holds objects referenced by ruby objects in order to
  prevent them from being GC'ed.
* A ruby object hold a key of the map. The key is removed from
  the map when the ruby object is freed by ruby's GC. After that
  the value is freed by Go's GC.


### License

BSD 2-clause License

[`database/sql`]: https://golang.org/pkg/database/sql/
[SQL database drivers]: https://github.com/golang/go/wiki/SQLDrivers
[go#25832 comment]: https://github.com/golang/go/issues/25832#issuecomment-571631784
