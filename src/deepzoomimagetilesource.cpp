/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * deepzoomimagetilesource.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007,2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include <stdio.h>
#include <math.h>
#include <errno.h>

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

class DZParserInfo {
 public:
	DeepZoomImageTileSource *source;
	bool error;
	int depth;
	int skip;
	
	bool is_collection;
	
	// Image attributes
	int image_width, image_height;
	DisplayRect *current_rect;
	GPtrArray *display_rects;
	int overlap;
	
	// Collection attributes
	SubImage *current_subimage;
	GPtrArray *subimages;
	int max_level;
	
	// Common attributes
	char *server_format;
	char *format;
	int tile_size;
	
	DZParserInfo ()
	{
		depth = 0;
		skip = -1;
		error = false;
		format = NULL;
		server_format = NULL;
		image_width = image_height = tile_size = overlap = 0;
		current_rect = NULL;
		display_rects = g_ptr_array_new ();
		subimages = g_ptr_array_new ();
		current_subimage = NULL;
		source = NULL;
	}
	
	~DZParserInfo ()
	{
		if (current_rect)
			delete current_rect;
		
		if (display_rects) {
			for (guint i = 0; i < display_rects->len; i++)
				delete (DisplayRect *) display_rects->pdata[i];
			
			g_ptr_array_free (display_rects, true);
		}
		
		if (current_subimage)
			delete current_subimage;
		
		if (subimages) {
			for (guint i = 0; i < subimages->len; i++)
				((MultiScaleSubImage *) subimages->pdata[i])->unref ();
			
			g_ptr_array_free (subimages, true);
		}
		
		g_free (server_format);
		g_free (format);
	}
};

static void start_element (void *data, const char *el, const char **attr);
static void end_element (void *data, const char *el);

