/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef MOON_CLIPBOARD_ANDROID_H
#define MOON_CLIPBOARD_ANDROID_H

#include <glib.h>

#include "window-android.h"
#include "pal.h"

namespace Moonlight {

class MoonClipboardAndroid : public MoonClipboard {
public:
	MoonClipboardAndroid (MoonWindowAndroid *win, MoonClipboardType clipboardType);

	virtual bool ContainsText ();
	virtual void SetText (const char *text);
	virtual void AsyncGetText (MoonClipboardGetTextCallback cb, gpointer data);
	virtual char* GetText ();
};

};
#endif
