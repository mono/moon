/*
 * debug.cpp: 
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
 
#include "debug.h"
 
#if STACK_DEBUG

#define MONO_STACK_ENABLED 1

// Type safety at it's best.
#if MONO_STACK_ENABLED
#define MonoMethod void
#define MonoJitInfo void
#define MonoDomain void
typedef struct _MonoDebugSourceLocation	MonoDebugSourceLocation;

// Very very hackish, but it seems to work.
extern "C" char* mono_pmip (void *ip);
extern "C" MonoMethod* mono_jit_info_get_method (MonoJitInfo* ji);
extern "C" MonoDomain* mono_domain_get ();
extern "C" MonoJitInfo* mono_jit_info_table_find (MonoDomain* domain, void* ip);
extern "C" char* mono_method_full_name (MonoMethod *method, gboolean signature);

extern "C" MonoDebugSourceLocation * mono_debug_lookup_source_location (MonoMethod *method, guint32 address, MonoDomain *domain);
extern "C" void mono_debug_free_source_location (MonoDebugSourceLocation *location);

extern "C" gpointer mono_jit_info_get_code_start (MonoJitInfo* ji);

extern "C" int mono_jit_info_get_code_size (MonoJitInfo* ji);

struct _MonoDebugSourceLocation {
	gchar *source_file;
	guint32 row, column;
	guint32 il_offset;
};
#endif

static bool vm_stack_trace_enabled = FALSE;
void
enable_vm_stack_trace (bool enable)
{
	vm_stack_trace_enabled = enable;
}

static char*
get_method_from_ip (void *ip)
{
	if (!vm_stack_trace_enabled)
		return NULL;
	
#if MONO_STACK_ENABLED
	MonoJitInfo *ji;
	MonoMethod *mi;
	char *method;
	char *res;
	gpointer jit_start;
	int jit_size;
	MonoDomain *domain = mono_domain_get ();
	MonoDebugSourceLocation *location;
	
	ji = mono_jit_info_table_find (domain, (char*) ip);
	if (!ji) {
		return NULL;
	}
	mi = mono_jit_info_get_method (ji);
	jit_start = mono_jit_info_get_code_start (ji);
	jit_size = mono_jit_info_get_code_size (ji);
	method = mono_method_full_name (mi, TRUE);
	
	location = mono_debug_lookup_source_location (mi, (guint32)((guint8*)ip - (guint8*)jit_start), domain);

	if (location) {
		res = g_strdup_printf (" %s in %s:%i,%i", method, location->source_file, location->row, location->column);
	} else {
		res = g_strdup_printf (" %s + 0x%x", method, (int)((char*)ip - (char*)jit_start));
	}
	mono_debug_free_source_location (location);
	
	g_free (method);

	return res;
#else
	return NULL;
#endif
}

char* get_stack_trace () 
{
	return get_stack_trace_prefix ("\t"); 
}
void print_stack_trace ()
{
	print_stack_trace_prefix ("\t");
}

typedef struct Addr2LineData Addr2LineData;

struct Addr2LineData {
	Addr2LineData *next;
	FILE *pipein;
	FILE *pipeout;
	char *binary;
	int child_pid;
	gpointer base;
};

static Addr2LineData *addr2line_pipes = NULL;

static char*
library_of_ip (gpointer ip, gpointer* base_address)
{
	/* non-linux platforms will need different code here */
	
	FILE* maps = fopen ("/proc/self/maps", "r");
	char * buffer = NULL;
	size_t buffer_length = 0;
	char* result = NULL;
	char* current_library = NULL;
	gpointer current_base_address = NULL; 
	gpointer start, end;
	
	while (true) {
		gint64 buffer_read = getline (&buffer, &buffer_length, maps);
		
		if (buffer_read < 0)
			break;
		
		if (buffer_read < 20)
			continue;
			
		buffer [buffer_read - 1] = 0; // Strip off the newline.
		
		const char delimiters[] = " ";
		char* range = strtok (buffer, delimiters);
		char* dummy = strtok (NULL, delimiters);
		dummy = strtok (NULL, delimiters);
		dummy = strtok (NULL, delimiters);
		dummy = strtok (NULL, delimiters);
		char* lib = strtok (NULL, delimiters);
		
		if (lib == NULL) {
			current_library = NULL;
			continue;
		}

		char* start_range = strtok (range, "-");
		char* end_range = strtok (NULL, "-");
		
		char* tail;
		start = (gpointer) strtoull (start_range, &tail, 16);
		end = (gpointer) strtoull (end_range, &tail, 16);
		
		if (current_library == NULL || strcmp (lib, current_library) != 0) {
			current_library = lib;
			current_base_address = start;
		}
		
		if (start <= ip && end >= ip) {
			result = g_strdup (lib);
			*base_address = current_base_address;
			// printf ("IP %p is in library %s\n", ip, result);
			break;
		}
	}
	
	free (buffer);
	fclose (maps);
	
	return result;
}

