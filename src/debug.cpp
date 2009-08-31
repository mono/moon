/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * debug.cpp: 
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include "runtime.h"
#include "debug.h"

#include <mono/jit/jit.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
G_BEGIN_DECLS
/* because this header sucks */
#include <mono/metadata/mono-debug.h>
G_END_DECLS
#include <mono/metadata/mono-config.h>
#include <mono/metadata/threads.h>
 
#include <signal.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <execinfo.h>
#include <unistd.h>
#include <ctype.h>

#ifdef HAVE_UNWIND
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <demangle.h>
#endif

#if DEBUG

#if SL_2_0
// Define to enable stack traces for managed frames.
#define MONO_STACK_ENABLED 1
#endif

static bool vm_stack_trace_enabled = false;

void
enable_vm_stack_trace (void)
{
	vm_stack_trace_enabled = true;
}

static char*
get_method_name_from_ip (void *ip)
{
	if (!vm_stack_trace_enabled)
		return NULL;

#if MONO_STACK_ENABLED
	MonoJitInfo *ji;
	MonoMethod *mi;
	char *method;
	char *res;
	MonoDomain *domain = mono_domain_get ();

	ji = mono_jit_info_table_find (domain, (char*) ip);
	if (!ji) {
		return NULL;
	}
	mi = mono_jit_info_get_method (ji);
	method = mono_method_full_name (mi, TRUE);

	res = g_strdup_printf ("%s", method);

	g_free (method);

	return res;
#else
	return NULL;
#endif
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

char* get_stack_trace (void)
{
	return get_stack_trace_prefix ("\t"); 
}

void print_stack_trace (void)
{
	print_stack_trace_prefix ("\t");
}

static char tohex[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

void
hexdump_addr (void *addr, size_t n)
{
	const unsigned char *mem = (const unsigned char *) addr;
	char outbuf[80], *outptr;
	unsigned char c;
	size_t i, j;
	
	for (i = 0; i < n; i += 16) {
		outptr = outbuf;
		
		// write out the offset
		*outptr++ = tohex[(i >> 28) & 0xf];
		*outptr++ = tohex[(i >> 24) & 0xf];
		*outptr++ = tohex[(i >> 20) & 0xf];
		*outptr++ = tohex[(i >> 16) & 0xf];
		*outptr++ = tohex[(i >> 12) & 0xf];
		*outptr++ = tohex[(i >> 8) & 0xf];
		*outptr++ = tohex[(i >> 4) & 0xf];
		*outptr++ = tohex[i & 0xf];
		
		// write out up to 16 hex-encoded octets
		for (j = i; j < n && j < i + 16; j++) {
			if ((j & 0x1) == 0)
				*outptr++ = ' ';
			
			c = mem[j];
			
			*outptr++ = tohex[(c >> 4) & 0xf];
			*outptr++ = tohex[c & 0xf];
		}
		
		// pad out to the expected column
		for ( ; j < i + 16; j++) {
			if ((j & 0x1) == 0)
				*outptr++ = ' ';
			
			*outptr++ = ' ';
			*outptr++ = ' ';
		}
		
		*outptr++ = ' ';
		*outptr++ = ' ';
		*outptr++ = ' ';
		*outptr++ = ' ';
		
		// write out up to 16 raw octets
		for (j = i; j < n && j < i + 16; j++) {
			c = mem[j];
			if (isprint ((int) c)) {
				*outptr++ = (char) c;
			} else {
				*outptr++ = '.';
			}
		}
		
		*outptr++ = '\n';
		*outptr = '\0';
		
		fputs (outbuf, stdout);
	}
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

static __thread Addr2LineData *addr2line_pipes = NULL;

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
		
	if (binary [0] == '[') {
		g_free (binary);
		return NULL;
	}

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
			g_free (binary);
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

	g_free (binary);

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
get_stack_trace_prefix (const char* prefix, int maxframes)
{
	int address_count;
	gpointer ip;
	int total_length = 0;
	int prefix_length = strlen (prefix);
	void *ips [maxframes];
	char *frames [maxframes];
	char **names;
	
	address_count = backtrace (ips, maxframes);

	for (int i = 2; i < address_count; i++) {
		ip = ips [i];

		char* frame = addr2line (ip);
		
		if (frame == NULL && mono_domain_get ())
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
	
	char* result = (char*) g_malloc0 (total_length + 1);
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
print_stack_trace_prefix (const char* prefix, int maxframes)
{
	char* st = get_stack_trace_prefix (prefix, maxframes);
	printf (st);
	g_free (st);
}

#ifdef HAVE_UNWIND

enum FrameType {
	UKNOWN = 0,
	INSTANCE = 1,
	STATIC = 2,
	MONO = 3,
	C = 4
};

struct Frame : List::Node {
	long ptr;
	char *name;
	FrameType type;
	virtual ~Frame () {
		g_free (name);
	}

	virtual char *ToString ()
	{
		return g_strdup_printf ("%d|%lx|%s", type, ptr, name);
	}
};

struct Frames : List::Node {
	List *list;
	char *act;
	char *typname;
	int refcount;
	virtual ~Frames () {
		g_free (act);
		g_free (typname);
		delete (list);
	}
};

List *allframes = NULL;
#endif

void
print_reftrace (const char * act, const char * typname, int refcount, bool keep)
{
#ifdef HAVE_UNWIND
	unw_cursor_t cursor; unw_context_t uc;
	unw_word_t ip, sp, bp;

	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);

	char framename [1024];
	Frame *frame;
	List *frames = new List ();

	int count = 0;
	while (unw_step(&cursor) > 0 && count < MAX_STACK_FRAMES) {

		frame = new Frame ();

		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		unw_get_reg(&cursor, UNW_REG_SP, &sp);
#if (__i386__)
		unw_get_reg(&cursor, UNW_X86_EBP, &bp);
#elif (__amd64__)
		unw_get_reg(&cursor, UNW_X86_64_RBP, &bp);
#endif

		unw_get_proc_name (&cursor, framename, sizeof(framename), 0);

		if (!*framename) {
			if (mono_domain_get ()) {
				frame->ptr = (long)ip;
				char * ret = get_method_name_from_ip ((void*)ip);
				frame->name = g_strdup (ret);
				g_free (ret);
				frame->type = MONO;
			} else {
				delete frame;
				continue;
			}
		} else {

			char * demangled = cplus_demangle (framename, 0);
			if (demangled) {
				if (strstr (demangled, ":") > 0) {
					frame->ptr = *(int*)(bp+8);
					frame->name = g_strdup (demangled);
					frame->type = INSTANCE;
				} else {
					frame->ptr = (long)bp;
					frame->name = g_strdup (demangled);
					frame->type = STATIC;
				}
			} else {
				frame->ptr = (long)bp;
				frame->name = g_strdup (framename);
				frame->type = C;
			}

		}
		frames->Append (frame);
		count++;
	}

	if (keep) {
		if (allframes == NULL)
			allframes = new List ();
		if (allframes->Length() % 50)
			dump_frames ();
		Frames *f = new Frames ();
		f->list = frames;
		f->act = g_strdup (act);
		f->typname = g_strdup (typname);
		f->refcount = refcount;
		allframes->Append (f);
	} else {

		printf("trace:%s|%s|%d;", act, typname,refcount);
		frame = (Frame*)frames->First ();
		while (frame != NULL) {
			char *s = frame->ToString ();
			printf ("%s;", s);
			g_free(s);
			frame = (Frame*)frame->next;
		}
		printf ("\n");

		delete frames;
	}
#endif
}


void dump_frames (void)
{
#if HAVE_UNWIND
	Frames *frames = (Frames*)allframes->First ();
	while (frames != NULL) {
		printf("trace:%s|%s|%d;", frames->act, frames->typname, frames->refcount);
		Frame *frame = (Frame*)frames->list->First ();
		while (frame != NULL) {
			char *s = frame->ToString ();
			printf ("%s;", s);
			g_free(s);
			frame = (Frame*)frame->next;
		}
		printf ("\n");
		frames = (Frames*)frames->next;
	}
	allframes->Clear (true);
	//delete (frames);
	//frames = NULL;
#endif
}

#if SANITY

/*
 * moonlight_handle_native_sigsegv: 
 *   (this is a slightly modified version of mono_handle_native_sigsegv from mono/mono/mini/mini-exceptions.c)
 *
 */


static bool handlers_installed = false;
static bool handling_sigsegv = false;

static void
print_gdb_trace ()
{
	/* Try to get more meaningful information using gdb */
#if !defined(PLATFORM_WIN32)
	/* From g_spawn_command_line_sync () in eglib */
	int res;
	int stdout_pipe [2] = { -1, -1 };
	pid_t pid;
	const char *argv [16];
	char buf1 [128];
	int status;
	char buffer [1024];

	res = pipe (stdout_pipe);
	g_assert (res != -1);
		
	/*
	 * glibc fork acquires some locks, so if the crash happened inside malloc/free,
	 * it will deadlock. Call the syscall directly instead.
	 */
	pid = syscall (SYS_fork);
	if (pid == 0) {
		close (stdout_pipe [0]);
		dup2 (stdout_pipe [1], STDOUT_FILENO);

		for (int i = getdtablesize () - 1; i >= 3; i--)
			close (i);

		argv [0] = g_find_program_in_path ("gdb");
		if (argv [0] == NULL) {
			close (STDOUT_FILENO);
			exit (1);
		}

		argv [1] = "-ex";
		sprintf (buf1, "attach %ld", (long)getpid ());
		argv [2] = buf1;
		argv [3] = "--ex";
		argv [4] = "info threads";
		argv [5] = "--ex";
		argv [6] = "thread apply all bt";
		argv [7] = "--batch";
		argv [8] = 0;

		execv (argv [0], (char**)argv);
		exit (1);
	}

	close (stdout_pipe [1]);

	fprintf (stderr, "\nDebug info from gdb:\n\n");

	while (1) {
		int nread = read (stdout_pipe [0], buffer, 1024);

		if (nread <= 0)
			break;
		write (STDERR_FILENO, buffer, nread);
	}		

	waitpid (pid, &status, WNOHANG);
	
#endif
}

void
static moonlight_handle_native_sigsegv (int signal)
{
	const char *signal_str;
	
	switch (signal) {
	case SIGSEGV: signal_str = "SIGSEGV"; break;
	case SIGFPE: signal_str = "SIGFPE"; break;
	case SIGABRT: signal_str = "SIGABRT"; break;
	case SIGQUIT: signal_str = "SIGQUIT"; break;
	default: signal_str = "UNKNOWN"; break;
	}
	
	if (handling_sigsegv) {
		/*
		 * In our normal sigsegv handling we do signal-unsafe things to provide better 
		 * output to what actually happened. If we get another one, do only signal-safe
		 * things
		 */
		_exit (1);
		return;
	}

	/* To prevent infinite loops when the stack walk causes a crash */
	handling_sigsegv = true;

	/*
	 * A SIGSEGV indicates something went very wrong so we can no longer depend
	 * on anything working. So try to print out lots of diagnostics, starting 
	 * with ones which have a greater chance of working.
	 */
	fprintf (stderr,
			 "\n"
			 "=============================================================\n"
			 "Got a %s while executing native code.                        \n"
			 " We'll first ask gdb for a stack trace, then try our own     \n"
			 " stack walking method (usually not as good as gdb, but it    \n"
			 " can do managed and native stack traces together)            \n"
			 "=============================================================\n"
			 "\n", signal_str);

	print_gdb_trace ();		

	fprintf (stderr, "\nDebug info from libmoon:\n\n");
	print_stack_trace ();	
	
	if (signal != SIGQUIT) {
		abort ();
	} else {
		handling_sigsegv = false;
	}
}

void
moonlight_install_signal_handlers ()
{
	struct sigaction sa;

	if (handlers_installed)
		return;
	handlers_installed = true;

	printf ("Moonlight: Installing signal handlers for crash reporting.\n");
	
	sa.sa_handler = moonlight_handle_native_sigsegv;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;

	g_assert (sigaction (SIGSEGV, &sa, NULL) != -1);
	g_assert (sigaction (SIGFPE, &sa, NULL) != -1);
	g_assert (sigaction (SIGQUIT, &sa, NULL) != -1);
}
#endif /* SANITY */

#endif /* DEBUG */
