/*
 * lunar-launcher.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */
 
#include <config.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <dlfcn.h>

#include "nsXPCOMGlue.h"
// this fixes a few redefined macros warnings:
#define nscore_h__

#if HAVE_GECKO_1_9
#include <gtkmozembed.h>
#else
#include "gtkembedmoz/gtkmozembed.h"
#endif
  
#include "lunar-launcher.h"

#define LOG_LAUNCH(...) printf (__VA_ARGS__)

/*
 * LunarLauncher
 */
LunarLauncher::LunarLauncher ()
{
	application = NULL;
	title = NULL;
	width = 0;
	height = 0;
}

LunarLauncher::~LunarLauncher ()
{
	g_free (application);
	g_free (title);
}

void
LunarLauncher::SetApplication (const char *value)
{
	g_free (application);
	application = g_strdup (value);
}

void
LunarLauncher::SetTitle (const char *value)
{
	g_free (title);
	title = g_strdup (value);
}

void
LunarLauncher::ShowUsage ()
{
	fprintf (stderr,
"Usage is: lunar-launcher [options]\n"
"\n"
"Options:\n"
"  -moonapp <application>\n"
"  -moontitle <title>\n"
"  -moonwidth <width>\n"
"  -moonheight <height\n");
}

int
LunarLauncher::Launch (int argc, char **argv)
{
	// fprintf (fp, "firefox -moonapp \"file://%s/index.html\" -moonwidth %d -moonheight %d -moontitle \"%s\"\n", app_dir, width, height, settings->GetShortName());

	for (int i = 1; i < argc; i += 2) {
		if (!strcmp (argv [i], "-moonapp")) {
			SetApplication (argv [i + 1]);
		} else if (!strcmp (argv [i], "-moonwidth")) {
			SetWidth (atoi (argv [i + 1]));
		} else if (!strcmp (argv [i], "-moonheight")) {
			SetHeight (atoi (argv [i + 1]));
		} else if (!strcmp (argv [i], "-moontitle")) {
			SetTitle (argv [i + 1]);
		} else {
			ShowUsage ();
			return 1;
		}
	}

	if (application == NULL || title == NULL || height == 0 || width == 0) {
		ShowUsage ();
		return 1;
	}

	LOG_LAUNCH ("[%i lunar launcher] Launching %s (%s width: %i height; %i)\n", getpid (), title, application, width, height);

	top_level_window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
	gtk_window_set_title (top_level_window, title);

	static const GREVersionRange gre_version = {
		"1.8", PR_TRUE,
		"9.9", PR_TRUE
	};

	char xpcom_lib_path [PATH_MAX];
	char* xpcom_dir_path;

	GRE_GetGREPathWithProperties (&gre_version, 1, nsnull, 0, xpcom_lib_path, sizeof (xpcom_lib_path));
	xpcom_dir_path = g_path_get_dirname (xpcom_lib_path);

#if HAVE_GECKO_1_9
	gtk_moz_embed_set_path (xpcom_dir_path);
#else
	gtk_moz_embed_set_comp_path (xpcom_dir_path);
#endif
	g_free (xpcom_dir_path);

	moz_embed = gtk_moz_embed_new();
	gtk_container_add (GTK_CONTAINER (top_level_window), moz_embed);

	g_signal_connect (G_OBJECT (top_level_window), "delete-event",    G_CALLBACK (DeleteCallback),   top_level_window);
	g_signal_connect (G_OBJECT (top_level_window), "key_press_event", G_CALLBACK (KeyPressCallback), top_level_window);

	gtk_widget_set_usize (moz_embed, width, height);
	gtk_widget_show_all (moz_embed);
	gtk_widget_show_all (GTK_WIDGET (top_level_window));

	gtk_moz_embed_load_url (GTK_MOZ_EMBED (moz_embed), application);
	gtk_main ();

	return 0;
}

gboolean
LunarLauncher::DeleteCallback (GtkWidget* widget, GdkEventKey* event, GtkWindow* window)
{
	gtk_main_quit ();
	return false;
}

gboolean
LunarLauncher::KeyPressCallback (GtkWidget* widget, GdkEventKey* event, GtkWindow* window)
{
	LOG_LAUNCH ("[%i lunar launcher]: key press: keyval: 0x%x state: 0x%x\n", getpid (), event->keyval, event->state);
	return false;
}

/*
 * Signal handling
 */

/*
 * launcher_handle_native_sigsegv: 
 *   (this is a slightly modified version of mono_handle_native_sigsegv from mono/mono/mini/mini-exceptions.c)
 *
 */

typedef void void_method ();

static void
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

static bool handling_sigsegv = false;

void
launcher_handle_native_sigsegv (int signal)
{
	const char *signal_str = (signal == SIGSEGV) ? "SIGSEGV" : "SIGABRT";
	switch (signal) {
	case SIGSEGV: signal_str = "SIGSEGV"; break;
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
			 "Moonlight (OOB) got a %s while executing native code.        \n"
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

static void
launcher_add_signal_handlers ()
{
	struct sigaction sa;

	sa.sa_handler = launcher_handle_native_sigsegv;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;

	g_assert (sigaction (SIGSEGV, &sa, NULL) != -1);
	g_assert (sigaction (SIGQUIT, &sa, NULL) != -1);
}

int
main (int argc, char **argv)
{
	LunarLauncher *launcher;
	int result;

	launcher_add_signal_handlers ();

	gtk_init (&argc, &argv);

	launcher = new LunarLauncher ();
	result = launcher->Launch (argc, argv);
	delete launcher;

	return result;
}
