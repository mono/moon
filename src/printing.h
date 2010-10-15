/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * printing.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MOON_PRINTING_H__
#define __MOON_PRINTING_H__

#include "dependencyobject.h"

namespace Moonlight {

/* @Namespace=System.Windows.Printing */
class PrintDocument : public DependencyObject {
protected:
	virtual ~PrintDocument () {}

public:
	/* @GeneratePInvoke */
	PrintDocument ();

	/* @DelegateType=EventHandler<EndPrintEventArgs> */
	const static int EndPrintEvent;

	/* @PropertyType=gint32,DefaultValue=0,GenerateAccessors */
	const static int PrintedPageCountProperty;

	/* @DelegateType=EventHandler<PrintPageEventArgs> */
	const static int PrintPageEvent;

	/* @DelegateType=EventHandler<BeginPrintEventArgs> */
	const static int BeginPrintEvent;

	const char *GetDocumentName ();
	void SetDocumentName (const char *name);
	
	void SetPrintedPageCount (gint32 value);
	gint32 GetPrintedPageCount ();
};

};
#endif /*  __MOON_PRINTING_H__ */
