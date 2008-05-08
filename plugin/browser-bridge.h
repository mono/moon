#ifndef BROWSER_BRIDGE
#define BROWSER_BRIDGE

#include "plugin-class.h"

typedef void (* HttpHeaderHandler) (const char *name, const char *value);

class BrowserHttpResponse {
 public:
	BrowserHttpResponse () { }

	virtual ~BrowserHttpResponse () { }

	virtual void VisitHeaders (HttpHeaderHandler handler) = 0;
	virtual char *GetStatus (int *code) = 0;
	virtual void *Read (int *length) = 0;
};


class SyncBrowserHttpResponse : public BrowserHttpResponse {
 public:
	SyncBrowserHttpResponse() { }
	virtual ~SyncBrowserHttpResponse() { }
};


class AsyncBrowserHttpResponse;

typedef void (* AsyncResponseAvailableHandler) (BrowserHttpResponse *response, gpointer context);

class AsyncBrowserHttpResponse : public BrowserHttpResponse {
 private:
	AsyncResponseAvailableHandler handler;
	gpointer context;
	char *buffer;
	int size;
 public:
	AsyncBrowserHttpResponse (AsyncResponseAvailableHandler handler, gpointer context)
	{
		buffer = NULL;
		size = 0;
		this->handler = handler;
		this->context = context;
	}

	virtual ~AsyncBrowserHttpResponse () { }
};

class BrowserHttpRequest {
 protected:
	char *uri;
	char *method;

 public:
	BrowserHttpRequest (const char *method, const char *uri)
	{
		this->method = g_strdup (method);
		this->uri = g_strdup (uri);
	}

	virtual ~BrowserHttpRequest ()
	{
		g_free (uri);
		g_free (method);
	}

	virtual void Abort () = 0;
	virtual BrowserHttpResponse *GetResponse () = 0;
	virtual bool GetAsyncResponse (AsyncResponseAvailableHandler handler, gpointer context) = 0;
	virtual void SetHttpHeader (const char *name, const char *value) = 0;
	virtual void SetBody (const char *body, int size) = 0;
};

G_BEGIN_DECLS

BrowserBridge *CreateBrowserBridge ();

BrowserHttpRequest *browser_http_request_new (PluginInstance *plugin, const char *method, const char *uri);
void browser_http_request_destroy (BrowserHttpRequest *request);
void browser_http_request_abort (BrowserHttpRequest *request);
void browser_http_request_set_header (BrowserHttpRequest *request, const char *name, const char *value);
void browser_http_request_set_body (BrowserHttpRequest *request, const char *body, int size);
BrowserHttpResponse *browser_http_request_get_response (BrowserHttpRequest *request);
bool browser_http_request_get_async_response (BrowserHttpRequest *request, AsyncResponseAvailableHandler handler, gpointer context);
void *browser_http_response_read (BrowserHttpResponse *response, int *size);
void browser_http_response_visit_headers (BrowserHttpResponse *response, HttpHeaderHandler handler);
char *browser_http_response_get_status (BrowserHttpResponse *response, int *code);
void browser_http_response_destroy (BrowserHttpResponse *response);

#if BROWSER_HTTP_TEST

void browser_http_test ();

#endif

G_END_DECLS


class BrowserBridge {
 public:
	// HtmlObject
	virtual const char *HtmlElementGetText (NPP npp, const char *element_id) = 0;
	virtual gpointer HtmlObjectAttachEvent (NPP npp, NPObject *npobj, const char *name, callback_dom_event cb) = 0;
	virtual void     HtmlObjectDetachEvent (NPP npp, const char *name, gpointer listener_ptr) = 0;

	// System.Windows.Browser.Net
	virtual BrowserHttpRequest* CreateBrowserHttpRequest (const char *method, const char *uri) = 0;
};




#endif /* BROWSER_BRIDGE */
