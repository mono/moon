/*
 * media.cpp: 
 *
 * Author: Jeffrey Stedfast <fejj@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#define Visual _XVisual
#include <gdk/gdkx.h>
#include <cairo-xlib.h>
#undef Visual

#include "runtime.h"
#include "media.h"
#include "downloader.h"

// still too ugly to be exposed in the header files ;-)
cairo_pattern_t *image_brush_create_pattern (cairo_t *cairo, cairo_surface_t *surface, int sw, int sh, double opacity);
void image_brush_compute_pattern_matrix (cairo_matrix_t *matrix, double width, double height, int sw, int sh, 
					 Stretch stretch, AlignmentX align_x, AlignmentY align_y, Transform *transform);


// MediaBase

DependencyProperty *MediaBase::SourceProperty;
DependencyProperty *MediaBase::StretchProperty;

MediaBase::MediaBase ()
{
}

MediaBase *
media_base_new (void)
{
	return new MediaBase ();
}

char *
media_base_get_source (MediaBase *media)
{
	Value *value = media->GetValue (MediaBase::SourceProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
media_base_set_source (MediaBase *media, const char *value)
{
	media->SetValue (MediaBase::SourceProperty, Value (value));
}

Stretch
media_base_get_stretch (MediaBase *media)
{
	return (Stretch) media->GetValue (MediaBase::StretchProperty)->AsInt32 ();
}

void
media_base_set_stretch (MediaBase *media, Stretch value)
{
	media->SetValue (MediaBase::StretchProperty, Value (value));
}



// MediaElement

DependencyProperty *MediaElement::AttributesProperty;
DependencyProperty *MediaElement::AudioStreamCountProperty;
DependencyProperty *MediaElement::AudioStreamIndexProperty;
DependencyProperty *MediaElement::AutoPlayProperty;
DependencyProperty *MediaElement::BalanceProperty;
DependencyProperty *MediaElement::BufferingProgressProperty;
DependencyProperty *MediaElement::BufferingTimeProperty;
DependencyProperty *MediaElement::CanPauseProperty;
DependencyProperty *MediaElement::CanSeekProperty;
DependencyProperty *MediaElement::CurrentStateProperty;
DependencyProperty *MediaElement::DownloadProgressProperty;
DependencyProperty *MediaElement::IsMutedProperty;
DependencyProperty *MediaElement::MarkersProperty;
DependencyProperty *MediaElement::NaturalDurationProperty;
DependencyProperty *MediaElement::NaturalVideoHeightProperty;
DependencyProperty *MediaElement::NaturalVideoWidthProperty;
DependencyProperty *MediaElement::PositionProperty;
DependencyProperty *MediaElement::VolumeProperty;

static gboolean
advance_frame (void *user_data)
{
	MediaElement *media = (MediaElement *) user_data;
	double opacity = media->GetTotalOpacity ();
	int64_t position;
	
	if (media->mplayer->AdvanceFrame () && opacity > 0.0f)
		media->Invalidate ();
	
	media->updating = true;
	if ((position = media->mplayer->Position ()) < 0)
		position = 0;
	media_element_set_position (media, position);
	media->updating = false;
	
	// FIXME: need to disconnect the timeout if the video is complete
	//        and set the CurrentState to "Stopped"?
	
	return true;
}

MediaElement::MediaElement ()
{
	mplayer = new MediaPlayer ();
	mplayer->SetBalance (media_element_get_balance (this));
	mplayer->SetVolume (media_element_get_volume (this));
	
	updating = false;
	timeout_id = 0;
	
	BufferingProgressChangedEvent = RegisterEvent ("BufferingProgressChanged");
	CurrentStateChangedEvent = RegisterEvent ("CurrentStateChanged");
	DownloadProgressChangedEvent = RegisterEvent ("DownloadProgressChanged");
	MarkerReachedEvent = RegisterEvent ("MarkerReached");
	MediaEndedEvent = RegisterEvent ("MediaEnded");
	MediaFailedEvent = RegisterEvent ("MediaFailed");
	MediaOpenedEvent = RegisterEvent ("MediaOpened");
	
	downloader = NULL;
	part_name = NULL;
}

void
MediaElement::StopLoader ()
{
	if (downloader) {
		downloader_abort (downloader);
		downloader->unref ();
		downloader = NULL;
	}
}

MediaElement::~MediaElement ()
{
	if (timeout_id != 0)
		g_source_remove (timeout_id);
	
	g_free (part_name);
	StopLoader ();
	
	delete mplayer;
}

void
MediaElement::ComputeBounds ()
{
	double x1, y1, x2, y2;
	cairo_t *cr = measuring_context_create ();
	
	cairo_save (cr);
	cairo_set_matrix (cr, &absolute_xform);
	cairo_rectangle (cr, 0, 0, mplayer->width, mplayer->height);
	// XXX this next call will hopefully become unnecessary in a
	// later version of cairo.
	cairo_identity_matrix (cr);
	cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);
	cairo_restore (cr);
	
	bounds = Rect (x1 - 1, y1 - 1, x2-x1 + 2, y2-y1 + 2);
	
	measuring_context_destroy (cr);
}

Point
MediaElement::GetTransformOrigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();
	return Point (user_xform_origin.x * mplayer->width,
		      user_xform_origin.y * mplayer->height);
}

void
MediaElement::Render (cairo_t *cr, int x, int y, int width, int height)
{
	Stretch stretch = media_base_get_stretch (this);
	double h = framework_element_get_height (this);
	double w = framework_element_get_width (this);
	double opacity = GetTotalOpacity ();
	cairo_surface_t *surface;
	cairo_pattern_t *pattern;
	cairo_matrix_t matrix;
	
	if (!(surface = mplayer->GetSurface ()))
		return;
	
	if (w == 0.0 && h == 0.0) {
		h = (double) mplayer->height;
		w = (double) mplayer->width;
	}
	
	cairo_save (cr);
	cairo_set_matrix (cr, &absolute_xform);
	
	pattern = image_brush_create_pattern (cr, surface, mplayer->width, mplayer->height, opacity);
	image_brush_compute_pattern_matrix (&matrix, w, h, mplayer->width, mplayer->height, stretch, 
					    AlignmentXCenter, AlignmentYCenter, NULL);
	cairo_pattern_set_matrix (pattern, &matrix);
	
	cairo_set_source (cr, pattern);
	cairo_pattern_destroy (pattern);
	
	cairo_new_path (cr);
	cairo_rectangle (cr, 0, 0, w, h);
	cairo_close_path (cr);
	
	cairo_fill (cr);
	cairo_restore (cr);
}

void
MediaElement::UpdateProgress ()
{
	double progress = downloader->GetValue (DownloadProgressProperty)->AsDouble ();
	
	SetValue (MediaElement::DownloadProgressProperty, Value (progress));
}

void 
MediaElement::DataWrite (guchar *buf, gsize offset, gsize count)
{
	UpdateProgress ();
}

void 
MediaElement::data_write (guchar *buf, gsize offset, gsize count, gpointer data)
{
	((MediaElement *) data)->DataWrite (buf, offset, count);
}

void
MediaElement::size_notify (int64_t size, gpointer data)
{
	// Do something with it?
	// if size == -1, we do not know the size of the file, can happen
	// if the server does not return a Content-Length header
	printf ("The file size is %lld\n", size);
}

void
MediaElement::downloader_complete (EventObject *sender, gpointer calldata, gpointer closure)
{
	((MediaElement *) closure)->DownloaderComplete ();
}

void
MediaElement::DownloaderComplete ()
{
	char *filename = downloader_get_response_file (downloader, part_name);
	bool autoplay = media_element_get_auto_play (this);
	
	printf ("video source changed to `%s'\n", filename);
	
	// FIXME: specify which audio stream index the player should use
	
	if (mplayer->Open (filename)) {
		printf ("video succesfully opened\n");
		
		media_element_set_can_seek (this, mplayer->CanSeek ());
		media_element_set_can_pause (this, mplayer->CanPause ());
		media_element_set_audio_stream_count (this, mplayer->GetAudioStreamCount ());
		media_element_set_natural_duration (this, Duration (mplayer->Duration ()));
		media_element_set_natural_video_height (this, mplayer->height);
		media_element_set_natural_video_width (this, mplayer->width);
		
		media_element_set_current_state (this, "Buffering");
	} else {
		media_element_set_current_state (this, "Error");
		printf ("video failed to open\n");
	}
	
	Invalidate ();
	
	// FIXME: if the Source finishes downloading before the xaml
	// parser gets to the AutoPlay="False" property, then we start
	// autoplaying (we need to somehow wait until all properties
	// are read?)
	
	if (autoplay)
		Play ();
}

void
MediaElement::SetSource (DependencyObject *dl, const char *PartName)
{
	g_return_if_fail (dl->GetObjectType () == Type::DOWNLOADER);
	
	// if we have something opened already...
	media_element_set_current_state (this, "Closed");
	
	dl->ref ();
	if (downloader)
		downloader->unref ();
	downloader = (Downloader *) dl;
	part_name = g_strdup (PartName);
	
	media_element_set_current_state (this, "Opening");
	media_element_set_current_state (this, "Buffering");
	
	Invalidate ();
	
	downloader->AddHandler (downloader->CompletedEvent, downloader_complete, this);
	if (downloader->Started () || downloader->Completed ()) {
		if (downloader->Completed ())
			DownloaderComplete ();
		
		UpdateProgress ();
	} else {
		downloader->SetWriteFunc (data_write, size_notify, this);
		
		// This is what actually triggers the download
		downloader->Send ();
	}
}

void
MediaElement::Pause ()
{
	if (!mplayer->CanPause ())
		return;
	
	mplayer->Pause ();
	
	if (timeout_id != 0) {
		g_source_remove (timeout_id);
		timeout_id = 0;
	}
	
	media_element_set_current_state (this, "Paused");
}

void
MediaElement::Play ()
{
	if (downloader && downloader->Completed () && timeout_id == 0 && !mplayer->IsPlaying ()) {
		timeout_id = mplayer->Play (advance_frame, this);
		media_element_set_current_state (this, "Playing");
		printf ("video playing, timeout_id = %d\n", timeout_id);
	}
}

void
MediaElement::Stop ()
{
	if (!mplayer->IsPlaying () && !mplayer->IsPaused ())
		return;
	
	mplayer->Stop ();
	
	if (timeout_id != 0) {
		g_source_remove (timeout_id);
		timeout_id = 0;
	}
	
	media_element_set_current_state (this, "Stopped");
}

Value *
MediaElement::GetValue (DependencyProperty *prop)
{
	if (prop == MediaElement::PositionProperty) {
		int64_t position = mplayer->Position ();
		Value v = Value (position, Type::TIMESPAN);
		
		updating = true;
		SetValue (prop, &v);
		updating = false;
	}
	
	return MediaBase::GetValue (prop);
}

void
MediaElement::SetValue (DependencyProperty *prop, Value value)
{
	MediaBase::SetValue (prop, value);
}

void
MediaElement::SetValue (DependencyProperty *prop, Value *value)
{
	Value v;
	
	if (prop == MediaElement::PositionProperty && !updating) {
		Duration *duration = media_element_get_natural_duration (this);
		TimeSpan position = value->AsTimeSpan ();
		
		if (duration->HasTimeSpan () && position > duration->GetTimeSpan ())
			position = duration->GetTimeSpan ();
		else if (position < 0)
			position = 0;
		
		mplayer->Seek (position);
		
		if (position != value->AsTimeSpan ()) {
			v = Value (position);
			MediaBase::SetValue (prop, &v);
			return;
		}
	}
	
	MediaBase::SetValue (prop, value);
}

void
MediaElement::OnPropertyChanged (DependencyProperty *prop)
{
	bool invalidate = false;
	bool autoplay = false;
	
	if (prop == MediaBase::SourceProperty) {
		StopLoader ();
		
		char *uri = media_base_get_source (this);
		
		if (timeout_id != 0) {
			media_element_set_current_state (this, "Closed");
			g_source_remove (timeout_id);
			timeout_id = 0;
		}
		
		mplayer->Stop ();
		
		media_element_set_current_state (this, "Opening");
		
		Downloader *dl = new Downloader ();
		downloader_open (dl, "GET", uri);
		SetSource (dl, "");
	} else if (prop == MediaElement::AudioStreamCountProperty) {
		// read-only property
	} else if (prop == MediaElement::AudioStreamIndexProperty) {
		if (!updating) {
			// FIXME: set the audio stream index
		}
	} else if (prop == MediaElement::AutoPlayProperty) {
		// handled below
		autoplay = media_element_get_auto_play (this);
	} else if (prop == MediaElement::BalanceProperty) {
		mplayer->SetBalance (media_element_get_balance (this));
	} else if (prop == MediaElement::BufferingProgressProperty) {
		// read-only property
	} else if (prop == MediaElement::BufferingTimeProperty) {
		if (!updating) {
			// FIXME: set the buffering time
		}
	} else if (prop == MediaElement::CanPauseProperty) {
		// read-only property
	} else if (prop == MediaElement::CanSeekProperty) {
		// read-only property
	} else if (prop == MediaElement::CurrentStateProperty) {
		Emit (CurrentStateChangedEvent);
	} else if (prop == MediaElement::DownloadProgressProperty) {
		Emit (DownloadProgressChangedEvent);
	} else if (prop == MediaElement::IsMutedProperty) {
		bool muted = media_element_get_is_muted (this);
		if (!muted)
			mplayer->UnMute ();
		else
			mplayer->Mute ();
	} else if (prop == MediaElement::MarkersProperty) {
		if (!updating) {
			// FIXME: set the markers
		}
	} else if (prop == MediaElement::NaturalDurationProperty) {
		// read-only property
	} else if (prop == MediaElement::NaturalVideoHeightProperty) {
		// read-only property
	} else if (prop == MediaElement::NaturalVideoWidthProperty) {
		// read-only property
	} else if (prop == MediaElement::PositionProperty) {
		// handled elsewhere...
	} else if (prop == MediaElement::VolumeProperty) {
		mplayer->SetVolume (media_element_get_volume (this));
	}
	
	if (autoplay) {
		printf ("video autoplayed\n");
		Play ();
	}
	
	if (invalidate)
		Invalidate ();
	
	if (prop->type == Type::MEDIAELEMENT) {
		NotifyAttacheesOfPropertyChange (prop);
	} else {
		// propagate to parent class
		MediaBase::OnPropertyChanged (prop);
	}
}

MediaElement *
media_element_new (void)
{
	return new MediaElement ();
}

void
media_element_stop (MediaElement *media)
{
	media->Stop ();
}

void 
media_element_play (MediaElement *media)
{
	media->Play ();
}

void
media_element_pause (MediaElement *media)
{
	media->Pause ();
}

void
media_element_set_source (MediaElement *media, DependencyObject* Downloader, const char* PartName)
{
	media->SetSource (Downloader, PartName);
}

int
media_element_get_audio_stream_count (MediaElement *media)
{
	return (int) media->GetValue (MediaElement::AudioStreamCountProperty)->AsInt32 ();
}

void
media_element_set_audio_stream_count (MediaElement *media, int value)
{
	media->SetValue (MediaElement::AudioStreamCountProperty, Value (value));
}

int
media_element_get_audio_stream_index (MediaElement *media)
{
	int idx = (int) media->GetValue (MediaElement::AudioStreamIndexProperty)->AsInt32 ();
	
	return idx < 0 ? -1 : idx;
}

void
media_element_set_audio_stream_index (MediaElement *media, int value)
{
	if (value >= 0)
		media->SetValue (MediaElement::AudioStreamIndexProperty, Value (value));
	else
		media->SetValue (MediaElement::AudioStreamIndexProperty, Value (-1));
}

bool
media_element_get_auto_play (MediaElement *media)
{
	return (bool) media->GetValue (MediaElement::AutoPlayProperty)->AsBool ();
}

void
media_element_set_auto_play (MediaElement *media, bool value)
{
	media->SetValue (MediaElement::AutoPlayProperty, Value (value));
}

double
media_element_get_balance (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::BalanceProperty)->AsDouble ();
}

void
media_element_set_balance (MediaElement *media, double value)
{
	media->SetValue (MediaElement::BalanceProperty, Value (value));
}

double
media_element_get_buffering_progress (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::BufferingProgressProperty)->AsDouble ();
}

void
media_element_set_buffering_progress (MediaElement *media, double value)
{
	media->SetValue (MediaElement::BufferingProgressProperty, Value (value));
}

TimeSpan
media_element_get_buffering_time (MediaElement *media)
{
	return (TimeSpan) media->GetValue (MediaElement::BufferingTimeProperty)->AsTimeSpan ();
}

void
media_element_set_buffering_time (MediaElement *media, TimeSpan value)
{
	media->SetValue (MediaElement::BufferingTimeProperty, Value (value, Type::TIMESPAN));
}

bool
media_element_get_can_pause (MediaElement *media)
{
	return (bool) media->GetValue (MediaElement::CanPauseProperty)->AsBool ();
}

void
media_element_set_can_pause (MediaElement *media, bool value)
{
	media->SetValue (MediaElement::CanPauseProperty, Value (value));
}

bool
media_element_get_can_seek (MediaElement *media)
{
	return (bool) media->GetValue (MediaElement::CanSeekProperty)->AsBool ();
}

void
media_element_set_can_seek (MediaElement *media, bool value)
{
	media->SetValue (MediaElement::CanSeekProperty, Value (value));
}

char *
media_element_get_current_state (MediaElement *media)
{
	Value *value = media->GetValue (MediaElement::CurrentStateProperty);
	
	return value ? (char *) value->AsString () : NULL;
}

void
media_element_set_current_state (MediaElement *media, const char *value)
{
	media->SetValue (MediaElement::CurrentStateProperty, Value (value));
}

double
media_element_get_download_progress (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::DownloadProgressProperty)->AsDouble ();
}

void
media_element_set_download_progress (MediaElement *media, double value)
{
	media->SetValue (MediaElement::DownloadProgressProperty, Value (value));
}

bool
media_element_get_is_muted (MediaElement *media)
{
	return (bool) media->GetValue (MediaElement::IsMutedProperty)->AsBool ();
}

void
media_element_set_is_muted (MediaElement *media, bool value)
{
	media->SetValue (MediaElement::IsMutedProperty, Value (value));
}

TimelineMarkerCollection *
media_element_get_markers (MediaElement *media)
{
	Value *value = media->GetValue (MediaElement::MarkersProperty);
	
	return value ? (TimelineMarkerCollection *) value->AsTimelineMarkerCollection () : NULL;
}

void
media_element_set_markers (MediaElement *media, TimelineMarkerCollection *value)
{
	media->SetValue (MediaElement::MarkersProperty, Value (value));
}

Duration*
media_element_get_natural_duration (MediaElement *media)
{
	return media->GetValue (MediaElement::NaturalDurationProperty)->AsDuration ();
}

void
media_element_set_natural_duration (MediaElement *media, Duration value)
{
	media->SetValue (MediaElement::NaturalDurationProperty, Value (value));
}

double
media_element_get_natural_video_height (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::NaturalVideoHeightProperty)->AsDouble ();
}

void
media_element_set_natural_video_height (MediaElement *media, double value)
{
	media->SetValue (MediaElement::NaturalVideoHeightProperty, Value (value));
}

double
media_element_get_natural_video_width (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::NaturalVideoWidthProperty)->AsDouble ();
}

void
media_element_set_natural_video_width (MediaElement *media, double value)
{
	media->SetValue (MediaElement::NaturalVideoWidthProperty, Value (value));
}

TimeSpan
media_element_get_position (MediaElement *media)
{
	return (TimeSpan) media->GetValue (MediaElement::PositionProperty)->AsTimeSpan ();
}

void
media_element_set_position (MediaElement *media, TimeSpan value)
{
	media->SetValue (MediaElement::PositionProperty, Value (value, Type::TIMESPAN));
}

double
media_element_get_volume (MediaElement *media)
{
	return (double) media->GetValue (MediaElement::VolumeProperty)->AsDouble ();
}

void
media_element_set_volume (MediaElement *media, double value)
{
	media->SetValue (MediaElement::VolumeProperty, Value (value));
}

//
// Image
//
GHashTable* Image::surface_cache = NULL;
DependencyProperty* Image::DownloadProgressProperty;

Image::Image ()
  : brush (NULL),
    create_xlib_surface (true),
    downloader (NULL),
    surface (NULL),
    part_name(NULL),
    pattern (NULL),
    pattern_opacity (1.0)
{
}

Image::~Image ()
{
	StopLoader ();
	CleanupSurface ();
	g_free (part_name);
}

void
Image::StopLoader ()
{
	if (downloader){
		downloader_abort (downloader);
		downloader->unref ();
		downloader = NULL;
	}
}


void
Image::CleanupPattern ()
{
	if (pattern) {
		cairo_pattern_destroy (pattern);
		pattern = NULL;
	}
	pattern_opacity = 1.0;
}

void
Image::CleanupSurface ()
{
	CleanupPattern ();

	if (surface) {
		surface->ref_cnt --;
		if (surface->ref_cnt == 0) {
			g_hash_table_remove (surface_cache, surface->fname);
			g_free (surface->fname);
			cairo_surface_destroy (surface->cairo);
			g_object_unref (surface->backing_pixbuf);
			g_free (surface);
		}

		surface = NULL;
	}
}

void
Image::UpdateProgress ()
{
	double progress = downloader->GetValue (DownloadProgressProperty)->AsDouble ();

	SetValue (Image::DownloadProgressProperty, Value (progress));
}

void
Image::SetSource (DependencyObject *dl, const char* PartName)
{
	g_return_if_fail (dl->GetObjectType() == Type::DOWNLOADER);

	dl->ref ();

	if (downloader)
		downloader->unref ();

	part_name = g_strdup (PartName);

	CleanupSurface ();
	Invalidate (); 

	downloader = (Downloader*) dl;

	downloader->AddHandler (downloader->CompletedEvent, downloader_complete, this);

	if (downloader->Started () || downloader->Completed ()) {
		if (downloader->Completed ())
			DownloaderComplete ();
		
		UpdateProgress ();
	} else {
		downloader->SetWriteFunc (pixbuf_write, size_notify, this);
		
		// This is what actually triggers the download
		downloader->Send ();
	}
}

void
Image::PixbufWrite (guchar *buf, gsize offset, gsize count)
{
	UpdateProgress ();
}

void
Image::DownloaderComplete ()
{
	char *file = downloader_get_response_file (downloader, part_name);
	CreateSurface (file);
	g_free (file);

	if (GetValueNoDefault (FrameworkElement::WidthProperty) == NULL)
		SetValue (FrameworkElement::WidthProperty, (double) surface->width);

	if (GetValueNoDefault (FrameworkElement::HeightProperty) == NULL)
		SetValue (FrameworkElement::HeightProperty, (double) surface->height);

	if (brush)
		brush->OnPropertyChanged (ImageBrush::DownloadProgressProperty);
	else 
		Invalidate ();
}

#ifdef WORDS_BIGENDIAN
#define set_pixel_bgra(pixel,index,b,g,r,a) do { \
		((unsigned char *)(pixel))[index]   = a; \
		((unsigned char *)(pixel))[index+1] = r; \
		((unsigned char *)(pixel))[index+2] = g; \
		((unsigned char *)(pixel))[index+3] = b; \
	} while (0)
#else
#define set_pixel_bgra(pixel,index,b,g,r,a) do { \
		((unsigned char *)(pixel))[index]   = b; \
		((unsigned char *)(pixel))[index+1] = g; \
		((unsigned char *)(pixel))[index+2] = r; \
		((unsigned char *)(pixel))[index+3] = a; \
	} while (0)
#endif
#define get_pixel_bgra(color, b, g, r, a) do { \
		a = ((color & 0xff000000) >> 24); \
		r = ((color & 0x00ff0000) >> 16); \
		g = ((color & 0x0000ff00) >> 8); \
		b = (color & 0x000000ff); \
	} while(0)
#define get_pixel_rgba(color, b, g, r, a) do { \
		a = ((color & 0xff000000) >> 24); \
		b = ((color & 0x00ff0000) >> 16); \
		g = ((color & 0x0000ff00) >> 8); \
		r = (color & 0x000000ff); \
	} while(0)

#include "alpha-premul-table.inc"


void
Image::CreateSurface (const char *fname)
{
	if (surface) {
		printf ("surface already created..\n");
		return;
	}

	CleanupPattern ();

	if (!surface_cache)
		surface_cache = g_hash_table_new (g_str_hash, g_str_equal);

	surface = (CachedSurface*)g_hash_table_lookup (surface_cache, fname);
	if (surface) {
		surface->ref_cnt ++;
	}
	else {
		GError *error = NULL;

		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (fname, &error);
		fprintf (stderr, "TODO/WARNING: We need to register Image callbacks to raise events in managed land\n");
		if (!pixbuf){
			printf ("Failed to load image %s\n", fname);
			return;
		}

		surface = g_new0 (CachedSurface, 1);

		surface->ref_cnt = 1;
		surface->fname = g_strdup (fname);
		surface->height = gdk_pixbuf_get_height (pixbuf);
		surface->width = gdk_pixbuf_get_width (pixbuf);

		if (gdk_pixbuf_get_n_channels (pixbuf) == 4) {
			g_object_ref (pixbuf);
		} else {
			/* gdk-pixbuf packs its pixel data into 24 bits for
			   rgb, instead of 32 with an unused byte for alpha,
			   like cairo expects */

			GdkPixbuf *pb = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, surface->width, surface->height);
			gdk_pixbuf_copy_area (pixbuf,
					      0, 0, surface->width, surface->height,
					      pb,
					      0, 0);
			pixbuf = pb;
		}

		guchar *pb_pixels = gdk_pixbuf_get_pixels (pixbuf);
		guchar *p;
		for (int y = 0; y < surface->height; y ++) {
			p = pb_pixels + y * gdk_pixbuf_get_rowstride (pixbuf);
			for (int x = 0; x < surface->width; x ++) {
				guint32 color = *(guint32*)p;
				guchar r, g, b, a;

				get_pixel_rgba (color, r, g, b, a);

				/* pre-multipled alpha */
				if (a == 0) {
					r = g = b = 0;
				}
				else if (a < 255) {
					r = pre_multiplied_table [r][a];
					g = pre_multiplied_table [b][a];
					b = pre_multiplied_table [g][a];
				}

				/* store it back, swapping red and blue */
				set_pixel_bgra (p, 0, r, g, b, a);

				p += 4;
			}
		}

		surface->backing_pixbuf = pixbuf;
		surface->cairo = cairo_image_surface_create_for_data (pb_pixels,
								      CAIRO_FORMAT_ARGB32,
								      surface->width,
								      surface->height,
								      gdk_pixbuf_get_rowstride (pixbuf));

		g_hash_table_insert (surface_cache, surface->fname, surface);
	}
}

