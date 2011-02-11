 /*
 * npstream-downloader.h: NPStream Browser Request
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __NPSTREAM_DOWNLOADER_H
#define __NPSTREAM_DOWNLOADER_H

#include "moonlight.h"
#include "plugin-downloader.h"

namespace Moonlight {

class NPStreamRequest : public BrowserHttpRequest {
private:
	bool pending_unref;
	NPStream *stream;

	virtual void OpenImpl ();
	virtual void SendImpl ();
	virtual void AbortImpl ();
	virtual void SetBodyImpl (const void *body, guint32 size);
	virtual void SetHeaderImpl (const char *name, const char *value, bool disable_folding);

public:
	NPStreamRequest (BrowserHttpHandler *handler, HttpRequest::Options options);

	void DestroyStream ();
	void UrlNotify (const char *url, NPReason reason);
	void Write (gint32 offset, gint32 len, void *buffer);
	void NewStream (NPStream *stream);
};

};
#endif /* __NPSTREAM_DOWNLOADER_H */
