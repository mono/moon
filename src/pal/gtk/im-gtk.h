/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef MOON_IM_GTK_H
#define MOON_IM_GTK_H

#include "pal.h"
#include "window.h"

#include <gtk/gtk.h>

namespace Moonlight {

class MoonIMContextGtk  : public MoonIMContext {
public:
	MoonIMContextGtk ();
	virtual ~MoonIMContextGtk ();

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

 private:
	GtkIMContext *im;
};

};
#endif /* MOON_IM_GTK_H */