void
Image::size_notify (int64_t size, gpointer data)
{
	// Do something with it?
	// if size == -1, we do not know the size of the file, can happen
	// if the server does not return a Content-Length header
	printf ("The file size is %lld\n", size);
}

void
Image::downloader_complete (EventObject *sender, gpointer calldata, gpointer closure)
{
	((Image*)closure)->DownloaderComplete ();
}

void
Image::pixbuf_write (guchar *buf, gsize offset, gsize count, gpointer data)
{
	((Image*)data)->PixbufWrite (buf, offset, count);
}

void
Image::Render (cairo_t *cr, int, int, int, int)
{
	if (!surface)
		return;

	if (create_xlib_surface && !surface->xlib_surface_created) {
		surface->xlib_surface_created = true;

		cairo_surface_t *xlib_surface = cairo_surface_create_similar (cairo_get_target (cr),
									      CAIRO_CONTENT_COLOR_ALPHA,
									      surface->width, surface->height);
		cairo_t *cr = cairo_create (xlib_surface);
		cairo_set_source_surface (cr, surface->cairo, 0, 0);
		cairo_rectangle (cr, 0, 0, surface->width, surface->height);
		cairo_fill (cr);
		cairo_destroy (cr);
		cairo_surface_destroy (surface->cairo);
		surface->cairo = xlib_surface;
	}

	cairo_save (cr);

	cairo_set_matrix (cr, &absolute_xform);

	Stretch stretch = media_base_get_stretch (this);

	double w = framework_element_get_width (this);
	double h = framework_element_get_height (this);

	double opacity = GetTotalOpacity ();
	if (!pattern || (pattern_opacity != opacity)) {
		if (pattern)
			cairo_pattern_destroy (pattern);
		pattern = image_brush_create_pattern (cr, surface->cairo, surface->width, surface->height, opacity);
		pattern_opacity = opacity;
	}

	cairo_matrix_t matrix;
	image_brush_compute_pattern_matrix (&matrix, w, h, surface->width, surface->height, stretch, 
		AlignmentXCenter, AlignmentYCenter, NULL);
	cairo_pattern_set_matrix (pattern, &matrix);

	cairo_set_source (cr, pattern);

	cairo_new_path (cr);
	cairo_rectangle (cr, 0, 0, w, h);
	cairo_close_path (cr);

	cairo_fill (cr);
	cairo_restore (cr);
}

