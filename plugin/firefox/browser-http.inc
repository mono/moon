#include <nsCOMPtr.h>
#include <nsXPCOM.h>
#include <nsIURI.h>
#include <nsIServiceManager.h>
#include <nsIComponentManager.h>
#include <nsComponentManagerUtils.h>
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

class GECKO_SYM(BrowserHttpRequest);

/*
 * HeaderVisitor
 */

class GECKO_SYM(HeaderVisitor) : public nsIHttpHeaderVisitor {
 public:
	NS_DECL_ISUPPORTS
	NS_DECL_NSIHTTPHEADERVISITOR
 
	GECKO_SYM(HeaderVisitor) (HttpResponse *response)
	{
		this->response = response;
	}

 private:
	HttpResponse *response;
};

/*
 * StreamListener
 */

class GECKO_SYM(StreamListener) : public nsIStreamListener {
 private:
	GECKO_SYM(BrowserHttpRequest) *request;

 protected:
	NS_DECL_NSIREQUESTOBSERVER
	NS_DECL_NSISTREAMLISTENER

 public:
	NS_DECL_ISUPPORTS

	GECKO_SYM(StreamListener) (GECKO_SYM(BrowserHttpRequest) *request);
	virtual ~GECKO_SYM(StreamListener) ();
	void ClearRequest () { request = NULL; }
};

/*
 * BrowserRequest
 */

class GECKO_SYM(BrowserHttpRequest) : public BrowserHttpRequest {
 private:
	nsCOMPtr<GECKO_SYM(StreamListener)> listener;
	nsCOMPtr<nsIHttpChannel> httpchannel;
	GString *headers;

	void CreateChannel ();

public:
	GECKO_SYM(BrowserHttpRequest) (BrowserHttpHandler *handler, Options options)
	    : BrowserHttpRequest (handler, options)
	{
		headers = NULL;
	}

	virtual ~GECKO_SYM(BrowserHttpRequest) ()
	{
		if (headers != NULL)
			g_string_free (headers, true);
		if (listener != NULL)
			listener->ClearRequest ();
	}

	virtual void AbortImpl ();
	virtual void OpenImpl ();
	virtual void SendImpl ();
	virtual void SetHeaderImpl (const char *name, const char *value, bool disable_folding);
	virtual void SetBodyImpl (void *body, guint32 size);
	
	void OnStartRequest ();
	void OnStopRequest (nsresult status);
	void OnDataAvailable (guint32 offset, void *buffer, guint32 count);
};

/*
 * HeaderVisitor
 */

NS_IMPL_ISUPPORTS1(GECKO_SYM(HeaderVisitor), nsIHttpHeaderVisitor)

NS_IMETHODIMP
GECKO_SYM(HeaderVisitor)::VisitHeader(const nsACString &header, const nsACString &value)
{
	char *name, *val;
	PRUint32 nl, vl;

	nl = NS_CStringGetData (header, (const char **) &name);
	vl = NS_CStringGetData (value, (const char **) &val);

	/* We need to strndup since the name/val data we get might not be null-terminated */
	name = g_strndup (name, nl);
	val = g_strndup (val, vl);

	response->AppendHeader (name, val);

	g_free (name);
	g_free (val);

	return NS_OK;
}

/*
 * StreamListener
 */

NS_IMPL_ISUPPORTS1 (GECKO_SYM(StreamListener), nsIStreamListener)

GECKO_SYM(StreamListener)::GECKO_SYM(StreamListener) (GECKO_SYM(BrowserHttpRequest) *request)
{
	this->request = request;
}

GECKO_SYM(StreamListener)::~GECKO_SYM(StreamListener) ()
{
	this->request = NULL;
}

NS_IMETHODIMP
GECKO_SYM(StreamListener)::OnStartRequest (nsIRequest *request, nsISupports *context)
{
	if (this->request != NULL)
		this->request->OnStartRequest ();
	return NS_OK;
}

NS_IMETHODIMP
GECKO_SYM(StreamListener)::OnStopRequest (nsIRequest *request, nsISupports *ctx, nsresult status)
{
	if (this->request != NULL)
		this->request->OnStopRequest (status);
	return NS_OK;
}

NS_IMETHODIMP
GECKO_SYM(StreamListener)::OnDataAvailable (nsIRequest *request, nsISupports *context, nsIInputStream *input, PRUint32 offset, PRUint32 count)
{
	PRUint32 length = 0;

	if (this->request != NULL) {
		void *buffer = NS_Alloc (count);
		input->Read ((char *) buffer, count, &length);
		this->request->OnDataAvailable (offset, buffer, length);
		NS_Free (buffer);
	}

	return NS_OK;
}

/*
 * BrowserRequest
 */

