modules_LTLIBRARIES = module_c_memcache.la
module_c_memcache_la_CFLAGS = -I@includedir@/frozen/ -DFROZEN_MODULE
module_c_memcache_la_LDFLAGS = -module -avoid-version -shared -lmemcached -lfrozen
module_c_memcache_la_SOURCES = main_memcache.c
EXTRA_DIST = errors_list.c
