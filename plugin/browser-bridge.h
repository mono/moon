#ifndef BROWSER_BRIDGE
#define BROWSER_BRIDGE

#include "plugin-class.h"
#include "plugin-downloader.h"

#define DOWNLOADER_OK 0
#define DOWNLOADER_ERR -1

G_BEGIN_DECLS

BrowserBridge *CreateBrowserBridge ();

G_END_DECLS

class BrowserBridge {
 public:
	// HtmlObject
	virtual const char *HtmlElementGetText (NPP npp, const char *element_id) = 0;
	virtual gpointer HtmlObjectAttachEvent (NPP npp, NPObject *npobj, const char *name, callback_dom_event cb) = 0;
	virtual void     HtmlObjectDetachEvent (NPP npp, const char *name, gpointer listener_ptr) = 0;

	virtual DownloaderRequest* CreateDownloaderRequest (const char *method, const char *uri) = 0;
};

#endif /* BROWSER_BRIDGE */
