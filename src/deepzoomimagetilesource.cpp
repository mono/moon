/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * deepzoomimagetilesource.cpp
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <stdio.h>

#include <expat.h>

#include "deepzoomimagetilesource.h"
#include "uri.h"
#include "runtime.h"
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
	char *source;
	long width;
	long height;
	double vp_x;
	double vp_y;
	double vp_w;

	SubImage ()
	{
		source = NULL;
	}
};

class DZParserinfo
{
 public:
	int depth;
	int skip;
	bool error;

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
	}
};

void start_element (void *data, const char *el, const char **attr);
void end_element (void *data, const char *el);

void
DeepZoomImageTileSource::Init ()
{
	SetObjectType (Type::DEEPZOOMIMAGETILESOURCE);

	downloader = NULL;
	downloaded = false;
	format = NULL;
	get_tile_func = get_tile_layer;
	display_rects = NULL;
	parsed_callback = NULL;	
	isCollection = false;
}

DeepZoomImageTileSource::DeepZoomImageTileSource ()
{
	Init ();
}

DeepZoomImageTileSource::DeepZoomImageTileSource (const char *uri)
{
	Init ();
	SetValue (DeepZoomImageTileSource::UriSourceProperty, new Value (uri, true));
}

DeepZoomImageTileSource::~DeepZoomImageTileSource ()
{
	if (downloader) {
		downloader_abort (downloader);
		downloader->unref ();
	}		
}

void
DeepZoomImageTileSource::Download ()
{
	if (downloaded)
		return;
	char *stringuri;
	if ((stringuri = GetValue (DeepZoomImageTileSource::UriSourceProperty)->AsString ())) {
		downloaded = true;
		download_uri (stringuri);
	}
}

void 
DeepZoomImageTileSource::download_uri (const char* url)
{
	Uri *uri = new Uri ();

	Surface* surface = GetSurface ();
	if (!surface)
		return;
	
	if (g_str_has_prefix (url, "/"))
		url++;

	if (!(uri->Parse (url)))
		return;
	
	if (!downloader)
		downloader = surface->CreateDownloader ();
	
	if (!downloader)
		return;

printf ("DZITS: download_uri (%s)\n", uri->ToString ());

	downloader->Open ("GET", uri->ToString (), NoPolicy);
	
	downloader->AddHandler (downloader->CompletedEvent, downloader_complete, this);

	downloader->Send ();

	if (downloader->Started () || downloader->Completed ()) {
		if (downloader->Completed ())
			DownloaderComplete ();
	} else 
		downloader->Send ();
	delete uri;
}

void
DeepZoomImageTileSource::DownloaderComplete ()
{
	const char *filename;

	if (!(filename = downloader->getFileDownloader ()->GetDownloadedFile ()))
		return;

	Parse (filename);
}

void
DeepZoomImageTileSource::Parse (const char* filename)
{
#define BUFFSIZE	1024

printf ("Parsing DeepZoom %s\n", filename);
	XML_Parser p = XML_ParserCreate (NULL);
	XML_SetElementHandler (p, start_element, end_element);
	DZParserinfo *info = new DZParserinfo ();

	XML_SetUserData (p, info);

	FILE *f = fopen (filename, "r");
	int done = 0;
	char buffer[BUFFSIZE];
	while (!done && !info->error) {
		int len = fread (buffer, 1, BUFFSIZE, f);
		if (ferror(f)) {
			printf ("Read error\n");
			done = 1;
		}
		done = feof (f);
		if (!XML_Parse (p, buffer, len, done)) {
			printf ("Parser error at line %d:\n%s\n", (int)XML_GetCurrentLineNumber (p), XML_ErrorString(XML_GetErrorCode(p)));
			done = 1;
		}
	}
	fclose (f);

	if (!info->isCollection) {
		imageWidth = info->image_width;
		imageHeight = info->image_height;
		tileOverlap = info->overlap;
		display_rects = info->display_rects;
	} else {
		//do something with the info->subimages
		isCollection = info->isCollection;
		maxLevel = info->max_level;
	}

	tileWidth = tileHeight = info->tile_size;
	format = g_strdup (info->format);

printf ("Done parsing...\n");
	if (parsed_callback)
		parsed_callback (cb_userdata);
}

