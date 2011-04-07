/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * deepzoomimagetilesource.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <errno.h>
#include <math.h>

#include "application.h"
#include "debug.h"
#include "runtime.h"
#include "deployment.h"

#include "deepzoomimagetilesource.h"
#include "multiscalesubimage.h"
#include "uri.h"

namespace Moonlight {

class DisplayRect {
 public:
	int min_level;
	int max_level;
	Rect rect;
	
	DisplayRect (int minLevel, int maxLevel)
	{
		min_level = minLevel;
		max_level = maxLevel;
	}
};

class SubImage {
 public:
 	int id;
	int n;
	Uri *source;
	int width;
	int height;
	double vp_x;
	double vp_y;
	double vp_w;

	bool has_viewport;
	bool has_size;

	SubImage ()
	{
		source = NULL;
		has_viewport = false;
		has_size = false;
	}
	
	~SubImage ()
	{
		if (source)
			delete source;
	}
};

struct DZCollection {
	char *server_format, *format;
	SubImage *current_subimage;
	GPtrArray *subimages;
	int tile_size;
	int max_level;
};

struct DZImage {
	char *server_format, *format;
	DisplayRect *current_rect;
	GPtrArray *display_rects;
	int width, height;
	bool has_size;
	int tile_size;
	int overlap;
};

enum content_t {
	ContentUnknown = -1,
	ContentCollection,
	ContentImage
};

struct DZElementNode {
	DZElementNode *parent;
	const char *name;
};
	
struct DZParserInfo {
	DeepZoomImageTileSource *source;
	DZElementNode *current;
	content_t type;
	bool error;
	
	union {
		DZCollection collection;
		DZImage image;
	} content;
	
	DZParserInfo ()
	{
		type = ContentUnknown;
		current = NULL;
		source = NULL;
		error = false;
		
		memset ((void *) &content, 0, sizeof (content));
	}
	
