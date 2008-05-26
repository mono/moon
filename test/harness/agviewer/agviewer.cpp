/*
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Copyright (c) 2007-2008 Novell, Inc.
 *
 * Authors:
 *	Jackson Harper (jackson@ximian.com)
 *
 */


#include <string.h>
#include "config.h"
#include <stdlib.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <dlfcn.h>

#if HAVE_GECKO_1_9
#include <gtkmozembed.h>
#else
#include "gtkembedmoz/gtkmozembed.h"
#endif


#include "agserver.h"
#include "AgserverObject.h"

#include "nsXPCOMGlue.h"



#define DRT_SERVICE             "mono.moonlight.tests"

#define DRT_LOGGER_PATH         "/mono/moonlight/tests/logger"
#define DRT_LOGGER_INTERFACE    "mono.moonlight.tests.logger.ITestLogger"

#define DRT_RUNNER_PATH         "/mono/moonlight/tests/runner"
#define DRT_RUNNER_INTERFACE    "mono.moonlight.tests.runner.ITestRunner"


#define DRT_AGSERVER_SERVICE	"mono.moonlight.agserver"
#define DRT_AGSERVER_PATH       "/mono/moonlight/agserver"
#define DRT_AGSERVER_INTERFACE  "mono.moonlight.agserver.IAgserver"


#define DEFAULT_TIMEOUT 20000


typedef struct _AgViewer {
	GtkWindow  *top_level_window;
	GtkWidget  *moz_embed;
} AgViewer;



static AgViewer* browser = NULL;
static char* test_path = NULL;
static const char* working_dir = NULL;
static guint timeout_id = 0;

static void run_test (char* test_path, int timeout);
static bool move_to_next_test ();
static void request_test_runner_shutdown ();
static void signal_test_complete (const char* test_name, bool successful);
static bool wait_for_next_test (int* timeout);
static void mark_test_as_complete_and_start_next_test (bool successful);
static void log_message (const char* test_name, const char* message);
static void set_current_dir (const char* test_path);
static void agviewer_handle_native_sigsegv (int signal);
static void agviewer_add_signal_handler ();


static void
agserver_class_init (AgserverClass* agserver_class)
{
	dbus_g_object_type_install_info (AGSERVER_TYPE, &dbus_glib_agserver_object_info);
}

static void
agserver_init (Agserver* agserver)
{
}

//
// Called from libshocker when a test is finished
//
gboolean
signal_shutdown (Agserver* server, GError** error)
{
	mark_test_as_complete_and_start_next_test (true);
	return true;
}


static void
register_agserver_object ()
{
	DBusGConnection *connection;
	DBusGProxy *proxy;
	GError *error = NULL;
	GObject *obj;
	guint request_name_ret;

	g_type_init ();

	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (connection == NULL) {
		printf ("Failed to open connection to bus  %s\n", error->message);
		return;
	}

	proxy = dbus_g_proxy_new_for_name (connection, DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);

	if (!org_freedesktop_DBus_request_name (proxy, DRT_AGSERVER_SERVICE, 0, &request_name_ret, &error)) {
		printf ("Unable to request name:  %s\n", error->message);
		return;
	}

	if (request_name_ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		printf ("Unable to become the primary owner of IAgserver interface.\n");
		return;
	}

	obj = G_OBJECT (g_object_new (AGSERVER_TYPE, NULL));
	dbus_g_connection_register_g_object (connection, DRT_AGSERVER_PATH, obj);
}




static int close_key_pressed_count = 0;
static AgViewer *new_gtk_browser ();
static gboolean key_press_cb (GtkWidget* widget, GdkEventKey* event, GtkWindow* window);

static gboolean test_timeout (gpointer data);


static void
set_current_dir (const char* test_path)
{
	char *dir = NULL;
	
	if (g_str_has_prefix (test_path, "http")) {
		if (chdir (working_dir) != 0)
			g_warning ("Unable to set working directory to: %s\n", working_dir);
	} else {
		if (!g_path_is_absolute (test_path)) {
			char* wd = get_current_dir_name ();
			char* abs_path = g_build_filename (wd, test_path, NULL);
			
			dir = g_path_get_dirname (test_path);
			
			free (wd);
			g_free (abs_path);
		} else {
			dir = g_path_get_dirname (test_path);
		}
		
		if (chdir (dir) != 0)
			g_warning ("Unable to set working directory to: %s\n", dir);
			
		g_free (dir);
	}
}

