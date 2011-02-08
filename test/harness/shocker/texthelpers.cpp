/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * texthelpers.cpp:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2011 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <gtk/gtk.h>
#include <pthread.h>
#include <string.h>

#include "debug.h"
#include "texthelpers.h"

guint32
TextHelpers_TextPositionToPixelPosition (UIElement *pUIElement,
					 guint32 textPosition,
					 guint32 textGravity,
					 float *pixelOffset,
					 float *characterTop,
					 float *characterHeight,
					 float *lineTop,
					 float *lineHeight,
					 float *lineBaseline,
					 float *lineOffset)
{
	return 0;
}

guint32
TextHelpers_TransformToTextView (UIElement *pUIElement,
				 float originalX,
				 float originalY,
				 float *transformedX,
				 float *transformedY)
{
	return 0;
}

guint32
TextHelpers_GetContentType (UIElement *pUIElement,
			    guint32 iTextPosition,
			    TextHelpers_ContentType *pContentType)
{
	return 0;
}
