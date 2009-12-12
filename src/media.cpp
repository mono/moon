/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * media.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

#include "writeablebitmap.h"
#include "bitmapimage.h"
#include "uri.h"
#include "runtime.h"
#include "media.h"
#include "error.h"
#include "downloader.h"
#include "geometry.h"
#include "timeline.h"
#include "debug.h"

/*
 * MediaBase
 */

MediaBase::MediaBase ()
{
	SetObjectType (Type::MEDIABASE);

	source.downloader = NULL;
	source.part_name = NULL;
	source.queued = false;
	downloader = NULL;
	part_name = NULL;
	allow_downloads = false;
	source_changed = false;
}

MediaBase::~MediaBase ()
{
	DownloaderAbort ();
}

void
MediaBase::downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	MediaBase *media = (MediaBase *) closure;
	
	media->DownloaderComplete ();
}

void
MediaBase::downloader_failed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	MediaBase *media = (MediaBase *) closure;
	
	media->DownloaderFailed (calldata);
}

void
MediaBase::DownloaderComplete ()
{
	// Nothing for MediaBase to do...
}

void
MediaBase::DownloaderFailed (EventArgs *args)
{
	// Nothing for MediaBase to do...
}

void
MediaBase::DownloaderAbort ()
{
	if (downloader) {
		downloader->RemoveHandler (Downloader::DownloadFailedEvent, downloader_failed, this);
		downloader->RemoveHandler (Downloader::CompletedEvent, downloader_complete, this);
		downloader->SetStreamFunctions (NULL, NULL, NULL);
		downloader->Abort ();
		downloader->unref ();
		g_free (part_name);
		downloader = NULL;
		part_name = NULL;
	}
}

void
MediaBase::SetAllowDownloads (bool allow)
{
	Surface *surface = GetSurface ();
	const char *uri;
	Downloader *dl;
	
	if ((allow_downloads && allow) || (!allow_downloads && !allow))
		return;
	
	if (allow && surface && source_changed) {
		source_changed = false;
		
		if ((uri = GetSource ()) && *uri) {
			if (!(dl = surface->CreateDownloader ())) {
				// we're shutting down
				return;
			}
			
			dl->Open ("GET", uri, GetDownloaderPolicy (uri));
			SetSource (dl, "");
			dl->unref ();
		}
	}
	
	allow_downloads = allow;
}

void
MediaBase::OnLoaded ()
{
	FrameworkElement::OnLoaded ();
	SetAllowDownloads (true);
}

void
MediaBase::SetSourceAsyncCallback ()
{
	Downloader *downloader;
	char *part_name;

	DownloaderAbort ();

	downloader = source.downloader;
	part_name = source.part_name;

	source.queued = false;
	source.downloader = NULL;
	source.part_name = NULL;
	
	if (GetSurface () == NULL)
		return;
	
	SetSourceInternal (downloader, part_name);
	
	if (downloader)
		downloader->unref ();
}

void
MediaBase::SetSourceInternal (Downloader *downloader, char *PartName)
{
	this->downloader = downloader;
	part_name = PartName;
	
	if (downloader)
		downloader->ref ();
}

void
MediaBase::set_source_async (EventObject *user_data)
{
	MediaBase *media = (MediaBase *) user_data;
	
	media->SetSourceAsyncCallback ();
}

void
MediaBase::SetSource (Downloader *downloader, const char *PartName)
{
	source_changed = false;
	
	if (source.queued) {
		if (source.downloader)
			source.downloader->unref ();

		g_free (source.part_name);
		source.downloader = NULL;
		source.part_name = NULL;
	}
	
	source.part_name = g_strdup (PartName);
	source.downloader = downloader;
	
	if (downloader)
		downloader->ref ();

	if (source.downloader && source.downloader->Completed ()) {
		SetSourceInternal (source.downloader, source.part_name);
		source.downloader->unref ();
	} else if (!source.queued) {
		AddTickCall (MediaBase::set_source_async);
		source.queued = true;
	}
}