void
Image::ComputeBounds ()
{
	double x1, y1, x2, y2;
	cairo_t* cr = measuring_context_create ();

	cairo_save (cr);
	cairo_set_matrix (cr, &absolute_xform);
	cairo_set_line_width (cr, 1.0);
	cairo_rectangle (cr, 0, 0, framework_element_get_width (this), framework_element_get_height (this));
	// XXX this next call will hopefully become unnecessary in a
	// later version of cairo.
	cairo_identity_matrix (cr);
	cairo_stroke_extents (cr, &x1, &y1, &x2, &y2);
	cairo_new_path (cr);
	cairo_restore (cr);
	
	bounds = Rect (x1 - 1, y1 - 1, x2-x1 + 2, y2-y1 + 2);

	measuring_context_destroy (cr);
}

Point
Image::GetTransformOrigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();

	return Point (framework_element_get_width (this) * user_xform_origin.x, 
		      framework_element_get_height (this) * user_xform_origin.y);
}

cairo_surface_t *
Image::GetCairoSurface ()
{
	return surface ? surface->cairo : NULL;
}

void
Image::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop == MediaBase::SourceProperty) {
		StopLoader ();

		char *source = media_base_get_source (this);
		
		Downloader *dl = new Downloader ();
		downloader_open (dl, "GET", source);
		SetSource (dl, "");
	}

	if (prop->type != Type::IMAGE) {
		MediaBase::OnPropertyChanged (prop);
		return;
	}

	// we need to notify attachees if our DownloadProgress changed.
	NotifyAttacheesOfPropertyChange (prop);
}