	~DZParserInfo ()
	{
		DZCollection *collection = &content.collection;
		DZImage *image = &content.image;
		DZElementNode *parent;
		
		while (current != NULL) {
			parent = current->parent;
			delete current;
			current = parent;
		}
		
		switch (type) {
		case ContentCollection:
			if (collection->current_subimage)
				delete collection->current_subimage;
			
			if (collection->subimages) {
				for (guint i = 0; i < collection->subimages->len; i++)
					((MultiScaleSubImage *) collection->subimages->pdata[i])->unref ();
				
				g_ptr_array_free (collection->subimages, true);
			}
			
			g_free (collection->server_format);
			g_free (collection->format);
			break;
		case ContentImage:
			if (image->current_rect)
				delete image->current_rect;
			
			if (image->display_rects) {
				for (guint i = 0; i < image->display_rects->len; i++)
					delete (DisplayRect *) image->display_rects->pdata[i];
				
				g_ptr_array_free (image->display_rects, true);
			}
			
			g_free (image->server_format);
			g_free (image->format);
			break;
		default:
			break;
		}
	}
};

static void start_element (void *data, const char *el, const char **attr);
static void end_element (void *data, const char *el);

static bool
get_tile_layer (MultiScaleTileSource* msts, int level, int x, int y, Uri **uri)
{
	return ((DeepZoomImageTileSource *)msts)->GetTileLayer (level, x, y, uri);
}

void
DeepZoomImageTileSource::Init ()
{
	SetObjectType (Type::DEEPZOOMIMAGETILESOURCE);

	downloaded = false;
	parsed = false;
	format = NULL;
	server_format = NULL;
	get_tile_func = get_tile_layer;
	display_rects = NULL;
	is_collection = false;
	max_level = 0;
	subimages = NULL;
	nested = false;
	get_resource_aborter = NULL;
	parser = NULL;
}

DeepZoomImageTileSource::DeepZoomImageTileSource ()
{
	Init ();
}

DeepZoomImageTileSource::DeepZoomImageTileSource (Uri *uri, bool nested)
{
	Init ();
	this->nested = nested;
	if (uri)
		SetUriSource (uri);

	EnsureManagedPeer ();
}

DeepZoomImageTileSource::~DeepZoomImageTileSource ()
{
	Abort ();
	g_free (format);
	g_free (server_format);
	delete get_resource_aborter;
	
	if (display_rects) {
		for (guint i = 0; i < display_rects->len; i++)
			delete (DisplayRect *) display_rects->pdata[i];
		
		g_ptr_array_free (display_rects, true);
	}
	
	if (subimages) {
		for (guint i = 0; i < subimages->len; i++)
			((MultiScaleSubImage *) subimages->pdata[i])->unref ();
		
		g_ptr_array_free (subimages, true);
	}
}

void
DeepZoomImageTileSource::SetServerFormat (const char *server_format)
{
	g_free (this->server_format);
	this->server_format = g_strdup (server_format);
}

void
DeepZoomImageTileSource::SetFormat (const char *format)
{
	g_free (this->format);
	this->format = g_strdup (format);
}

void
DeepZoomImageTileSource::Abort ()
{
	DZParserInfo *info;
	
	if (get_resource_aborter)
		get_resource_aborter->Cancel ();
	
	if (parser) {
		info = (DZParserInfo *) XML_GetUserData (parser);
		delete info;
		
		XML_ParserFree (parser);
		parser = NULL;
	}
}

bool
DeepZoomImageTileSource::GetTileLayer (int level, int x, int y, Uri **uri)
{
	// check if there tile is listed in DisplayRects
	if (display_rects && display_rects->len > 0) {
		int tile_width = GetTileWidth ();
		DisplayRect *cur;
		bool found = false;
		int layers;
		
		frexp ((double) MAX (GetImageWidth (), GetImageHeight ()), &layers);
		
		for (guint i = 0; i < display_rects->len; i++) {
			cur = (DisplayRect *) display_rects->pdata[i];
			
			if (!(cur->min_level <= level && level <= cur->max_level))
				continue;

			int vtilesize = tile_width * (layers + 1 - level);
			Rect virtualtile = Rect (x * vtilesize, y * vtilesize, vtilesize, vtilesize);
			if (cur->rect.IntersectsWith (virtualtile)) {
				found = true;
				break;
			}
		}

		if (!found)
			return false;
	}
	
	const Uri *baseUri = GetUriSource ();
	Uri *image_uri;
	const char *filename, *ext;
	char *image;
	
	if (!baseUri)
		return false;
	
	if ((filename = strrchr (baseUri->GetPath (), '/')))
		filename ++;
	else
		filename = baseUri->GetPath ();
	
	if (GetServerFormat () && !g_ascii_strcasecmp (GetServerFormat (), "SmoothStreaming")) {
		image = g_strdup_printf ("/QualityLevels(%d)/RawFragments(tile=%d)", 100+level, y*1000000000+x);
	} else {
		if (!(ext = strrchr (filename, '.')))
			return false;
		
		image = g_strdup_printf ("%.*s_files/%d/%d_%d.%s", (int) (ext - filename), filename, level, x, y, format);
		//printf ("%p requesting image %s\n", this, image);
	}
	
	// DRTs: #511: checks for rooted baseUri (/foo/bar.xml), #265: checks for relative baseUri (../../bar.xml).
	// the first one is resolved with the application root as the root, while the second is allowed to escape out of the application root
	// we check baseUri [0] == '/' to distinguish these two cases
	if (!baseUri->IsAbsolute ()) {
		image_uri = Uri::Create (image);
		*uri = Uri::CombineWithSourceLocation (GetDeployment (), baseUri, image_uri, (baseUri == NULL || baseUri->GetPath () == NULL || baseUri->GetPath () [0] != '/'));
		delete image_uri;
	} else {
		*uri = Uri::Create (baseUri, image);
	}
	
	g_free (image);

	return true;
}

static void
resource_notify (NotifyType type, gint64 args, gpointer user_data)
{
	DeepZoomImageTileSource *dzits = (DeepZoomImageTileSource *)user_data;
	if (type == NotifyFailed)
		dzits->DownloaderFailed ();
	else if (type == NotifyCompleted)
		dzits->DownloaderComplete ();
}

static void
dz_write (EventObject *sender, EventArgs *calldata, gpointer data)
{
	HttpRequestWriteEventArgs *args = (HttpRequestWriteEventArgs *) calldata;
	DeepZoomImageTileSource *dzits = (DeepZoomImageTileSource *) data;
	dzits->XmlWrite ((char *) args->GetData (), args->GetOffset (), args->GetCount ());
}

void
DeepZoomImageTileSource::XmlWrite (char* buffer, gint32 offset, gint32 n)
{
	DZParserInfo *info;
	
	if (offset == 0) {
		// Init xml parser
		LOG_MSI ("Start parsing DeepZoom\n");
		parser = XML_ParserCreate (NULL);
		XML_SetElementHandler (parser, start_element, end_element);
		info = new DZParserInfo ();
		info->source = this;
		XML_SetUserData (parser, info);
	}

	if (!XML_Parse (parser, buffer, n, 0)) {
		g_warning ("Parser error at line %d:\n%s\n", (int)XML_GetCurrentLineNumber (parser), XML_ErrorString(XML_GetErrorCode(parser)));
		Abort ();
		DownloaderFailed ();
	}
}

void
DeepZoomImageTileSource::Download ()
{
	LOG_MSI ("DZITS::Download ()\n");
	if (downloaded)
		return;

	Application *current = Application::GetCurrent ();
	const Uri *uri = GetUriSource ();

	if (current && uri) {
		downloaded = true;
		if (get_resource_aborter)
			delete get_resource_aborter;
		get_resource_aborter = new Cancellable ();
		//printf ("Dzits %p fetching %s\n", this, uri->ToString ());
		current->GetResource (GetResourceBase(), uri, resource_notify, dz_write, MediaPolicy, HttpRequest::DisableFileStorage, get_resource_aborter, this);
	}
}

void
DeepZoomImageTileSource::DownloaderComplete ()
{
	DZParserInfo *info = (DZParserInfo *) XML_GetUserData (parser);
	DZCollection *collection = &info->content.collection;
	DZImage *image = &info->content.image;
	
	// set isFinal for the parser to complete
	if (!XML_Parse (parser, NULL, 0, 1)) {
		g_warning ("Parser error at line %d:\n%s\n", (int)XML_GetCurrentLineNumber (parser), XML_ErrorString(XML_GetErrorCode(parser)));
		Abort ();
		DownloaderFailed ();
		return;
	}
	
	if (info->error || info->type == ContentUnknown) {
		Abort ();
		DownloaderFailed ();
		return;
	}
	
	switch (info->type) {
	case ContentCollection:
		is_collection = true;
		server_format = g_strdup (collection->server_format);
		format = g_strdup (collection->format);
		SetTileHeight (collection->tile_size);
		SetTileWidth (collection->tile_size);
		max_level = collection->max_level;
		subimages = collection->subimages;
		collection->subimages = NULL;
		parsed = true;
		break;
	case ContentImage:
		is_collection = false;
		server_format = g_strdup (image->server_format);
		format = g_strdup (image->format);
		display_rects = image->display_rects;
		image->display_rects = NULL;
		SetTileOverlap (image->overlap);
		SetTileHeight (image->tile_size);
		SetTileWidth (image->tile_size);
		SetImageHeight (image->height);
		SetImageWidth (image->width);
		parsed = true;
		break;
	default:
		break;
	}
	
	LOG_MSI ("Done parsing...\n");
	
	XML_ParserFree (parser);
	parser = NULL;
	delete info;
	
	if (HasHandlers (DownloaderCompletedEvent))
		Emit (DownloaderCompletedEvent);
}

void
DeepZoomImageTileSource::DownloaderFailed ()
{
	if (IsDisposing () || IsDisposed ()) {
		/* This may happen if we cancelled the request in our dtor */
		return;
	}

	LOG_MSI ("DZITS::dl failed\n");
	if (HasHandlers (DownloaderFailedEvent))
		Emit (DownloaderFailedEvent);
}

void
DeepZoomImageTileSource::UriSourceChanged ()
{
	downloaded = false;
	parsed = false;
	
	if (!nested)
		Download ();
	
	if (HasHandlers (UriSourceChangedEvent))
		Emit (UriSourceChangedEvent);
}

void
DeepZoomImageTileSource::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetId () == DeepZoomImageTileSource::UriSourceProperty) {
		Abort ();
		UriSourceChanged ();
	}

	if (args->GetProperty ()->GetOwnerType () != Type::DEEPZOOMIMAGETILESOURCE) {
		DependencyObject::OnPropertyChanged (args, error);
		return;
	}
	
	NotifyListenersOfPropertyChange (args, error);
}

