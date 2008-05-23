#include "plugin.h"

#include "ff2-bridge.h"

BrowserBridge* CreateBrowserBridge ()
{
	return new FF2BrowserBridge ();
}