Image *
image_new (void)
{
	return new Image ();
}

void
image_set_download_progress (Image *img, double progress)
{
	img->SetValue (Image::DownloadProgressProperty, Value(progress));
}

double
image_get_download_progress (Image *img)
{
	return img->GetValue (Image::DownloadProgressProperty)->AsDouble();
}

void
image_set_source (Image *img, DependencyObject *Downloader, const char *PartName)
{
	img->SetSource (Downloader, PartName);
}

//
// MediaAttribute
//

DependencyProperty *MediaAttribute::ValueProperty;

MediaAttribute *
media_attribute_new (void)
{
	return new MediaAttribute ();
}


void
media_init (void)
{
	/* MediaAttribute */
	MediaAttribute::ValueProperty = DependencyObject::Register (Type::MEDIAATTRIBUTE, "Value", new Value (""));
	
	/* MediaBase */
	MediaBase::SourceProperty = DependencyObject::Register (Type::MEDIABASE, "Source", Type::STRING);
	MediaBase::StretchProperty = DependencyObject::Register (Type::MEDIABASE, "Stretch", new Value (StretchUniform));
	
	/* Image */
	Image::DownloadProgressProperty = DependencyObject::Register (Type::IMAGE, "DownloadProgress", new Value (0.0));
	
	/* MediaElement */
	MediaElement::AttributesProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Attributes", Type::MEDIAATTRIBUTE_COLLECTION);
	MediaElement::AudioStreamCountProperty = DependencyObject::Register (Type::MEDIAELEMENT, "AudioStreamCount", new Value (0));
	MediaElement::AudioStreamIndexProperty = DependencyObject::Register (Type::MEDIAELEMENT, "AudioStreamIndex", new Value (-1));
	MediaElement::AutoPlayProperty = DependencyObject::Register (Type::MEDIAELEMENT, "AutoPlay", new Value (true));
	MediaElement::BalanceProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Balance", new Value (0.0));
	MediaElement::BufferingProgressProperty = DependencyObject::Register (Type::MEDIAELEMENT, "BufferingProgress", new Value (0.0));
	MediaElement::BufferingTimeProperty = DependencyObject::Register (Type::MEDIAELEMENT, "BufferingTime", Type::TIMESPAN);
	MediaElement::CanPauseProperty = DependencyObject::Register (Type::MEDIAELEMENT, "CanPause", new Value (true));
	MediaElement::CanSeekProperty = DependencyObject::Register (Type::MEDIAELEMENT, "CanSeek", new Value (false));
	MediaElement::CurrentStateProperty = DependencyObject::Register (Type::MEDIAELEMENT, "CurrentState", Type::STRING);
	MediaElement::DownloadProgressProperty = DependencyObject::Register (Type::MEDIAELEMENT, "DownloadProgress", new Value (0.0));
	MediaElement::IsMutedProperty = DependencyObject::Register (Type::MEDIAELEMENT, "IsMuted", new Value (false));
	MediaElement::MarkersProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Markers", Type::TIMELINEMARKER_COLLECTION);
	MediaElement::NaturalDurationProperty = DependencyObject::Register (Type::MEDIAELEMENT, "NaturalDuration", new Value(Duration::Automatic));
	MediaElement::NaturalVideoHeightProperty = DependencyObject::Register (Type::MEDIAELEMENT, "NaturalVideoHeight", Type::DOUBLE);
	MediaElement::NaturalVideoWidthProperty = DependencyObject::Register (Type::MEDIAELEMENT, "NaturalVideoWidth", Type::DOUBLE);
	MediaElement::PositionProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Position", Type::TIMESPAN);
	MediaElement::VolumeProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Volume", new Value (0.5));
}
