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

#if INCLUDE_MONO_RUNTIME

#include <glib.h>
#include "panel.h"

enum GridUnitType {
       Auto,
       Pixel,
       Star
};

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
class ColumnDefinition : public DependencyObject {
protected:
       ~ColumnDefinition() {}
public:
       // Actual width computed
       double actual;

       ColumnDefinition () { actual = 0; }
       virtual Type::Kind GetObjectType () { return Type::COLUMNDEFINITION; }

       static DependencyProperty *WidthProperty;
       static DependencyProperty *MaxWidthProperty;
       static DependencyProperty *MinWidthProperty;
};

/* @SilverlightVersion="2" */
class RowDefinition : public DependencyObject {
protected:
       ~RowDefinition() { actual = 0; }
public:
       // Actual height computed
       double actual;

       RowDefinition () {}
       virtual Type::Kind GetObjectType () { return Type::ROWDEFINITION; }

       static DependencyProperty *HeightProperty;
       static DependencyProperty *MaxHeightProperty;
       static DependencyProperty *MinHeightProperty;
};

/* @SilverlightVersion="2" */
class ColumnDefinitionCollection : public Collection {
protected:
       virtual ~ColumnDefinitionCollection () {}

public:
       ColumnDefinitionCollection () {}
       virtual Type::Kind GetObjectType ()  { return Type::COLUMNDEFINITION_COLLECTION; }
       virtual Type::Kind GetElementType () { return Type::COLUMNDEFINITION; }
};


/* @SilverlightVersion="2" */
class RowDefinitionCollection : public Collection {
protected:
       virtual ~RowDefinitionCollection () {}
public:
       RowDefinitionCollection () {}
       virtual Type::Kind GetObjectType ()  { return Type::ROWDEFINITION_COLLECTION; }
       virtual Type::Kind GetElementType () { return Type::ROWDEFINITION; }
};


/* @SilverlightVersion="2" */
class Grid : public Panel {
 private:

 protected:
       virtual ~Grid () {}

 public:
       static DependencyProperty *ColumnProperty;
       static DependencyProperty *ColumnSpanProperty;
       static DependencyProperty *RowProperty;
       static DependencyProperty *RowSpanProperty;
       static DependencyProperty *ShowGridLinesProperty;

       Grid ();
       virtual Type::Kind GetObjectType () { return Type::GRID; }

       virtual void OnPropertyChanged (PropertyChangedEventArgs *args);
       virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args);
       virtual void OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args);

       //
       // Grid: internals, these are not really exposed to the user
       // as DependencyProperties but as regular properties on the c#
       // side
       //
       static DependencyProperty *ColumnDefinitions;
       static DependencyProperty *RowDefinitions;
};

G_BEGIN_DECLS

ColumnDefinitionCollection *column_definition_collection_new ();
RowDefinitionCollection    *row_definition_collection_new ();

ColumnDefinition           *column_definition_new ();
RowDefinition              *row_definition_new ();

double            row_definition_get_actual_height    (RowDefinition *def);
double            column_definition_get_actual_width (ColumnDefinition *def);

Grid *grid_new (void);
void grid_init (void);

G_END_DECLS

#endif

#endif /* __MOON_PANEL_H__ */
