#include <nsCOMPtr.h>
#include <nsXPCOM.h>
#include <nsIURI.h>
#include <nsIServiceManager.h>
#include <nsIComponentManager.h>
#include <nsIIOService.h>
#include <nsStringAPI.h>
#include <nsIInputStream.h>
#include <nsIOutputStream.h>
#include <nsIStreamListener.h>
#include <nsEmbedString.h>
#include <nsIChannel.h>
#include <nsIRequest.h>
#include <nsIRequestObserver.h>
#include <nsIHttpChannel.h>
#include <nsIHttpHeaderVisitor.h>
#include <nsEmbedString.h>
#include <nsIUploadChannel.h>

// unfrozen apis
#if WITH_LIBXUL_UNSTABLE
#include <nsNetError.h>
#include <nsIStorageStream.h>
#else
#include <necko/nsNetError.h>
#include <xpcom/nsIStorageStream.h>
#endif

class GECKO_SYM(BrowserHttpResponse) : public BrowserHttpResponse, public nsIHttpHeaderVisitor {
private:
	nsCOMPtr<nsIChannel> channel;
	HttpHeaderHandler handler;

protected:
	NS_DECL_NSIHTTPHEADERVISITOR
public:
	NS_DECL_ISUPPORTS

	GECKO_SYM(BrowserHttpResponse) (nsCOMPtr<nsIChannel> channel)
	{
		this->channel = channel;
		this->handler = NULL;
	}

	virtual ~GECKO_SYM(BrowserHttpResponse) ()
	{
	}

	virtual void VisitHeaders (HttpHeaderHandler handler);
	virtual char *GetStatus (int *code);
};

class GECKO_SYM(SyncBrowserHttpResponse) : public GECKO_SYM(BrowserHttpResponse) {
private:
	nsCOMPtr<nsIInputStream> response_stream;
public:
	GECKO_SYM(SyncBrowserHttpResponse) (nsCOMPtr<nsIChannel> channel, nsCOMPtr<nsIInputStream> response) : GECKO_SYM(BrowserHttpResponse) (channel)
	{
		this->response_stream = response;
	}

	virtual ~GECKO_SYM(SyncBrowserHttpResponse) ()
	{
		if (response_stream)
			response_stream->Close ();
	}

	virtual void *Read (int *length);
};

class GECKO_SYM(AsyncBrowserHttpResponse) : public GECKO_SYM(BrowserHttpResponse), public nsIStreamListener {
private:
	AsyncResponseAvailableHandler handler;
	gpointer context;
	char *buffer;
	int size;
protected:
	NS_DECL_NSIREQUESTOBSERVER
	NS_DECL_NSISTREAMLISTENER
public:
	NS_DECL_ISUPPORTS

	GECKO_SYM(AsyncBrowserHttpResponse) (nsCOMPtr<nsIChannel> channel, AsyncResponseAvailableHandler handler, gpointer context)
		: GECKO_SYM(BrowserHttpResponse) (channel)
	{
		buffer = NULL;
		size = 0;
		this->handler = handler;
		this->context = context;
	}

	virtual ~GECKO_SYM(AsyncBrowserHttpResponse) ()
	{
	}

	virtual void *Read (int *size);
};

class GECKO_SYM(BrowserHttpRequest) : public BrowserHttpRequest {
private:
	nsCOMPtr<nsIChannel> channel;

	void CreateChannel ();
public:

	GECKO_SYM(BrowserHttpRequest) (const char *method, const char *uri)
	    : BrowserHttpRequest (method, uri)
	{
		CreateChannel ();
	}

	~GECKO_SYM(BrowserHttpRequest) ()
	{
	}

	void Abort ();
	BrowserHttpResponse *GetResponse ();
	bool GetAsyncResponse (AsyncResponseAvailableHandler handler, gpointer context);
	void SetHttpHeader (const char *name, const char *value);
	void SetBody (const char *body, int size);
};

// BrowserHttpResponse

NS_IMPL_ISUPPORTS1 (GECKO_SYM(BrowserHttpResponse), nsIHttpHeaderVisitor)

NS_IMETHODIMP
GECKO_SYM(BrowserHttpResponse)::VisitHeader (const nsACString &header, const nsACString &value)
{
	if (handler == NULL)
		return NS_OK;

	const char *name, *val;
	PRUint32 nl, vl;

	nl = NS_CStringGetData (header, &name);
	vl = NS_CStringGetData (value, &val);

	name = g_strndup (name, nl);
	val = g_strndup (val, vl);

	handler (name, val);

	// g_free ((gpointer) name);
	// g_free ((gpointer) val);

	return NS_OK;
}

void
GECKO_SYM(BrowserHttpResponse)::VisitHeaders (HttpHeaderHandler handler)
{
	nsCOMPtr<nsIHttpChannel> httpchannel = do_QueryInterface (channel);
	if (!httpchannel)
		return;

	this->handler = handler;

	httpchannel->VisitResponseHeaders (this);

	this->handler = NULL;
}

char *
GECKO_SYM(BrowserHttpResponse)::GetStatus (int *code)
{
	nsCOMPtr<nsIHttpChannel> httpchannel = do_QueryInterface (channel);
	if (!httpchannel) {
		*code = 0;
		return NULL;
	}

	nsEmbedCString status_desc;
	PRUint32 status;
	const char *desc;
	int dl;

	httpchannel->GetResponseStatusText (status_desc);

	dl = NS_CStringGetData (status_desc, &desc);

	httpchannel->GetResponseStatus (&status);

	*code = status;
	return g_strndup (desc, dl);
}

