#include <string.h>
#include <gtk/gtk.h>
#include <libgnomevfs/gnome-vfs.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include "runtime.h"
#include "transform.h"
#include "animation.h"
#include "shape.h"
#include "media.h"
#include "text.h"
#include "downloader.h"

// the downloader callbacks
static gpointer downloader_create_state (Downloader* dl);
static void downloader_destroy_state (gpointer data);
static void downloader_open (char *verb, char *uri, bool async, gpointer state);
static void downloader_send (gpointer state);
static void downloader_abort (gpointer state);
static void downloader_abort (gpointer state);
static char* downloader_get_response_text (char *part, gpointer state);

// our parser callbacks, which would be filled in with awesome .so
// hackery if I wasn't so tired.
DependencyObject *
_custom_element_callback (const char *xmlns, const char *name)
{
	return NULL;
}

void
_custom_attribute_callback (void *target, const char *name, const char *value)
{
}

void
_event_callback (void *target, const char *ename, const char *evalue)
{
}

static GtkWidget *w;
static int do_fps = FALSE;
static uint64_t last_time;
static GnomeVFSURI *xaml_uri = NULL;

static gboolean
invalidator (gpointer data)
{
	Surface *s = (Surface *) data;
	int64_t now = get_now ();
	int64_t diff = now - last_time;
	
	if (diff > 1000000) {
		float seconds = diff / 1000000.0;
		last_time = now;
		char *res = g_strdup_printf ("%.2f fps", s->frames / seconds);

		gtk_window_set_title (GTK_WINDOW (w), res);
		g_free (res);
		s->frames = 0;
	}

	return TRUE;
}

static gboolean
delete_event (GtkWidget *widget, GdkEvent *e, gpointer data)
{
	gtk_main_quit ();
	return 1;
}

static void
usage()
{
	printf ("usage is: gnomoon [-fps] uri-to-xaml\n");
	exit (0);
}

int
main(int argc, char **argv)
{
	gtk_init (&argc, &argv);
	g_thread_init (NULL);
	gdk_threads_init ();
	gnome_vfs_init ();
	runtime_init ();

	downloader_set_functions (downloader_create_state,
				  downloader_destroy_state,
				  downloader_open,
				  downloader_send,
				  downloader_abort,
				  downloader_get_response_text);


	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	Surface *t = surface_new (600, 600);
	gtk_signal_connect (GTK_OBJECT (w), "delete_event", G_CALLBACK (delete_event), t);
	gtk_container_add (GTK_CONTAINER(w), t->drawing_area);
		
	for (int i = 1; i < argc; i++){
		if (!strcmp (argv [i], "-h")) {
			usage ();
		}
		else if (strcmp (argv [i], "-fps")== 0) {
			do_fps = TRUE;
		}
		else
			xaml_uri = gnome_vfs_uri_new (argv [i]);
	}
	if (xaml_uri == NULL)
		usage();

	Type::Kind kind;

	gtk_window_set_title (GTK_WINDOW (w),
			      gnome_vfs_uri_to_string (xaml_uri, (GnomeVFSURIHideOptions)(GNOME_VFS_URI_HIDE_USER_NAME | GNOME_VFS_URI_HIDE_PASSWORD)));

	/* load the uri synchronously to a tmp file and start the process going */
	char tmp_file[] = "/tmp/gnomoon.XXXXXX";
	int fd = mkstemp (tmp_file);

	GnomeVFSHandle *handle;
	gnome_vfs_open_uri (&handle, xaml_uri, GNOME_VFS_OPEN_READ);
	GnomeVFSResult res;
	char buf[8192];

	do {
		GnomeVFSFileSize nread;
		res = gnome_vfs_read (handle, buf, sizeof (buf), &nread);
		write (fd, buf, nread);
	} while (res == GNOME_VFS_OK);

	close (fd);


	/* parse the file, which in turn starts the image downloads if
	   there are any, using our gnomevfs downloader. */
	UIElement *e = xaml_create_from_file (tmp_file, true, _custom_element_callback, _custom_attribute_callback, _event_callback, &kind);

	unlink (tmp_file);

	if (e == NULL){
		printf ("Was not able to load the uri\n");
		return 1;
	}
	if (kind != Type::CANVAS){
		printf ("Currently we only support Canvas toplevel elements\n");
		return 1;
	}

	surface_attach (t, e);

	if (do_fps){
		t->frames = 0;
		last_time = get_now ();
		gtk_timeout_add (1000, invalidator, t);
	}

	gtk_widget_set_usize (w, 600, 400);
	gtk_widget_show_all (w);
	gtk_main ();
	
	surface_destroy (t);
	runtime_shutdown ();
	
	return 0;

}

