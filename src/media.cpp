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
#include "cutil.h"
#include "media.h"
#include "downloader.h"
#include "cutil.h"


// still too ugly to be exposed in the header files ;-)
cairo_pattern_t *image_brush_create_pattern (cairo_t *cairo, cairo_surface_t *surface, int sw, int sh, double opacity);
void image_brush_compute_pattern_matrix (cairo_matrix_t *matrix, double width, double height, int sw, int sh, 
					 Stretch stretch, AlignmentX align_x, AlignmentY align_y, Transform *transform);


// MediaBase

DependencyProperty *MediaBase::SourceProperty;
DependencyProperty *MediaBase::StretchProperty;

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
media_base_set_source (MediaBase *media, char *value)
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
DependencyProperty *MediaElement::AutoPlayProperty;
DependencyProperty *MediaElement::BalanceProperty;
DependencyProperty *MediaElement::BufferingProgressProperty;
DependencyProperty *MediaElement::BufferingTimeProperty;
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
	int64_t pos;
	
	if (media->mplayer->AdvanceFrame () && opacity > 0.0f)
		item_invalidate (media);
	
	pos = media->mplayer->Position ();
	media_element_set_position (media, pos);
	
	return true;
}

MediaElement::MediaElement ()
{
	mplayer = new MediaPlayer ();
	timeout_id = 0;
}

MediaElement::~MediaElement ()
{
	if (timeout_id != 0)
		g_source_remove (timeout_id);
	
	delete mplayer;
}

void
MediaElement::getbounds ()
{
	Surface *s = item_get_surface (this);
	
	if (s == NULL)
		return;
	
	cairo_save (s->cairo);
	cairo_set_matrix (s->cairo, &absolute_xform);
	cairo_rectangle (s->cairo, 0, 0, mplayer->width, mplayer->height);
	cairo_stroke_extents (s->cairo, &x1, &y1, &x2, &y2);
	cairo_restore (s->cairo);
	
	// The extents are in the coordinates of the transform, translate to device coordinates
	x_cairo_matrix_transform_bounding_box (&absolute_xform, &x1, &y1, &x2, &y2);
}

Point
MediaElement::getxformorigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();
	return Point (user_xform_origin.x * mplayer->width,
		      user_xform_origin.y * mplayer->height);
}

void
MediaElement::render (cairo_t *cr, int x, int y, int width, int height)
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
MediaElement::SetSource (DependencyObject *Downloader, char *PartName)
{
	// if we have something opened already...
	media_element_set_current_state (this, "Closed");
	
	media_element_set_current_state (this, "Opening");
	
	// FIXME: implement me
	
	media_element_set_current_state (this, "Buffering");
}

void
MediaElement::Pause ()
{
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
	if (timeout_id == 0 && !mplayer->IsPlaying ()) {
		timeout_id = mplayer->Play (advance_frame, this);
		media_element_set_current_state (this, "Playing");
	}
}

void
MediaElement::Stop ()
{
	mplayer->Stop ();
	
	if (timeout_id != 0) {
		g_source_remove (timeout_id);
		timeout_id = 0;
	}
	
	media_element_set_current_state (this, "Stopped");
}

