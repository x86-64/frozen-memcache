This is glue code for memcache support in frozen.

Installation
-----------------
1. Compile and install frozen
2. Compile and install libmemcached
3. Download or clone this repo
4. ./configure --prefix=/usr
5. make
6. make install


Usage
-----------------
	{
	     class                   = "modules/memcache",
	     config                  = "--SERVER=1.2.3.4:1234",// configuration string for libmemcached
	     key                     = (hashkey_t)"input",     // hashkey for key, default "key"
	     value                   = (hashkey_t)"output",    // hashkey for value, default "value"
	      
	     // Value type management modes, pick any
	     // Mode 1: all values saved to db in specified format (DEFAULT)
	     value_same              = (uint_t)"1",            // default 1
	     value_type              = (datatype_t)"ipv4_t",   // restrict type to this one, default no
	     value_format            = (format_t)"packed",     // storage format, default "native"
	     
	     // Mode 2: value can have any type, type would be saved along with data itself
	     //         value data would be stored in "packed" format
	     value_any               = (uint_t)"1",            // default 0
	     value_format            = (format_t)"packed"      // storage format, default "native"
 	},
	{ class = "stdout", input = (hashkey_t)"value" }


Accept only read, write and delete requests. Any successful request passed to next machine.