int
main(int argc, char **argv)
{
	int frame_width = 800;
	int frame_height = 800;

	agviewer_add_signal_handler ();
	
	gtk_init (&argc, &argv);

	int i;
	for (i = 1; i < argc && argv[i][0] == '-'; i++) {
		if (!g_strcasecmp ("-framewidth", argv [i]) && i < argc) {
			frame_width = strtol (argv [++i], NULL, 10);
			continue;
		}
		
		if (!g_strcasecmp ("-frameheight", argv [i]) && i < argc) {
			frame_height = strtol (argv [++i], NULL, 10);
			continue;
		}
		
		if (!g_strcasecmp ("-working-dir", argv [i]) && i < argc) {
			working_dir = argv [++i];
			continue;
		}
		
		if (!g_strcasecmp ("-server", argv [i]))
			continue;
	}
	
	if (i < argc)
		test_path = g_strdup (argv [argc - 1]);

	browser = new_gtk_browser ();

	gtk_widget_set_usize (browser->moz_embed, frame_width, frame_height);
	gtk_window_fullscreen (browser->top_level_window);
	gtk_widget_show_all (browser->moz_embed);
	gtk_widget_show_all (GTK_WIDGET (browser->top_level_window));

	register_agserver_object ();
	
	if (test_path && test_path[0]) {
		run_test (test_path, DEFAULT_TIMEOUT);
		gtk_main ();
	} else {
		if (move_to_next_test ())
			gtk_main ();
	}
	
	g_free (test_path);

	return 0;
}

static void
run_test (char* test_path, int timeout)
{
	if (timeout_id)
		g_source_remove (timeout_id);
	timeout_id = g_timeout_add (timeout, test_timeout, test_path);
	
	set_current_dir (test_path);
	
	gtk_moz_embed_load_url (GTK_MOZ_EMBED (browser->moz_embed), test_path);
}


static bool
move_to_next_test ()
{
	int timeout = 0;
	if (wait_for_next_test (&timeout)) {
		run_test (test_path, timeout);
		return true;
	} else {
		if (gtk_main_level ())
			gtk_main_quit ();
	}
	return false;
}

static void
mark_test_as_complete_and_start_next_test (bool successful)
{
	DBusGProxy* dbus_proxy;
	DBusGConnection* connection;
	GError* error = NULL;  

	error = NULL;
	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (!connection) {
		g_warning ("Failed to open connection to bus to get next test, agivewer process will not be reused: %s\n", error->message);
		g_error_free (error);
		return;
	}

	dbus_proxy = dbus_g_proxy_new_for_name (connection,
			DRT_SERVICE,
			DRT_RUNNER_PATH,
			DRT_RUNNER_INTERFACE);

	int timeout;
	bool available;
	char* test_name = g_path_get_basename (test_path);

	g_free (test_path);
	test_path = NULL;

	if (!dbus_g_proxy_call (dbus_proxy, "MarkTestAsCompleteAndGetNextTest", &error,
			G_TYPE_STRING, test_name,
			G_TYPE_BOOLEAN, successful,
			G_TYPE_INVALID,
			G_TYPE_BOOLEAN, &available,
			G_TYPE_STRING, &test_path,
			G_TYPE_INT, &timeout,
			G_TYPE_INVALID)) {
		printf ("error while making MarkTestAsCompleteAndGetNextTest dbus call:   %s\n", error->message);
		printf ("if you are running standalone tests without a dbus test runner you can ignore this error message\n");

		available = false;
	}
	g_free (test_name);

	if (available)
		run_test (test_path, timeout);
	else
		gtk_main_quit ();
}


static AgViewer *
new_gtk_browser ()
{
	AgViewer *browser = 0;
	
	browser = g_new0 (AgViewer, 1);
	browser->top_level_window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));

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

	browser->moz_embed = gtk_moz_embed_new();
	gtk_container_add (GTK_CONTAINER (browser->top_level_window),
			browser->moz_embed);

	g_signal_connect (G_OBJECT (browser->top_level_window), "key_press_event", G_CALLBACK (key_press_cb), browser->top_level_window);

	return browser;
}

static gboolean
key_press_cb (GtkWidget* widget, GdkEventKey* event, GtkWindow* window)
{
	printf ("agviewer keypress:  0x%x\n", event->keyval);
	if ((event->state & GDK_CONTROL_MASK) == 0)
		return FALSE;

	switch (event->keyval) {
	case GDK_c:
	case GDK_C:
		if (close_key_pressed_count == 0)
			request_test_runner_shutdown ();
		else if (close_key_pressed_count > 0)
			exit (1);
		close_key_pressed_count++;
		return TRUE;
	case GDK_n:
	case GDK_N:
		mark_test_as_complete_and_start_next_test (false);
		return TRUE;
	}

	return FALSE;
}

