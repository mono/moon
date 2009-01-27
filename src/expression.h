/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * expressions.h: base Expression class
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__

#include "dependencyobject.h"

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
class Expression : public EventObject {
 protected:
	virtual ~Expression ();

public:
	Expression ();
};

#endif /* __EXPRESSION_H__ */
