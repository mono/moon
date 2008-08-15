/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * grid.h
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_GRID_H__
#define __MOON_GRID_H__

#include <glib.h>
#include "panel.h"

enum GridUnitType {
       Auto,
       Pixel,
       Star
};

/* @IncludeInKinds */
/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
struct GridLength {
 public:
	double val;
	GridUnitType type;
	
	GridLength () {
		val = 0;
		type = Auto;
	}
	
	GridLength (double v, GridUnitType t)
	{
		val = v;
		type = t;
	}
};

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class ColumnDefinition : public DependencyObject {
	// Actual width computed
	double actual;
	
 protected:
	virtual ~ColumnDefinition () {}
	
 public:
 	/* @PropertyType=double */
	static DependencyProperty *MaxWidthProperty;
 	/* @PropertyType=double */
	static DependencyProperty *MinWidthProperty;
 	/* @PropertyType=GridLength */
	static DependencyProperty *WidthProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ColumnDefinition () { actual = 0; }
	
	/* @GenerateCBinding,GeneratePInvoke */
	double GetActualWidth () { return actual; }
	
	virtual Type::Kind GetObjectType () { return Type::COLUMNDEFINITION; }
};

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class RowDefinition : public DependencyObject {
	// Actual height computed
	double actual;
	
 protected:
	virtual ~RowDefinition () {}
	
 public:
 	/* @PropertyType=GridLength */
	static DependencyProperty *HeightProperty;
 	/* @PropertyType=double */
	static DependencyProperty *MaxHeightProperty;
 	/* @PropertyType=double */
	static DependencyProperty *MinHeightProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	RowDefinition () { }
	
	/* @GenerateCBinding,GeneratePInvoke */
	double GetActualHeight () { return actual; }
	
	virtual Type::Kind GetObjectType () { return Type::ROWDEFINITION; }
};

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class ColumnDefinitionCollection : public DependencyObjectCollection {
 protected:
	virtual ~ColumnDefinitionCollection () {}
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	ColumnDefinitionCollection () {}
	
	virtual Type::Kind GetObjectType ()  { return Type::COLUMNDEFINITION_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::COLUMNDEFINITION; }
};


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class RowDefinitionCollection : public DependencyObjectCollection {
 protected:
	virtual ~RowDefinitionCollection () {}
	
 public:
	/* @GenerateCBinding,GeneratePInvoke */
	RowDefinitionCollection () {}
	
	virtual Type::Kind GetObjectType ()  { return Type::ROWDEFINITION_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::ROWDEFINITION; }
};


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class Grid : public Panel {
 protected:
	virtual ~Grid () {}

 public:
 	/* @PropertyType=gint32,DefaultValue=0,Attached */
	static DependencyProperty *ColumnProperty;
	/* @PropertyType=ColumnDefinitionCollection,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal */
	static DependencyProperty *ColumnDefinitionsProperty;
 	/* @PropertyType=gint32,DefaultValue=0,Attached */
	static DependencyProperty *ColumnSpanProperty;
 	/* @PropertyType=gint32,DefaultValue=0,Attached */
	static DependencyProperty *RowProperty;
	/* @PropertyType=RowDefinitionCollection,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal */
	static DependencyProperty *RowDefinitionsProperty;
 	/* @PropertyType=gint32,DefaultValue=0,Attached */
	static DependencyProperty *RowSpanProperty;
 	/* @PropertyType=bool,DefaultValue=false,Attached */
	static DependencyProperty *ShowGridLinesProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Grid ();
	
	virtual Type::Kind GetObjectType () { return Type::GRID; }
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
};

#endif /* __MOON_PANEL_H__ */
