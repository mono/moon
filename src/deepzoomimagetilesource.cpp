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

#include "debug.h"
#include "runtime.h"

#include "deepzoomimagetilesource.h"
#include "multiscalesubimage.h"
#include "uri.h"
#include "file-downloader.h"

class DisplayRect
{
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

class SubImage
{
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
};

class DZParserinfo
{
 public:
	int depth;
	int skip;
	bool error;
	DeepZoomImageTileSource *source;

	bool isCollection;

	//Image attributes
	int overlap;
	long image_width, image_height;
	DisplayRect *current_rect;
	GList *display_rects;

	//Collection attributes
	int max_level;
	SubImage *current_subimage;
	GList *sub_images;

	//Common attributes
	char *format;
	int tile_size;


	DZParserinfo ()
	{
		depth = 0;
		skip = -1;
		error = false;
		format = NULL;
		image_width = image_height = tile_size = overlap = 0;
		current_rect = NULL;
		display_rects = NULL;
		sub_images = NULL;
		current_subimage = NULL;
		source = NULL;
	}
};

void start_element (void *data, const char *el, const char **attr);
void end_element (void *data, const char *el);

bool
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
	format = NULL;
	get_tile_func = get_tile_layer;
	display_rects = NULL;
	parsed_callback = NULL;	
	failed_callback = NULL;
	sourcechanged_callback = NULL;
	isCollection = false;
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
	delete get_resource_aborter;
}

void
DeepZoomImageTileSource::Abort ()
{
	if (get_resource_aborter)
		get_resource_aborter->Cancel ();
	if (parser)
		XML_ParserFree (parser);
	parser = NULL;
}

