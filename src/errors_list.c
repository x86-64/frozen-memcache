
static err_item errs_list[] = {
 { -336, "src/main_memcache.c: memcache_enum error" },
 { -324, "src/main_memcache.c: memcache_delete error" },
 { -312, "src/main_memcache.c: memcache_set error" },
 { -284, "src/main_memcache.c: memcache_get error" },
 { -268, "src/main_memcache.c: no action specified" },
 { -186, "src/main_memcache.c: memcache_value_unserialize unknown output datatype: pass output key or set value_type in configuration" },
 { -180, "src/main_memcache.c: memcache_value_unserialize wrong output data supplied" },
 { -143, "src/main_memcache.c: memcache init failed" },
 { -131, "src/main_memcache.c: memcache configuration error: wrong management modes configuration" },
 { -125, "src/main_memcache.c: invalid path specified" },
 { -77, "src/main_memcache.c: calloc failed" },

	{ 0, NULL }
};
#define            errs_list_size      sizeof(errs_list[0])
#define            errs_list_nelements sizeof(errs_list) / errs_list_size
