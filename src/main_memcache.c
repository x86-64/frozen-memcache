#include <libfrozen.h>
#include <libmemcached/memcached.h>

#include <errors_list.c>

/**
 * @ingroup machine
 * @addtogroup mod_machine_memcache module/memcache
 */
/**
 * @ingroup mod_machine_memcache
 * @page page_memcache_info Description
 *
 */
/**
 * @ingroup mod_machine_memcache
 * @page page_memcache_config Configuration
 *  
 * Accepted configuration:
 * @code
 * {
 *              class                   = "modules/memcache",
 *              config                  = "--SERVER=1.2.3.4:1234",// configuration string for libmemcached
 *              key                     = (hashkey_t)"input",     // hashkey for key, default "key"
 *              value                   = (hashkey_t)"output",    // hashkey for value, default "value"
 *              
 *              // Value type management modes, pick any
 *
 *              // Mode 1: all values saved to db in specified format (DEFAULT)
 *              value_same              = (uint_t)"1",            // default 1
 *              value_type              = (datatype_t)"ipv4_t",   // restrict type to this one, default no
 *              value_format            = (format_t)"packed",     // storage format, default "native"
 *              
 *              // Mode 2: value can have any type, type would be saved along with data itself
 *              //         value data would be stored in "packed" format
 *              value_any               = (uint_t)"1",            // default 0
 *              value_format            = (format_t)"packed"      // storage format, default "native"
 * }
 * @endcode
 */

typedef enum memcache_value_mode {
	VALUE_MODE_SAME_TYPE,
	VALUE_MODE_ANY_TYPE,
} memcache_value_mode;

typedef struct memcache_userdata {
	char                  *config;
	hashkey_t              key;
	hashkey_t              value;
	memcache_value_mode    value_mode;
	uintmax_t              value_type_restrict;
	datatype_t             value_type;
	format_t               value_format;
	
	thread_data_ctx_t      thread_data;
} memcache_userdata;

typedef struct memcache_threaddata {
	memcached_st          *mc;
} memcache_threaddata;

typedef struct memcache_get_ctx {
	machine_t             *machine;
	request_t             *request;
	ssize_t                ret;
	hashkey_t              hashkey;
} memcache_get_ctx;

typedef struct slice {
	char                  *data;
	uintmax_t              size;
} slice;


static void *  memcache_threaddata_create(memcache_userdata *userdata){ // {{{
	memcache_threaddata      *threaddata;
	
	if( (threaddata = malloc(sizeof(memcache_threaddata))) == NULL)
		return NULL;
	
	if( (threaddata->mc = memcached(userdata->config, strlen(userdata->config))) == NULL)
		goto error;
	
	return threaddata;

error:
	free(threaddata);
	return NULL;
} // }}}
static void    memcache_threaddata_destroy(memcache_threaddata *threaddata){ // {{{
	memcached_free(threaddata->mc);
	free(threaddata);
} // }}}

static ssize_t memcache_init(machine_t *machine){ // {{{
	memcache_userdata         *userdata;
	
	if((userdata = machine->userdata = calloc(1, sizeof(memcache_userdata))) == NULL)
		return error("calloc failed");
	
	userdata->key                 = HDK(key);
	userdata->value               = HDK(value);
	userdata->value_mode          = VALUE_MODE_SAME_TYPE;
	userdata->value_type_restrict = 0;
	userdata->value_type          = TYPE_VOIDT;
	userdata->value_format        = FORMAT(native);
	return thread_data_init(
		&userdata->thread_data, 
		(f_thread_create)&memcache_threaddata_create,
		(f_thread_destroy)&memcache_threaddata_destroy,
		userdata);
} // }}}
static ssize_t memcache_destroy(machine_t *machine){ // {{{
	memcache_userdata     *userdata          = (memcache_userdata *)machine->userdata;
	
	thread_data_destroy(&userdata->thread_data);
	free(userdata);
	return 0;
} // }}}
static ssize_t memcache_configure(machine_t *machine, config_t *config){ // {{{
	ssize_t                ret;
	uintmax_t              c_value_same      = 0;
	uintmax_t              c_value_any       = 0;
	memcache_userdata     *userdata          = (memcache_userdata *)machine->userdata;
	
	hash_data_get(ret, TYPE_HASHKEYT,  userdata->key,          config, HDK(key));
	hash_data_get(ret, TYPE_HASHKEYT,  userdata->value,        config, HDK(value));
	hash_data_get(ret, TYPE_FORMATT,   userdata->value_format, config, HDK(value_format));
	hash_data_get(ret, TYPE_UINTT,     c_value_same,           config, HDK(value_same));
	hash_data_get(ret, TYPE_UINTT,     c_value_any,            config, HDK(value_any));
	
	hash_data_get(ret, TYPE_DATATYPET, userdata->value_type,   config, HDK(value_type));
	if(ret == 0){
		userdata->value_type_restrict = 1;
	}
	
	hash_data_convert(ret, TYPE_STRINGT, userdata->config,     config, HDK(config));
	if(ret != 0 || userdata->config == NULL)
		return error("invalid config specified");
	
	c_value_same   = (c_value_same   > 0) ? 1 : 0;
	c_value_any    = (c_value_any    > 0) ? 1 : 0;
	
	if(c_value_same + c_value_any > 1)
		return error("memcache configuration error: wrong management modes configuration");
	
	userdata->value_mode = c_value_same   ? VALUE_MODE_SAME_TYPE : userdata->value_mode;
	userdata->value_mode = c_value_any    ? VALUE_MODE_ANY_TYPE  : userdata->value_mode;
	return 0;
} // }}}

