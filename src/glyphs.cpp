/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * glyphs.cpp: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <config.h>

#include <cairo.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

#include "file-downloader.h"
#include "runtime.h"
#include "glyphs.h"
#include "utils.h"
#include "debug.h"
#include "uri.h"
#include "geometry.h"
#include "deployment.h"

#if DEBUG
#define d(x) x
#else
#define d(x)
#endif

#define ORIGIN_IS_SET(x) ((x) > -HUGE)

//
// Glyphs
//

enum GlyphAttrMask {
	Cluster = 1 << 1,
	Index   = 1 << 2,
	Advance = 1 << 3,
	uOffset = 1 << 4,
	vOffset = 1 << 5,
};

class GlyphAttr : public List::Node {
 public:
	guint32 glyph_count;
	guint32 code_units;
	guint32 index;
	double advance;
	double uoffset;
	double voffset;
	guint8 set;
	
	GlyphAttr ();
};

GlyphAttr::GlyphAttr ()
{
	glyph_count = 1;
	code_units = 1;
	set = 0;
}

Glyphs::Glyphs ()
{
	SetObjectType (Type::GLYPHS);
	
	downloader = NULL;
	
	fill = NULL;
	path = NULL;
	
	attrs = new List ();
	text = NULL;
	font = NULL;
	
	height = 0.0;
	width = 0.0;
	left = 0.0;
	top = 0.0;
	
	uri_changed = false;
	invalid = false;
	dirty = false;
}

Glyphs::~Glyphs ()
{
	CleanupDownloader ();
	
	if (path)
		moon_path_destroy (path);
	
	attrs->Clear (true);
	delete attrs;
	
	g_free (text);
	
	delete font;
}

void
Glyphs::CleanupDownloader ()
{
	if (downloader) {
		downloader->RemoveHandler (Downloader::CompletedEvent, downloader_complete, this);
		downloader->Abort ();
		downloader->unref ();
		downloader = NULL;
	}
}

