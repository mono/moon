#include "browser-bridge.h"

//// BrowserHttp*

BrowserHttpRequest *
browser_http_request_new (PluginInstance *plugin, const char *method, const char *uri)
{
        if (!plugin->GetBridge())
                return NULL;
	return plugin->GetBridge()->CreateBrowserHttpRequest (method, uri);
}

void
browser_http_request_abort (BrowserHttpRequest *request)
{
	request->Abort ();
}

void
browser_http_request_set_header (BrowserHttpRequest *request, const char *name, const char *value)
{
	request->SetHttpHeader (name, value);
}

void
browser_http_request_set_body (BrowserHttpRequest *request, const char *body, int size)
{
	request->SetBody (body, size);
}

void
browser_http_request_destroy (BrowserHttpRequest *request)
{
	delete request;
}

BrowserHttpResponse *
browser_http_request_get_response (BrowserHttpRequest *request)
{
	return request->GetResponse ();
}

bool
browser_http_request_get_async_response (BrowserHttpRequest *request, AsyncResponseAvailableHandler handler, gpointer context)
{
	return request->GetAsyncResponse (handler, context);
}

void
browser_http_response_visit_headers (BrowserHttpResponse *response, HttpHeaderHandler handler)
{
	response->VisitHeaders (handler);
}

char *
browser_http_response_get_status (BrowserHttpResponse *response, int *code)
{
	return response->GetStatus (code);
}

void
browser_http_response_destroy (BrowserHttpResponse *response)
{
	delete response;
}

void *
browser_http_response_read (BrowserHttpResponse *response, int *size)
{
	return response->Read (size);
}

#if BROWSER_HTTP_TEST

void
browser_http_test_print_response (BrowserHttpResponse *response)
{
	int len;
	void *data = response->Read (&len);

	char *text = g_strndup ((char *) data, len);

	printf ("response: \n%s", text);

	g_free (data);
	g_free (text);
}

void
browser_http_sync_test ()
{
	BrowserHttpRequest *req = new BrowserHttpRequest ("GET", "http://evain.net/gdb.txt");
	SyncBrowserHttpResponse *response = req->GetResponse ();

	browser_http_test_print_response (response);

	delete response;
	delete req;
}

void
browser_http_async_test_callback (BrowserHttpResponse *response, gpointer context)
{
	browser_http_test_print_response (response);

	delete response;
}

void
browser_http_async_test ()
{
	BrowserHttpRequest *req = new BrowserHttpRequest ("GET", "http://evain.net/gdb.txt");

	req->GetAsyncResponse (browser_http_async_test_callback, NULL);

	delete req;
}

#endif