static gboolean
test_timeout (gpointer data)
{
	const char* test = (const char *) data;
	log_message (test, "test timed out.");

	mark_test_as_complete_and_start_next_test (false);
	return FALSE;
}

static void
request_test_runner_shutdown ()
{
	g_type_init ();

	DBusGProxy* dbus_proxy;
	DBusGConnection* connection;
	GError* error = NULL;  

	error = NULL;
	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (!connection) {
		g_warning ("Failed to open connection to bus: %s\n", error->message);
		g_error_free (error);
	}

	dbus_proxy = dbus_g_proxy_new_for_name (connection,
			DRT_SERVICE,
			DRT_RUNNER_PATH,
			DRT_RUNNER_INTERFACE);

	dbus_g_proxy_call_no_reply (dbus_proxy, "RequestShutdown", G_TYPE_INVALID);
}

static void
signal_test_complete (const char *test_path, bool successful)
{
	g_type_init ();

	DBusGProxy* dbus_proxy;
	DBusGConnection* connection;
	GError* error = NULL;  

	error = NULL;
	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (!connection) {
		g_warning ("Failed to open connection to bus while signalling shutdown: %s\n", error->message);
		g_error_free (error);
	}

	dbus_proxy = dbus_g_proxy_new_for_name (connection,
			DRT_SERVICE,
			DRT_RUNNER_PATH,
			DRT_RUNNER_INTERFACE);

	char* test_name = g_path_get_basename (test_path);
	dbus_g_proxy_call_no_reply (dbus_proxy, "TestComplete", G_TYPE_STRING, test_name, G_TYPE_BOOLEAN, successful, G_TYPE_INVALID);

	g_free (test_name);
}


static bool
wait_for_next_test (int *timeout)
{
	g_type_init ();

	DBusGProxy* dbus_proxy;
	DBusGConnection* connection;
	GError* error = NULL;  

	error = NULL;
	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (!connection) {
		g_warning ("Failed to open connection to bus to get next test, agivewer process will not be reused: %s\n", error->message);
		g_error_free (error);
		return false;
	}

	dbus_proxy = dbus_g_proxy_new_for_name (connection,
			DRT_SERVICE,
			DRT_RUNNER_PATH,
			DRT_RUNNER_INTERFACE);

	if (test_path) {
		g_free (test_path);
		test_path = NULL;
	}

	bool available;
	if (!dbus_g_proxy_call (dbus_proxy, "GetNextTest", &error,
			G_TYPE_INVALID,
			G_TYPE_BOOLEAN, &available,
			G_TYPE_STRING, &test_path,
			G_TYPE_INT, timeout,
			G_TYPE_INVALID)) {
		printf ("error while making GetNextTest dbus call:   %s\n", error->message);
	}

	return available;
}


void
log_message (const char* test_name, const char* message)
{
	DBusGProxy* dbus_proxy;
	DBusGConnection* connection;
	GError* error = NULL;  

	error = NULL;
	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (!connection) {
		g_warning ("Failed to open connection to bus to log message %s\n", error->message);
		g_error_free (error);
	}

	dbus_proxy = dbus_g_proxy_new_for_name (connection,
			DRT_SERVICE,
			DRT_LOGGER_PATH,
			DRT_LOGGER_INTERFACE);

	dbus_g_proxy_call_no_reply (dbus_proxy, "Log", G_TYPE_STRING, test_name, G_TYPE_STRING, "Error", G_TYPE_STRING, message, G_TYPE_INVALID);
}


static void
agviewer_add_signal_handler ()
{
	struct sigaction sa;

	sa.sa_handler = agviewer_handle_native_sigsegv;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;

	g_assert (sigaction (SIGSEGV, &sa, NULL) != -1);
}

/*
 * agviewer_handle_native_sigsegv: 
 *   (this is a slightly modified version of mono_handle_native_sigsegv from mono/mono/mini/mini-exceptions.c)
 *
 */
 
static bool handling_sigsegv = false;

typedef void void_method ();

void
agviewer_handle_native_sigsegv (int signal)
{
	const char *signal_str = (signal == SIGSEGV) ? "SIGSEGV" : "SIGABRT";
	
	if (handling_sigsegv) {
		fprintf (stderr, "\nGot a SIGSEGV while executing the SIGSEGV handler, aborting.\n");
		abort ();
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
	
	abort ();
}
