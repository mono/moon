#ifndef FF2_BRIDGE
#define FF2_BRIDGE

#include "browser-bridge.h"
#include "plugin-class.h"

class FF2BrowserBridge : public BrowserBridge {
 public:
	FF2BrowserBridge ();

	virtual const char *HtmlElementGetText (NPP npp, const char *element_id);
	virtual gpointer HtmlObjectAttachEvent (NPP npp, NPObject *npobj, const char *name, callback_dom_event cb, gpointer context);
	virtual void     HtmlObjectDetachEvent (NPP npp, const char *name, gpointer listener_ptr);

	virtual DownloaderRequest* CreateDownloaderRequest (const char *method, const char *uri, bool disable_cache);
};

struct FF2DomEventType : MoonlightObjectType {
	FF2DomEventType ();
};

extern FF2DomEventType *FF2DomEventClass;

#endif // FF2_BRIDGE

