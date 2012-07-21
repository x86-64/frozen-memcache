
static err_item errs_list[] = {
 { -327, "src/main_memcache.c: memcache_delete error" },
 { -309, "src/main_memcache.c: memcache_set error" },
 { -224, "src/main_memcache.c: no action specified" },
 { -197, "src/main_memcache.c: memcache_value_unserialize unknown output datatype: pass output key or set value_type in configuration" },
 { -191, "src/main_memcache.c: memcache_value_unserialize wrong output data supplied" },
 { -155, "src/main_memcache.c: memcache configuration error: wrong management modes configuration" },
 { -149, "src/main_memcache.c: invalid config specified" },
 { -107, "src/main_memcache.c: calloc failed" },

	{ 0, NULL }
};
#define            errs_list_size      sizeof(errs_list[0])
#define            errs_list_nelements sizeof(errs_list) / errs_list_size