void
Glyphs::Layout ()
{
	double size = GetFontRenderingEmSize ();
	guint32 code_units, glyph_count, i;
	bool first_char = true;
	double x0, x1, y0, y1;
	double bottom, right;
	double bottom0, top0;
	GlyphInfo *glyph;
	GlyphAttr *attr;
	int nglyphs = 0;
	double offset;
	bool cluster;
	double scale;
	int n = 0;
	
	invalid = false;
	dirty = false;
	
	height = 0.0;
	width = 0.0;
	left = 0.0;
	top = 0.0;
	
	if (path) {
		moon_path_destroy (path);
		path = NULL;
	}
	
	if (!font) {
		// required font fields have not been set
		return;
	}
	
	if (((!text || !text[0]) && attrs->IsEmpty ())) {
		// no glyphs to render
		return;
	}
	
	if (fill == NULL) {
		// no fill specified (unlike TextBlock, there is no default brush)
		return;
	}
	
	// scale Advance, uOffset and vOffset units to pixels
	scale = round (size) / 100.0;
	
	x0 = GetOriginX ();
	if (!ORIGIN_IS_SET (x0))
		x0 = 0.0;
	
	right = x0;
	left = x0;
	
	// OriginY is the baseline if specified
	y0 = GetOriginY ();
	if (ORIGIN_IS_SET (y0)) {
		top0 = y0 - font->Ascender ();
	} else {
		y0 = font->Ascender ();
		top0 = 0.0;
	}
	
	bottom0 = top0 + font->Height ();
	
	bottom = bottom0;
	top = top0;
	
	path = moon_path_new (16);
	
	attr = (GlyphAttr *) attrs->First ();
	
	if (text && text[0]) {
		gunichar *c = text;
		
		while (*c != 0) {
			if (attr && (attr->set & Cluster)) {
				// get the cluster's GlyphCount and CodeUnitCount
				glyph_count = attr->glyph_count;
				code_units = attr->code_units;
			} else {
				glyph_count = 1;
				code_units = 1;
			}
			
			if (glyph_count == 1 && code_units == 1)
				cluster = false;
			else
				cluster = true;
			
			// render the glyph cluster
			i = 0;
			do {
				if (attr && (attr->set & Index)) {
					if (!(glyph = font->GetGlyphInfoByIndex (attr->index)))
						goto next1;
				} else if (cluster) {
					// indexes MUST be specified for each glyph in a cluster
					moon_path_destroy (path);
					invalid = true;
					path = NULL;
					return;
				} else {
					if (!(glyph = font->GetGlyphInfo (*c)))
						goto next1;
				}
				
				y1 = y0;
				if (attr && (attr->set & vOffset)) {
					offset = -(attr->voffset * scale);
					bottom = MAX (bottom, bottom0 + offset);
					top = MIN (top, top0 + offset);
					y1 += offset;
				} 
				
				if (attr && (attr->set & uOffset)) {
					offset = (attr->uoffset * scale);
					left = MIN (left, x0 + offset);
					x1 = x0 + offset;
				} else if (first_char) {
					if (glyph->metrics.horiBearingX < 0)
						x0 -= glyph->metrics.horiBearingX;
					
					first_char = false;
					x1 = x0;
				} else {
					x1 = x0;
				}
				
				right = MAX (right, x1 + glyph->metrics.horiAdvance);
				
				font->AppendPath (path, glyph, x1, y1);
				nglyphs++;
				
				if (attr && (attr->set & Advance))
					x0 += attr->advance * scale;
				else
					x0 += glyph->metrics.horiAdvance;
				
			 next1:
				
				attr = attr ? (GlyphAttr *) attr->next : NULL;
				i++;
				
				if (i == glyph_count)
					break;
				
				if (!attr) {
					// there MUST be an attr for each glyph in a cluster
					moon_path_destroy (path);
					invalid = true;
					path = NULL;
					return;
				}
				
				if ((attr->set & Cluster)) {
					// only the first glyph in a cluster may specify a cluster mapping
					moon_path_destroy (path);
					invalid = true;
					path = NULL;
					return;
				}
			} while (true);
			
			// consume the code units
			for (i = 0; i < code_units && *c != 0; i++)
				c++;
			
			n++;
		}
	}
	
	while (attr) {
		if (attr->set & Cluster) {
			LOG_TEXT (stderr, "Can't use clusters past the end of the UnicodeString\n");
			moon_path_destroy (path);
			invalid = true;
			path = NULL;
			return;
		}
		
		if (!(attr->set & Index)) {
			LOG_TEXT (stderr, "No index specified for glyph %d\n", n + 1);
			moon_path_destroy (path);
			invalid = true;
			path = NULL;
			return;
		}
		
		if (!(glyph = font->GetGlyphInfoByIndex (attr->index)))
			goto next;
		
		y1 = y0;
		if ((attr->set & vOffset)) {
			offset = -(attr->voffset * scale);
			bottom = MAX (bottom, bottom0 + offset);
			top = MIN (top, top0 + offset);
			y1 += offset;
		}
		
		if ((attr->set & uOffset)) {
			offset = (attr->uoffset * scale);
			left = MIN (left, x0 + offset);
			x1 = x0 + offset;
		} else if (first_char) {
			if (glyph->metrics.horiBearingX < 0)
				x0 -= glyph->metrics.horiBearingX;
			
			first_char = false;
			x1 = x0;
		} else {
			x1 = x0;
		}
		
		right = MAX (right, x1 + glyph->metrics.horiAdvance);
		
		font->AppendPath (path, glyph, x1, y1);
		nglyphs++;
		
		if ((attr->set & Advance))
			x0 += attr->advance * scale;
		else
			x0 += glyph->metrics.horiAdvance;
		
	 next:
		
		attr = (GlyphAttr *) attr->next;
		n++;
	}
	
	if (nglyphs > 0) {
		height = bottom - top;
		width = right - left;
	} else {
		moon_path_destroy (path);
		path = NULL;
	}
}

void
Glyphs::GetSizeForBrush (cairo_t *cr, double *width, double *height)
{
	if (dirty)
		Layout ();
	
	*height = this->height;
	*width = this->width;
}

Point
Glyphs::GetOriginPoint () 
{
	double x0 = GetOriginX ();
	double y0 = GetOriginY ();
	
	if (!ORIGIN_IS_SET (x0))
		x0 = 0.0;
	
	if (ORIGIN_IS_SET (y0)) {
		double ascend = font ? font->Ascender () : 0.0;
		
		return Point (x0, y0 - ascend);
	} else {
		return Point (x0, 0);
	}
}

