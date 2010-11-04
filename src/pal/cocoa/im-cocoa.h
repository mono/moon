/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * im-cocoa.h
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef MOON_IM_COCOA_H
#define MOON_IM_COCOA_H

#include "pal.h"
#include "window.h"

#include <cocoa/cocoa.h>

namespace Moonlight {

class MoonIMContextCocoa  : public MoonIMContext {
public:
	MoonIMContextCocoa ();
	virtual ~MoonIMContextCocoa ();

	virtual void SetUsePreedit (bool flag);
	virtual void SetClientWindow (MoonWindow*  window);
	virtual void SetSurroundingText (const char *text, int offset, int length);
	virtual void Reset ();

	virtual void FocusIn ();
	virtual void FocusOut ();

	virtual void SetCursorLocation (Rect r);

	virtual bool FilterKeyPress (MoonKeyEvent* event);

	virtual void SetRetrieveSurroundingCallback (MoonCallback cb, gpointer data);
	virtual void SetDeleteSurroundingCallback (MoonCallback cb, gpointer data);
	virtual void SetCommitCallback (MoonCallback cb, gpointer data);

	virtual gpointer GetPlatformIMContext ();
};

};
#endif /* MOON_IM_COCOA_H */
