// define this here so that protypes.h isn't included (and doesn't
// muck with our npapi.h)
#define NO_NSPR_10_SUPPORT

#include "plugin.h"

#include "ff3-bridge.h"

#define CONCAT(x,y) x##y
#define GECKO_SYM(x) CONCAT(FF3,x)
#include "../browser-http.inc"

BrowserHttpRequest*
FF3BrowserBridge::CreateBrowserHttpRequest (const char *method, const char *uri)
{
	return new FF3BrowserHttpRequest (method, uri);
}
