#ifndef FF3_BRIDGE
#define FF3_BRIDGE

#include "browser-bridge.h"
#include "plugin-class.h"

class FF3BrowserBridge : public BrowserBridge {
 public:
	FF3BrowserBridge ();
	virtual const char *HtmlElementGetText (NPP npp, const char *element_id);
	virtual gpointer HtmlObjectAttachEvent (NPP npp, NPObject *npobj, const char *name, callback_dom_event cb, gpointer context);
	virtual void     HtmlObjectDetachEvent (NPP npp, const char *name, gpointer listener_ptr);

	virtual DownloaderRequest* CreateDownloaderRequest (const char *method, const char *uri);
};

struct FF3DomEventType : MoonlightObjectType {
	FF3DomEventType ();
};

extern FF3DomEventType *FF3DomEventClass;

static const MoonNameIdMapping
dom_event_mapping[] = {
	{ "stoppropagation", MoonId_StopPropagation },
	{ "preventdefault", MoonId_PreventDefault },
	{ "detail", MoonId_Detail}
};


#endif // FF3_BRIDGE