void
Glyphs::Render (cairo_t *cr, Region *region, bool path_only)
{
	if (width == 0.0 && height == 0.0)
		return;
	
	if (invalid) {
		// do not render anything if our state is invalid to keep with Silverlight's behavior.
		// (Note: rendering code also assumes everything is kosher)
		return;
	}
	
	if (path == NULL || path->cairo.num_data == 0) {
		// No glyphs to render
		return;
	}
	
	cairo_save (cr);
	cairo_set_matrix (cr, &absolute_xform);
	
	if (!path_only)
		RenderLayoutClip (cr);
	
	Rect area = Rect (left, top, width, height);
	fill->SetupBrush (cr, area);
	
	cairo_append_path (cr, &path->cairo);
	fill->Fill (cr);
	
	cairo_restore (cr);
}

Size
Glyphs::ComputeActualSize ()
{
	if (dirty)
		Layout ();

	return Size (left + width, top + height);
}

Size
Glyphs::MeasureOverride (Size availableSize)
{
	if (dirty)
		Layout ();

	return Size (left + width, top + height).Min (availableSize);
}

Size
Glyphs::ArrangeOverride (Size finalSize)
{
	if (dirty)
		Layout ();

	finalSize = ApplySizeConstraints (finalSize);
	return Size (left + width, top + height).Max (finalSize);
}

void 
Glyphs::ComputeBounds ()
{
	if (dirty)
		Layout ();
	
	bounds = IntersectBoundsWithClipPath (Rect (left, top, width, height), false).Transform (&absolute_xform);
}

Point
Glyphs::GetTransformOrigin ()
{
	// Glyphs seems to always use 0,0 no matter what is specified in the RenderTransformOrigin nor the OriginX/Y points
	return Point (0,0);
}

void
Glyphs::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (prop && prop->GetId () == Glyphs::FillProperty) {
		Invalidate ();
	} else {
		FrameworkElement::OnSubPropertyChanged (prop, obj, subobj_args);
	}
}

void
Glyphs::LoadFont (const Uri *uri, const char *path)
{
	FontManager *manager = Deployment::GetCurrent ()->GetFontManager ();
	StyleSimulations simulate = GetStyleSimulations ();
	double size = GetFontRenderingEmSize ();
	char *resource;
	int index;
	
	if (uri->GetFragment ()) {
		if ((index = strtol (uri->GetFragment (), NULL, 10)) < 0 || index == G_MAXINT)
			index = 0;
	} else {
		index = 0;
	}
	
	resource = uri->ToString ((UriToStringFlags) (UriHidePasswd | UriHideFragment | UriHideQuery));
	manager->AddResource (resource, path);
	font = TextFont::Load (resource, index, size, simulate);
	g_free (resource);
}

void
Glyphs::downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((Glyphs *) closure)->DownloaderComplete ();
}

void
Glyphs::DownloaderComplete ()
{
	Uri *uri = GetFontUri ();
	char *filename;
	
	delete font;
	font = NULL;
	
	// get the downloaded file path
	if (!(filename = downloader->GetDownloadedFilename (NULL))) {
		UpdateBounds (true);
		Invalidate ();
		dirty = true;
		return;
	}
	
	LoadFont (uri, filename);
	g_free (filename);
	
	UpdateBounds (true);
	Invalidate ();
	dirty = true;
}

#if DEBUG
static void
print_parse_error (const char *in, const char *where, const char *reason)
{
	if (debug_flags & RUNTIME_DEBUG_TEXT) {
		fprintf (stderr, "Glyph Indices parse error: \"%s\": %s\n", in, reason);
		fprintf (stderr, "                            ");
		for (int i = 0; i < (where - in); i++)
			fputc (' ', stderr);
		fprintf (stderr, "^\n");
	}
}
#endif

