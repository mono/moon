/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * textoptions.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __TEXTOPTIONS_H__
#define __TEXTOPTIONS_H__

#include <glib.h>
#include "dependencyobject.h"

/* @IncludeInKinds,Namespace=System.Windows.Media */
class TextOptions {
 	/* @PropertyType=TextHintingMode,Attached,GenerateAccessors */
	const static int TextHintingModeProperty;

	//
	// Property Accessors
	//

	static void SetTextHintingMode (DependencyObject *item, TextHintingMode mode);
	static TextHintingMode GetTextHintingMode (DependencyObject *item);
};

#endif /* __TEXTOPTIONS_H__ */
