// /* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
// /*
//  * tabnavigationwalker.cpp
//  *
//  * Contact:
//  *   Moonlight List (moonlight-list@lists.ximian.com)
//  *
//  * Copyright 2009 Novell, Inc. (http://www.novell.com)
//  *
//  * See the LICENSE file included with the distribution for details.
//  */
// 

 #include "control.h"
 #include "deployment.h"
 #include "tabnavigationwalker.h"
 #include "type.h"

TabNavigationWalker::TabNavigationWalker (UIElement *root, UIElement *current, bool forwards, Types *types)
{
	this->root = root;
	this->current = current;
	this->forwards = forwards;
	this->types = types;
	
	this->tab_sorted = g_ptr_array_new ();
}

TabNavigationWalker::~TabNavigationWalker ()
{
	g_ptr_array_free (tab_sorted, true);
}

bool
TabNavigationWalker::FocusChild ()
{
	UIElement *child;
	bool child_is_control;
	int current_index = -1;

	// Add each branch of the visual tree to the array and then sort them
	// based on the TabIndex of the first Control in each branch
	VisualTreeWalker child_walker (root);
	while ((child = child_walker.Step ()))
		g_ptr_array_add (tab_sorted, child);

	if (tab_sorted->len > 1) {
		Sort (tab_sorted, types);

		if (ReverseTab ()) {
			GPtrArray *reverse = g_ptr_array_sized_new (tab_sorted->len);
			for (int i = tab_sorted->len - 1; i >= 0; i--)
				g_ptr_array_add (reverse, tab_sorted->pdata [i]);
				
			g_ptr_array_free (tab_sorted, true);
			tab_sorted = reverse;
		}
	}

	// Find the index of the currently selected node so we know which node to
	// tab to next
	for (uint i = 0; i < tab_sorted->len; i++)
		if (tab_sorted->pdata [i] == current)
			current_index = i;

	// If a child of the root element is Focused and we're forward-tabbing, it means we should
	// skip the entire subtree and go to the next 'root'. If we're reverse-tabbing it means we
	// should Focus the root.
	if (current_index != -1 && GetActiveNavigationMode (root, types) == KeyboardNavigationModeOnce) {
		// If we're tabbing backwards and a child of this control is currently Focused, we
		// should focus this control.
		if (ReverseTab () && types->IsSubclassOf (root->GetObjectType (), Type::CONTROL))
			return TabTo ((Control *)root);
		return false;
	}

	if (tab_sorted->len > 0) {
		// If the currently selected element was found at index 'i' we need to tab
		// to the *next* index. If the currently selected item was not here, we
		// need to start at index 0.
		for (unsigned int i = 0; i < tab_sorted->len; i++) {
			// If we are not cycling, it means we've tabbed to the last element of this node and so should 
			if ((i + current_index + 1) == tab_sorted->len && GetActiveNavigationMode (root, types) != KeyboardNavigationModeCycle)
				break;

			child = (UIElement *) tab_sorted->pdata [(i + current_index + 1) % tab_sorted->len];
			child_is_control = types->IsSubclassOf (child->GetObjectType (), Type::CONTROL);

			if (child_is_control && !((Control *)child)->GetIsEnabled ())
				continue;

			// When tabbing backwards, we recurse all children *before* attempting to select this node
			if (ReverseTab () && WalkChildren (child))
				return true;

			if (child_is_control && TabTo ((Control *)child))
				return true;

			if (ForwardTab () && WalkChildren (child))
				return true;
		}
	}

	// If we're tabbing backwards and a child of this control is currently Focused, we
	// should focus this control.
	if (current_index != -1 && ReverseTab ()) {
		if (types->IsSubclassOf (root->GetObjectType (), Type::CONTROL))
			return TabTo ((Control *)root);
	}

	// Nothing could be tabbed to on this visual level
	return false;
}

bool
TabNavigationWalker::TabTo (Control *control)
{
	return control->GetIsEnabled () && control->GetIsTabStop () && control->Focus (false);
}

bool
TabNavigationWalker::WalkChildren (UIElement *root, UIElement *current)
{
	return TabNavigationWalker::WalkChildren (root, current, ForwardTab (), types);
}

//
// Static Methods
//

bool
TabNavigationWalker::Focus (UIElement *element, bool forwards)
{
	bool focused = false;
	UIElement *current = element;
	UIElement *root = element;
	Types *types = Deployment::GetCurrent ()->GetTypes ();

	// If tabbing in reverse, we immediately go up a level from initial root as
	// we know we will not be focusing any of its children.
	if (!forwards)
		root = root->GetVisualParent ();

	do {
		// If we're starting a tab sequence and the parent of the current element is set to
		// the 'Once' navigation mode, we should not traverse the children
		if (!(root == current &&
			root->GetVisualParent () &&
			GetActiveNavigationMode (root->GetVisualParent (), types) == KeyboardNavigationModeOnce)) {
			focused |= WalkChildren (root, current, forwards, types);
		}

		if (!focused && GetActiveNavigationMode (root, types) == KeyboardNavigationModeCycle)
			return true;

		current = root;
		root = root->GetVisualParent ();
	} while (!focused && root);

	if (!focused)
		focused |= WalkChildren (current, NULL, forwards, types);
	
	return focused;
}

KeyboardNavigationMode
TabNavigationWalker::GetActiveNavigationMode (UIElement *element, Types *types)
{
	while (element) {
		if (types->IsSubclassOf (element->GetObjectType (), Type::CONTROL))
			return ((Control *)element)->GetTabNavigation ();
		else
			element = element->GetVisualParent ();
	}
	return KeyboardNavigationModeLocal;
}

void
TabNavigationWalker::Sort (GPtrArray *array, Types *types)
{
	int end = array->len;
	bool swapped;

	do {
		end --;
		swapped = false;
		for (int i = 0; i < end; i++) {
			UIElement *left = NULL;
			UIElement *right = NULL;

			DeepTreeWalker left_walker ((UIElement *) array->pdata [i], Logical, types);
			DeepTreeWalker right_walker ((UIElement *) array->pdata [i + 1], Logical, types);

			while ((left = left_walker.Step ()) && !types->IsSubclassOf (left->GetObjectType (), Type::CONTROL)) { }
			while ((right = right_walker.Step ()) && !types->IsSubclassOf (right->GetObjectType (), Type::CONTROL)) { }

			if (TabCompare ((Control *)left, (Control *)right) > 0) {
				left = (UIElement *) array->pdata [i];
				array->pdata [i] = array->pdata [i + 1];
				array->pdata [i + 1] = left;
				swapped = true;
			}
		}
	} while (swapped);
}


int
TabNavigationWalker::TabCompare (Control* left, Control* right)
{
	if (!left)
		return !right ? 0 : -1;
	if (!right)
		return 1;

	Value *v1 = left->ReadLocalValue (Control::TabIndexProperty);
	Value *v2 = right->ReadLocalValue (Control::TabIndexProperty);

	if (!v1) {
		return v2 ? -1 : 0;
	} else if (!v2) {
		return 1;
	} else {
		int l = v1->AsInt32 ();
		int r = v2->AsInt32 ();
		if (l > r)
			return 1;
		return l == r ? 0 : -1;
	}
}

bool
TabNavigationWalker::WalkChildren (UIElement *root, UIElement *current, bool forwards, Types *types)
{
	TabNavigationWalker walker (root, current, forwards, types);
	return walker.FocusChild ();
}
