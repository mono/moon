/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef MOON_CLIPBOARD_COCOA_H
#define MOON_CLIPBOARD_COCOA_H

#include <glib.h>

#include "window-cocoa.h"
#include "pal.h"

namespace Moonlight {

class MoonClipboardCocoa : public MoonClipboard {
public:
	MoonClipboardCocoa (MoonWindowCocoa *win, MoonClipboardType clipboardType);

	virtual bool ContainsText ();
	virtual void SetText (const char *text, int length);
	virtual void AsyncGetText (MoonClipboardGetTextCallback cb, gpointer data);
	virtual char* GetText ();

private:
	void *pasteboard;
};

};
#endif
