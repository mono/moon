/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */
 
#ifndef __MOON_TABNAVIGATIONWALKER_H__
#define __MOON_TABNAVIGATIONWALKER_H__

#include "dependencyobject.h"
#include "control.h"

/* @Namespace=None */
class TabNavigationWalker {
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	static bool Focus (UIElement *element, bool forwards);

 private:
	TabNavigationWalker (UIElement *root, UIElement *current, bool forwards, Types *types);
	~TabNavigationWalker ();
	
	bool forwards;
	UIElement *current;
	UIElement *root;
	GPtrArray *tab_sorted;
	Types *types;

	bool FocusChild ();
	bool ForwardTab () { return forwards; }
	bool ReverseTab () { return !forwards; }
	bool TabTo (Control *control);
	bool WalkChildren (UIElement *root, UIElement *current = NULL);

	static KeyboardNavigationMode GetActiveNavigationMode (UIElement *element, Types *types);
	static void Sort (GPtrArray *array, Types *types);
	static int TabCompare (Control *left, Control *right);
	static bool WalkChildren (UIElement *root, UIElement *current, bool forwards, Types *types);
};

#endif /* __MOON_TABNAVIGATIONWALKER_H__ */