void
MediaElement::OnPropertyChanged (DependencyProperty *prop)
{
	bool autoplay = media_element_get_auto_play (this);
	
	if (prop == MediaBase::SourceProperty) {
		char *uri = media_base_get_source (this);
		
		printf ("video source changed to `%s'\n", uri);
		
		if (timeout_id != 0) {
			media_element_set_current_state (this, "Closed");
			g_source_remove (timeout_id);
			timeout_id = 0;
		}
		
		mplayer->Stop ();
		
		media_element_set_current_state (this, "Opening");
		
		if (mplayer->Open (uri)) {
			printf ("video succesfully opened\n");
			media_element_set_natural_video_height (this, mplayer->height);
			media_element_set_natural_video_width (this, mplayer->width);
			media_element_set_current_state (this, "Buffering");
		} else {
			media_element_set_current_state (this, "Error");
			printf ("video failed to open\n");
		}
	} else if (prop == MediaElement::AutoPlayProperty) {
		// handled below
	} else if (prop == MediaElement::BalanceProperty) {
		// FIXME: implement me
		// audio balance?
		return;
	} else if (prop == MediaElement::BufferingProgressProperty) {
		// this can only be set by us, no-op
		return;
	} else if (prop == MediaElement::BufferingTimeProperty) {
		// FIXME: set amount of time to buffer
		return;
	} else if (prop == MediaElement::CanSeekProperty) {
		// this can only be set by us, no-op
		return;
	} else if (prop == MediaElement::CurrentStateProperty) {
		// FIXME: raise CurrentStateChanged event
		return;
	} else if (prop == MediaElement::DownloadProgressProperty) {
		// this can only be set by us, no-op
		return;
	} else if (prop == MediaElement::IsMutedProperty) {
		bool muted = media_element_get_is_muted (this);
		if (!muted)
			mplayer->UnMute ();
		else
			mplayer->Mute ();
		return;
	} else if (prop == MediaElement::MarkersProperty) {
		// FIXME: 
		return;
	} else if (prop == MediaElement::NaturalDurationProperty) {
		// this can only be set by us, no-op
		return;
	} else if (prop == MediaElement::NaturalVideoHeightProperty) {
		// this can only be set by us, no-op
		return;
	} else if (prop == MediaElement::NaturalVideoWidthProperty) {
		// this can only be set by us, no-op
		return;
	} else if (prop == MediaElement::PositionProperty) {
		// FIXME: needs to seek?
		NotifyAttacheesOfPropertyChange (prop);
		return;
	} else if (prop == MediaElement::VolumeProperty) {
		// FIXME: implement me
		return;
	}
	
	if (autoplay && timeout_id == 0 && !mplayer->IsPlaying ()) {
		timeout_id = mplayer->Play (advance_frame, this);
		printf ("video autoplayed, timeout = %d\n", timeout_id);
	}
	
	FullInvalidate (false);
	
	// propagate to parent class
	MediaBase::OnPropertyChanged (prop);
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
media_element_setsource (MediaElement *media, DependencyObject* Downloader, char* PartName)
{
	media->SetSource (Downloader, PartName);
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
media_element_set_current_state (MediaElement *media, char *value)
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

Duration *
media_element_get_natural_duration (MediaElement *media)
{
	return (Duration *) media->GetValue (MediaElement::NaturalDurationProperty)->AsDuration ();
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
DependencyProperty* Image::DownloadProgressProperty;

Image::Image ()
  : pixbuf_width (0),
    pixbuf_height (0),
    create_xlib_surface (true),
    pixbuf (NULL),
    loader (NULL),
    downloader (NULL),
    surface (NULL),
    brush (NULL),
    render_progressive (false),
    pattern (NULL),
    pattern_opacity (1.0)
{
}

Image::~Image ()
{
	StopLoader ();
	CleanupSurface ();
}

void
Image::StopLoader ()
{
	if (loader) {
		// disconnect all our signal handlers at once
		g_signal_handlers_disconnect_matched (loader,
						      (GSignalMatchType) G_SIGNAL_MATCH_DATA,
						      0, 0, NULL, NULL, this);
		gdk_pixbuf_loader_close (loader, NULL);
		g_object_unref (loader);
		loader = NULL;
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
	if (pixbuf) {
		g_object_unref (pixbuf);
		pixbuf = NULL;
	}

	CleanupPattern ();

	if (surface) {
		cairo_surface_destroy (surface);
		surface = NULL;
	}

	pixbuf_width =
	pixbuf_height = 0;
}

void
Image::UpdateProgress ()
{
	double progress = downloader->GetValue (DownloadProgressProperty)->AsDouble ();

	SetValue (Image::DownloadProgressProperty, Value (progress));
}

void
Image::SetSource (DependencyObject *dl, char* PartName)
{
	g_return_if_fail (dl->GetObjectType() == Type::DOWNLOADER);

	if (loader) {
		StopLoader ();
		CleanupSurface ();
		item_invalidate (this); /* so we erase the old image */
	}
	loader = gdk_pixbuf_loader_new ();

	g_signal_connect (loader, "size_prepared", G_CALLBACK(loader_size_prepared), this);
	g_signal_connect (loader, "area_prepared", G_CALLBACK(loader_area_prepared), this);
	g_signal_connect (loader, "area_updated", G_CALLBACK(loader_area_updated), this);

	downloader = (Downloader*) dl;
	downloader->ref ();

	downloader_want_events (downloader, downloader_event, this);

	if (downloader->Started ()) {
		// Load the existing data that has been downloaded

		PixbufWrite (downloader->byte_array_contents->data, 0, downloader->byte_array_contents->len);

		// If it was also finished, notify
		if (downloader->Completed ())
			DownloaderEvent (Downloader::NOTIFY_COMPLETED);
		
		UpdateProgress ();
	} else {
		downloader->SetWriteFunc (pixbuf_write, size_notify, this);
		downloader_open (downloader, "GET", PartName, true);

		// This is what actually triggers the download
		downloader_send (downloader);
	}
}

void
Image::PixbufWrite (guchar *buf, gsize offset, gsize count)
{
	gdk_pixbuf_loader_write (loader, buf + offset, count, NULL);
	UpdateProgress ();
}

void
Image::DownloaderEvent (int kind)
{
	if (kind == Downloader::NOTIFY_COMPLETED) {
		if (!render_progressive)
			CreateSurface ();

		if (GetValueNoDefault (FrameworkElement::WidthProperty) == NULL)
			SetValue (FrameworkElement::WidthProperty, (double) pixbuf_width);

		if (GetValueNoDefault (FrameworkElement::HeightProperty) == NULL)
			SetValue (FrameworkElement::HeightProperty, (double) pixbuf_height);

		if (brush)
			brush->OnPropertyChanged (ImageBrush::DownloadProgressProperty);
		else 
			FullInvalidate (true);
	}
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
Image::CreateSurface ()
{
	if (surface) {
		printf ("surface already created..\n");
		return;
	}

	CleanupPattern ();

	pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
	if (!pixbuf)
		return;

	if (gdk_pixbuf_get_n_channels (pixbuf) == 4) {
		g_object_ref (pixbuf);
	}
	else {
		/* gdk-pixbuf packs its pixel data into 24 bits for
		   rgb, instead of 32 with an unused byte for alpha,
		   like cairo expects */

		GdkPixbuf *pb = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, pixbuf_width, pixbuf_height);
		gdk_pixbuf_copy_area (pixbuf,
				      0, 0, pixbuf_width, pixbuf_height,
				      pb,
				      0, 0);
		pixbuf = pb;
	}

	guchar *pb_pixels = gdk_pixbuf_get_pixels (pixbuf);
	guchar *p;
	for (int y = 0; y < pixbuf_height; y ++) {
		p = pb_pixels + y * gdk_pixbuf_get_rowstride (pixbuf);
		for (int x = 0; x < pixbuf_width; x ++) {
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

	surface = cairo_image_surface_create_for_data (pb_pixels,
						       CAIRO_FORMAT_ARGB32,
						       pixbuf_width,
						       pixbuf_height,
						       gdk_pixbuf_get_rowstride (pixbuf));
}

void
Image::LoaderSizePrepared (int width, int height)
{
	printf ("image has size %dx%d\n", width, height);
	pixbuf_width = width;
	pixbuf_height = height;

#if PROGRESSIVE_IMAGE_LOADING
	if (render_progressive && !surface)
		CreateSurface ();
#endif

	item_update_bounds (this);
}

void
Image::LoaderAreaPrepared ()
{
#if PROGRESSIVE_IMAGE_LOADING
	/* the pixbuf can be retrieved from the loader, but it hasn't
	   been filled in yet.  create our surface data here since we
	   can ask the pixbuf what width/height/rowstride it will
	   be. */
#endif
}

void
Image::LoaderAreaUpdated (int x, int y, int width, int height)
{
#if PROGRESSIVE_IMAGE_LOADING
	if (!render_progressive)
	  return;


	item_invalidate (this);
#endif
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
Image::downloader_event (int kind, gpointer data)
{
	((Image*)data)->DownloaderEvent (kind);
}

void
Image::pixbuf_write (guchar *buf, gsize offset, gsize count, gpointer data)
{
	((Image*)data)->PixbufWrite (buf, offset, count);
}

void
Image::loader_size_prepared (GdkPixbufLoader *loader, int width, int height, gpointer data)
{
	((Image*)data)->LoaderSizePrepared (width, height);
}

void
Image::loader_area_prepared (GdkPixbufLoader *loader, gpointer data)
{
	((Image*)data)->LoaderAreaPrepared ();
}

void
Image::loader_area_updated (GdkPixbufLoader *loader, int x, int y, int width, int height, gpointer data)
{
	((Image*)data)->LoaderAreaUpdated (x, y, width, height);
}

void
Image::render (cairo_t *cr, int x, int y, int width, int height)
{
	if (!surface)
		return;

	if (create_xlib_surface) {
		create_xlib_surface = false;
		cairo_surface_t *xlib_surface = cairo_surface_create_similar (cairo_get_target (cr),
									      CAIRO_CONTENT_COLOR_ALPHA,
									      pixbuf_width, pixbuf_height);
		cairo_t *cr = cairo_create (xlib_surface);
		cairo_set_source_surface (cr, surface, 0, 0);
		cairo_rectangle (cr, 0, 0, pixbuf_width, pixbuf_height);
		cairo_fill (cr);
		cairo_destroy (cr);
		cairo_surface_destroy (surface);
		surface = xlib_surface;
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
		pattern = image_brush_create_pattern (cr, surface, pixbuf_width, pixbuf_height, opacity);
		pattern_opacity = opacity;
	}

	cairo_matrix_t matrix;
	image_brush_compute_pattern_matrix (&matrix, w, h, pixbuf_width, pixbuf_height, stretch, 
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
Image::getbounds ()
{
	Surface *s = item_get_surface (this);
	
	if (s == NULL)
		return;
	
	cairo_save (s->cairo);
	cairo_set_matrix (s->cairo, &absolute_xform);
	cairo_set_line_width (s->cairo, 1.0);
	cairo_rectangle (s->cairo, 0, 0, framework_element_get_width (this), framework_element_get_height (this));
	cairo_stroke_extents (s->cairo, &x1, &y1, &x2, &y2);
	cairo_new_path (s->cairo);
	cairo_restore (s->cairo);
	
	// The extents are in the coordinates of the transform, translate to device coordinates
	x_cairo_matrix_transform_bounding_box (&absolute_xform, &x1, &y1, &x2, &y2);
}

Point
Image::getxformorigin ()
{
	Point user_xform_origin = GetRenderTransformOrigin ();

	return Point (framework_element_get_width (this) * user_xform_origin.x, 
		      framework_element_get_height (this) * user_xform_origin.y);
}

cairo_surface_t *
Image::GetSurface ()
{
	return surface;
}

void
Image::OnPropertyChanged (DependencyProperty *prop)
{
	if (prop == MediaBase::SourceProperty) {
		if (downloader) {
			// we have a previously running download.  stop it.
			downloader_abort (downloader);
			downloader->unref ();
			downloader = NULL;
		}
		
		char *source = GetValue (prop)->AsString();
		
		printf ("setting image source to '%s'\n", source);
		SetSource (new Downloader (), source);
	}
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
image_set_source (Image *img, DependencyObject *Downloader, char *PartName)
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
	MediaElement::AutoPlayProperty = DependencyObject::Register (Type::MEDIAELEMENT, "AutoPlay", new Value (true));
	MediaElement::BalanceProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Balance", new Value (0.0));
	MediaElement::BufferingProgressProperty = DependencyObject::Register (Type::MEDIAELEMENT, "BufferingProgress", new Value (0.0));
	MediaElement::BufferingTimeProperty = DependencyObject::Register (Type::MEDIAELEMENT, "BufferingTime", Type::TIMESPAN);
	MediaElement::CanSeekProperty = DependencyObject::Register (Type::MEDIAELEMENT, "CanSeek", new Value (false));
	MediaElement::CurrentStateProperty = DependencyObject::Register (Type::MEDIAELEMENT, "CurrentState", Type::STRING);
	MediaElement::DownloadProgressProperty = DependencyObject::Register (Type::MEDIAELEMENT, "DownloadProgress", new Value (0.0));
	MediaElement::IsMutedProperty = DependencyObject::Register (Type::MEDIAELEMENT, "IsMuted", new Value (false));
	MediaElement::MarkersProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Markers", Type::TIMELINEMARKER_COLLECTION);
	MediaElement::NaturalDurationProperty = DependencyObject::Register (Type::MEDIAELEMENT, "NaturalDuration", Type::DURATION);
	MediaElement::NaturalVideoHeightProperty = DependencyObject::Register (Type::MEDIAELEMENT, "NaturalVideoHeight", Type::DOUBLE);
	MediaElement::NaturalVideoWidthProperty = DependencyObject::Register (Type::MEDIAELEMENT, "NaturalVideoWidth", Type::DOUBLE);
	MediaElement::PositionProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Position", Type::TIMESPAN);
	MediaElement::VolumeProperty = DependencyObject::Register (Type::MEDIAELEMENT, "Volume", new Value (0.5));
}
