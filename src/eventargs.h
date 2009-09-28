/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * eventargs.h
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __EVENTARGS_H__
#define __EVENTARGS_H__

#include <glib.h>

#include <cairo.h>
#include "dependencyobject.h"
#include "keyboard.h"
#include "enums.h"
#include "pal.h"

class StylusInfo;
class StylusPointCollection;
class UIElement;

/*
 * EventArgs needs to be ref counted since js can keep objects around until
 * after the event has been emitted.
 */

/* @Namespace=None */
class EventArgs : public DependencyObject {
public:
	EventArgs ();

protected:
	virtual ~EventArgs ();
	EventArgs (Type::Kind type);
};

/* @Namespace=None */
class RenderingEventArgs : public EventArgs {
public:
	RenderingEventArgs (TimeSpan renderingTime);

	/* @GenerateCBinding,GeneratePInvoke */
	TimeSpan GetRenderingTime ();

protected:
	virtual ~RenderingEventArgs ();

private:

	TimeSpan renderingTime;
};

enum CollectionChangedAction {
	CollectionChangedActionAdd,
	CollectionChangedActionRemove,
	CollectionChangedActionReplace,
	CollectionChangedActionClearing,
	CollectionChangedActionCleared,
};

/* @Namespace=None */
class CollectionChangedEventArgs : public EventArgs {
public:
	/* @GenerateCBinding,GeneratePInvoke */
	CollectionChangedEventArgs ();

	CollectionChangedEventArgs (CollectionChangedAction action, Value *new_value, Value *old_value, int index);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetChangedAction (CollectionChangedAction action);
	
	/* @GenerateCBinding,GeneratePInvoke */
	CollectionChangedAction GetChangedAction ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetNewItem (Value *item);
	
	/* @GenerateCBinding,GeneratePInvoke */
	Value *GetNewItem ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetOldItem (Value *item);
	
	/* @GenerateCBinding,GeneratePInvoke */
	Value *GetOldItem ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetIndex (int index);
	
	/* @GenerateCBinding,GeneratePInvoke */
	int GetIndex ();

protected:
	virtual ~CollectionChangedEventArgs ();


private:
	CollectionChangedAction action;
	Value *old_item;
	Value *new_item;
	int index;
};

/* @Namespace=None */
class CollectionItemChangedEventArgs : public EventArgs {
public:
	CollectionItemChangedEventArgs (DependencyObject *collectionItem,
					DependencyProperty *property,
					Value *oldValue,
					Value *newValue)
	{
		this->collectionItem = collectionItem;
		this->property = property;
		this->oldValue = oldValue;
		this->newValue = newValue;
	}

	DependencyObject*   GetCollectionItem() { return collectionItem; }
	DependencyProperty* GetProperty()       { return property; }
	Value*              GetOldValue ()      { return oldValue; }
	Value*              GetNewValue ()      { return newValue; }

private:
	DependencyObject *collectionItem;
	DependencyProperty *property;
	Value *oldValue;
	Value *newValue;
};

/* @Namespace=None */
class DownloadProgressEventArgs : public EventArgs {
 private:
	double progress;

 protected:
	virtual ~DownloadProgressEventArgs ();
 
 public:
	DownloadProgressEventArgs ();
	DownloadProgressEventArgs (double progress);
	
	void SetProgress (double progress);
	
	/* @GenerateCBinding,GeneratePInvoke */
	double GetProgress ();
};

/* @Namespace=None */
class RoutedEventArgs : public EventArgs {
public:
 	/* @GenerateCBinding,GeneratePInvoke */
	RoutedEventArgs ();
	
	RoutedEventArgs (DependencyObject *source);
	
	/* @GenerateCBinding,GeneratePInvoke */
	DependencyObject *GetSource ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetSource(DependencyObject *el);
	
	/* @GenerateCBinding,GeneratePInvoke */
	void SetHandled (bool handled);
	
	/* @GenerateCBinding,GeneratePInvoke */
	bool GetHandled ();

protected:
	virtual ~RoutedEventArgs ();
	RoutedEventArgs (DependencyObject *source, Type::Kind kind);
	RoutedEventArgs (Type::Kind kind);
	
private:
	DependencyObject *source;
	bool handled;
};

/* @Namespace=None */
class ExceptionRoutedEventArgs : public RoutedEventArgs {
 protected:
	virtual ~ExceptionRoutedEventArgs ();
	
 public:
	ExceptionRoutedEventArgs ();
};


/* @Namespace=None */
class KeyEventArgs : public RoutedEventArgs {
public:
 	/* @GenerateCBinding,GeneratePInvoke */
	KeyEventArgs ();
	KeyEventArgs (MoonKeyEvent *event);
	
	/* @GenerateCBinding,GeneratePInvoke */
	int GetKey ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	int GetPlatformKeyCode ();
	
	MoonKeyEvent *GetEvent ();
protected:
	virtual ~KeyEventArgs ();
	
private:
	MoonKeyEvent *event;
};

/* @Namespace=None */
class MouseEventArgs : public RoutedEventArgs {
public:
 	/* @GenerateCBinding,GeneratePInvoke */
	MouseEventArgs ();
	MouseEventArgs (MoonMouseEvent *event);
	
	MoonMouseEvent *GetEvent () { return event; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	void GetPosition (UIElement *relative_to, double *x, double *y);
	
	/* @GenerateCBinding,GeneratePInvoke */
	StylusInfo *GetStylusInfo ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	StylusPointCollection *GetStylusPoints (UIElement *ink_presenter);
	
protected:
	virtual ~MouseEventArgs ();
	
private:
	MoonMouseEvent *event;
};

/* @Namespace=None */
class MouseWheelEventArgs : public RoutedEventArgs {
public:
 	/* @GenerateCBinding,GeneratePInvoke */
	MouseWheelEventArgs ();
	MouseWheelEventArgs (MoonScrollWheelEvent *event);
	
	MoonEvent *GetEvent () { return event; }

	/* @GenerateCBinding,GeneratePInvoke */
	int GetWheelDelta ();
	
protected:
	virtual ~MouseWheelEventArgs ();
	
private:
	MoonScrollWheelEvent *event;
};

#endif /* __EVENTARGS_H__ */
