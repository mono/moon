#ifndef BROWSER_BRIDGE
#define BROWSER_BRIDGE

#include "plugin-class.h"
#include "plugin-downloader.h"

class BrowserResponse;

#define DOWNLOADER_OK 0
#define DOWNLOADER_ERR -1

typedef uint32_t (* BrowserResponseStartedHandler) (BrowserResponse *response, gpointer context);
typedef uint32_t (* BrowserResponseDataAvailableHandler) (BrowserResponse *response, gpointer context, char *buffer, uint32_t length);
typedef uint32_t (* BrowserResponseFinishedHandler) (BrowserResponse *response, gpointer context, gpointer data);

G_BEGIN_DECLS

BrowserBridge *CreateBrowserBridge ();

G_END_DECLS

class BrowserResponse {
 protected:
	BrowserResponseStartedHandler started;
	BrowserResponseDataAvailableHandler available;
	BrowserResponseFinishedHandler finished;
	gpointer context;

	bool aborted;

 public:
	BrowserResponse ()
	{
		aborted = false;
	}

	BrowserResponse (BrowserResponseStartedHandler started, BrowserResponseDataAvailableHandler available, BrowserResponseFinishedHandler finished, gpointer context)
	{
		this->aborted = false;
		this->started = started;
		this->available = available;
		this->finished = finished;
		this->context = context;
	}

	virtual ~BrowserResponse ()
	{
	}

	virtual void Abort () = 0;
	virtual const bool IsAborted () { return this->aborted; }
};

class BrowserRequest {
 protected:
	char *uri;
	char *method;
	
	bool aborted;

 public:
	BrowserRequest (const char *method, const char *uri)
	{
		this->method = g_strdup (method);
		this->uri = g_strdup (uri);
	}

	virtual ~BrowserRequest ()
	{
		g_free (method);
		g_free (uri);
	}

	virtual void Abort () = 0;
	virtual bool GetResponse (BrowserResponseStartedHandler started, BrowserResponseDataAvailableHandler available, BrowserResponseFinishedHandler finished, gpointer context) = 0;
	virtual const bool IsAborted () { return this->aborted; }
	virtual void SetHttpHeader (const char *name, const char *value) = 0;
	virtual void SetBody (void *body, int size) = 0;
};

class BrowserBridge {
 public:
	// HtmlObject
	virtual const char *HtmlElementGetText (NPP npp, const char *element_id) = 0;
	virtual gpointer HtmlObjectAttachEvent (NPP npp, NPObject *npobj, const char *name, callback_dom_event cb) = 0;
	virtual void     HtmlObjectDetachEvent (NPP npp, const char *name, gpointer listener_ptr) = 0;

	virtual BrowserRequest* CreateBrowserRequest (const char *method, const char *uri) = 0;
};

#endif /* BROWSER_BRIDGE */
