/*
 * signal-handler.cpp
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <signal.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "signal-handler.h"

typedef void void_method ();
void print_stack_traces ();
static bool handlers_installed = false;
static bool handling_sigsegv = false;

void
shocker_install_signal_handlers ()
{
	struct sigaction sa;
	const char *env;

	if (handlers_installed)
		return;
	handlers_installed = true;
	
	env = getenv ("MOONLIGHT_SHOCKER_INSTALL_SIGNAL_HANDLER");
	if (env == NULL || env [0] == 0)
		return;
	
	printf ("[Shocker]: Installing signal handlers for crash reporting.\n");
	
	sa.sa_handler = shocker_handle_native_sigsegv;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;

	g_assert (sigaction (SIGSEGV, &sa, NULL) != -1);
	g_assert (sigaction (SIGFPE, &sa, NULL) != -1);
	g_assert (sigaction (SIGQUIT, &sa, NULL) != -1);
}

/*
 * shocker_handle_native_sigsegv: 
 *   (this is a slightly modified version of mono_handle_native_sigsegv from mono/mono/mini/mini-exceptions.c)
 *
 */

void
shocker_handle_native_sigsegv (int signal)
{
	const char *signal_str = (signal == SIGSEGV) ? "SIGSEGV" : "SIGABRT";
	switch (signal) {
	case SIGSEGV: signal_str = "SIGSEGV"; break;
	case SIGFPE: signal_str = "SIGFPE"; break;
	case SIGABRT: signal_str = "SIGABRT"; break;
	case SIGQUIT: signal_str = "SIGQUIT"; break;
	default:
		signal_str = "UNKNOWN"; break;
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

	print_stack_traces ();		

	if (signal != SIGQUIT) {
		abort ();
	} else {
		handling_sigsegv = false;
	}
}

void
print_stack_traces ()
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

	fprintf (stderr, "\nDebug info from libmoon:\n\n");
	void *libmoon = dlopen ("libmoon.so", RTLD_LOCAL | RTLD_LAZY);
	void_method *print_stack_trace = (libmoon == NULL) ? NULL : (void_method *) dlsym (libmoon, "print_stack_trace");
	if (print_stack_trace)
		print_stack_trace ();
	
}
