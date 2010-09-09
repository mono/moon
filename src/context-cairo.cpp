/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * context-cairo.cpp
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#include <config.h>

#include "context-cairo.h"

namespace Moonlight {

CairoContext::CairoContext (CairoSurface *surface)
{
	AbsoluteTransform transform = AbsoluteTransform ();
	Surface           *cs = new Surface (surface, Rect ());

	Stack::Push (new Context::Node (cs, &transform.m, NULL));
	cs->unref ();
}

};