void
Glyphs::SetIndicesInternal (const char *in)
{
	register const char *inptr = in;
	GlyphAttr *glyph;
	double value = 0;
	char *end;
	uint bit;
	int n;
	
	attrs->Clear (true);
	
	if (in == NULL)
		return;
	
	while (g_ascii_isspace (*inptr))
		inptr++;
	
	while (*inptr) {
		glyph = new GlyphAttr ();
		
		while (g_ascii_isspace (*inptr))
			inptr++;
		
		// check for a cluster
		if (*inptr == '(') {
			inptr++;
			while (g_ascii_isspace (*inptr))
				inptr++;
			
			errno = 0;
			glyph->code_units = strtoul (inptr, &end, 10);
			if (glyph->code_units == 0 || (glyph->code_units == LONG_MAX && errno != 0)) {
				// invalid cluster
				d(print_parse_error (in, inptr, errno ? strerror (errno) : "invalid cluster mapping; CodeUnitCount cannot be 0"));
				delete glyph;
				return;
			}
			
			inptr = end;
			while (g_ascii_isspace (*inptr))
				inptr++;
			
			if (*inptr != ':') {
				// invalid cluster
				d(print_parse_error (in, inptr, "expected ':'"));
				delete glyph;
				return;
			}
			
			inptr++;
			while (g_ascii_isspace (*inptr))
				inptr++;
			
			errno = 0;
			glyph->glyph_count = strtoul (inptr, &end, 10);
			if (glyph->glyph_count == 0 || (glyph->glyph_count == LONG_MAX && errno != 0)) {
				// invalid cluster
				d(print_parse_error (in, inptr, errno ? strerror (errno) : "invalid cluster mapping; GlyphCount cannot be 0"));
				delete glyph;
				return;
			}
			
			inptr = end;
			while (g_ascii_isspace (*inptr))
				inptr++;
			
			if (*inptr != ')') {
				// invalid cluster
				d(print_parse_error (in, inptr, "expected ')'"));
				delete glyph;
				return;
			}
			
			glyph->set |= Cluster;
			inptr++;
			
			while (g_ascii_isspace (*inptr))
				inptr++;
		}
		
		if (*inptr >= '0' && *inptr <= '9') {
			errno = 0;
			glyph->index = strtoul (inptr, &end, 10);
			if ((glyph->index == 0 || glyph->index == LONG_MAX) && errno != 0) {
				// invalid glyph index
				d(print_parse_error (in, inptr, strerror (errno)));
				delete glyph;
				return;
			}
			
			glyph->set |= Index;
			
			inptr = end;
			while (g_ascii_isspace (*inptr))
				inptr++;
		}
		
		bit = (uint) Advance;
		n = 0;
		
		while (*inptr == ',' && n < 3) {
			inptr++;
			while (g_ascii_isspace (*inptr))
				inptr++;
			
			if (*inptr != ',') {
				value = g_ascii_strtod (inptr, &end);
				if ((value == 0.0 || value == HUGE_VAL || value == -HUGE_VAL) && errno != 0) {
					// invalid advance or offset
					d(print_parse_error (in, inptr, strerror (errno)));
					delete glyph;
					return;
				}
			} else {
				end = (char *) inptr;
			}
			
			if (end > inptr) {
				switch ((GlyphAttrMask) bit) {
				case Advance:
					glyph->advance = value;
					glyph->set |= Advance;
					break;
				case uOffset:
					glyph->uoffset = value;
					glyph->set |= uOffset;
					break;
				case vOffset:
					glyph->voffset = value;
					glyph->set |= vOffset;
					break;
				default:
					break;
				}
			}
			
			inptr = end;
			while (g_ascii_isspace (*inptr))
				inptr++;
			
			bit <<= 1;
			n++;
		}
		
		attrs->Append (glyph);
		
		while (g_ascii_isspace (*inptr))
			inptr++;
		
		if (*inptr && *inptr != ';') {
			d(print_parse_error (in, inptr, "expected ';'"));
			return;
		}
		
		if (*inptr == '\0')
			break;
		
		inptr++;
	}
}

void
Glyphs::DownloadFont (Surface *surface, Uri *uri, MoonError *error)
{
	if ((downloader = surface->CreateDownloader ())) {
		char *str = uri->ToString (UriHideFragment);
		downloader->Open ("GET", str, FontPolicy);
		g_free (str);
		
		if (downloader->GetFailedMessage () != NULL) {
			MoonError::FillIn (error, MoonError::ARGUMENT_OUT_OF_RANGE, 1000, downloader->GetFailedMessage ());
			downloader->unref ();
			downloader = NULL;
			return;
		}
		
		downloader->AddHandler (downloader->CompletedEvent, downloader_complete, this);
		if (downloader->Started () || downloader->Completed ()) {
			if (downloader->Completed ())
				DownloaderComplete ();
		} else {
			// This is what actually triggers the download
			downloader->Send ();
		}
	} else {
		// we're shutting down
	}
}