void
MediaBase::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetId () == MediaBase::SourceProperty) {
		const char *uri = args->GetNewValue() ? args->GetNewValue()->AsString () : NULL;
		Surface *surface = GetSurface ();
		
		if (surface && AllowDownloads ()) {
			if (uri && *uri) {
				Downloader *dl;
				if ((dl = surface->CreateDownloader ())) {
					dl->Open ("GET", uri, GetDownloaderPolicy (uri));
					SetSource (dl, "");
					dl->unref ();
				} else {
					// we're shutting down
				}
			} else {
				SetSource (NULL, NULL);
			}
		} else {
			source_changed = true;
		}
	}
	
	if (args->GetProperty ()->GetOwnerType() != Type::MEDIABASE) {
		FrameworkElement::OnPropertyChanged (args, error);
		return;
	}
	
	NotifyListenersOfPropertyChange (args, error);
}

//
// Image
//
Image::Image ()
{
	SetObjectType (Type::IMAGE);
}

Image::~Image ()
{
	BitmapSource *source = (BitmapSource*)GetSource ();

	if (source)
		source->RemoveHandler (BitmapSource::PixelDataChangedEvent, source_pixel_data_changed, this);
}

void
Image::download_progress (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	Image *media = (Image *) closure;

	media->DownloadProgress ();
}

void
Image::image_opened (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	Image *media = (Image *) closure;

	media->ImageOpened ((RoutedEventArgs*)calldata);
}

void
Image::image_failed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	Image *media = (Image *) closure;

	media->ImageFailed ((ImageErrorEventArgs*)calldata);
}

void
Image::source_pixel_data_changed (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	Image *media = (Image *) closure;

	media->SourcePixelDataChanged ();
}

void
Image::DownloadProgress ()
{
	BitmapImage *source = (BitmapImage *) GetSource ();

	SetDownloadProgress (source->GetProgress ());
	Emit (DownloadProgressChangedEvent);
}

void
Image::ImageOpened (RoutedEventArgs *args)
{
	BitmapSource *source = (BitmapSource*)GetSource ();

	if (source->Is (Type::BITMAPIMAGE)) {
		source->RemoveHandler (BitmapImage::DownloadProgressEvent, download_progress, this);
		source->RemoveHandler (BitmapImage::ImageOpenedEvent, image_opened, this);
		source->RemoveHandler (BitmapImage::ImageFailedEvent, image_failed, this);
	}

	InvalidateArrange ();
	InvalidateMeasure ();
	UpdateBounds ();
	Invalidate ();

	args->ref (); // to counter the unref in Emit
	Emit (ImageOpenedEvent, args);
}

void
Image::ImageFailed (ImageErrorEventArgs *args)
{
	BitmapSource *source = (BitmapSource*) GetSource ();

	if (source->Is (Type::BITMAPIMAGE)) {
		source->RemoveHandler (BitmapImage::DownloadProgressEvent, download_progress, this);
		source->RemoveHandler (BitmapImage::ImageOpenedEvent, image_opened, this);
		source->RemoveHandler (BitmapImage::ImageFailedEvent, image_failed, this);
	}
	source->RemoveHandler (BitmapSource::PixelDataChangedEvent, source_pixel_data_changed, this);


	InvalidateArrange ();
	InvalidateMeasure ();
	UpdateBounds ();
	Invalidate ();

	args->ref (); // to counter the unref in Emit
	Emit (ImageFailedEvent, args);
}

void
Image::SourcePixelDataChanged ()
{
	Invalidate();
}

void
Image::SetSourceInternal (Downloader *downloader, char *PartName)
{
	BitmapImage *source = (BitmapImage *) GetSource ();

	MediaBase::SetSourceInternal (downloader, PartName);

	source->AddHandler (BitmapImage::DownloadProgressEvent, download_progress, this);
	source->AddHandler (BitmapImage::ImageOpenedEvent, image_opened, this);
	source->AddHandler (BitmapImage::ImageFailedEvent, image_failed, this);

	source->SetDownloader (downloader, NULL, PartName);
}

