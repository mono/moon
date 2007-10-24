#include <gtk/gtk.h>
#include <libmoon.h>
#include <cairo.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "clock.h"

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

		if (g_str_has_prefix (uri, "file://"))
			uri += 7;

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
		size = sb.st_size;
		downloader_notify_size (downloader, size);
		close (fd);
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
strip_metadata (const char *png_filename)
{
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (png_filename, NULL);
	unlink (png_filename);
	gdk_pixbuf_save (pixbuf, png_filename, "png", NULL, NULL);
	g_object_unref (pixbuf);
}

void
fill_background (Surface *s, cairo_t *cr)
{
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);
	cairo_rectangle (cr, 0, 0, s->GetWidth(), s->GetHeight());
	cairo_paint (cr);
}

void
runTest (const char *xaml_file, const char *output_prefix, bool multiple, int delta, int max)
{
	Surface *s = new Surface (TEST_WIDTH, TEST_HEIGHT);
	Type::Kind type;

	XamlLoader* loader = new XamlLoader (xaml_file, NULL, s);
	Canvas *canvas = (Canvas*)xaml_create_from_file (loader, xaml_file, true, &type);
	delete loader;

	s->Attach (canvas);

	int width = TEST_WIDTH;
	int height = TEST_HEIGHT;

	if (framework_element_get_width (canvas) > 0 &&
	    framework_element_get_height (canvas) > 0) {

		width = (int)framework_element_get_width (canvas);
		height = (int)framework_element_get_height (canvas);
	}

	cairo_surface_t* surf = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
							    width, height);

	cairo_t * cr = cairo_create (surf);

	if (multiple) {
		TimeSpan delta_t = (TimeSpan)(TIMESPANTICKS_IN_SECOND_FLOAT / 1000 * delta);
		TimeSpan t = 0;

		for (int i = 0; i < max; i ++) {
			fill_background (s, cr);

			printf ("generating image for timestamp %lld\n", t);

			((ManualTimeSource*)TimeManager::Instance()->GetSource())->SetCurrentTime (t);

			s->Paint (cr, 0, 0, s->GetWidth(), s->GetHeight());

			char *timestamped_filename = g_strdup_printf ("%s-%lld", output_prefix, t);
			cairo_surface_write_to_png (surf, timestamped_filename);

			strip_metadata (timestamped_filename);
			g_free (timestamped_filename);

			t += delta_t;
		}
	}
	else {
		fill_background (s, cr);

		s->Paint (cr, 0, 0, s->GetWidth(), s->GetHeight());

		cairo_surface_write_to_png (surf, output_prefix);

		strip_metadata (output_prefix);
	}

	cairo_surface_destroy (surf);
	cairo_destroy (cr);

	delete s;
}

int
main (int argc, char **argv)
{
	if (argc < 3 || argc > 4) {
		printf ("usage:  xaml2png file.xaml output.png millis,max\n");
		exit (1);
	}

	bool gen_expected = false;

	gtk_init (&argc, &argv);
	g_thread_init (NULL);
	gdk_threads_init ();
	
	runtime_init (RUNTIME_INIT_BROWSER | RUNTIME_INIT_TIMESOURCE_MANUAL | RUNTIME_INIT_AUDIO_DISABLE);

	downloader_set_functions (downloader_create_state,
				  downloader_destroy_state,
				  downloader_open,
				  downloader_send,
				  downloader_abort);

	char *test = argv[1];
	char *png = argv[2];

	int millis = 0;
	int max = 0;
	bool multiple = false;

	if (argc == 4) {
		char *arg = g_strdup (argv[3]);
		char *comma;
		comma = strchr (arg, ',');
		*comma = '\0';
		millis = atoi (arg);
		max = atoi (comma + 1);
		multiple = true;

		printf ("generating %d images, one image every %d milliseconds\n", max, millis);
	}

	testdir = g_path_get_dirname (test);
	if (!strcmp ("", testdir))
		testdir = g_get_current_dir();
	runTest (test, png, multiple, millis, max);
}