gpointer
get_tile_layer (int level, int x, int y, void *userdata)
{
	return ((DeepZoomImageTileSource *)userdata)->GetTileLayer (level, x, y);
}

gpointer
DeepZoomImageTileSource::GetTileLayer (int level, int x, int y)
{
	//check if there tile is listed in DisplayRects
	if (display_rects) {
		DisplayRect *cur;
		int i =0;
		bool found = false;
		int layers;
		frexp (MAX (imageWidth, imageHeight), &layers);

		while ((cur = (DisplayRect*)g_list_nth_data (display_rects, i))) {
			i++;

			if (!(cur->min_level <= level && level <= cur->max_level))
				continue;

			int vtilesize = tileWidth * (layers + 1 - level);
			Rect virtualtile = Rect (x * vtilesize, y * vtilesize, vtilesize, vtilesize);
			if (cur->rect.IntersectsWith (virtualtile)) {
				found = true;
				break;
			}
		}

		if (!found)
			return NULL;
	}
	char *sourceuri = GetValue (DeepZoomImageTileSource::UriSourceProperty)->AsString ();
	char buffer[strlen (sourceuri) + 32];
	char* p = g_stpcpy (buffer, sourceuri);
	sprintf (p-4, "_files/%d/%d_%d.%s", level, x, y, format);
	if (g_str_has_prefix (buffer, "/"))
		return g_strdup(buffer +1);
	else
		return g_strdup(buffer);
}

void
DeepZoomImageTileSource::downloader_complete (EventObject *sender, EventArgs *calldata, gpointer closure)
{
	((DeepZoomImageTileSource *) closure)->DownloaderComplete ();
}

