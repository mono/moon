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

#include "application.h"
#include "debug.h"
#include "runtime.h"
#include "deployment.h"

#include "deepzoomimagetilesource.h"
#include "multiscalesubimage.h"
#include "uri.h"

class DisplayRect {
 public:
	long min_level;
	long max_level;
	Rect rect;

	DisplayRect (long minLevel, long maxLevel)
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
	long width;
	long height;
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
	int depth;
	int skip;
	bool error;
	DeepZoomImageTileSource *source;

	bool is_collection;

	// Image attributes
	int overlap;
	long image_width, image_height;
	DisplayRect *current_rect;
	GPtrArray *display_rects;

	// Collection attributes
	int max_level;
	SubImage *current_subimage;
	GPtrArray *subimages;

	// Common attributes
	char *format;
	char *server_format;
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
	}
};

static void start_element (void *data, const char *el, const char **attr);
static void end_element (void *data, const char *el);

static bool
get_tile_layer (int level, int x, int y, Uri *uri, void *userdata)
{
	return ((DeepZoomImageTileSource *)userdata)->GetTileLayer (level, x, y, uri);
}

void
DeepZoomImageTileSource::Init ()
{
	SetObjectType (Type::DEEPZOOMIMAGETILESOURCE);

	downloaded = false;
	parsed = false;
	format = g_strdup ("");
	server_format = NULL;
	get_tile_func = get_tile_layer;
	display_rects = NULL;
	parsed_callback = NULL;
	failed_callback = NULL;
	sourcechanged_callback = NULL;
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
DeepZoomImageTileSource::GetTileLayer (int level, int x, int y, Uri *uri)
{
	//check if there tile is listed in DisplayRects
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
	const char *filename, *ext;
	char *image;
	
	if (!baseUri)
		return false;
	
	if ((filename = strrchr (baseUri->path, '/')))
		filename ++;
	else
		filename = baseUri->path;
	
	if (GetServerFormat () && !g_ascii_strcasecmp (GetServerFormat (), "SmoothStreaming")) {
		image = g_strdup_printf ("/QualityLevels(%d)/RawFragments(tile=%d)", 100+level, y*1000000000+x);
	} else {
		if (!(ext = strrchr (filename, '.')))
			return false;
		
		image = g_strdup_printf ("%.*s_files/%d/%d_%d.%s", (int) (ext - filename), filename, level, x, y, format);
	}

	Uri::Copy (baseUri, uri);
	uri->Combine (image);
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
		//Init xml parser
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
	Uri *uri = GetUriSource ();

	if (current && uri) {
		downloaded = true;
		if (get_resource_aborter)
			delete get_resource_aborter;
		get_resource_aborter = new Cancellable ();
		current->GetResource (GetResourceBase(), uri, resource_notify, dz_write, MediaPolicy, HttpRequest::DisableFileStorage, get_resource_aborter, this);
	}
}

void
DeepZoomImageTileSource::DownloaderComplete ()
{
	//set isFinal for the parser to complete
	if (!XML_Parse (parser, NULL, 0, 1)) {
		g_warning ("Parser error at line %d:\n%s\n", (int)XML_GetCurrentLineNumber (parser), XML_ErrorString(XML_GetErrorCode(parser)));
		Abort ();
		DownloaderFailed ();
		return;
	}

	DZParserInfo *info = (DZParserInfo *) XML_GetUserData (parser);

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
	
	if (parsed_callback)
		parsed_callback (cb_userdata);
}

void
DeepZoomImageTileSource::DownloaderFailed ()
{
	LOG_MSI ("DZITS::dl failed\n");
	if (failed_callback)
		failed_callback (cb_userdata);
}

void
DeepZoomImageTileSource::UriSourceChanged ()
{
	parsed = false;
	downloaded = false;
	if (!nested) {
		Download ();
	}	
		
	if (sourcechanged_callback)
		sourcechanged_callback (cb_userdata);
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

//DeepZoomParsing
static void
start_element (void *data, const char *el, const char **attr)
{
	DZParserInfo *info = (DZParserInfo *) data;

	if (info->skip >= 0) {
		(info->depth)++;
		return;
	}
	
	switch (info->depth) {
	case 0:
		//Image or Collection
		if (!g_ascii_strcasecmp ("Image", el)) {
			info->is_collection = false;
			
			for (int i = 0; attr[i]; i += 2) {
				if (!g_ascii_strcasecmp ("Format", attr[i]))
					info->format = g_strdup (attr[i+1]);
			        else if (!g_ascii_strcasecmp ("ServerFormat", attr[i]))
					info->server_format = g_strdup (attr[i+1]);
				else if (!g_ascii_strcasecmp ("TileSize", attr[i]))
					info->tile_size = atoi (attr[i+1]);
				else if (!g_ascii_strcasecmp ("Overlap", attr[i]))
					info->overlap = atoi (attr[i+1]);
				else
					LOG_MSI ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
			}
		} else if (!g_ascii_strcasecmp ("Collection", el)) {
			info->is_collection = true;
			
			for (int i = 0; attr[i]; i += 2) {
				if (!g_ascii_strcasecmp ("Format", attr[i]))
					info->format = g_strdup (attr[i+1]);
				else if (!g_ascii_strcasecmp ("ServerFormat", attr[i]))
					info->server_format = g_strdup (attr[i+1]);
				else if (!g_ascii_strcasecmp ("TileSize", attr[i]))
					info->tile_size = atoi (attr[i+1]);
				else if (!g_ascii_strcasecmp ("MaxLevel", attr[i]))
					info->max_level = atoi (attr[i+1]);
				else
					LOG_MSI ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
			}
		} else {
			g_warning ("Unexpected element %s\n", el);
			info->error = true;
		}
		break;
	case 1:
		if (!info->is_collection) {
			//Size or DisplayRects
			if (!g_ascii_strcasecmp ("Size", el)) {
				for (int i = 0; attr[i]; i += 2) {
					if (!g_ascii_strcasecmp ("Width", attr[i]))
						info->image_width = atol (attr[i+1]);
					else if (!g_ascii_strcasecmp ("Height", attr[i]))
						info->image_height = atol (attr[i+1]);
					else
						LOG_MSI ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
				}
			} else if (!g_ascii_strcasecmp ("DisplayRects", el)) {
				//no attributes, only contains DisplayRect element
			} else {
				g_warning ("Unexpected element %s\n", el);
				info->error = true;
			}
		} else {
			if (!g_ascii_strcasecmp ("Items", el)) {
				//no attributes, only contains <I> elements
			} else {
				g_warning ("Unexpected element %d %s\n", info->depth, el);
				info->error = true;
			}
		}
		break;
	case 2:
		if (!info->is_collection) {
			//DisplayRect elts
			if (!g_ascii_strcasecmp ("DisplayRect", el)) {
				long min_level = 0, max_level = 0;
				
				for (int i = 0; attr[i]; i += 2) {
					if (!g_ascii_strcasecmp ("MinLevel", attr[i]))
						min_level = atol (attr[i+1]);
					else if (!g_ascii_strcasecmp ("MaxLevel", attr[i]))
						max_level = atol (attr[i+1]);
					else
						LOG_MSI ("\tunparsed arg %s: %s\n", attr[i], attr[i+1]);
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
					if (!g_ascii_strcasecmp ("N", attr[i]))
						info->current_subimage->n = atoi (attr[i+1]);
					else if (!g_ascii_strcasecmp ("Id", attr[i]))
						info->current_subimage->id = atoi (attr[i+1]);
					else if (!g_ascii_strcasecmp ("Source", attr[i])) {
						info->current_subimage->source = new Uri ();
						info->current_subimage->source->Parse (attr[i+1]);
					} else
						LOG_MSI ("\tunparsed arg %s: %s\n", attr[i], attr[i+1]);
				}
			} else {
				g_warning ("Unexpected element %d %s\n", info->depth, el);
				info->error = true;
			}
		}
		break;
	case 3:
		if (!info->is_collection) {
			//Rect elt
			if (!g_ascii_strcasecmp ("Rect", el)) {
				if (!info->current_rect) {
					info->error = true;
					break;
				}
				
				for (int i = 0; attr[i]; i += 2) {
					if (!g_ascii_strcasecmp ("X", attr[i]))
						info->current_rect->rect.x = (double)atol (attr[i+1]);
					else if (!g_ascii_strcasecmp ("Y", attr[i]))
						info->current_rect->rect.y = (double)atol (attr[i+1]);
					else if (!g_ascii_strcasecmp ("Width", attr[i]))
						info->current_rect->rect.width = (double)atol (attr[i+1]);
					else if (!g_ascii_strcasecmp ("Height", attr[i]))
						info->current_rect->rect.height = (double)atol (attr[i+1]);
					else
						LOG_MSI ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
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
					if (!g_ascii_strcasecmp ("Width", attr[i]))
						info->current_subimage->width = atol (attr[i+1]);
					else if (!g_ascii_strcasecmp ("Height", attr[i]))
						info->current_subimage->height = atol (attr[i+1]);
					else
						LOG_MSI ("\tunparsed attr %s.%s: %s\n", el, attr[i], attr[i+1]);
				}
			} else if (!g_ascii_strcasecmp ("Viewport", el)) {
				if (!info->current_subimage) {
					info->error = true;
					break;
				}

				info->current_subimage->has_viewport = true;
				
				for (int i = 0; attr [i]; i += 2) {
					if (!g_ascii_strcasecmp ("X", attr[i]))
						info->current_subimage->vp_x = g_ascii_strtod (attr[i+1], NULL);
					else if (!g_ascii_strcasecmp ("Y", attr[i]))
						info->current_subimage->vp_y = g_ascii_strtod (attr[i+1], NULL);
					else if (!g_ascii_strcasecmp ("Width", attr[i]))
						info->current_subimage->vp_w = g_ascii_strtod (attr[i+1], NULL);
					else
						LOG_MSI ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
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
	
	(info->depth)--;

	if (info->skip < 0) {
		switch (info->depth) {
		case 2:
			if (info->is_collection && !g_ascii_strcasecmp ("I", el)) {
				DeepZoomImageTileSource *subsource = new DeepZoomImageTileSource (info->current_subimage->source, TRUE);
				MultiScaleSubImage *subi = new MultiScaleSubImage (info->source->GetUriSource (),
										   subsource, 
										   info->current_subimage->id, 
										   info->current_subimage->n);
				subsource->SetImageWidth (info->current_subimage->width);
				subsource->SetImageHeight (info->current_subimage->height);
				subsource->format = info->format;
				subsource->server_format = info->format;
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
