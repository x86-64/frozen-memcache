
static err_item errs_list[] = {
 { -527, "src/main_memcache.c: memcache_delete error" },
 { -509, "src/main_memcache.c: memcache_set error" },
 { -424, "src/main_memcache.c: no action specified" },
 { -397, "src/main_memcache.c: memcache_value_unserialize unknown output datatype: pass output key or set value_type in configuration" },
 { -391, "src/main_memcache.c: memcache_value_unserialize wrong output data supplied" },
 { -355, "src/main_memcache.c: memcache configuration error: wrong management modes configuration" },
 { -349, "src/main_memcache.c: invalid config specified" },
 { -307, "src/main_memcache.c: calloc failed" },

};
#define            errs_list_size      sizeof(errs_list[0])
#define            errs_list_nelements sizeof(errs_list) / errs_list_size