#define DZParsedServerFormat  (1 << 0)
#define DZParsedFormat        (1 << 1)
#define DZParsedTileSize      (1 << 2)
#define DZParsedOverlap       (1 << 3)
#define DZParsedMinLevel      (1 << 4)
#define DZParsedMaxLevel      (1 << 5)
#define DZParsedNextItemId    (1 << 6)
#define DZParsedQuality       (1 << 7)
#define DZParsedHeight        (1 << 8)
#define DZParsedWidth         (1 << 9)
#define DZParsedSource        (1 << 10)
#define DZParsedIsPath        (1 << 11)
#define DZParsedType          (1 << 12)
#define DZParsedId            (1 << 13)
#define DZParsedN             (1 << 14)
#define DZParsedX             (1 << 15)
#define DZParsedY             (1 << 16)

#define DZParsedCollection (/*DZParsedServerFormat |*/ DZParsedFormat | DZParsedTileSize | DZParsedMaxLevel | DZParsedNextItemId)
#define DZParsedImage (/*DZParsedServerFormat |*/ DZParsedFormat | DZParsedTileSize | DZParsedOverlap)
#define DZParsedRect (DZParsedWidth | DZParsedHeight | DZParsedX | DZParsedY)
#define DZParsedDisplayRect (DZParsedMinLevel | DZParsedMaxLevel)
#define DZParsedViewport (DZParsedWidth | DZParsedX | DZParsedY)
#define DZParsedSize (DZParsedWidth | DZParsedHeight)
#define DZParsedI (DZParsedId | DZParsedN)

