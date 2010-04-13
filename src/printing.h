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

/* @Namespace=System.Windows.Printing */
class PrintDocument : public DependencyObject {
protected:
	virtual ~PrintDocument () {}

public:
	/* @GeneratePInvoke,GenerateCBinding */
	PrintDocument () {}

	/* @PropertyType=string,GenerateAccessors */
	const static int DocumentNameProperty;

	/* @DelegateType=EventHandler<EndPrintEventArgs> */
	const static int EndPrintEvent;

	/* @DelegateType=EventHandler<PrintPageEventArgs> */
	const static int PrintPageEvent;

	/* @DelegateType=EventHandler<StartPrintEventArgs> */
	const static int StartPrintEvent;

	const char *GetDocumentName ();
	void SetDocumentName (const char *name);
	
};

#endif /*  __MOON_PRINTING_H__ */