void
DeepZoomImageTileSource::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property == DeepZoomImageTileSource::UriSourceProperty) {
		downloaded = false;
	}

	if (args->property->GetOwnerType () != Type::DEEPZOOMIMAGETILESOURCE) {
		DependencyObject::OnPropertyChanged (args);
		return;
	}
	
	NotifyListenersOfPropertyChange (args);
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
		if (!strcmp ("Image", el)) {
			info->isCollection = false;
			int i;
			for (i = 0; attr[i]; i+=2)
				if (!strcmp ("Format", attr[i]))
					info->format = g_strdup (attr[i+1]);
				else if (!strcmp ("TileSize", attr[i]))
					info->tile_size = atoi (attr[i+1]);
				else if (!strcmp ("Overlap", attr[i]))
					info->overlap = atoi (attr[i+1]);
				else
					printf ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
		} else if (!strcmp ("Collection", el)) {
			info->isCollection = true;
			int i;
			for (i = 0; attr[i]; i+=2)
				if (!strcmp ("Format", attr[i]))
					info->format = g_strdup (attr[i+1]);
				else if (!strcmp ("TileSize", attr[i]))
					info->tile_size = atoi (attr[i+1]);
				else if (!strcmp ("MaxLevel", attr[i]))
					info->max_level = atoi (attr[i+1]);
				else
					printf ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
		} else {
			printf ("Unexpected element %s\n", el);
			info->error = true;
		}
		break;
	case 1:
		if (!info->isCollection) {
			//Size or DisplayRects
			if (!strcmp ("Size", el)) {
				int i;
				for (i = 0; attr[i]; i+=2)
					if (!strcmp ("Width", attr[i]))
						info->image_width = atol (attr[i+1]);
					else if (!strcmp ("Height", attr[i]))
						info->image_height = atol (attr[i+1]);
					else
						printf ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
			} else if (!strcmp ("DisplayRects", el)) {
				//no attributes, only contains DisplayRect element
			} else {
				printf ("Unexpected element %s\n", el);
				info->error = true;
			}
		} else {
			if (!strcmp ("Items", el)) {
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
			if (!strcmp ("DisplayRect", el)) {
				long min_level, max_level;
				int i;
				for (i = 0; attr[i]; i+=2)
					if (!strcmp ("MinLevel", attr[i]))
						min_level = atol (attr[i+1]);
					else if (!strcmp ("MaxLevel", attr[i]))
						max_level = atol (attr[i+1]);
					else
						printf ("\tunparsed arg %s: %s\n", attr[i], attr[i+1]);
				info->current_rect = new DisplayRect (min_level, max_level);
			} else {
				printf ("Unexpected element %s\n", el);
				info->error = true;
			}
		} else {
			if (!strcmp ("I", el)) {
				info->current_subimage = new SubImage ();
				int i;
				for (i = 0; attr[i]; i+=2)
					if (!strcmp ("N", attr[i]))
						info->current_subimage->n = atoi (attr[i+1]);
					else if (!strcmp ("Id", attr[i]))
						info->current_subimage->id = atoi (attr[i+1]);
					else if (!strcmp ("Source", attr[i]))
						info->current_subimage->source = g_strdup (attr[i+1]);
					else
						printf ("\tunparsed arg %s: %s\n", attr[i], attr[i+1]);

			} else {
				printf ("Unexpected element %d %s\n", info->depth, el);
				info->error = true;
			}
		}
		break;
	case 3:
		if (!info->isCollection) {
			//Rect elt
			if (!strcmp ("Rect", el)) {
				if (!info->current_rect) {
					info->error = true;
					break;
				}
				int i;
				for (i = 0; attr[i]; i+=2)
					if (!strcmp ("X", attr[i]))
						info->current_rect->rect.x = (double)atol (attr[i+1]);
					else if (!strcmp ("Y", attr[i]))
						info->current_rect->rect.y = (double)atol (attr[i+1]);
					else if (!strcmp ("Width", attr[i]))
						info->current_rect->rect.width = (double)atol (attr[i+1]);
					else if (!strcmp ("Height", attr[i]))
						info->current_rect->rect.height = (double)atol (attr[i+1]);
					else
						printf ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
				info->display_rects = g_list_append (info->display_rects, info->current_rect);
				info->current_rect = NULL;
			} else {
				printf ("Unexpected element %s\n", el);
				info->error = true;
			}
		} else {
			if (!strcmp ("Size", el)) {
				if (!info->current_subimage) {
					info->error = true;
					break;
				}
				int i;
				for (i = 0; attr [i]; i+=2)
					if (!strcmp ("Width", attr[i]))
						info->current_subimage->width = atol (attr[i+1]);
					else if (!strcmp ("Height", attr[i]))
						info->current_subimage->height = atol (attr[i+1]);
					else
						printf ("\tunparsed attr %s.%s: %s\n", el, attr[i], attr[i+1]);
			} else if (!strcmp ("Viewport", el)) {
				if (!info->current_subimage) {
					info->error = true;
					break;
				}
				int i;
				for (i = 0; attr [i]; i+=2)
					if (!strcmp ("X", attr[i]))
						info->current_subimage->vp_x = atol (attr[i+1]);
					else if (!strcmp ("Y", attr[i]))
						info->current_subimage->vp_y = atol (attr[i+1]);
					else if (!strcmp ("Width", attr[i]))
						info->current_subimage->vp_w = atol (attr[i+1]);
					else
						printf ("\tunparsed attr %s: %s\n", attr[i], attr[i+1]);
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
end_element (void *data, const char *el)
{
	DZParserinfo *info = (DZParserinfo*)data;
	(info->depth)--;

	if (info->skip < 0) {
		switch (info->depth) {
		case 2:
			if (info->isCollection)
				if (!strcmp ("I", el)) {
					info->sub_images = g_list_append (info->sub_images, info->current_subimage);
					info->current_subimage = NULL;
				}
			break;
		}
	}

	if (info->skip == info->depth)
		info->skip = -1;
}
