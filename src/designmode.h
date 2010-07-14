/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * designmode.h:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __DESIGNMODE_H__
#define __DESIGNMODE_H__

#include <glib.h>
#include "dependencyobject.h"

namespace Moonlight {

/* @IncludeInKinds,Namespace=System.ComponentModel */
class DesignerProperties {
 public:
	/* @PropertyType=bool,Attached,DefaultValue=false */
	const static int IsInDesignModeProperty;
};

};

#endif /* __DESIGNMODE_H__ */