static bool
get_tile_layer (int level, int x, int y, Uri **uri, void *userdata)
{
	return ((DeepZoomImageTileSource *)userdata)->GetTileLayer (level, x, y, uri);
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

	image_uri = Uri::Create (image);
	g_free (image);

	// DRTs: #511
	if (!baseUri->IsAbsolute ()) {
		*uri = Uri::CombineWithSourceLocation (GetDeployment (), baseUri, image_uri, true);
	} else {
		*uri = Uri::Create (baseUri, image);
	}

	delete image_uri;

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
	
	// set isFinal for the parser to complete
	if (!XML_Parse (parser, NULL, 0, 1)) {
		g_warning ("Parser error at line %d:\n%s\n", (int)XML_GetCurrentLineNumber (parser), XML_ErrorString(XML_GetErrorCode(parser)));
		Abort ();
		DownloaderFailed ();
		return;
	}
	
	if (info->error) {
		Abort ();
		DownloaderFailed ();
		return;
	}
	
	if (!info->is_collection) {
		SetImageWidth (info->image_width);
		SetImageHeight (info->image_height);
		SetTileOverlap (info->overlap);
		display_rects = info->display_rects;
		info->display_rects = NULL;
	} else {
		is_collection = info->is_collection;
		max_level = info->max_level;
		subimages = info->subimages;
		info->subimages = NULL;
	}

	SetTileWidth (info->tile_size);
	SetTileHeight (info->tile_size);
	format = g_strdup (info->format);
	server_format = g_strdup (info->server_format);
	parsed = true;

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
#define DZParsedHeight        (1 << 6)
#define DZParsedWidth         (1 << 7)
#define DZParsedSource        (1 << 8)
#define DZParsedId            (1 << 9)
#define DZParsedN             (1 << 10)
#define DZParsedX             (1 << 11)
#define DZParsedY             (1 << 12)

#define DZParsedCollection (/*DZParsedServerFormat |*/ DZParsedFormat | DZParsedTileSize | DZParsedMaxLevel)
#define DZParsedImage (/*DZParsedServerFormat |*/ DZParsedFormat | DZParsedTileSize | DZParsedOverlap)
#define DZParsedRect (DZParsedWidth | DZParsedHeight | DZParsedX | DZParsedY)
#define DZParsedDisplayRect (DZParsedMinLevel | DZParsedMaxLevel)
#define DZParsedViewport (DZParsedWidth | DZParsedX | DZParsedY)
#define DZParsedSize (DZParsedWidth | DZParsedHeight)

// DeepZoomParsing
static void
start_element (void *data, const char *el, const char **attr)
{
	DZParserInfo *info = (DZParserInfo *) data;
	bool failed = false;
	guint32 parsed = 0;
	int err;
	
	if (info->error)
		return;
	
	if (info->skip >= 0) {
		(info->depth)++;
		return;
	}
	
	switch (info->depth) {
	case 0:
		// Image or Collection
		if (!g_ascii_strcasecmp ("Image", el)) {
			info->is_collection = false;
			
			for (int i = 0; attr[i] && !failed; i += 2) {
				if (!g_ascii_strcasecmp ("ServerFormat", attr[i])) {
					if (!(parsed & DZParsedServerFormat)) {
						info->server_format = g_strdup (attr[i+1]);
						parsed |= DZParsedServerFormat;
					} else
						failed = true;
				} else if (!g_ascii_strcasecmp ("Format", attr[i])) {
					if (!(parsed & DZParsedFormat)) {
						info->format = g_strdup (attr[i+1]);
						parsed |= DZParsedFormat;
					} else
						failed = true;
				} else if (!g_ascii_strcasecmp ("TileSize", attr[i])) {
					if (!(parsed & DZParsedTileSize)) {
						failed = !Int32TryParse (attr[i+1], &info->tile_size, &err) || info->tile_size <= 0;
						parsed |= DZParsedTileSize;
					} else
						failed = true;
				} else if (!g_ascii_strcasecmp ("Overlap", attr[i])) {
					if (!(parsed & DZParsedOverlap)) {
						failed = !Int32TryParse (attr[i+1], &info->overlap, &err) || info->overlap < 0;
						parsed |= DZParsedOverlap;
					} else
						failed = true;
				} else {
					LOG_MSI ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
				}
			}
			
			if (failed || (parsed & DZParsedImage) != DZParsedImage) {
				printf ("DeepZoom Image error: failed=%s; parsed=%x\n", failed ? "true" : "false", parsed);
				info->error = true;
			}
		} else if (!g_ascii_strcasecmp ("Collection", el)) {
			info->is_collection = true;
			
			for (int i = 0; attr[i] && !failed; i += 2) {
				if (!g_ascii_strcasecmp ("ServerFormat", attr[i])) {
					if (!(parsed & DZParsedServerFormat)) {
						info->server_format = g_strdup (attr[i+1]);
						parsed |= DZParsedServerFormat;
					} else
						failed = true;
				} else if (!g_ascii_strcasecmp ("Format", attr[i])) {
					if (!(parsed & DZParsedFormat)) {
						info->format = g_strdup (attr[i+1]);
						parsed |= DZParsedFormat;
					} else
						failed = true;
				} else if (!g_ascii_strcasecmp ("TileSize", attr[i])) {
					if (!(parsed & DZParsedTileSize)) {
						failed = !Int32TryParse (attr[i+1], &info->tile_size, &err) || info->tile_size <= 0;
						parsed |= DZParsedTileSize;
					} else
						failed = true;
				} else if (!g_ascii_strcasecmp ("MaxLevel", attr[i])) {
					if (!(parsed & DZParsedMaxLevel)) {
						failed = !Int32TryParse (attr[i+1], &info->max_level, &err) || info->max_level < 0;
						parsed |= DZParsedMaxLevel;
					} else
						failed = true;
				} else {
					LOG_MSI ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
				}
			}
			
			if (failed || (parsed & DZParsedCollection) != DZParsedCollection) {
				printf ("DeepZoom Collection error: failed=%s; parsed=%x\n", failed ? "true" : "false", parsed);
				info->error = true;
			}
		} else {
			g_warning ("Unexpected element %s\n", el);
			info->error = true;
		}
		break;
	case 1:
		if (!info->is_collection) {
			// Size or DisplayRects
			if (!g_ascii_strcasecmp ("Size", el)) {
				for (int i = 0; attr[i] && !failed; i += 2) {
					if (!g_ascii_strcasecmp ("Width", attr[i])) {
						if (!(parsed & DZParsedWidth)) {
							failed = !Int32TryParse (attr[i+1], &info->image_width, &err) || info->image_width <= 0;
							parsed |= DZParsedWidth;
						} else
							failed = true;
					} else if (!g_ascii_strcasecmp ("Height", attr[i])) {
						if (!(parsed & DZParsedHeight)) {
							failed = !Int32TryParse (attr[i+1], &info->image_height, &err) || info->image_height <= 0;
							parsed |= DZParsedHeight;
						} else
							failed = true;
					} else {
						LOG_MSI ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
					}
				}
				
				if (failed || (parsed & DZParsedSize) != DZParsedSize) {
					printf ("DeepZoom Image Size error: failed=%s; parsed=%x\n", failed ? "true" : "false", parsed);
					info->error = true;
				}
			} else if (!g_ascii_strcasecmp ("DisplayRects", el)) {
				// no attributes, only contains DisplayRect element
			} else {
				g_warning ("Unexpected element %s\n", el);
				info->error = true;
			}
		} else {
			if (!g_ascii_strcasecmp ("Items", el)) {
				// no attributes, only contains <I> elements
			} else {
				g_warning ("Unexpected element %d %s\n", info->depth, el);
				info->error = true;
			}
		}
		break;
	case 2:
		if (!info->is_collection) {
			// DisplayRect elements
			if (!g_ascii_strcasecmp ("DisplayRect", el)) {
				int min_level = 0, max_level = 0;
				
				for (int i = 0; attr[i]; i += 2) {
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
					} else {
						LOG_MSI ("\tunparsed arg %s: %s\n", attr[i], attr[i+1]);
					}
				}
				
				if (max_level < min_level)
					failed = true;
				
				if (failed || (parsed & DZParsedDisplayRect) != DZParsedDisplayRect) {
					printf ("DeepZoom DisplayRect error: failed=%s; parsed=%x\n", failed ? "true" : "false", parsed);
					info->error = true;
					break;
				}
				
				info->current_rect = new DisplayRect (min_level, max_level);
			} else {
				g_warning ("Unexpected element %s\n", el);
				info->error = true;
			}
		} else {
			if (!g_ascii_strcasecmp ("I", el)) {
				info->current_subimage = new SubImage ();
				
				for (int i = 0; attr[i]; i += 2) {
					if (!g_ascii_strcasecmp ("Source", attr[i])) {
						if (!(parsed & DZParsedSource)) {
							info->current_subimage->source = Uri::Create (attr [i+1]);
							failed = info->current_subimage->source == NULL;
							parsed |= DZParsedSource;
						} else
							failed = true;
					} else if (!g_ascii_strcasecmp ("Id", attr[i])) {
						if (!(parsed & DZParsedId)) {
							failed = !Int32TryParse (attr[i+1], &info->current_subimage->id, &err) ||
								info->current_subimage->id < 0;
							parsed |= DZParsedId;
						} else
							failed = true;
					} else if (!g_ascii_strcasecmp ("N", attr[i])) {
						if (!(parsed & DZParsedN)) {
							failed = !Int32TryParse (attr[i+1], &info->current_subimage->n, &err) ||
								info->current_subimage->n < 0;
							parsed |= DZParsedN;
						} else
							failed = true;
					} else {
						LOG_MSI ("\tunparsed arg %s: %s\n", attr[i], attr[i+1]);
					}
					
					if (failed || !parsed) {
						printf ("DeepZoom I error: failed=%s; parsed=%x\n", failed ? "true" : "false", parsed);
						info->error = true;
					}
				}
			} else {
				g_warning ("Unexpected element %d %s\n", info->depth, el);
				info->error = true;
			}
		}
		break;
	case 3:
		if (!info->is_collection) {
			// Rect element
			if (!g_ascii_strcasecmp ("Rect", el)) {
				int val = 0;
				
				if (!info->current_rect) {
					info->error = true;
					break;
				}
				
				for (int i = 0; attr[i]; i += 2) {
					if (!g_ascii_strcasecmp ("Height", attr[i])) {
						if (!(parsed & DZParsedHeight)) {
							failed = !Int32TryParse (attr[i+1], &val, &err) || val <= 0;
							info->current_rect->rect.height = (double) val;
							parsed |= DZParsedHeight;
						} else
							failed = true;
					} else if (!g_ascii_strcasecmp ("Width", attr[i])) {
						if (!(parsed & DZParsedWidth)) {
							failed = !Int32TryParse (attr[i+1], &val, &err) || val <= 0;
							info->current_rect->rect.width = (double) val;
							parsed |= DZParsedWidth;
						} else
							failed = true;
					} else if (!g_ascii_strcasecmp ("X", attr[i])) {
						if (!(parsed & DZParsedX)) {
							failed = !Int32TryParse (attr[i+1], &val, &err) || val < 0;
							info->current_rect->rect.x = (double) val;
							parsed |= DZParsedX;
						} else
							failed = true;
					} else if (!g_ascii_strcasecmp ("Y", attr[i])) {
						if (!(parsed & DZParsedY)) {
							failed = !Int32TryParse (attr[i+1], &val, &err) || val < 0;
							info->current_rect->rect.y = (double) val;
							parsed |= DZParsedY;
						} else
							failed = true;
					} else {
						LOG_MSI ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
					}
					
					if (failed || !(parsed & DZParsedRect)) {
						printf ("DeepZoom Rect error: failed=%s; parsed=%x\n", failed ? "true" : "false", parsed);
						info->error = true;
						break;
					}
				}
				
				g_ptr_array_add (info->display_rects, info->current_rect);
				info->current_rect = NULL;
			} else {
				g_warning ("Unexpected element %s\n", el);
				info->error = true;
			}
		} else {
			if (!g_ascii_strcasecmp ("Size", el)) {
				if (!info->current_subimage) {
					info->error = true;
					break;
				}
				
				info->current_subimage->has_size = true;
				
				for (int i = 0; attr [i]; i += 2) {
					if (!g_ascii_strcasecmp ("Width", attr[i])) {
						if (!(parsed & DZParsedWidth)) {
							failed = !Int32TryParse (attr[i+1], &info->current_subimage->width, &err) ||
								info->current_subimage->width <= 0;
							parsed |= DZParsedWidth;
						} else
							failed = true;
					} else if (!g_ascii_strcasecmp ("Height", attr[i])) {
						if (!(parsed & DZParsedHeight)) {
							failed = !Int32TryParse (attr[i+1], &info->current_subimage->height, &err) ||
								info->current_subimage->height <= 0;
							parsed |= DZParsedHeight;
						} else
							failed = true;
					} else {
						LOG_MSI ("\tunparsed attr %s.%s: %s\n", el, attr[i], attr[i+1]);
					}
				}
				
				if (failed || (parsed & DZParsedSize) != DZParsedSize) {
					printf ("DeepZoom Subimage Size error: failed=%s; parsed=%x\n", failed ? "true" : "false", parsed);
					info->error = true;
				}
			} else if (!g_ascii_strcasecmp ("Viewport", el)) {
				char *inend;
				
				if (!info->current_subimage) {
					info->error = true;
					break;
				}
				
				info->current_subimage->has_viewport = true;
				
				for (int i = 0; attr [i]; i += 2) {
					inend = (char *) "";
					
					if (!g_ascii_strcasecmp ("Width", attr[i])) {
						if (!(parsed & DZParsedWidth)) {
							info->current_subimage->vp_w = g_ascii_strtod (attr[i+1], &inend);
							if (errno != 0 || info->current_subimage->vp_w < 0)
								failed = true;
							parsed |= DZParsedWidth;
						} else
							failed = true;
					} else if (!g_ascii_strcasecmp ("X", attr[i])) {
						if (!(parsed & DZParsedX)) {
							info->current_subimage->vp_x = g_ascii_strtod (attr[i+1], &inend);
							if (errno != 0)
								failed = true;
							parsed |= DZParsedX;
						} else
							failed = true;
					} else if (!g_ascii_strcasecmp ("Y", attr[i])) {
						if (!(parsed & DZParsedY)) {
							info->current_subimage->vp_y = g_ascii_strtod (attr[i+1], &inend);
							if (errno != 0)
								failed = true;
							parsed |= DZParsedY;
						} else
							failed = true;
					} else {
						LOG_MSI ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
					}
					
					// make sure there is no trailing data (except lwsp)
					while (*inend == ' ')
						inend++;
					
					if (*inend)
						failed = true;
				}
				
				if (failed || (parsed & DZParsedViewport) != DZParsedViewport) {
					printf ("DeepZoom Viewport error: failed=%s; parsed=%x\n", failed ? "true" : "false", parsed);
					info->error = true;
				}
			} else {
				g_warning ("Unexpected element %s\n", el);
				info->error = true;
			}
		}
		break;
	}
	
	(info->depth)++;
}

