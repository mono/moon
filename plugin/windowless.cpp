#include "config.h"
#include "windowless.h"

WindowlessSurface::WindowlessSurface (int width, int height, PluginInstance *plugin)
  : Surface (width, height, true)
{
	this->plugin = plugin;
	SetTrans (true);
}

void
WindowlessSurface::SetCursor (GdkCursor *cursor)
{
	// turned off for now.  hopefully we can get this switched on for
	// newer versions of ff3
	// see https://bugzilla.mozilla.org/show_bug.cgi?id=430451

#if 0 && (NP_VERSION_MINOR >= NPVERS_HAS_CURSOR)
	NPN_SetValue (plugin->GetInstance(), NPNVcursor, GDK_CURSOR_XCURSOR(cursor));
#endif
}

void
WindowlessSurface::Invalidate (Rect r)
{
	NPRect nprect;

	// Mozilla gets seriously confused about invalidations 
	// outside the windowless bounds.
	r = r.Intersection (Rect (0, 0, GetWidth(), GetHeight())).RoundOut ();

	nprect.left = (uint16)r.x;
	nprect.top = (uint16)r.y;
	nprect.right = (uint16)(r.x + r.w);
	nprect.bottom = (uint16)(r.y + r.h);

	NPN_InvalidateRect (plugin->GetInstance(), &nprect);
}

void
WindowlessSurface::ProcessUpdates ()
{
	NPN_ForceRedraw (plugin->GetInstance());
}