bool
DeepZoomImageTileSource::GetTileLayer (int level, int x, int y, Uri *uri)
{
	//check if there tile is listed in DisplayRects
	if (display_rects) {
		DisplayRect *cur;
		int i =0;
		bool found = false;
		int layers;
		
		frexp ((double) MAX (GetImageWidth (), GetImageHeight ()), &layers);

		while ((cur = (DisplayRect*)g_list_nth_data (display_rects, i))) {
			i++;

			if (!(cur->min_level <= level && level <= cur->max_level))
				continue;

			int vtilesize = GetTileWidth () * (layers + 1 - level);
			Rect virtualtile = Rect (x * vtilesize, y * vtilesize, vtilesize, vtilesize);
			if (cur->rect.IntersectsWith (virtualtile)) {
				found = true;
				break;
			}
		}

		if (!found)
			return false;
	}
	
	const Uri *baseUri = GetValue (DeepZoomImageTileSource::UriSourceProperty)->AsUri ();
	const char *filename, *ext;
	char *image;
	
	if (!baseUri)
		return false;
	
	if ((filename = strrchr (baseUri->path, '/')))
		filename ++;
	else
		filename = baseUri->path;
	
	if (!(ext = strrchr (filename, '.')))
		return false;
	
	image = g_strdup_printf ("%.*s_files/%d/%d_%d.%s", (int) (ext - filename), filename, level, x, y, format);
	
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
dz_write (void *buffer, gint32 offset, gint32 n, gpointer data)
{
	DeepZoomImageTileSource *dzits = (DeepZoomImageTileSource *) data;
	dzits->XmlWrite ((char *) buffer, offset, n);
}

void
DeepZoomImageTileSource::XmlWrite (char* buffer, gint32 offset, gint32 n)
{
	if (offset == 0) {
		//Init xml parser
		LOG_MSI ("Start parsing DeepZoom\n");
		parser = XML_ParserCreate (NULL);
		XML_SetElementHandler (parser, start_element, end_element);
		DZParserinfo *info = new DZParserinfo ();
		info->source = this;
		XML_SetUserData (parser, info);
	}

	if (!XML_Parse (parser, buffer, n, 0)) {
		printf ("Parser error at line %d:\n%s\n", (int)XML_GetCurrentLineNumber (parser), XML_ErrorString(XML_GetErrorCode(parser)));
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
		current->GetResource (GetResourceBase(), uri, resource_notify, dz_write, MediaPolicy, get_resource_aborter, this);
	}
}

void
DeepZoomImageTileSource::DownloaderComplete ()
{
	//set isFinal for the parser to complete
	if (!XML_Parse (parser, NULL, 0, 1)) {
		printf ("Parser error at line %d:\n%s\n", (int)XML_GetCurrentLineNumber (parser), XML_ErrorString(XML_GetErrorCode(parser)));
		Abort ();
		DownloaderFailed ();
		return;
	}

	DZParserinfo *info = (DZParserinfo *)XML_GetUserData (parser);

	if (!info->isCollection) {
		SetImageWidth (info->image_width);
		SetImageHeight (info->image_height);
		SetTileOverlap (info->overlap);
		display_rects = info->display_rects;
	} else {
		subimages = info->sub_images;
		isCollection = info->isCollection;
		maxLevel = info->max_level;
	}

	SetTileWidth (info->tile_size);
	SetTileHeight (info->tile_size);
	format = g_strdup (info->format);

	parsed = true;

	LOG_MSI ("Done parsing...\n");

	XML_ParserFree (parser);
	parser = NULL;

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
void
start_element (void *data, const char *el, const char **attr)
{
	DZParserinfo *info = (DZParserinfo*)data;

	if (info->skip >= 0) {
		(info->depth)++;
		return;
	}
	switch (info->depth) {
	case 0:
		//Image or Collection
		if (!g_ascii_strcasecmp ("Image", el)) {
			info->isCollection = false;
			int i;
			for (i = 0; attr[i]; i+=2)
				if (!g_ascii_strcasecmp ("Format", attr[i]))
					info->format = g_strdup (attr[i+1]);
				else if (!g_ascii_strcasecmp ("TileSize", attr[i]))
					info->tile_size = atoi (attr[i+1]);
				else if (!g_ascii_strcasecmp ("Overlap", attr[i]))
					info->overlap = atoi (attr[i+1]);
				else
					LOG_MSI ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
		} else if (!g_ascii_strcasecmp ("Collection", el)) {
			info->isCollection = true;
			int i;
			for (i = 0; attr[i]; i+=2)
				if (!g_ascii_strcasecmp ("Format", attr[i]))
					info->format = g_strdup (attr[i+1]);
				else if (!g_ascii_strcasecmp ("TileSize", attr[i]))
					info->tile_size = atoi (attr[i+1]);
				else if (!g_ascii_strcasecmp ("MaxLevel", attr[i]))
					info->max_level = atoi (attr[i+1]);
				else
					LOG_MSI ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
		} else {
			printf ("Unexpected element %s\n", el);
			info->error = true;
		}
		break;
	case 1:
		if (!info->isCollection) {
			//Size or DisplayRects
			if (!g_ascii_strcasecmp ("Size", el)) {
				int i;
				for (i = 0; attr[i]; i+=2)
					if (!g_ascii_strcasecmp ("Width", attr[i]))
						info->image_width = atol (attr[i+1]);
					else if (!g_ascii_strcasecmp ("Height", attr[i]))
						info->image_height = atol (attr[i+1]);
					else
						LOG_MSI ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
			} else if (!g_ascii_strcasecmp ("DisplayRects", el)) {
				//no attributes, only contains DisplayRect element
			} else {
				printf ("Unexpected element %s\n", el);
				info->error = true;
			}
		} else {
			if (!g_ascii_strcasecmp ("Items", el)) {
				//no attributes, only contains <I> elements
			} else {
				printf ("Unexpected element %d %s\n", info->depth, el);
				info->error = true;
			}
		}
		break;
	case 2:
		if (!info->isCollection) {
			//DisplayRect elts
			if (!g_ascii_strcasecmp ("DisplayRect", el)) {
				long min_level = 0, max_level = 0;
				int i;
				for (i = 0; attr[i]; i+=2)
					if (!g_ascii_strcasecmp ("MinLevel", attr[i]))
						min_level = atol (attr[i+1]);
					else if (!g_ascii_strcasecmp ("MaxLevel", attr[i]))
						max_level = atol (attr[i+1]);
					else
						LOG_MSI ("\tunparsed arg %s: %s\n", attr[i], attr[i+1]);
				info->current_rect = new DisplayRect (min_level, max_level);
			} else {
				printf ("Unexpected element %s\n", el);
				info->error = true;
			}
		} else {
			if (!g_ascii_strcasecmp ("I", el)) {
				info->current_subimage = new SubImage ();
				int i;
				for (i = 0; attr[i]; i+=2)
					if (!g_ascii_strcasecmp ("N", attr[i]))
						info->current_subimage->n = atoi (attr[i+1]);
					else if (!g_ascii_strcasecmp ("Id", attr[i]))
						info->current_subimage->id = atoi (attr[i+1]);
					else if (!g_ascii_strcasecmp ("Source", attr[i])) {
						info->current_subimage->source = new Uri ();
						info->current_subimage->source->Parse (attr[i+1]);
					} else
						LOG_MSI ("\tunparsed arg %s: %s\n", attr[i], attr[i+1]);

			} else {
				printf ("Unexpected element %d %s\n", info->depth, el);
				info->error = true;
			}
		}
		break;
	case 3:
		if (!info->isCollection) {
			//Rect elt
			if (!g_ascii_strcasecmp ("Rect", el)) {
				if (!info->current_rect) {
					info->error = true;
					break;
				}
				int i;
				for (i = 0; attr[i]; i+=2)
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
				info->display_rects = g_list_append (info->display_rects, info->current_rect);
				info->current_rect = NULL;
			} else {
				printf ("Unexpected element %s\n", el);
				info->error = true;
			}
		} else {
			if (!g_ascii_strcasecmp ("Size", el)) {
				if (!info->current_subimage) {
					info->error = true;
					break;
				}

				info->current_subimage->has_size = true;

				int i;
				for (i = 0; attr [i]; i+=2)
					if (!g_ascii_strcasecmp ("Width", attr[i]))
						info->current_subimage->width = atol (attr[i+1]);
					else if (!g_ascii_strcasecmp ("Height", attr[i]))
						info->current_subimage->height = atol (attr[i+1]);
					else
						LOG_MSI ("\tunparsed attr %s.%s: %s\n", el, attr[i], attr[i+1]);
			} else if (!g_ascii_strcasecmp ("Viewport", el)) {
				if (!info->current_subimage) {
					info->error = true;
					break;
				}

				info->current_subimage->has_viewport = true;

				int i;
				for (i = 0; attr [i]; i+=2)
					if (!g_ascii_strcasecmp ("X", attr[i]))
						info->current_subimage->vp_x = g_ascii_strtod (attr[i+1], NULL);
					else if (!g_ascii_strcasecmp ("Y", attr[i]))
						info->current_subimage->vp_y = g_ascii_strtod (attr[i+1], NULL);
					else if (!g_ascii_strcasecmp ("Width", attr[i]))
						info->current_subimage->vp_w = g_ascii_strtod (attr[i+1], NULL);
					else
						LOG_MSI ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
			} else {
				printf ("Unexpected element %s\n", el);
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
	DZParserinfo *info = (DZParserinfo*)data;
	(info->depth)--;

	if (info->skip < 0) {
		switch (info->depth) {
		case 2:
			if (info->isCollection)
				if (!g_ascii_strcasecmp ("I", el)) {
					DeepZoomImageTileSource *subsource = new DeepZoomImageTileSource (info->current_subimage->source, TRUE);
					MultiScaleSubImage *subi = new MultiScaleSubImage (info->source->GetUriSource (),
											   subsource, 
											   info->current_subimage->id, 
											   info->current_subimage->n);
					subsource->SetImageWidth (info->current_subimage->width);
					subsource->SetImageHeight (info->current_subimage->height);
					subsource->format = info->format;
					if (info->current_subimage->has_viewport) {
						subi->SetViewportOrigin (new Point (info->current_subimage->vp_x, info->current_subimage->vp_y));
						subi->SetViewportWidth (info->current_subimage->vp_w);
					}

					if (info->current_subimage->has_size) {
						subi->SetValue (MultiScaleSubImage::AspectRatioProperty, Value ((double)info->current_subimage->width/(double)info->current_subimage->height));
					}
					info->sub_images = g_list_append (info->sub_images, subi);
					info->current_subimage = NULL;
				}
			break;
		}
	}

	if (info->skip == info->depth)
		info->skip = -1;	
}

void
end_element (void *data, const char *el)
{
	DZParserinfo *info = (DZParserinfo*)data;
	DeepZoomImageTileSource *dzits = (DeepZoomImageTileSource*)info->source;
	if (!dzits)
		g_assert ("That's wrong...\n");
	dzits->EndElement (info, el);
}