void
Image::SetSource (Downloader *downloader, const char *PartName)
{
	MediaBase::SetSource (downloader, PartName);
}

void
Image::Render (cairo_t *cr, Region *region, bool path_only)
{
	ImageSource *source = GetSource ();
	cairo_pattern_t *pattern = NULL;
	cairo_matrix_t matrix;
	
	if (!source)
		return;

	source->Lock ();

	cairo_save (cr);
	cairo_set_matrix (cr, &absolute_xform);
       
	Size specified (GetActualWidth (), GetActualHeight ());
	Size stretched = ApplySizeConstraints (specified);
	bool adjust = specified != GetRenderSize ();

	if (GetStretch () != StretchUniformToFill)
		specified = specified.Min (stretched);

	Rect paint = Rect (0, 0, specified.width, specified.height);
	
	if (!path_only) {
		Rect image = Rect (0, 0, source->GetPixelWidth (), source->GetPixelHeight ());

		if (GetStretch () == StretchNone)
			paint = paint.Union (image);

		if (image.width == 0.0 && image.height == 0.0)
			return;

		pattern = cairo_pattern_create_for_surface (source->GetSurface (cr));
		image_brush_compute_pattern_matrix (&matrix, paint.width, paint.height, 
						    image.width, image.height,
						    GetStretch (), 
						    AlignmentXCenter, AlignmentYCenter, NULL, NULL);
		
		cairo_pattern_set_matrix (pattern, &matrix);
#if MAKE_EVERYTHING_SLOW_AND_BUGGY
		cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);
#endif
		if (cairo_pattern_status (pattern) == CAIRO_STATUS_SUCCESS) {
			cairo_set_source (cr, pattern);
		}
		cairo_pattern_destroy (pattern);
	}

	if (adjust) {
		specified = MeasureOverride (specified);
		paint = Rect ((stretched.width - specified.width) * 0.5, (stretched.height - specified.height) * 0.5, specified.width, specified.height);
	}
	
	if (!path_only)
		RenderLayoutClip (cr);

	paint = paint.Intersection (Rect (0, 0, stretched.width, stretched.height));
	paint.Draw (cr);
	
	if (!path_only)
		cairo_fill (cr);

	cairo_restore (cr);

	source->Unlock ();
}

Size
Image::ComputeActualSize ()
{
	Size result = MediaBase::ComputeActualSize ();
	UIElement *parent = GetVisualParent ();
	ImageSource *source = GetSource ();
	
	if (parent && !parent->Is (Type::CANVAS))
		if (LayoutInformation::GetLayoutSlot (this))
			return result;
		
	if (source && source->GetSurface (NULL)) {
		Size available = Size (INFINITY, INFINITY);
		available = ApplySizeConstraints (available);
		result = MeasureOverride (available);
		result = ApplySizeConstraints (result);
	}

	return result;
}

Size
Image::MeasureOverride (Size availableSize)
{
	Size desired = availableSize;
	Rect shape_bounds = Rect ();
	ImageSource *source = GetSource ();
	double sx = 0.0;
	double sy = 0.0;

	if (source)
		shape_bounds = Rect (0,0,source->GetPixelWidth (),source->GetPixelHeight ());

	/* don't stretch to infinite size */
	if (isinf (desired.width))
		desired.width = shape_bounds.width;
	if (isinf (desired.height))
		desired.height = shape_bounds.height;
	
	/* compute the scaling */
	if (shape_bounds.width > 0)
		sx = desired.width / shape_bounds.width;
	if (shape_bounds.height > 0)
		sy = desired.height / shape_bounds.height;

	/* don't use infinite dimensions as constraints */
	if (isinf (availableSize.width))
		sx = sy;
	if (isinf (availableSize.height))
		sy = sx;

	switch (GetStretch ()) {
	case StretchUniform:
		sx = sy = MIN (sx, sy);
		break;
	case StretchUniformToFill:
		sx = sy = MAX (sx, sy);
		break;
	case StretchFill:
		if (isinf (availableSize.width))
			sx = sy;
		if (isinf (availableSize.height))
			sy = sx;
		break;
	case StretchNone:
		sx = sy = 1.0;
		break;
	}

	desired = Size (shape_bounds.width * sx, shape_bounds.height * sy);

	return desired;
}

