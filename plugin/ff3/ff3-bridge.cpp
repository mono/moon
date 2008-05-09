#include "plugin.h"

#include "ff3-bridge.h"

BrowserBridge* CreateBrowserBridge ()
{
	return new FF3BrowserBridge ();
}
