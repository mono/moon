/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * textselection.h: 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __TEXTSELECTION_H__
#define __TEXTSELECTION_H__

#include <glib.h>
#include "textpointer.h"
#include "dependencyproperty.h"
#include "textelement.h"
#include "value.h"
#include "error.h"

namespace Moonlight {

class RichTextBox;

class TextSelection {
public:
	TextSelection ();

	virtual ~TextSelection () {}

	/* @GeneratePInvoke */
	static void Free (TextSelection *selection) { delete selection; }
	
	//
	// Methods
	//
	/* @GeneratePInvoke */
	void ApplyPropertyValue (DependencyProperty *formatting, Value *value);
	/* @GeneratePInvoke */
	Value *GetPropertyValue (DependencyProperty *formatting);
	/* @GeneratePInvoke */
	void Insert (TextElement *element);
	/* @GeneratePInvoke */
	bool SelectWithError (TextPointer *anchorPosition, TextPointer *movingPosition, MoonError *error);
	bool Select (TextPointer *anchorPosition, TextPointer *movingPosition);
	
	/* @GeneratePInvoke */
	void SetText (const char *text);
	/* @GeneratePInvoke */
	char *GetText ();
	
	/* @GeneratePInvoke */
	void SetXaml (const char *xaml);
	/* @GeneratePInvoke */
	char *GetXaml ();

	/* @GeneratePInvoke */
	TextPointer *GetStart ();

	/* @GeneratePInvoke */
	TextPointer *GetEnd ();

	bool IsEmpty ();

private:
	void ClearSelection ();

	TextPointer anchor;
	TextPointer moving;
};

};

#endif /* __TEXTSELECTION_H__ */

