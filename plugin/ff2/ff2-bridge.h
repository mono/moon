#ifndef FF2_BRIDGE
#define FF2_BRIDGE

#include "browser-bridge.h"

class FF2BrowserBridge : public BrowserBridge {
 public:
	FF2BrowserBridge () { }

	virtual const char *HtmlElementGetText (NPP npp, const char *element_id);
	virtual gpointer HtmlObjectAttachEvent (NPP npp, NPObject *npobj, const char *name, callback_dom_event cb);
	virtual void     HtmlObjectDetachEvent (NPP npp, const char *name, gpointer listener_ptr);

	virtual BrowserHttpRequest* CreateBrowserHttpRequest (const char *method, const char *uri);
};

#endif // FF2_BRIDGE