Size
Image::ArrangeOverride (Size finalSize)
{
	Size arranged = finalSize;
	Rect shape_bounds = Rect ();
	ImageSource *source = GetSource ();
	double sx = 1.0;
	double sy = 1.0;


	if (source)
		shape_bounds = Rect (0, 0, source->GetPixelWidth (), source->GetPixelHeight ());

	/* compute the scaling */
	if (shape_bounds.width == 0)
		shape_bounds.width = arranged.width;
	if (shape_bounds.height == 0)
		shape_bounds.height = arranged.height;

	if (shape_bounds.width != arranged.width)
		sx = arranged.width / shape_bounds.width;
	if (shape_bounds.height != arranged.height)
		sy = arranged.height / shape_bounds.height;

	switch (GetStretch ()) {
	case StretchUniform:
		sx = sy = MIN (sx, sy);
		break;
	case StretchUniformToFill:
		sx = sy = MAX (sx, sy);
		break;
	case StretchNone:
		sx = sy = 1.0;
	default:
		break;
	}

	arranged = Size (shape_bounds.width * sx, shape_bounds.height * sy);

	return arranged;
}

Rect
Image::GetCoverageBounds ()
{
	Stretch stretch = GetStretch ();
	ImageSource *source = GetSource ();

	if (!source || source->GetPixelFormat () == PixelFormatPbgra32)
		return Rect ();

	if (stretch == StretchFill || stretch == StretchUniformToFill)
		return bounds;

	cairo_matrix_t matrix;
	Rect image = Rect (0, 0, source->GetPixelWidth (), source->GetPixelHeight ());
	Rect paint = Rect (0, 0, GetActualWidth (), GetActualHeight ());

	image_brush_compute_pattern_matrix (&matrix, 
					    paint.width, paint.height,
					    image.width, image.height, stretch, 
					    AlignmentXCenter, AlignmentYCenter, NULL, NULL);

	cairo_matrix_invert (&matrix);
	cairo_matrix_multiply (&matrix, &matrix, &absolute_xform);

	image = image.Transform (&matrix);
	image = image.Intersection (bounds);
	
	return image;
}

