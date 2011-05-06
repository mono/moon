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

#include <config.h>
#include <gtk/gtk.h>
#include <string.h>

#include "debug.h"
#include "shocker.h"
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
	Shocker_FailTestFast ("TextHelpers_TextPositionToPixelPosition not implemented\n");
	return 0;
}

guint32
TextHelpers_TransformToTextView (UIElement *pUIElement,
				 float originalX,
				 float originalY,
				 float *transformedX,
				 float *transformedY)
{
	Shocker_FailTestFast ("TextHelpers_TransformToTextView not implemented\n");
	return 0;
}

guint32
TextHelpers_GetContentType (UIElement *pUIElement,
			    guint32 iTextPosition,
			    TextHelpers_ContentType *pContentType)
{
	Shocker_FailTestFast ("TextHelpers_GetContentType not implemented\n");
	return 0;
}


void
TextHelpers_GetGlyphRuns ()
{
	Shocker_FailTestFast ("TextHelpers_GetGlyphRuns not implemented\n");
}

void GeneralHelpers_IsRunningCheck (void *element, bool *isCheck)
{
#if DEBUG
	*isCheck = true;
#else
	*isCheck = false;
#endif
	printf ("GeneralHelpers_IsRunningCheck () The test wanted to know if we're in a debug or release build (we're in a %s build).\n", *isCheck ? "debug" : "release");
}

void TextHelpers_GetTextRun ()
{
	Shocker_FailTestFast ("TextHelpers_GetTextRun not implemented\n");
}

void TextHelpers_SetFontSource ()
{
	Shocker_FailTestFast ("TextHelpers_SetFontSource not implemented\n");
}

void TextHelpers_CopyFontSource ()
{
	Shocker_FailTestFast ("TextHelpers_CopyFontSource not implemented\n");
}

void TextHelpers_PixelToTextPosition ()
{
	Shocker_FailTestFast ("TextHelpers_PixelToTextPosition not implemented\n");
}

void TextHelpers_SendKeyInputMessage ()
{
	Shocker_FailTestFast ("TextHelpers_SendKeyInputMessage not implemented\n");
}

void DirectInputHelper_SendKeyInputMessage ()
{
	Shocker_FailTestFast ("DirectInputHelper_SendKeyInputMessage not implemented\n");
}

void DirectInputHelper_SendMouseInputMessage ()
{
	Shocker_FailTestFast ("DirectInputHelper_SendMouseInputMessage not implemented\n");
}

void DirectInputHelper_SetCore ()
{
	Shocker_FailTestFast ("DirectInputHelper_SetCore not implemented\n");
}

void DirectInputHelper_SendIMELangChangeMessage ()
{
	Shocker_FailTestFast ("DirectInputHelper_SendIMELangChangeMessage not implemented\n");
}

void DirectInputHelper_SwitchLayoutLanguage ()
{
	Shocker_FailTestFast ("DirectInputHelper_SwitchLayoutLanguage not implemented\n");
}