// our gnome-vfs file loader

class GnomeVFSDownloader {
 public:
	GnomeVFSDownloader (Downloader *dl) : downloader(dl), handle (NULL) { }

	virtual ~GnomeVFSDownloader () { Close (); }

	void Send() { }

	void Abort () {
		if (handle) {
		  gnome_vfs_async_cancel (handle);
		  handle = NULL;
		}
	}

	char* GetResponseText (char* PartName) { return NULL; } // XXX

	void Open (char *verb, char *uri, bool async)
	{
		GnomeVFSURI *relative_uri = gnome_vfs_uri_resolve_relative (xaml_uri, uri);
		printf ("relative uri = %s\n", gnome_vfs_uri_to_string (relative_uri, GNOME_VFS_URI_HIDE_NONE));
		gnome_vfs_async_open_uri (&handle, relative_uri,
					  GNOME_VFS_OPEN_READ, GNOME_VFS_PRIORITY_DEFAULT,
					  open_callback, this);
		gnome_vfs_uri_unref (relative_uri);
	}

	void Close ()
	{
		if (handle)
			handle = NULL;
	}

 private:
	GnomeVFSAsyncHandle *handle;
	Downloader *downloader;
	guchar buf[8192];

	static void open_callback (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer callback_data)
	{
		((GnomeVFSDownloader*)callback_data)->OpenResult (handle, result);
	}

	void OpenResult (GnomeVFSAsyncHandle *, GnomeVFSResult result)
	{
		if (result != GNOME_VFS_OK) {
			printf ("OpenResult error: %s\n", gnome_vfs_result_to_string (result));
			return;
		}

		// we should really have a step where we determine the
		// filesize (if possible), and call
		// downloader_notify_size.

		gnome_vfs_async_read (handle, buf, sizeof (buf), read_callback, this);
	}

	static void read_callback (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer buffer,
			    GnomeVFSFileSize bytes_requested,
			    GnomeVFSFileSize bytes_read,
			    gpointer callback_data)
	{
		((GnomeVFSDownloader*)callback_data)->ReadResult (handle, result, buffer, bytes_requested, bytes_read);
	}

	void ReadResult (GnomeVFSAsyncHandle *handle, GnomeVFSResult result, gpointer buffer,
			 GnomeVFSFileSize bytes_requested,
			 GnomeVFSFileSize bytes_read)
	{
	  	if (result == GNOME_VFS_OK ||
		    result == GNOME_VFS_ERROR_EOF) {
			downloader_write (downloader, (guchar*)buffer, 0, bytes_read); 
			if (result == GNOME_VFS_ERROR_EOF) {
				downloader_notify_finished (downloader);
				handle = NULL;
			}
			else
				gnome_vfs_async_read (handle, buf, sizeof (buf), read_callback, this);
		}
		else {
			/* what do we do with errors? */
		}
	  
	}

};

static gpointer
downloader_create_state (Downloader *dl)
{
	return new GnomeVFSDownloader (dl);
}

static void
downloader_destroy_state (gpointer data)
{
	delete ((GnomeVFSDownloader*)data);
}

static void
downloader_open (char *verb, char *uri, bool async, gpointer state)
{
	((GnomeVFSDownloader*)state)->Open (verb, uri, async);
}

static void
downloader_send (gpointer state)
{
	((GnomeVFSDownloader*)state)->Send ();
}

static void
downloader_abort (gpointer state)
{
	((GnomeVFSDownloader*)state)->Abort ();
}

static char*
downloader_get_response_text (char *part, gpointer state)
{
	return ((GnomeVFSDownloader*)state)->GetResponseText (part);
}