void
Image::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop && (prop->GetId () == Image::SourceProperty
		     || prop->GetId () == MediaBase::SourceProperty)) {
		InvalidateMeasure ();
		Invalidate ();
		return;
	}

	MediaBase::OnSubPropertyChanged (prop, obj, subobj_args);
}
void
Image::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetId () == Image::SourceProperty) {
		ImageSource *source = args->GetNewValue () ? args->GetNewValue ()->AsImageSource () : NULL; 
		ImageSource *old = args->GetOldValue () ? args->GetOldValue ()->AsImageSource () : NULL;

		if (old && old->Is(Type::BITMAPSOURCE)) {
			old->RemoveHandler (BitmapSource::PixelDataChangedEvent, source_pixel_data_changed, this);
		}
		if (source && source->Is(Type::BITMAPSOURCE)) {
			source->AddHandler (BitmapSource::PixelDataChangedEvent, source_pixel_data_changed, this);
		}

		if (old && old->Is(Type::BITMAPIMAGE)) {
			old->RemoveHandler (BitmapImage::DownloadProgressEvent, download_progress, this);
			old->RemoveHandler (BitmapImage::ImageOpenedEvent, image_opened, this);
			old->RemoveHandler (BitmapImage::ImageFailedEvent, image_failed, this);
		}
		if (source && source->Is(Type::BITMAPIMAGE)) {
			BitmapImage *bitmap = (BitmapImage *) source;
			Uri *uri = bitmap->GetUriSource ();
			
			source->AddHandler (BitmapImage::DownloadProgressEvent, download_progress, this);
			source->AddHandler (BitmapImage::ImageOpenedEvent, image_opened, this);
			source->AddHandler (BitmapImage::ImageFailedEvent, image_failed, this);
			
			if (bitmap->GetPixelWidth () > 0 && bitmap->GetPixelHeight () > 0) {
				RoutedEventArgs *args = new RoutedEventArgs ();
				ImageOpened (args);
				args->unref ();
			}
			
			// can uri ever be null?
			if (uri && GetSurface ()) {
				ImageErrorEventArgs *args = NULL;
				
				if (uri->IsInvalidPath ()) {
					args = new ImageErrorEventArgs (MoonError (MoonError::ARGUMENT_OUT_OF_RANGE, 0, "invalid path found in uri"));
				} else if (!bitmap->ValidateDownloadPolicy ()) {
					args = new ImageErrorEventArgs (MoonError (MoonError::ARGUMENT_OUT_OF_RANGE, 0, "Security Policy Violation"));
				}
				
				if (args != NULL) {
					source->RemoveHandler (BitmapImage::ImageFailedEvent, image_failed, this);
					GetSurface ()->EmitError (args);
				}
			}
		}
		InvalidateMeasure ();
	}

	if (args->GetProperty ()->GetOwnerType() != Type::IMAGE) {
		MediaBase::OnPropertyChanged (args, error);
		return;
	}
	
	// we need to notify attachees if our DownloadProgress changed.
	NotifyListenersOfPropertyChange (args, error);
}

bool
Image::InsideObject (cairo_t *cr, double x, double y)
{
	if (!FrameworkElement::InsideObject (cr, x, y))
		return false;

	cairo_save (cr);
	cairo_new_path (cr);
	cairo_set_matrix (cr, &absolute_xform);

	double nx = x;
	double ny = y;

	TransformPoint (&nx, &ny);

	Render (cr, NULL, true);
	bool inside = cairo_in_fill (cr, nx, ny);
	cairo_restore (cr);

	if (inside)
		inside = InsideLayoutClip (x, y);

	if (inside)
		inside = InsideClip (cr, x, y);

	return inside;
}

Value *
Image::CreateDefaultImageSource (DependencyObject *instance, DependencyProperty *property)
{
	return Value::CreateUnrefPtr (new BitmapImage ());
}

//
// MediaAttributeCollection
//

MediaAttribute *
MediaAttributeCollection::GetItemByName (const char *name)
{
	MediaAttribute *attr;
	const char *value;
	
	for (guint i = 0; i < array->len; i++) {
		attr = ((Value *) array->pdata[i])->AsMediaAttribute ();
		if (!(value = attr->GetName ()))
			continue;
		
		if (!g_ascii_strcasecmp (value, name))
			return attr;
	}
	
	return NULL;
}


//
// TimelineMarkerCollection
//

int
TimelineMarkerCollection::AddWithError (Value *value, MoonError *error)
{
	TimelineMarker *marker, *cur;
	
	marker = value->AsTimelineMarker ();
	
	for (guint i = 0; i < array->len; i++) {
		cur = ((Value *) array->pdata[i])->AsTimelineMarker ();
		if (cur->GetTime () >= marker->GetTime ()) {
			DependencyObjectCollection::InsertWithError (i, value, error);
			return i;
		}
	}
	
	return DependencyObjectCollection::InsertWithError (array->len, value, error) ? array->len - 1 : -1;
}

bool
TimelineMarkerCollection::InsertWithError (int index, Value *value, MoonError *error)
{
	return AddWithError (value, error) != -1;
}

