
#ifndef __MOON_WINDOW__
#define __MOON_WINDOW__

#include <gtk/gtk.h>

#define Visual _XxVisual
#define Region _XxRegion
#include <gdk/gdkx.h>
#undef Visual
#undef Region

#include "rect.h"

class Surface;

class MoonWindow {
 public:
	MoonWindow (int w, int h) : width(w), height(h), surface(NULL) { }

	virtual ~MoonWindow () { }

	virtual void Resize (int width, int height) = 0;

	virtual void SetCursor (GdkCursor *cursor) = 0;
	virtual void Invalidate (Rect r) = 0;
	virtual void Invalidate () { Invalidate (Rect (0, 0, width, height)); }
	virtual void ProcessUpdates () = 0;

	virtual gboolean HandleEvent (XEvent *event) = 0;

	virtual void Show () = 0;
	virtual void Hide () = 0;

	virtual void EnableEvents (bool first) = 0;
	virtual void DisableEvents () = 0;
	
	virtual void GrabFocus () = 0;
	virtual bool HasFocus () = 0;

	int GetWidth () { return width; }
	int GetHeight () { return height; }

	virtual void SetSurface (Surface* s) { surface = s; }
	Surface *GetSurface () { return surface; }

	virtual bool IsFullScreen () = 0;

 protected:
	int width;
	int height;
	Surface *surface;
};

#endif /* __MOON_WINDOW__ */
