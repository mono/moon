#include <gtk/gtk.h>
#include <libmoon.h>
#include <cairo.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define TEST_WIDTH 400
#define TEST_HEIGHT 400

char *testdir;

static gpointer downloader_create_state (Downloader* dl);
static void downloader_destroy_state (gpointer data);
static void downloader_open (const char *verb, const char *uri, gpointer state);
static void downloader_send (gpointer state);
static void downloader_abort (gpointer state);
static void downloader_abort (gpointer state);

class FileDownloadState {
 public:
	FileDownloadState (Downloader *dl) : uri(NULL), downloader(dl) { }

	virtual ~FileDownloadState () { Close (); }
	size_t size;
	char *uri;

	void Abort () { Close (); }
	char* GetResponseText (char *fname, char* PartName) { return NULL; } // XXX
	void Open (const char *verb, const char *URI)
	{
		uri = g_strdup (URI);

		int fd = open (uri, O_RDONLY);
		if (fd == -1) {
			if (!g_path_is_absolute (uri)) {
				g_free (uri);
				uri = g_strconcat (testdir, G_DIR_SEPARATOR_S, URI, NULL);
				fd = open (uri, O_RDONLY);
			}
		  
			if (fd == -1) {
				const char *msg = g_strerror (errno);
				printf ("downloader failed to open %s: %s\n", URI, msg);
				downloader_notify_error (downloader, msg);
				return;
			}
		}

		struct stat sb;
		fstat (fd, &sb);
		close (fd);
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
downloader_open (const char *verb, const char *uri, gpointer state)
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


void
runTest (const char *xaml_file, const char *output_prefix)
{
	Surface *s = new Surface (TEST_WIDTH, TEST_HEIGHT);
	GdkPixbuf *pixbuf;
	Type::Kind type;

	DependencyObject *dob = xaml_create_from_file (xaml_file, true, &type);

	s->Attach ((UIElement*)dob);

	cairo_surface_t* surf = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
							    s->GetWidth (), s->GetHeight ());

	cairo_t *cr = cairo_create (surf);

	s->Paint (cr, 0, 0, s->GetWidth(), s->GetHeight());

	cairo_surface_write_to_png (surf, output_prefix);

	cairo_surface_destroy (surf);
	cairo_destroy (cr);

	delete s;

	pixbuf = gdk_pixbuf_new_from_file (output_prefix, NULL);
	unlink (output_prefix);
	gdk_pixbuf_save (pixbuf, output_prefix, "png", NULL, NULL);
	g_object_unref (pixbuf);
}

int
main (int argc, char **argv)
{
	bool gen_expected = false;

	gtk_init (&argc, &argv);
	g_thread_init (NULL);
	gdk_threads_init ();
	runtime_init ();

	downloader_set_functions (downloader_create_state,
				  downloader_destroy_state,
				  downloader_open,
				  downloader_send,
				  downloader_abort);
	
	char *test = argv[1];
	char *png = argv[2];

	testdir = g_path_get_dirname (test);
	if (!strcmp ("", testdir))
		testdir = g_get_current_dir();
	runTest (test, png);
}
