//
// mopen1.cpp: Moonlight Open Application
//
// Opens a XAML file
//
// Author:
//   Rolf Bjarne Kvinge (RKvinge@novell.com)
//
//
// See LICENSE file in the Moonlight distribution for licensing details
//
// TODO:
//    Implement everything mopen implements
//

#include <config.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "downloader.h"
#include "runtime.h"
#include "xaml.h"
#include "canvas.h"
#include "window-gtk.h"
#include "frameworkelement.h"

static void 
Help ()
{
	printf ("Usage is: mopen1 [args] [file.xaml]\n\n"
			   "Arguments are:\n"
			   "   (none supported yet)\n");
/*
	TODO: 
			   "   --desklet       Remove window decoration for desklets use\n" +
			   "   --fixed         Disable window resizing\n"  +
			   "   --geometry WxH  Overrides the geometry to be W x H pixels\n" +
			   "   --host NAME     Specifies that this file should be loaded in host NAME\n" +
			   "   --parseonly     Only parse (don't display) the XAML input\n" + 
			   "   --story N1[,Nx] Plays the storyboard name N1, N2, .. Nx when the clicked\n" +
			   "   -s,--stories    Automatically prepare to play all stories on click\n" + 
			   "   --sync          Make the gdk connection synchronous\n" +
			   "   --transparent   Transparent toplevel\n" +
			   "   --timeout T     Time, in seconds, before closing the window\n"
			   );
*/
}

class FileDownloadState {
 public:
	FileDownloadState (Downloader *dl) : uri(NULL), downloader(dl) { }

	virtual ~FileDownloadState () { Close (); }
	size_t size;
	char *uri;

	void Abort () { Close (); }
	char* GetResponseText (char *fname, char* PartName) { return NULL; } // XXX
	void Open (const char *verb, const char *uri)
	{
		int fd = open (uri, O_RDONLY);
		if (fd == -1) {
			const char *msg = g_strerror (errno);
			printf ("downloader failed to open %s: %s\n", uri, msg);
			downloader_notify_error (downloader, msg);
			return;
		}

		struct stat sb;
		fstat (fd, &sb);
		close (fd);
		this->uri = g_strdup (uri);
		size = sb.st_size;
		downloader_notify_size (downloader, size);
	}

	void Send () {
		if (uri != NULL)
			downloader_notify_finished (downloader, uri);
	}

	void Close ()
	{
		g_free (uri);
		uri = NULL;
	}
 private:
	Downloader *downloader;
};

static gpointer
downloader_create_state (Downloader *dl)
{
	return new FileDownloadState (dl);
}

static void
downloader_destroy_state (gpointer data)
{
	delete ((FileDownloadState*)data);
}

static void
downloader_open (const char *verb, const char *uri, bool streaming, gpointer state)
{
	((FileDownloadState*)state)->Open (verb, uri);
}

static void
downloader_send (gpointer state)
{
	((FileDownloadState*)state)->Send ();
}

static void
downloader_abort (gpointer state)
{
	((FileDownloadState*)state)->Abort ();
}

static void
downloader_header (gpointer state, const char *header, const char *value)
{
	g_assert_not_reached ();
}

static void
downloader_body (gpointer state, void *body, uint32_t length)
{
	g_assert_not_reached ();
}

static void
*downloader_request (const char *method, const char *uri, gpointer context)
{
	g_assert_not_reached ();
}

static gboolean
delete_event (GtkWidget *widget, GdkEvent *e, gpointer data)
{
	gtk_main_quit ();
	return 1;
}


static int LoadXaml (const char* file)
{
	int result = 0;
	runtime_init (RUNTIME_INIT_BROWSER);

	char* dir =  g_path_get_dirname (file);
	chdir (dir);
	g_free (dir);
//	printf ("nopen: %s\n", strerror (errno));

	file = g_basename (file);

	downloader_set_functions (downloader_create_state, downloader_destroy_state, downloader_open, downloader_send, downloader_abort, downloader_header, downloader_body, downloader_request);

	Type::Kind et;

	MoonWindowGtk *moon_window = new MoonWindowGtk (false, 300, 300);
	Surface* surface = surface_new (moon_window);
	XamlLoader* loader = xaml_loader_new (file, NULL, surface);
	DependencyObject* dob = xaml_create_from_file (loader, file, FALSE, &et);

	delete loader;

	if (dob == NULL) {
		printf ("nopen::LoadXaml ('%s'): Could not create xaml from the file.\n", file);
		result = 1;
	} else if (dob->Is (Type::CANVAS)) {

		Canvas* ui = (Canvas*) dob;
		surface->Attach ((Canvas*) ui);
		ui->unref ();

		int width = framework_element_get_width (ui);
		int height = framework_element_get_height (ui);

		if (width < 300)
			width = 300;
		if (height < 300)
			height = 300;

		surface->Resize (width, height);

		GtkWidget *window;

		window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_widget_set_app_paintable (window, TRUE);

		gtk_signal_connect (GTK_OBJECT (window), "delete-event", G_CALLBACK (delete_event), surface);
		gtk_container_add (GTK_CONTAINER(window), moon_window->GetWidget ());

		gtk_widget_set_usize (window, width, height);

		gtk_widget_show_all (window);

		gtk_main ();

	} else {
		printf ("nopen::LoadXaml ('%s'): didn't get an uielement from the xaml.\n", file);
		result = 1;
	}
	
	surface->unref ();

	runtime_shutdown ();

	printf ("Shutting down...\n");

	return result;
}

int
main (int argc, char *argv [])
{
	if (argc != 2) {
		Help ();
		return 1;
	}

	gtk_init (&argc, &argv);
	g_thread_init (NULL);
	gdk_threads_init ();

	// printf ("nopen::main (): loading '%s'.\n", argv [1]);

	return LoadXaml (argv [1]);
}