static bool
FloatTryParse (const char *str, double *val)
{
	/* Note: sign + 38 + . + 38 + E + sign + 2 + null = 83 */
	const char *decimal, *exponent, *vptr, *inptr = str;
	char buf[84], *inend, *ptr;
	struct lconv *locale;
	size_t offset, len;
	int sign = 0;
	float f;
	
	while (*inptr == ' ' || *inptr == '\t')
		inptr++;
	
	if (*inptr == '\0')
		return false;
	
	/* validate input */
	vptr = inptr;
	
	/* leading sign */
	if (*vptr == '+' || *vptr == '-') {
		sign = 1;
		vptr++;
	}
	
	/* integral part of the decimal number */
	while (g_ascii_isdigit (*vptr))
		vptr++;
	
	/* fail if the integral part is > 38 digits */
	if ((vptr - inptr) > (38 + sign))
		return false;
	
	/* decimal point */
	if (*vptr == '.') {
		offset = vptr - inptr;
		decimal = vptr;
		vptr++;
		
		/* decimal point must either be followed by a digit or trailing lwsp */
		if (*vptr && !g_ascii_isdigit (*vptr) && *vptr != ' ' && *vptr != '\t')
			return false;
		
		/* fractional part of the decimal number */
		while (g_ascii_isdigit (*vptr))
			vptr++;
		
		/* fail if the fractional part is > 38 digits */
		if ((vptr - decimal) > (38 + 1))
			return false;
	}
	
	/* must contain at least 1 digit before exponent */
	if (vptr == inptr || !g_ascii_isdigit (vptr[-1]))
		return false;
	
	/* exponent expression */
	if (*vptr == 'e' || *vptr == 'E') {
		vptr++;
		
		exponent = vptr;
		
		/* exponent sign */
		if (*vptr == '+' || *vptr == '-') {
			sign = 1;
			vptr++;
		} else
			sign = 0;
		
		/* exponent value */
		while (g_ascii_isdigit (*vptr))
			vptr++;
		
		/* fail if the exponent is > 2 digits */
		if ((vptr - exponent) > (2 + sign))
			return false;
	}
	
	/* keep track of the length of the decimal number */
	len = vptr - inptr;
	
	/* allow trailing lwsp */
	while (*vptr == ' ' || *vptr == '\t')
		vptr++;
	
	if (*vptr != '\0') {
		/* invalid data in input string */
		return false;
	}
	
	/* Android implements localeconv, but doesn't have the structure, we're presuming
	 * its always "." there
	 */
#if !defined (PLATFORM_ANDROID)
	if (decimal && (locale = localeconv ()) && strcmp (locale->decimal_point, ".") != 0) {
		/* locale doesn't use '.' as decimal point so we need to munge
		 * the string into the locale's formatting */
		memcpy (buf, inptr, offset);
		ptr = buf + offset;
		len -= offset;
		
		ptr = g_stpcpy (ptr, locale->decimal_point);
		decimal++;
		len--;
		
		memcpy (ptr, decimal, len);
		ptr[len] = '\0';
		
		inptr = buf;
	}
#endif
	
	errno = 0;
	f = strtof (inptr, &inend);
	
	if (errno != 0 || inend != (inptr + len) || f == HUGE_VALF || f == -HUGE_VALF)
		return false;
	
	*val = (double) f;
	
	return true;
}

static int
Log2 (unsigned int v)
{
	/* algorithm based on http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog */
	/* Note: assumes v is already known to be a power of 2 */
	register unsigned int r = (v & 0xAAAAAAAA) != 0;
	
	r |= ((v & 0xFFFF0000) != 0) << 4;
	r |= ((v & 0xFF00FF00) != 0) << 3;
	r |= ((v & 0xF0F0F0F0) != 0) << 2;
	r |= ((v & 0xCCCCCCCC) != 0) << 1;
	
	return r;
}

static bool
is_valid_format (const char *format)
{
	static const char *formats[2] = { "jpg", "png" };
	
	for (guint i = 0; i < G_N_ELEMENTS (formats); i++) {
		if (!g_ascii_strcasecmp (formats[i], format))
			return true;
	}
	
	return false;
}

static bool
current_element_is (DZParserInfo *info, const char *name)
{
	return info->current && !strcmp (info->current->name, name);
}

