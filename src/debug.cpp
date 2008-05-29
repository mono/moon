/*
 * debug.cpp: 
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"
#include "debug.h"
 
#if DEBUG

#if INCLUDE_MONO_RUNTIME
// Define to enable stack traces for managed frames.
// You'll also need to call enable_vm_stack_trace when the vm is loaded
// (since we don't link with mono, you'll get some unresolved externals errors otherwise). 
#define MONO_STACK_ENABLED 1
#endif

// Type safety at it's best.
#if MONO_STACK_ENABLED
#define MonoMethod void
#define MonoJitInfo void
#define MonoDomain void
typedef struct _MonoDebugSourceLocation	MonoDebugSourceLocation;

// Very very hackish, but it seems to work.
extern "C" {
	extern char* mono_pmip (void *ip);
	extern MonoMethod* mono_jit_info_get_method (MonoJitInfo* ji);
	extern MonoDomain* mono_domain_get ();
	extern MonoJitInfo* mono_jit_info_table_find (MonoDomain* domain, void* ip);
	extern char* mono_method_full_name (MonoMethod *method, gboolean signature);

	extern MonoDebugSourceLocation * mono_debug_lookup_source_location (MonoMethod *method, guint32 address, MonoDomain *domain);
	extern void mono_debug_free_source_location (MonoDebugSourceLocation *location);

	extern gpointer mono_jit_info_get_code_start (MonoJitInfo* ji);

	extern int mono_jit_info_get_code_size (MonoJitInfo* ji);
};

struct _MonoDebugSourceLocation {
	gchar *source_file;
	guint32 row, column;
	guint32 il_offset;
};
#endif

static bool vm_stack_trace_enabled = false;

void
enable_vm_stack_trace ()
{
	vm_stack_trace_enabled = true;
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
	char entire_line [2000];
	
	while (true) {
		gint64 buffer_read = getline (&buffer, &buffer_length, maps);
		
		if (buffer_read < 0)
			break;
		
		memcpy (entire_line, buffer, buffer_read);
		entire_line [buffer_read + 1] = 0;

		if (buffer_read < 20)
			continue;
			
		buffer [buffer_read - 1] = 0; // Strip off the newline.
		
		const char delimiters[] = " ";
		char *saveptr = NULL;
		char *range = strtok_r (buffer, delimiters,  &saveptr);
		char *a = strtok_r (NULL, delimiters, &saveptr);
		char *b = strtok_r (NULL, delimiters, &saveptr);
		char *c = strtok_r (NULL, delimiters, &saveptr);
		char *d = strtok_r (NULL, delimiters, &saveptr);
		char *lib = strtok_r (NULL, delimiters, &saveptr);
		
		if (lib == NULL) {
			current_library = NULL;
			continue;
		}

		if (lib [0] != '/' && lib [0] != '[') {
			printf ("Something's wrong, lib: %s\n", lib);
			printf ("range: %s, a: %s, b: %s, c: %s, d: %s, lib: %s, line: %s", 
			range, a, b, c, d, lib, entire_line);
		}
		
		saveptr = NULL;
		char* start_range = strtok_r (range, "-",  &saveptr);
		char* end_range = strtok_r (NULL, "-", &saveptr);
		
		char* tail;
		start = start_range ? (gpointer) strtoull (start_range, &tail, 16) : NULL;
		end = end_range ? (gpointer) strtoull (end_range, &tail, 16) : NULL;
		
		if (current_library == NULL || strcmp (lib, current_library) != 0) {
			current_library = lib;
		}
		current_base_address = start;
		
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
	//printf ("library_of_ip (%p, %p): %s\n", ip, base_address, binary);
	
	if (binary == NULL)
		return NULL;
		
	if (binary [0] == '[')
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

void
print_gdb_trace ()
{
	/*

Put this in your ~/.gdbinit file and then do "gdb --eval-command=gdb_trace whatever.exe"
then it will print stacktraces whenever you call print_gdb_trace in the code. 

////////////////////////////
define mono_run
    pst
    run
end

define gdb_trace
        break print_gdb_trace
        commands
                bt 40
                c
        end
end
////////////////////////////
	
	*/
}

#endif
