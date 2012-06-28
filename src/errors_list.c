
static err_item errs_list[] = {
 { -326, "src/main_memcache.c: memcache_delete error" },
 { -308, "src/main_memcache.c: memcache_set error" },
 { -223, "src/main_memcache.c: no action specified" },
 { -196, "src/main_memcache.c: memcache_value_unserialize unknown output datatype: pass output key or set value_type in configuration" },
 { -190, "src/main_memcache.c: memcache_value_unserialize wrong output data supplied" },
 { -154, "src/main_memcache.c: memcache configuration error: wrong management modes configuration" },
 { -148, "src/main_memcache.c: invalid config specified" },
 { -106, "src/main_memcache.c: calloc failed" },

	{ 0, NULL }
};
#define            errs_list_size      sizeof(errs_list[0])
#define            errs_list_nelements sizeof(errs_list) / errs_list_size
