// define this here so that protypes.h isn't included (and doesn't
// muck with our npapi.h)
#define NO_NSPR_10_SUPPORT

#include "plugin.h"

#include "ff2-bridge.h"

#define CONCAT(x,y) x##y
#define GECKO_SYM(x) CONCAT(FF2,x)
#include "browser-http.inc"

BrowserHttpRequest*
FF2BrowserBridge::CreateBrowserHttpRequest (const char *method, const char *uri)
{
	return new FF2BrowserHttpRequest (method, uri);
}