static ssize_t memcache_value_serialize(memcache_userdata *userdata, data_t *input, slice *output, data_t *freeme){ // {{{
	switch(userdata->value_mode){
		case VALUE_MODE_SAME_TYPE:
			return data_make_flat(input,   userdata->value_format, freeme, (void **)&output->data, &output->size);
			
		case VALUE_MODE_ANY_TYPE:;
			data_t           d_data        = DATA_PTR_DATAT(input);
			
			return data_make_flat(&d_data, userdata->value_format, freeme, (void **)&output->data, &output->size);
	}
} // }}}
static ssize_t memcache_value_unserialize(memcache_userdata *userdata, slice *input, data_t *output){ // {{{
	ssize_t                ret;
	
	if(input == NULL)
		return 0;
	
	data_t                 d_input    = DATA_RAW(input->data, input->size);
	
	switch(userdata->value_mode){
		case VALUE_MODE_SAME_TYPE:
			// accept only void_t or same data type
			
			if(userdata->value_type_restrict != 0){
				// restrict type to user-supplied
				
				if(output->type == TYPE_VOIDT){                     // void_t can be overridden
					output->type = userdata->value_type;
				}else if(output->type != userdata->value_type){     // if types not match - emit error
					return error("memcache_value_unserialize wrong output data supplied");
				}
			}else{
				// do not restrict type
				
				if(output->type == TYPE_VOIDT && input->size != 0)  // void_t as output and have actual data?
					return error("memcache_value_unserialize unknown output datatype: pass output key or set value_type in configuration");
			}
			
			fastcall_convert_from r_same_convert  = { { 4, ACTION_CONVERT_FROM }, &d_input, userdata->value_format };
			return data_query(output, &r_same_convert);
			
		case VALUE_MODE_ANY_TYPE:;
			data_t                d_data          = DATA_PTR_DATAT(output);
			
			fastcall_convert_from r_convert       = { { 4, ACTION_CONVERT_FROM }, &d_input, userdata->value_format };
			return data_query(&d_data, &r_convert);
	}
} // }}}

static ssize_t memcache_handler(machine_t *machine, request_t *request){ // {{{
	ssize_t                ret;
	memcached_return_t     rc;
	action_t               action;
	slice                  key;
	slice                  value;
	data_t                 free_key;
	data_t                 free_value;
	memcache_userdata     *userdata          = (memcache_userdata *)machine->userdata;
	memcache_threaddata   *threaddata        = thread_data_get(&userdata->thread_data);
	
	hash_data_get(ret, TYPE_ACTIONT, action, request, HK(action));
	if(ret != 0)
		return error("no action specified");
	
	switch(action){
		case ACTION_READ:;
			uint32_t               flags;
			uintmax_t              need_free;
			data_t                *output;
			request_t             *r_next             = request;
			
			if( (ret = data_get_continious(hash_data_find(request, userdata->key), &free_key, (void **)&key.data, &key.size)) < 0){
				data_free(&free_key);
				return ret;
			}
			
			value.data = memcached_get(
				threaddata->mc,
				key.data,
				key.size,
				&value.size,
				&flags,
				&rc
			);
			
			if(value.data == NULL)
				break;
			
			request_t r_next_def[] = {
				{ userdata->value, DATA_VOID },
				hash_inline(request),
				hash_end
			};
			if( (output = hash_data_find(request, userdata->value)) == NULL){
				// assign new
				r_next = r_next_def;
				output = &r_next[0].data;
			}
			
			// deref
			fastcall_getdata r_getdata = { { 3, ACTION_GETDATA } };
			if( (ret = data_query(output, &r_getdata)) < 0)
				goto read_err;
			output = r_getdata.data;
			
			need_free = (output->type == TYPE_VOIDT) ? 1 : 0;
			
			if( (ret = memcache_value_unserialize(userdata, &value, output)) < 0)
				goto read_err;
			
			ret = machine_pass(machine, r_next);
			
			if(need_free)
				data_free(output);
			
		read_err:
			data_free(&free_key);
			return ret;
		
		case ACTION_WRITE:
			if( (ret = data_get_continious(hash_data_find(request, userdata->key), &free_key, (void **)&key.data, &key.size)) < 0){
				data_free(&free_key);
				return ret;
			}
			if( (ret = memcache_value_serialize(
				userdata,
				hash_data_find(request, userdata->value),
				&value,
				&free_value) < 0)
			){
				data_free(&free_value);
				return ret;
			}
			
			rc = memcached_set(
				threaddata->mc,
				key.data,
				key.size,
				value.data,
				value.size,
				(time_t)0,
				0
			);
			data_free(&free_key);
			data_free(&free_value);
			
			if(rc != MEMCACHED_SUCCESS)
				return error("memcache_set error");
			
			break;

		case ACTION_DELETE:
			if( (ret = data_get_continious(hash_data_find(request, userdata->key), &free_key, (void **)&key.data, &key.size)) < 0)
				return ret;
			
			rc = memcached_delete(
				threaddata->mc,
				key.data,
				key.size,
				(time_t)0
			);
			
			data_free(&free_key);
			
			if(rc != MEMCACHED_SUCCESS)
				return error("memcache_delete error");
			
			break;

		default:
			return -ENOSYS;
	};
	return machine_pass(machine, request);
} // }}}

static machine_t c_memcache_proto = {
	.class          = "modules/memcache",
	.supported_api  = API_HASH,
	.func_init      = &memcache_init,
	.func_configure = &memcache_configure,
	.func_destroy   = &memcache_destroy,
	.machine_type_hash = {
		.func_handler = &memcache_handler
	}
};

int main(void){
	errors_register((err_item *)&errs_list, &emodule);
	class_register(&c_memcache_proto);
	return 0;
}
