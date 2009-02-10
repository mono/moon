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

/* @IncludeInKinds */
/* @SilverlightVersion="2" */
/* @Namespace=System.Windows */
struct GridLength {
 public:
	double val;
	GridUnitType type;
	
	GridLength () {
		val = 0;
		type = GridUnitTypeAuto;
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
	virtual ~ColumnDefinition ();
	
 public:
 	/* @PropertyType=double,DefaultValue=INFINITY,GenerateAccessors */
	static DependencyProperty *MaxWidthProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *MinWidthProperty;
 	/* @PropertyType=GridLength,DefaultValue=GridLength (1.0\, GridUnitTypeStar),GenerateAccessors */
	static DependencyProperty *WidthProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	ColumnDefinition ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	double GetActualWidth () { return actual; }
	void SetActualWidth (double value) { actual = value; }

	// property accessors
	double GetMaxWidth();
	void SetMaxWidth (double value);

	double GetMinWidth();
	void SetMinWidth (double value);

	GridLength* GetWidth();
	void SetWidth (GridLength *value);
};

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class RowDefinition : public DependencyObject {
	// Actual height computed
	double actual;
	
 protected:
	virtual ~RowDefinition ();
	
 public:
 	/* @PropertyType=GridLength,DefaultValue=GridLength (1.0\, GridUnitTypeStar),GenerateAccessors */
	static DependencyProperty *HeightProperty;
 	/* @PropertyType=double,DefaultValue=INFINITY,GenerateAccessors */
	static DependencyProperty *MaxHeightProperty;
 	/* @PropertyType=double,DefaultValue=0.0,GenerateAccessors */
	static DependencyProperty *MinHeightProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	RowDefinition ();
	
	/* @GenerateCBinding,GeneratePInvoke */
	double GetActualHeight () { return actual; }
	void SetActualHeight (double value) { actual = value; }

	// property accessors
	double GetMaxHeight();
	void SetMaxHeight (double value);

	double GetMinHeight();
	void SetMinHeight (double value);

	GridLength* GetHeight();
	void SetHeight (GridLength *value);
};

/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class ColumnDefinitionCollection : public DependencyObjectCollection {
 protected:
	virtual ~ColumnDefinitionCollection ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	ColumnDefinitionCollection ();
	
	virtual Type::Kind GetElementType () { return Type::COLUMNDEFINITION; }
};


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class RowDefinitionCollection : public DependencyObjectCollection {
 protected:
	virtual ~RowDefinitionCollection ();
	
 public:
	/* @GenerateCBinding,GeneratePInvoke,ManagedAccess=Internal */
	RowDefinitionCollection ();
	
	virtual Type::Kind GetElementType () { return Type::ROWDEFINITION; }
};


/* @SilverlightVersion="2" */
/* @Namespace=System.Windows.Controls */
class Grid : public Panel {
	Size magic;

 protected:
	virtual ~Grid ();

 public:
 	/* @PropertyType=gint32,DefaultValue=0,Attached,GenerateAccessors,Validator=PositiveIntValidator */
	static DependencyProperty *ColumnProperty;
	/* @PropertyType=ColumnDefinitionCollection,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	static DependencyProperty *ColumnDefinitionsProperty;
 	/* @PropertyType=gint32,DefaultValue=1,Attached,GenerateAccessors,Validator=IntGreaterThanZeroValidator */
	static DependencyProperty *ColumnSpanProperty;
 	/* @PropertyType=gint32,DefaultValue=0,Attached,GenerateAccessors,Validator=PositiveIntValidator */
	static DependencyProperty *RowProperty;
	/* @PropertyType=RowDefinitionCollection,AutoCreateValue,ManagedFieldAccess=Internal,ManagedSetterAccess=Internal,GenerateAccessors */
	static DependencyProperty *RowDefinitionsProperty;
 	/* @PropertyType=gint32,DefaultValue=1,Attached,GenerateAccessors,Validator=IntGreaterThanZeroValidator */
	static DependencyProperty *RowSpanProperty;
 	/* @PropertyType=bool,DefaultValue=true,GenerateAccessors */
	static DependencyProperty *ShowGridLinesProperty;
	
	/* @GenerateCBinding,GeneratePInvoke */
	Grid ();
	
	virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
	virtual void OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args);
	
	virtual Size MeasureOverride (Size availableSize);
	virtual Size ArrangeOverride (Size finalSize);

	// property accessors
	ColumnDefinitionCollection *GetColumnDefinitions ();
	void SetColumnDefinitions (ColumnDefinitionCollection* value);

	static gint32 GetColumn (DependencyObject *obj);
	static void SetColumn (DependencyObject *obj, gint32 value);

	static gint32 GetColumnSpan (DependencyObject *obj);
	static void SetColumnSpan (DependencyObject *obj, gint32 value);

	RowDefinitionCollection *GetRowDefinitions ();
	void SetRowDefinitions (RowDefinitionCollection* value);

	static gint32 GetRow (DependencyObject *obj);
	static void SetRow (DependencyObject *obj, gint32 value);

	static gint32 GetRowSpan (DependencyObject *obj);
	static void SetRowSpan (DependencyObject *obj, gint32 value);

	bool GetShowGridLines ();
	void SetShowGridLines (bool value);
};

#endif /* __MOON_PANEL_H__ */