// SyncBrowserHttpResponse

void *
GECKO_SYM(SyncBrowserHttpResponse)::Read (int *size)
{
	PRUint32 read = 0;
	char *data = NULL;

	while (true) {
		PRUint32 available, len;
		response_stream->Available (&available);

		if (data == NULL) {
			data = (char *) NS_Alloc (available);
		} else if (available == 0) {
			break;
		} else {
			data = (char *)NS_Realloc (data, read + available);
		}

		response_stream->Read (data + read, available, &len);

		if (len == 0)
			break;

		read += len;
	}

	*size = read;

	return data;
}

// AsyncBrowserHttpResponse

NS_IMPL_ISUPPORTS1 (GECKO_SYM(AsyncBrowserHttpResponse), nsIStreamListener)

NS_IMETHODIMP
GECKO_SYM(AsyncBrowserHttpResponse)::OnStartRequest (nsIRequest *request, nsISupports *context)
{
	return NS_OK;
}

NS_IMETHODIMP
GECKO_SYM(AsyncBrowserHttpResponse)::OnStopRequest (nsIRequest *request, nsISupports *ctx, nsresult status)
{
	handler (this, this->context);
	return NS_OK;
}

NS_IMETHODIMP
GECKO_SYM(AsyncBrowserHttpResponse)::OnDataAvailable (nsIRequest *request, nsISupports *context, nsIInputStream *input, PRUint32 offset, PRUint32 count)
{
	if (buffer == NULL) {
		buffer = (char *) NS_Alloc (count);
	} else {
		buffer = (char *) NS_Realloc (buffer, size + count);
	}

	PRUint32 length;

	input->Read (buffer + size, count, &length);

	size += length;

	return NS_OK;
}

void *
GECKO_SYM(AsyncBrowserHttpResponse)::Read (int *size)
{
	*size = this->size;
	return this->buffer;
}

// BrowserHttpRequest

void
GECKO_SYM(BrowserHttpRequest)::CreateChannel ()
{
	nsresult rv = NS_OK;
	nsCOMPtr<nsIServiceManager> mgr;
	rv = NS_GetServiceManager (getter_AddRefs (mgr));
	if (NS_FAILED (rv)) {
		printf ("failed to ge a ServiceManager \n");
		return;
	}

	nsCOMPtr<nsIIOService> ioservice;
	rv = mgr->GetServiceByContractID ("@mozilla.org/network/io-service;1",
			NS_GET_IID (nsIIOService), getter_AddRefs (ioservice));

	if (NS_FAILED (rv)) {
		printf ("failed to get a IOService \n");
		return;
	}

	nsEmbedCString url;
	url = this->uri;

	printf ("BrowserHttpRequest: %s\n", uri);

	nsCOMPtr<nsIURI> uri;
	rv = ioservice->NewURI (url, nsnull, nsnull, getter_AddRefs (uri));

	ioservice->NewChannelFromURI (uri, getter_AddRefs (this->channel));

	nsCOMPtr<nsIHttpChannel> httpchannel = do_QueryInterface (channel);
	if (!httpchannel)
		return;

	nsEmbedCString meth;
	meth = this->method;
	httpchannel->SetRequestMethod (meth);
}

void
GECKO_SYM(BrowserHttpRequest)::Abort ()
{
	channel->Cancel (NS_BINDING_ABORTED);
}

BrowserHttpResponse *
GECKO_SYM(BrowserHttpRequest)::GetResponse ()
{
	nsresult rs = NS_OK;
	nsCOMPtr<nsIInputStream> input;
	rs = channel->Open (getter_AddRefs (input));
	if (NS_FAILED (rs))
		return NULL;

	return new GECKO_SYM(SyncBrowserHttpResponse) (channel, input);
}

bool
GECKO_SYM(BrowserHttpRequest)::GetAsyncResponse (AsyncResponseAvailableHandler handler, gpointer context)
{
	nsresult rs = NS_OK;
	GECKO_SYM(AsyncBrowserHttpResponse) *response;

	response = new GECKO_SYM(AsyncBrowserHttpResponse) (channel, handler, context);
	rs = channel->AsyncOpen (response, (GECKO_SYM(BrowserHttpResponse)*)response);
	return !NS_FAILED (rs);
}

void
GECKO_SYM(BrowserHttpRequest)::SetHttpHeader (const char *name, const char *value)
{
	nsCOMPtr<nsIHttpChannel> httpchannel = do_QueryInterface (channel);
	if (!httpchannel)
		return;

	nsEmbedCString nsname, nsvalue;
	nsname = name;
	nsvalue = value;

	httpchannel->SetRequestHeader (nsname, nsvalue, false);
}

void
GECKO_SYM(BrowserHttpRequest)::SetBody (const char *body, int size)
{
	nsCOMPtr<nsIUploadChannel> upload = do_QueryInterface (channel);
	if (!upload)
		return;

	nsEmbedCString type;

	nsCOMPtr<nsIStorageStream> storage;
	nsresult rv = NS_NewStorageStream (2048, PR_UINT32_MAX, getter_AddRefs (storage));

	nsCOMPtr<nsIOutputStream> output;
	storage->GetOutputStream (0, getter_AddRefs (output));

	PRUint32 written;
	output->Write (body, size, &written);

	nsCOMPtr<nsIInputStream> input;
	rv = storage->NewInputStream (0, getter_AddRefs (input));

	upload->SetUploadStream (input, type, -1);
}