static char* addr2line_offset (gpointer ip, bool use_offset);

static char* 
addr2line (gpointer ip) 
{
	char* result = addr2line_offset (ip, true);
	if (result == NULL)
		result = addr2line_offset (ip, false);
	return result;
}


static char*
addr2line_offset (gpointer ip, bool use_offset)
{
	char *res;
	Addr2LineData *addr2line;
	gpointer base_address;
	
	char* binary = library_of_ip (ip, &base_address);
	
	if (binary == NULL)
		return NULL;

	for (addr2line = addr2line_pipes; addr2line; addr2line = addr2line->next) {
		if (strcmp (binary, addr2line->binary) == 0)
			break;
	}

	if (!addr2line) {
		const char *addr_argv[] = {"addr2line", "-f", "-e", binary, "-C", NULL};
		int child_pid;
		int ch_in, ch_out;
		
		if (!g_spawn_async_with_pipes (NULL, (char**)addr_argv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL,
				&child_pid, &ch_in, &ch_out, NULL, NULL)) {
			return NULL;
		}
		
		addr2line = g_new0 (Addr2LineData, 1);
		addr2line->base = base_address;
		addr2line->child_pid = child_pid;
		addr2line->binary = g_strdup (binary);
		addr2line->pipein = fdopen (ch_in, "w");
		addr2line->pipeout = fdopen (ch_out, "r");
		addr2line->next = addr2line_pipes;
		addr2line_pipes = addr2line;
	}
		
	gpointer offset;
	if (use_offset)
		offset = (gpointer) (((size_t) ip) - ((size_t) addr2line->base));
	else
		offset = ip;
	
	// printf ("Checking ip: %p, offset: %p, base: %p\n", ip, offset, addr2line->base);
	fprintf (addr2line->pipein, "%p\n", offset);
	fflush (addr2line->pipein);
	
	/* we first get the func name and then file:lineno in a second line */
	char buf [1024];
	char* first;
	char* second;
	char* result;
	int result_length;
		
	result = fgets (buf, sizeof (buf), addr2line->pipeout);
	
	if (result == NULL)
		return NULL;
	
	if (result [0] == '?' || result [0] == 0)
		return NULL;

	result_length = strlen (result);
	result [result_length - 1] = 0;
	first = result;
		
	result = fgets (buf + result_length, sizeof (buf) - result_length, addr2line->pipeout);
	
	if (result == NULL)
		return NULL;

	result_length = strlen (result);
	result [result_length - 1] = 0;
	second = result;
	
	res = g_strdup_printf ("%s [%p] %s %s", addr2line->binary, ip, first, second);

	// printf ("Final result: %s\n", res);

	return res;
}

char*
get_managed_frame (gpointer ip)
{
	return get_method_from_ip (ip);
}

char* 
get_stack_trace_prefix (const char* prefix)
{
	int address_count;
	gpointer ip;
	int total_length = 0;
	int prefix_length = strlen (prefix);
	void *ips [MAX_STACK_FRAMES];
	char *frames [MAX_STACK_FRAMES];
	char **names;
	
	address_count = backtrace (ips, MAX_STACK_FRAMES);

	for (int i = 2; i < address_count; i++) {
		ip = ips [i];

		char* frame = addr2line (ip);
		
		if (frame == NULL)
			frame = get_managed_frame (ip);
		
		if (frame == NULL || strlen (frame) == 0 || frame [0] == '?') {
			g_free (frame);	
			names = backtrace_symbols (&ip, 1);
			frame = g_strdup (names [0]);
			free (names);
		}
		frames [i] = frame;
		total_length += prefix_length + strlen (frame) + 1;
	}
	
	char* result = (char*) g_malloc0 (total_length);
	int position = 0;
	for (int i = 2; i < address_count; i++) {
		char* frame = frames [i];
		size_t frame_length = strlen (frame);

		memcpy (result + position, prefix, prefix_length);
		position += prefix_length;
		memcpy (result + position, frame, frame_length);
		position += frame_length;
		memcpy (result + position, "\n", 1);
		position ++;
		
		g_free (frame);
	}
	
	return result;
}

void
print_stack_trace_prefix (const char* prefix)
{
	char* st = get_stack_trace_prefix (prefix);
	printf (st);
	g_free (st);
}

#endif
