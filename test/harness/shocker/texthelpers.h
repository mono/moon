/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * texthelpers.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __TEXTHELPERS_H__
#define __TEXTHELPERS_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "uielement.h"

using namespace Moonlight;

extern "C" {

guint32 TextHelpers_TextPositionToPixelPosition (UIElement *pUIElement,
						 guint32 textPosition,
						 guint32 textGravity,
						 float *pixelOffset,
						 float *characterTop,
						 float *characterHeight,
						 float *lineTop,
						 float *lineHeight,
						 float *lineBaseline,
						 float *lineOffset);

guint32 TextHelpers_TransformToTextView (UIElement *pUIElement,
					 float originalX,
					 float originalY,
					 float *transformedX,
					 float *transformedY);

enum TextHelpers_ContentType {
  ContentType_None = 0,
  ContentType_ElementStart = 1,
  ContentType_ElementEnd = 2,
  ContentType_Text = 3
};

guint32 TextHelpers_GetContentType (UIElement *pUIElement,
				    guint32 iTextPosition,
				    TextHelpers_ContentType *pContentType);

void TextHelpers_GetGlyphRuns ();

};

#endif // __TEXTHELPERS_H__