static void
image_start (DZParserInfo *info, const char **attr)
{
	DZImage *image = &info->content.image;
	bool failed = false;
	guint32 parsed = 0;
	int err;
	
	if (info->type != ContentUnknown) {
		info->error = true;
		return;
	}
	
	info->type = ContentImage;
	
	for (int i = 0; attr[i] && !failed; i += 2) {
		if (!g_ascii_strcasecmp ("ServerFormat", attr[i])) {
			if (!(parsed & DZParsedServerFormat)) {
				image->server_format = g_strdup (attr[i+1]);
				parsed |= DZParsedServerFormat;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("Format", attr[i])) {
			if (!(parsed & DZParsedFormat) && is_valid_format (attr[i+1])) {
				image->format = g_strdup (attr[i+1]);
				parsed |= DZParsedFormat;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("TileSize", attr[i])) {
			if (!(parsed & DZParsedTileSize)) {
				failed = !Int32TryParse (attr[i+1], &image->tile_size, &err) || image->tile_size <= 0;
				parsed |= DZParsedTileSize;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("Overlap", attr[i])) {
			if (!(parsed & DZParsedOverlap)) {
				failed = !Int32TryParse (attr[i+1], &image->overlap, &err) || image->overlap < 0;
				parsed |= DZParsedOverlap;
			} else
				failed = true;
		}
	}
	
	if (failed || (parsed & DZParsedImage) != DZParsedImage) {
		fprintf (stderr, "DeepZoom Image parse error: failed=%s; parsed=%x\n", failed ? "true" : "false", parsed);
		info->error = true;
	}
}

static void
image_end (DZParserInfo *info)
{
	DZImage *image = &info->content.image;
	
	if (!image->has_size) {
		/* must contain a Size element! */
		info->error = true;
	}
}

static void
collection_start (DZParserInfo *info, const char **attr)
{
	DZCollection *collection = &info->content.collection;
	bool failed = false;
	guint32 parsed = 0;
	double quality;
	int next, err;
	
	if (info->type != ContentUnknown) {
		info->error = true;
		return;
	}
	
	info->type = ContentCollection;
	
	for (int i = 0; attr[i] && !failed; i += 2) {
		if (!g_ascii_strcasecmp ("ServerFormat", attr[i])) {
			if (!(parsed & DZParsedServerFormat)) {
				collection->server_format = g_strdup (attr[i+1]);
				parsed |= DZParsedServerFormat;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("Format", attr[i])) {
			if (!(parsed & DZParsedFormat) && is_valid_format (attr[i+1])) {
				collection->format = g_strdup (attr[i+1]);
				parsed |= DZParsedFormat;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("TileSize", attr[i])) {
			if (!(parsed & DZParsedTileSize)) {
				// technically this value must be a power of 2
				failed = !Int32TryParse (attr[i+1], &collection->tile_size, &err) || collection->tile_size <= 0 ||
					(collection->tile_size & (collection->tile_size - 1)) != 0; /* checks TileSize is a power of 2 */
				parsed |= DZParsedTileSize;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("MaxLevel", attr[i])) {
			if (!(parsed & DZParsedMaxLevel)) {
				// must be less than or equal to log2(TileSize)
				failed = !Int32TryParse (attr[i+1], &collection->max_level, &err) || collection->max_level <= 0;
				parsed |= DZParsedMaxLevel;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("NextItemId", attr[i])) {
			// This attr would tell us the number
			// of items we're likely to encounter,
			// but beyond that it is useless.
			if (!(parsed & DZParsedNextItemId)) {
				failed = !Int32TryParse (attr[i+1], &next, &err) || next <= 0;
				parsed |= DZParsedNextItemId;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("Quality", attr[i])) {
			// This attr is not useful to us.
			if (!(parsed & DZParsedQuality)) {
				failed = !FloatTryParse (attr[i+1], &quality) || quality < 0.0 || quality > 1.0;
				parsed |= DZParsedQuality;
			} else
				failed = true;
		}
	}
	
	if (failed || (parsed & DZParsedCollection) != DZParsedCollection) {
		fprintf (stderr, "DeepZoom Collection parse error: failed=%s; parsed=%x\n", failed ? "true" : "false", parsed);
		info->error = true;
		return;
	}
	
	// verify MaxLevel <= log2(TileSize)
	if (collection->max_level > Log2 (collection->tile_size)) {
		info->error = true;
		return;
	}
}

static void
collection_end (DZParserInfo *info)
{
	DZCollection *collection = &info->content.collection;
	
	if (!collection->subimages) {
		/* must contain an Items element! */
		info->error = true;
		return;
	}
}

static void
display_rects_start (DZParserInfo *info, const char **attr)
{
	DZImage *image = &info->content.image;
	
	if (!(info->type == ContentImage && current_element_is (info, "Image"))) {
		info->error = true;
		return;
	}
	
	if (image->display_rects != NULL) {
		/* cannot have multiple DisplayRects elements! */
		info->error = true;
		return;
	}
	
	image->display_rects = g_ptr_array_new ();
}

static void
display_rects_end (DZParserInfo *info)
{
#if 0
	/* breaks drt Seadragon-520, 294HW and Seadragon-P0, but is needed by
	 * Seadragon-InvalidXml for image_displayrects_empty.xml */
	DZImage *image = &info->content.image;
	
	if (image->display_rects->len == 0) {
		/* must contain DisplayRect elements! */
		info->error = true;
		return;
	}
#endif
}

static void
display_rect_start (DZParserInfo *info, const char **attr)
{
	DZImage *image = &info->content.image;
	int min_level = 0, max_level = 0;
	bool failed = false;
	guint32 parsed = 0;
	int err;
	
	if (!(info->type == ContentImage && current_element_is (info, "DisplayRects"))) {
		info->error = true;
		return;
	}
	
	for (int i = 0; attr[i] && !failed; i += 2) {
		if (!g_ascii_strcasecmp ("MinLevel", attr[i])) {
			if (!(parsed & DZParsedMinLevel)) {
				failed = !Int32TryParse (attr[i+1], &min_level, &err) || min_level < 0;
				parsed |= DZParsedMinLevel;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("MaxLevel", attr[i])) {
			if (!(parsed & DZParsedMaxLevel)) {
				failed = !Int32TryParse (attr[i+1], &max_level, &err) || max_level < 0;
				parsed |= DZParsedMaxLevel;
			} else
				failed = true;
		}
	}
	
	if (max_level < min_level)
		failed = true;
	
	if (failed || (parsed & DZParsedDisplayRect) != DZParsedDisplayRect) {
		fprintf (stderr, "DeepZoom DisplayRect parse error: failed=%s; parsed=%x\n", failed ? "true" : "false", parsed);
		info->error = true;
		return;
	}
	
	image->current_rect = new DisplayRect (min_level, max_level);
}

static void
display_rect_end (DZParserInfo *info)
{
	DZImage *image = &info->content.image;
	
	g_ptr_array_add (image->display_rects, image->current_rect);
	image->current_rect = NULL;
}

static void
viewport_start (DZParserInfo *info, const char **attr)
{
	DZCollection *collection = &info->content.collection;
	bool failed = false;
	guint32 parsed = 0;
	
	if (!(info->type == ContentCollection && current_element_is (info, "I") && collection->current_subimage)) {
		info->error = true;
		return;
	}
	
	if (collection->current_subimage->has_viewport) {
		/* cannot contain multiple Viewport elements! */
		info->error = true;
		return;
	}
	
	collection->current_subimage->has_viewport = true;
	
	for (int i = 0; attr [i] && !failed; i += 2) {
		if (!g_ascii_strcasecmp ("Width", attr[i])) {
			if (!(parsed & DZParsedWidth)) {
				failed = !FloatTryParse (attr[i+1], &collection->current_subimage->vp_w);
				if (collection->current_subimage->vp_w < 0)
					failed = true;
				parsed |= DZParsedWidth;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("X", attr[i])) {
			if (!(parsed & DZParsedX)) {
				failed = !FloatTryParse (attr[i+1], &collection->current_subimage->vp_x);
				parsed |= DZParsedX;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("Y", attr[i])) {
			if (!(parsed & DZParsedY)) {
				failed = !FloatTryParse (attr[i+1], &collection->current_subimage->vp_y);
				parsed |= DZParsedY;
			} else
				failed = true;
		}
	}
	
	if (failed || (parsed & DZParsedViewport) != DZParsedViewport) {
		fprintf (stderr, "DeepZoom Viewport parse error: failed=%s; parsed=%x\n", failed ? "true" : "false", parsed);
		info->error = true;
	}
}

static void
items_start (DZParserInfo *info, const char **attr)
{
	DZCollection *collection = &info->content.collection;
	
	if (!(info->type == ContentCollection && current_element_is (info, "Collection"))) {
		info->error = true;
		return;
	}
	
	if (collection->subimages != NULL) {
		/* cannot contain multiple Items elements! */
		info->error = true;
		return;
	}
	
	collection->subimages = g_ptr_array_new ();
}

static void
items_end (DZParserInfo *info)
{
	DZCollection *collection = &info->content.collection;
	
	if (collection->subimages->len == 0) {
		/* must contain I elements! */
		info->error = true;
		return;
	}
}

static void
rect_start (DZParserInfo *info, const char **attr)
{
	DZImage *image = &info->content.image;
	bool failed = false;
	guint32 parsed = 0;
	int val, err;
	
	if (!(info->type == ContentImage && current_element_is (info, "DisplayRect") && image->current_rect)) {
		info->error = true;
		return;
	}
	
	for (int i = 0; attr[i] && !failed; i += 2) {
		if (!g_ascii_strcasecmp ("Height", attr[i])) {
			if (!(parsed & DZParsedHeight)) {
				failed = !Int32TryParse (attr[i+1], &val, &err) || val <= 0;
				image->current_rect->rect.height = (double) val;
				parsed |= DZParsedHeight;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("Width", attr[i])) {
			if (!(parsed & DZParsedWidth)) {
				failed = !Int32TryParse (attr[i+1], &val, &err) || val <= 0;
				image->current_rect->rect.width = (double) val;
				parsed |= DZParsedWidth;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("X", attr[i])) {
			if (!(parsed & DZParsedX)) {
				failed = !Int32TryParse (attr[i+1], &val, &err) || val < 0;
				image->current_rect->rect.x = (double) val;
				parsed |= DZParsedX;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("Y", attr[i])) {
			if (!(parsed & DZParsedY)) {
				failed = !Int32TryParse (attr[i+1], &val, &err) || val < 0;
				image->current_rect->rect.y = (double) val;
				parsed |= DZParsedY;
			} else
				failed = true;
		}
	}
	
	if (failed || (parsed & DZParsedRect) != DZParsedRect) {
		fprintf (stderr, "DeepZoom Rect parse error: failed=%s; parsed=%x\n", failed ? "true" : "false", parsed);
		info->error = true;
	}
}

static void
size_start (DZParserInfo *info, const char **attr)
{
	DZCollection *collection = &info->content.collection;
	DZImage *image = &info->content.image;
	int *width, *height;
	bool failed = false;
	guint32 parsed = 0;
	int err;
	
	switch (info->type) {
	case ContentCollection:
		if (!(current_element_is (info, "I") && collection->current_subimage)) {
			info->error = true;
			return;
		}
		
		if (collection->current_subimage->has_size) {
			/* cannot have more than 1 Size element! */
			info->error = true;
			return;
		}
		
		collection->current_subimage->has_size = true;
		height = &collection->current_subimage->height;
		width = &collection->current_subimage->width;
		break;
	case ContentImage:
		if (!current_element_is (info, "Image")) {
			info->error = true;
			return;
		}
		
		if (image->has_size) {
			/* cannot have more than 1 Size element! */
			info->error = true;
			return;
		}
		
		image->has_size = true;
		height = &image->height;
		width = &image->width;
		break;
	default:
		info->error = true;
		return;
	}
	
	for (int i = 0; attr[i] && !failed; i += 2) {
		if (!g_ascii_strcasecmp ("Width", attr[i])) {
			if (!(parsed & DZParsedWidth)) {
				failed = !Int32TryParse (attr[i+1], width, &err) || *width <= 0;
				parsed |= DZParsedWidth;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("Height", attr[i])) {
			if (!(parsed & DZParsedHeight)) {
				failed = !Int32TryParse (attr[i+1], height, &err) || *height <= 0;
				parsed |= DZParsedHeight;
			} else
				failed = true;
		}
	}
	
	if (failed || (parsed & DZParsedSize) != DZParsedSize) {
		fprintf (stderr, "DeepZoom Size parse error: failed=%s; parsed=%x\n", failed ? "true" : "false", parsed);
		info->error = true;
	}
}

static void
item_start (DZParserInfo *info, const char **attr)
{
	DZCollection *collection = &info->content.collection;
	SubImage *subimage;
	bool failed = false;
	guint32 parsed = 0;
	int err;
	
	// Note: <I> items are *supposed* to be within <Items></Items>, but I
	// am told that Silverlight seems to gracefully handle cases where the
	// <Items> element is missing in the hierarchy.
	if (!(info->type == ContentCollection && (current_element_is (info, "Items") || current_element_is (info, "Collection")))) {
		info->error = true;
		return;
	}
	
	if (!collection->subimages)
		collection->subimages = g_ptr_array_new ();
	
	subimage = collection->current_subimage = new SubImage ();
	
	for (int i = 0; attr[i] && !failed; i += 2) {
		if (!g_ascii_strcasecmp ("Source", attr[i])) {
			if (!(parsed & DZParsedSource)) {
				subimage->source = Uri::Create (attr [i+1]);
				failed = subimage->source == NULL;
				parsed |= DZParsedSource;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("IsPath", attr[i])) {
			// This attribute must always have a value of "1" or
			// "true" for our uses, so it is safe to ignore it.
			if (!(parsed & DZParsedIsPath)) {
				failed = strcmp (attr[i+1], "1") != 0;
				parsed |= DZParsedIsPath;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("Type", attr[i])) {
			// This attribute is the pixel source type. For
			// Silverlight, this value is always ImagePixelSource
			// so it is safe to ignore.
			if (!(parsed & DZParsedType)) {
				failed = strcmp (attr[i+1], "ImagePixelSource") != 0;
				parsed |= DZParsedType;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("Id", attr[i])) {
			if (!(parsed & DZParsedId)) {
				failed = !Int32TryParse (attr[i+1], &subimage->id, &err) || subimage->id < 0;
				parsed |= DZParsedId;
			} else
				failed = true;
		} else if (!g_ascii_strcasecmp ("N", attr[i])) {
			if (!(parsed & DZParsedN)) {
				failed = !Int32TryParse (attr[i+1], &subimage->n, &err) || subimage->n < 0;
				parsed |= DZParsedN;
			} else
				failed = true;
		}
	}
	
	if (failed || (parsed & DZParsedI) != DZParsedI) {
		fprintf (stderr, "DeepZoom I parse error: failed=%s; parsed=%x\n", failed ? "true" : "false", parsed);
		info->error = true;
	}
}

static void
item_end (DZParserInfo *info)
{
	DZCollection *collection = &info->content.collection;
	DeepZoomImageTileSource *subsource;
	MultiScaleSubImage *subimage;
	SubImage *item;
	const Uri *uri;
	
	if ((uri = info->source->GetUriSource ())) {
		item = collection->current_subimage;
		
		if (!item->has_size) {
			/* must contain a Size element! */
			info->error = true;
			return;
		}
		
		for (guint i = 0; i < collection->subimages->len; i++) {
			subimage = (MultiScaleSubImage *) collection->subimages->pdata[i];
			if (item->id == subimage->GetId ()) {
				/* must not contain duplicate entries! */
				info->error = true;
				return;
			}
		}
		
		subsource = new DeepZoomImageTileSource (item->source, true);
		subimage = new MultiScaleSubImage (uri, subsource, item->id, item->n);
		subsource->SetServerFormat (collection->server_format);
		subsource->SetFormat (collection->format);
		subsource->SetImageHeight (item->height);
		subsource->SetImageWidth (item->width);
		subsource->unref ();
		
		if (item->has_viewport) {
			subimage->SetViewportOrigin (new Point (item->vp_x, item->vp_y));
			subimage->SetViewportWidth (item->vp_w);
		}
		
		Value v((double) item->width / (double) item->height);
		subimage->SetValue (MultiScaleSubImage::AspectRatioProperty, &v);
		
		g_ptr_array_add (collection->subimages, subimage);
	} else {
		delete collection->current_subimage;
	}
	
	collection->current_subimage = NULL;
}

static struct {
	const char *name;
	void (* element_start) (DZParserInfo *info, const char **attr);
	void (* element_end) (DZParserInfo *info);
} dz_elements[] = {
	{ "Image",        image_start,         image_end         },
	{ "Collection",   collection_start,    collection_end    },
	{ "DisplayRects", display_rects_start, display_rects_end },
	{ "DisplayRect",  display_rect_start,  display_rect_end  },
	{ "Viewport",     viewport_start,      NULL              },
	{ "Items",        items_start,         items_end         },
	{ "Rect",         rect_start,          NULL              },
	{ "Size",         size_start,          NULL              },
	{ "I",            item_start,          item_end          },
};

// DeepZoomParsing
static void
start_element (void *data, const char *el, const char **attr)
{
	DZParserInfo *info = (DZParserInfo *) data;
	DZElementNode *parent;
	
	if (info->error)
		return;
	
	for (guint i = 0; i < G_N_ELEMENTS (dz_elements); i++) {
		if (!g_ascii_strcasecmp (dz_elements[i].name, el)) {
			dz_elements[i].element_start (info, attr);
			
			parent = info->current;
			info->current = new DZElementNode ();
			info->current->name = dz_elements[i].name;
			info->current->parent = parent;
			return;
		}
	}
	
	fprintf (stderr, "Deep Zoom parse error: unknown element <%s>\n", el);
	info->error = true;
}

static void
end_element (void *data, const char *el)
{
	DZParserInfo *info = (DZParserInfo *) data;
	DZElementNode *parent;
	
	if (!info->current)
		info->error = true;
	
	if (info->error)
		return;
	
	for (guint i = 0; i < G_N_ELEMENTS (dz_elements); i++) {
		if (!g_ascii_strcasecmp (dz_elements[i].name, el)) {
			if (dz_elements[i].element_end)
				dz_elements[i].element_end (info);
			
			parent = info->current->parent;
			delete info->current;
			info->current = parent;
			return;
		}
	}
	
	fprintf (stderr, "Deep Zoom parse error: unknown element </%s>\n", el);
	info->error = true;
}

MultiScaleSubImage *
DeepZoomImageTileSource::GetSubImage (guint index)
{
	if (!subimages || index >= subimages->len)
		return NULL;
	
	return (MultiScaleSubImage *) subimages->pdata[index];
}

guint
DeepZoomImageTileSource::GetSubImageCount ()
{
	return subimages ? subimages->len : 0;
}

};