void
GECKO_SYM(BrowserHttpRequest)::OnStartRequest ()
{
	nsCOMPtr<GECKO_SYM(HeaderVisitor)> hv;
	nsCString status_text;
	PRUint32 status;
	HttpResponse *response;

	SetCurrentDeployment ();

	if (IsAborted ())
		return;

	response = new BrowserHttpResponse (this);
	hv = new GECKO_SYM(HeaderVisitor) (response);
	httpchannel->VisitResponseHeaders (hv);

	httpchannel->GetResponseStatusText (status_text);
	httpchannel->GetResponseStatus (&status);

	response->SetStatus (status, status_text.get ());
	Started (response);
	response->unref ();
}

void
GECKO_SYM(BrowserHttpRequest)::OnStopRequest (nsresult status)
{
	SetCurrentDeployment ();

	if (IsAborted ())
		return;

	if (status == NS_OK) {
		Succeeded ();
	} else {
		Failed ("unknown error");
	}
}

void
GECKO_SYM(BrowserHttpRequest)::OnDataAvailable (guint32 offset, void *buffer, guint32 count)
{
	SetCurrentDeployment ();

	if (IsAborted ())
		return;

	Write (offset, buffer, count);
}

void
GECKO_SYM(BrowserHttpRequest)::AbortImpl ()
{
	if (!httpchannel)
		return;

	httpchannel->Cancel (NS_BINDING_ABORTED);
}

void
GECKO_SYM(BrowserHttpRequest)::OpenImpl ()
{
	nsCOMPtr<nsIChannel> channel;
	nsresult rv = NS_OK;
	nsCOMPtr<nsIServiceManager> mgr;
	nsEmbedCString meth;

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
	url = this->GetUri ();

	nsCOMPtr<nsIURI> uri;
	rv = ioservice->NewURI (url, nsnull, nsnull, getter_AddRefs (uri));

	ioservice->NewChannelFromURI (uri, getter_AddRefs (channel));

	httpchannel = do_QueryInterface (channel);
	if (!httpchannel)
		return;

	meth = this->GetVerb ();

	httpchannel->SetRequestMethod (meth);
}

void
GECKO_SYM(BrowserHttpRequest)::SendImpl ()
{
	nsresult rs = NS_OK;

	if (!httpchannel)
		return;

	if (headers != NULL && headers->len > 0)
		SetBody (NULL, 0); /* flush */

	if ((GetOptions () & DisableCache) != 0)
		httpchannel->SetLoadFlags (nsIRequest::INHIBIT_CACHING);

	listener = new GECKO_SYM(StreamListener) (this);

	LOG_DOWNLOADER ("FFxBrowserHttpRequest::SendImpl (): opening...\n");
	rs = httpchannel->AsyncOpen (listener, listener);

	if (NS_FAILED (rs))
		Failed ("Failed to open http channel");
}

void
GECKO_SYM(BrowserHttpRequest)::SetHeaderImpl (const char *name, const char *value, bool disable_folding)
{
	nsEmbedCString nsname, nsvalue;

	if (!httpchannel)
		return;

	if (disable_folding) {
		if (headers == NULL)
			headers = g_string_new ("");

		g_string_append_printf (headers, "%s: %s\r\n", name, value);
		return;
	}

	nsname = name;
	nsvalue = value;

	httpchannel->SetRequestHeader (nsname, nsvalue, true);
}

void
GECKO_SYM(BrowserHttpRequest)::SetBodyImpl (void *body, guint32 size)
{
	nsCOMPtr<nsIUploadChannel> upload;
	nsCOMPtr<nsIStorageStream> storage;
	nsCOMPtr<nsIOutputStream> output;
	nsCOMPtr<nsIInputStream> input;
	nsEmbedCString type;
	nsCString method;
	PRUint32 written;
	nsresult rv;

	if (!httpchannel)
		return;

	upload = do_QueryInterface (httpchannel);
	if (!upload)
		return;

	storage = do_CreateInstance ("@mozilla.org/storagestream;1", &rv);
	storage->Init (2048, PR_UINT32_MAX, nsnull);
	storage->GetOutputStream (0, getter_AddRefs (output));

	if (headers != NULL && headers->len > 0) {
		output->Write (headers->str, headers->len, &written);
		g_string_set_size (headers, 0);
	}
	if (size > 0) {
		output->Write ("\n", 1, &written);
		output->Write ((const char *)body, size, &written);
	} else {
		output->Write ("\r\n", 2, &written);
	}
	output->Close ();

	rv = storage->NewInputStream (0, getter_AddRefs (input));

	httpchannel->GetRequestMethod (method);
	
	upload->SetUploadStream (input, type, -1);
	
	httpchannel->SetRequestMethod (method);
}

/*
 *
 */

BrowserHttpRequest*
GECKO_SYM(BrowserBridge)::CreateRequest (BrowserHttpHandler *handler, HttpRequest::Options options)
{
	return new GECKO_SYM(BrowserHttpRequest) (handler, options);
}
