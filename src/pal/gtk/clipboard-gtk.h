/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef MOON_CLIPBOARD_GTK_H
#define MOON_CLIPBOARD_GTK_H

#include <glib.h>
#include <gtk/gtk.h>

#include "window-gtk.h"
#include "pal.h"

namespace Moonlight {

class MoonClipboardGtk : public MoonClipboard {
public:
	MoonClipboardGtk (MoonWindowGtk *win, MoonClipboardType clipboardType);

	virtual bool ContainsText ();
	virtual void SetText (const char *text);
	virtual void AsyncGetText (MoonClipboardGetTextCallback cb, gpointer data);
	virtual char* GetText ();

private:
	static void async_get_text (GtkClipboard *clipboard, const char *text, gpointer data);
	GtkClipboard *clipboard;
};

};
#endif