void
DeepZoomImageTileSource::EndElement (void *data, const char *el)
{
	DZParserInfo *info = (DZParserInfo *) data;
	
	if (info->error)
		return;
	
	(info->depth)--;

	if (info->skip < 0) {
		switch (info->depth) {
		case 2:
			if (info->is_collection && !g_ascii_strcasecmp ("I", el)) {
				const Uri *uri = info->source->GetUriSource ();
				DeepZoomImageTileSource *subsource;
				MultiScaleSubImage *subi;
				
				if (!uri)
					break;
				
				subsource = new DeepZoomImageTileSource (info->current_subimage->source, TRUE);
				subi = new MultiScaleSubImage (uri, subsource, info->current_subimage->id, 
							       info->current_subimage->n);
				subsource->SetImageWidth (info->current_subimage->width);
				subsource->SetImageHeight (info->current_subimage->height);
				subsource->server_format = g_strdup (info->server_format);
				subsource->format = g_strdup (info->format);
				
				if (info->current_subimage->has_viewport) {
					subi->SetViewportOrigin (new Point (info->current_subimage->vp_x, info->current_subimage->vp_y));
					subi->SetViewportWidth (info->current_subimage->vp_w);
				}
				
				if (info->current_subimage->has_size) {
					subi->SetValue (MultiScaleSubImage::AspectRatioProperty, Value ((double)info->current_subimage->width/(double)info->current_subimage->height));
				}
				
				g_ptr_array_add (info->subimages, subi);
				info->current_subimage = NULL;
			}
			break;
		}
	}

	if (info->skip == info->depth)
		info->skip = -1;	
}

static void
end_element (void *data, const char *el)
{
	DZParserInfo *info = (DZParserInfo *) data;
	DeepZoomImageTileSource *dzits = (DeepZoomImageTileSource *) info->source;
	
	g_assert (dzits != NULL);
	
	dzits->EndElement (info, el);
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
