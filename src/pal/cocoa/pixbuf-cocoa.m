/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <config.h>
#include <string.h>
#include "pal-cocoa.h"

#include "runtime.h"

#include "pixbuf-cocoa.h"

#import <AppKit/AppKit.h>

using namespace Moonlight;

MoonPixbufLoaderCocoa::MoonPixbufLoaderCocoa (const char *imageType)
{
	this->image = [[NSBitmapImageRep alloc] initForIncrementalLoad];
	this->data = [[NSMutableData alloc] initWithLength: 0];
}

MoonPixbufLoaderCocoa::MoonPixbufLoaderCocoa ()
{
	this->image = [[NSBitmapImageRep alloc] initForIncrementalLoad];
	this->data = [[NSMutableData alloc] initWithLength: 0];
}

MoonPixbufLoaderCocoa::~MoonPixbufLoaderCocoa ()
{
	[this->image release];
	[this->data release];
}

void
MoonPixbufLoaderCocoa::Write (const guchar *buffer, int buflen, MoonError **error)
{
	int res;

	[this->data appendBytes: buffer length: buflen];
	[this->image incrementalLoadFromData: (NSData *) data complete: NO];

	if (res == NSImageRepLoadStatusUnknownType)
		g_warning ("unknown type");
	if (res == NSImageRepLoadStatusReadingHeader)
		g_warning ("reading header");
	if (res == NSImageRepLoadStatusWillNeedAllData)
		g_warning ("need all data");
	if (res == NSImageRepLoadStatusInvalidData)
		g_warning ("invalid data");
	if (res == NSImageRepLoadStatusUnexpectedEOF)
		g_warning ("unexpected eof");
	if (res == NSImageRepLoadStatusCompleted)
		g_warning ("completed");
}

void
MoonPixbufLoaderCocoa::Close (MoonError **error)
{
	int res;

	[this->image incrementalLoadFromData: (NSData *) data complete: YES];
	
	if (res == NSImageRepLoadStatusUnknownType)
		g_warning ("unknown type");
	if (res == NSImageRepLoadStatusReadingHeader)
		g_warning ("reading header");
	if (res == NSImageRepLoadStatusWillNeedAllData)
		g_warning ("need all data");
	if (res == NSImageRepLoadStatusInvalidData)
		g_warning ("invalid data");
	if (res == NSImageRepLoadStatusUnexpectedEOF)
		g_warning ("unexpected eof");
	if (res == NSImageRepLoadStatusCompleted)
		g_warning ("completed");
}

MoonPixbuf*
MoonPixbufLoaderCocoa::GetPixbuf ()
{
	return new MoonPixbufCocoa (this->image, FALSE);
}


MoonPixbufCocoa::MoonPixbufCocoa (void *pixbuf, bool crc_error)
{
	this->image = [pixbuf retain];
}

MoonPixbufCocoa::~MoonPixbufCocoa ()
{
	[this->image release];
}

gint
MoonPixbufCocoa::GetWidth ()
{
	return (int) ((NSBitmapImageRep *) image).size.width;
}

gint
MoonPixbufCocoa::GetHeight ()
{
	return (int) ((NSBitmapImageRep *) image).size.height;
}

gint
MoonPixbufCocoa::GetRowStride ()
{
	return ((NSBitmapImageRep *) image).bytesPerRow;
}

gint
MoonPixbufCocoa::GetNumChannels ()
{
	return 4;
}

guchar*
MoonPixbufCocoa::GetPixels ()
{
	return ((NSBitmapImageRep *) image).bitmapData;
}

gboolean
MoonPixbufCocoa::IsPremultiplied ()
{
	return YES;
}

gpointer
MoonPixbufCocoa::GetPlatformPixbuf ()
{
	g_assert_not_reached ();
}