bool
Glyphs::SetFontResource (const Uri *uri)
{
	Application *application = Application::GetCurrent ();
	char *path;
	
	if (!application || !(path = application->GetResourceAsPath (GetResourceBase(), uri)))
		return false;
	
	LoadFont (uri, path);
	g_free (path);
	
	return true;
}

void
Glyphs::SetParent (DependencyObject *parent, MoonError *error)
{
	if (parent && GetSurface() && uri_changed) {
		// we've been added to the tree, kick off any pending
		// download we may have
		Uri *uri;
		
		if ((uri = GetFontUri ()))
			DownloadFont (GetSurface(), uri, error);
		
		uri_changed = false;
		
		if (error && error->number)
			return;
	}
	
	FrameworkElement::SetParent (parent, error);
}

void
Glyphs::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	bool invalidate = true;
	
	if (args->GetProperty ()->GetOwnerType() != Type::GLYPHS) {
		FrameworkElement::OnPropertyChanged (args, error);
		return;
	}
	
	if (args->GetId () == Glyphs::FontUriProperty) {
		Uri *uri = args->GetNewValue() ? args->GetNewValue()->AsUri () : NULL;
		Surface *surface = GetSurface ();
		
		CleanupDownloader ();
		dirty = true;
		delete font;
		font = NULL;
		
		if (!Uri::IsNullOrEmpty (uri)) {
			if (!SetFontResource (uri)) {
				if (uri->IsInvalidPath ()) {
					MoonError::FillIn (error, MoonError::ARGUMENT_OUT_OF_RANGE, 0, "invalid path found in uri");
					
					if (surface && uri->IsUncPath ())
						surface->EmitError (new ParserErrorEventArgs ("invalid uri", NULL, 0, 0, 0, NULL, NULL));
				} else {
					// need to create a downloader for this font...
					if (surface) {
						DownloadFont (surface, uri, error);
						uri_changed = false;
					} else {
						// queue a font download
						uri_changed = true;
					}
				}
			} else {
				uri_changed = false;
			}
		} else {
			uri_changed = false;
		}
	} else if (args->GetId () == Glyphs::FillProperty) {
		fill = args->GetNewValue() ? args->GetNewValue()->AsBrush() : NULL;
	} else if (args->GetId () == Glyphs::UnicodeStringProperty) {
		const char *str = args->GetNewValue() ? args->GetNewValue()->AsString () : NULL;
		g_free (text);
		
		if (str != NULL)
			text = g_utf8_to_ucs4_fast (str, -1, NULL);
		else
			text = NULL;
		
		dirty = true;
	} else if (args->GetId () == Glyphs::IndicesProperty) {
		const char *str = args->GetNewValue() ? args->GetNewValue()->AsString () : NULL;
		SetIndicesInternal (str);
		dirty = true;
	} else if (args->GetId () == Glyphs::FontRenderingEmSizeProperty) {
		if (font != NULL)
			dirty = font->SetSize (args->GetNewValue ()->AsDouble ());
		else
			dirty = true;
	} else if (args->GetId () == Glyphs::OriginXProperty) {
		dirty = true;
	} else if (args->GetId () == Glyphs::OriginYProperty) {
		dirty = true;
	} else if (args->GetId () == Glyphs::StyleSimulationsProperty) {
		StyleSimulations simulate = (StyleSimulations) args->GetNewValue ()->AsInt32 ();
		
		// clear any unsupported flags
		simulate = (StyleSimulations) (simulate & StyleSimulationsBoldItalic);
		
		if (font != NULL)
			dirty = font->SetStyleSimulations (simulate);
		else
			dirty = true;
	}
	
	if (invalidate)
		Invalidate ();
	
	if (dirty)
		UpdateBounds (true);
	
	NotifyListenersOfPropertyChange (args, error);
}
