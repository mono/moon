/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * downloader.h: Downloader class.
 *
 * Contact:
 *   Moonlight List (moonligt-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __DOWNLOADER_H__
#define __DOWNLOADER_H__

#include <glib.h>

#include "dependencyobject.h"

namespace Moonlight {

// Reference:	URL Access Restrictions in Silverlight 2
//		http://msdn.microsoft.com/en-us/library/cc189008(VS.95).aspx
enum DownloaderAccessPolicy {
	DownloaderPolicy,
	MediaPolicy,
	XamlPolicy,
	FontPolicy,
	StreamingPolicy,
	MsiPolicy,
	NoPolicy
	// Keep in sync with the DownloaderAccessPolicy enum in NativeMethods.cs
};
};

#include "network.h"

namespace Moonlight {

/* @Namespace=None */
/* @ManagedDependencyProperties=None */
/* @ManagedEvents=Manual */
class Downloader : public DependencyObject {
	HttpRequest *request;
	
	char *filename;
	char *failed_msg;
	char *unzipdir;
	
	int send_queued:1;
	int completed:1;
	int started:1;
	int aborted:1;
	int unzipped:1;
	
	bool DownloadedFileIsZipped ();
	void CleanupUnzipDir ();

	EVENTHANDLER (Downloader, ProgressChanged, HttpRequest, HttpRequestProgressChangedEventArgs);
	EVENTHANDLER (Downloader, Stopped, HttpRequest, HttpRequestStoppedEventArgs);
	
	void NotifyFinished ();
	void NotifyFailed (const char *msg);
	
	static void SendAsync (EventObject *user_data);
	void SendInternal ();
	void OpenInitialize ();

	void SetFilename (const char *fname);

	void SetStatusText (const char *text);
	void SetStatus (int status);
	
 protected:
	virtual ~Downloader ();

 public:
	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	const static int DownloadProgressProperty;
	/* @PropertyType=string */
	const static int ResponseTextProperty;
	/* @PropertyType=gint32,DefaultValue=0,GenerateAccessors */
	const static int StatusProperty;
	/* @PropertyType=string,DefaultValue=\"\",GenerateAccessors */
	const static int StatusTextProperty;
	/* @PropertyType=Uri,IsConstPropertyType,GenerateAccessors */
	const static int UriProperty;
	
	// Events you can AddHandler to
	const static int CompletedEvent;
	const static int DownloadProgressChangedEvent;
	const static int DownloadFailedEvent;
	
	/* @GeneratePInvoke */
	Downloader ();
	
	static bool ValidateDownloadPolicy (const Uri *location, const Uri *uri, DownloaderAccessPolicy policy);
	
	void Abort ();
	char *GetResponseText (const char *Partname, gint64 *size);
	char *GetDownloadedFilename (const char *partname);
	void Open (const char *verb, const char *uri, DownloaderAccessPolicy policy);
	void Open (const char *verb, const Uri *uri, DownloaderAccessPolicy policy);
	void Send ();
	
	HttpRequest *GetHttpRequest () { return request; }

	bool Started () { return started; }
	bool Completed () { return completed; }
	bool Failed () { return failed_msg != NULL; }
	const char *GetFailedMessage () { return failed_msg; }
	
	const char *GetUnzippedPath ();

	//
	// Property Accessors
	//
	void SetDownloadProgress (double progress);
	double GetDownloadProgress ();
	
	const char *GetStatusText ();
	int GetStatus ();
	
	void SetUri (const Uri *uri);
	const Uri *GetUri ();
};

};
#endif
