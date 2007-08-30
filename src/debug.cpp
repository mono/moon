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


static char*
addr2line (gpointer ip)
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
			return g_strdup (binary);
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
	offset = (gpointer) (((size_t) ip) - ((size_t) addr2line->base));
	
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
		return g_strdup (binary);
	
	if (result [0] == '?' || result [0] == 0)
		return g_strdup (binary);

	result_length = strlen (result);
	result [result_length - 1] = 0;
	first = result;
		
	result = fgets (buf + result_length, sizeof (buf) - result_length, addr2line->pipeout);
	
	if (result == NULL)
		return g_strdup (first);

	result_length = strlen (result);
	result [result_length - 1] = 0;
	second = result;
	
	res = g_strdup_printf ("%s [%p] %s %s", addr2line->binary, ip, first, second);

	// printf ("Final result: %s\n", res);

	return res;
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
		
		if (frame == NULL || strlen (frame) == 0 || strcmp ("??", frame) == 0) {
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
