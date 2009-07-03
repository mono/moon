/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "downloader.h"
#include "runtime.h"
#include "xaml.h"
#include "canvas.h"
#include "window-gtk.h"
#include "frameworkelement.h"
#include "asf.h"
#include "pipeline.h"

static bool
get_video_size (const char *filename, int *width, int *height)
{
	MediaResult tmp;
	FileSource *src = NULL;
	ASFParser *parser = NULL;
	bool result = false;
	const asf_stream_properties* stream_properties;
	const asf_video_stream_data* video_data;
	const BITMAPINFOHEADER* bmp;
	
	src = new FileSource (NULL, filename);
	if (!MEDIA_SUCCEEDED (tmp = src->Initialize ())) {
		fprintf (stderr, "mopen1: Error while opening the file %s\n", filename);
		goto cleanup;
	}

	parser = new ASFParser (src, NULL);
	if (!MEDIA_SUCCEEDED (tmp = parser->ReadHeader ())) {
		fprintf (stderr, "mopen1: Error while reading asf header in file %s\n", filename);
		goto cleanup;
	}

	for (int i = 0; i <= 127; i++) {
		if (!parser->IsValidStream (i))
			continue;
		
		stream_properties = parser->GetStream (i);

		if (!stream_properties->is_video ())
			continue;

 		video_data = stream_properties->get_video_data ();
 		
 		if (!video_data)
 			continue;

		bmp = video_data->get_bitmap_info_header ();
		if (!bmp)
			continue;


		*width = bmp->image_width;
		*height = bmp->image_height;
		result = true;
		goto cleanup;
	}

cleanup:
	if (parser)
		parser->unref ();
	if (src)
		src->unref ();
	return result;
}

static void 
Help (void)
{
	printf ("Usage is: mopen1 [args] [file.xaml]\n\n"
			   "Arguments are:\n"
			   "   --media <filename>  Automatically creates some xaml with a MediaElement whose source is set to the filename\n"
			   "                       This is also automatically assumed if 1 filename is passed and the extension is either\n"
			   "                       wmv, wma, vc1, asf or mp3.\n"
			   "   --timeout T         Time, in seconds, before closing the window\n"
			   );
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
			downloader->NotifyFailed (msg);
			return;
		}

		struct stat sb;
		fstat (fd, &sb);
		close (fd);
		this->uri = g_strdup (uri);
		size = sb.st_size;
		downloader->NotifySize (size);
	}

	void Send () {
		if (uri != NULL)
			downloader->NotifyFinished (uri);
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
downloader_open (gpointer state, const char *verb, const char *uri, bool streaming, bool disable_cache)
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
downloader_body (gpointer state, void *body, guint32 length)
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

	runtime_init_desktop();

	char* dir =  g_path_get_dirname (file);
	chdir (dir);
	g_free (dir);
//	printf ("nopen: %s\n", strerror (errno));

	file = g_basename (file);

	Downloader::SetFunctions (downloader_create_state,
				  downloader_destroy_state,
				  downloader_open,
				  downloader_send,
				  downloader_abort,
				  downloader_header,
				  downloader_body,
				  downloader_request,
				  NULL,
				  NULL);

	Type::Kind et;

	MoonWindowGtk *moon_window = new MoonWindowGtk (false, 300, 300);
	Surface* surface = new Surface (moon_window);
	XamlLoader* loader = new XamlLoader (file, NULL, surface);
	DependencyObject* dob = loader->CreateDependencyObjectFromFile (file, FALSE, &et);

	delete loader;

	if (dob == NULL) {
		printf ("nopen::LoadXaml ('%s'): Could not create xaml from the file.\n", file);
		result = 1;
	} else if (dob->Is (Type::CANVAS)) {

		Canvas* ui = (Canvas*) dob;
		surface->Attach ((Canvas*) ui);
		ui->unref ();

		int width = ui->GetWidth ();
		int height = ui->GetHeight ();

		if (width < 100)
			width = 100;
		if (height < 100)
			height = 100;

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

	surface->Zombify ();
	surface->unref ();

	runtime_shutdown ();

	printf ("Shutting down...\n");

	return result;
}

gboolean
QuitTimeout (gpointer data)
{
	gtk_main_quit ();
	return false;
}


int
main (int argc, char *argv [])
{
	int result = 0;
	const char *media_extensions [] = { ".wmv", ".wma", ".vc1", ".asf", ".mp3", NULL };
	const char *media_filename = NULL;
	const char *filename = NULL;
	const char *tmpfile = NULL;
	int media_width = 0;
	int media_height = 0;
	int timeout = 0;

	// quick out if no arguments are provided
	if (argc == 0) {
		Help ();
		return 1;
	}
	
	gtk_init (&argc, &argv);
	g_thread_init (NULL);
	gdk_threads_init ();

	
	for (int i = 1; i < argc; i++) {
		if (!strcmp (argv [i], "--media")) {
			if (i + 1 >= argc) {
				Help ();
				return 1;
			}
			media_filename = argv [++i];
		} else if (!strcmp (argv [i], "--timeout")) {
			if (i + 1 >= argc) {
				Help ();
				return 1;
			}
			timeout = atoi (argv [++i]);
		} else {
			if (filename != NULL) {
				Help ();
				return 1;
			}
			filename = argv [i];
			for (int j = 0; media_extensions [j] != NULL && media_filename == NULL; j++) {
				if (g_str_has_suffix (filename, media_extensions [j]))
					media_filename = filename;
			}
		}
	}

	if (media_filename) {
		char *xaml;
		
		tmpfile = "mopen1.default.xaml";
		
		if (!get_video_size (media_filename, &media_width, &media_height)) {
			fprintf (stdout, "mopen1: Could not get video size, using 400x300.\n");
			media_width = 400;
			media_height = 300;
		}
		
		xaml = g_strdup_printf (
"<Canvas xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' xmlns:x='http://schemas.microsoft.com/winfx/2006/xaml' Width='%i' Height='%i'>\n"
"       <MediaElement x:Name='TestVideo' Width='%i' Height='%i' Source='%s'/>\n"
"</Canvas>\n", media_width, media_height, media_width, media_height, media_filename);

		if (!g_file_set_contents (tmpfile, xaml, -1, NULL)) {
			fprintf (stderr, "mopen1: Error while writing temporary file.\n");
			return 1;
		}

		g_free (xaml);
		filename = tmpfile;

	}

	if (timeout > 0) {
		g_timeout_add (timeout * 1000, QuitTimeout, NULL);
	}

		
	result = LoadXaml (filename);

	if (tmpfile)
		unlink (tmpfile);


	return result;
}
