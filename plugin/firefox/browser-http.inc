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

class GECKO_SYM(DownloaderResponse) : public DownloaderResponse, public nsIStreamListener {
 private:
	nsCOMPtr<nsIChannel> channel;
	bool aborted;

 protected:
	NS_DECL_NSIREQUESTOBSERVER
	NS_DECL_NSISTREAMLISTENER

 public:
	NS_DECL_ISUPPORTS

	GECKO_SYM(DownloaderResponse) (nsCOMPtr<nsIChannel> channel, DownloaderResponseStartedHandler started, DownloaderResponseDataAvailableHandler available, DownloaderResponseFinishedHandler finished, gpointer context)
		: DownloaderResponse (started, available, finished, context)
	{
		this->channel = channel;
		this->aborted = false;
	}

	virtual ~GECKO_SYM(DownloaderResponse) ()
	{
	}

	void Abort ();
};

class GECKO_SYM(DownloaderRequest) : public DownloaderRequest {
 private:
	nsCOMPtr<nsIChannel> channel;

	void CreateChannel ();

 public:
	GECKO_SYM(DownloaderRequest) (const char *method, const char *uri)
	    : DownloaderRequest (method, uri)
	{
		CreateChannel ();
	}

	~GECKO_SYM(DownloaderRequest) ()
	{
	}

	DownloaderResponse *GetResponse ();
	bool GetResponse (DownloaderResponseStartedHandler started, DownloaderResponseDataAvailableHandler available, DownloaderResponseFinishedHandler finished, gpointer context);
	void SetHttpHeader (const char *name, const char *value);
	void SetBody (void *body, int size);
	
	void Abort ();
};

// DownloaderResponse

NS_IMPL_ISUPPORTS1 (GECKO_SYM(DownloaderResponse), nsIStreamListener)

void
GECKO_SYM(DownloaderResponse)::Abort ()
{
	if (!aborted) {
		this->channel->Cancel (NS_BINDING_ABORTED);
		aborted = true;
	}
}

NS_IMETHODIMP
GECKO_SYM(DownloaderResponse)::OnStartRequest (nsIRequest *request, nsISupports *context)
{
	if (!aborted)
		return started (this, this->context);
	return NS_OK;
}

NS_IMETHODIMP
GECKO_SYM(DownloaderResponse)::OnStopRequest (nsIRequest *request, nsISupports *ctx, nsresult status)
{
	if (!aborted)
		return finished (this, this->context, (status == NS_OK), NULL);
	return NS_OK;
}

NS_IMETHODIMP
GECKO_SYM(DownloaderResponse)::OnDataAvailable (nsIRequest *request, nsISupports *context, nsIInputStream *input, PRUint32 offset, PRUint32 count)
{
	PRUint32 length = 0;
	nsresult res;

	if (aborted)
		return NS_OK;

	char *buffer = (char *) NS_Alloc (count);
	input->Read (buffer, count, &length);
	res = available (this, this->context, buffer, length);
	NS_Free (buffer);

	return res;
}

// DownloaderRequest

void
GECKO_SYM(DownloaderRequest)::CreateChannel ()
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

	printf ("DownloaderRequest: %s\n", uri);

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
GECKO_SYM(DownloaderRequest)::Abort ()
{
	channel->Cancel (NS_BINDING_ABORTED);
}

bool
GECKO_SYM(DownloaderRequest)::GetResponse (DownloaderResponseStartedHandler started, DownloaderResponseDataAvailableHandler available, DownloaderResponseFinishedHandler finished, gpointer context)
{
	nsresult rs = NS_OK;
	GECKO_SYM(DownloaderResponse) *response;

	response = new GECKO_SYM(DownloaderResponse) (channel, started, available, finished, context);
	rs = channel->AsyncOpen (response, (GECKO_SYM(DownloaderResponse)*)response);
	return !NS_FAILED (rs);
}

void
GECKO_SYM(DownloaderRequest)::SetHttpHeader (const char *name, const char *value)
{
	nsCOMPtr<nsIHttpChannel> httpchannel = do_QueryInterface (channel);
	if (!httpchannel)
		return;

	nsEmbedCString nsname, nsvalue;
	nsname = name;
	nsvalue = value;

	httpchannel->SetRequestHeader (nsname, nsvalue, true);
}

void
GECKO_SYM(DownloaderRequest)::SetBody (void *body, int size)
{
	nsCOMPtr<nsIHttpChannel> httpchannel = do_QueryInterface (channel);
	if (!httpchannel)
		return;

	nsCOMPtr<nsIUploadChannel> upload = do_QueryInterface (channel);
	if (!upload)
		return;

	nsEmbedCString type;
	nsresult rv;

	nsCOMPtr<nsIStorageStream> storage = do_CreateInstance ("@mozilla.org/storagestream;1", &rv);
	storage->Init (2048, PR_UINT32_MAX, nsnull);

	nsCOMPtr<nsIOutputStream> output;
	storage->GetOutputStream (0, getter_AddRefs (output));

	PRUint32 written;
	output->Write ((const char *)body, size, &written);
	output->Close ();

	nsCOMPtr<nsIInputStream> input;
	rv = storage->NewInputStream (0, getter_AddRefs (input));

	nsCString method;
	httpchannel->GetRequestMethod (method);
	
	upload->SetUploadStream (input, type, -1);
	
	httpchannel->SetRequestMethod (method);
}
