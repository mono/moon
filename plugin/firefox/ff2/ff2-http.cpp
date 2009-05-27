// define this here so that protypes.h isn't included (and doesn't
// muck with our npapi.h)
#define NO_NSPR_10_SUPPORT

#include "plugin.h"

#include "ff2-bridge.h"

#define CONCAT(x,y) x##y
#define GECKO_SYM(x) CONCAT(FF2,x)
#include "../browser-http.inc"

DownloaderRequest*
FF2BrowserBridge::CreateDownloaderRequest (const char *method, const char *uri, bool disable_cache)
{
	return new FF2DownloaderRequest (method, uri, disable_cache);
}
