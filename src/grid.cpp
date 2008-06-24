/*
 * grid.cpp: canvas definitions.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include "brush.h"
#include "rect.h"
#include "canvas.h"
#include "grid.h"
#include "runtime.h"
#include "namescope.h"
#include "collection.h"

Grid::Grid ()
{
	SetValue (Grid::ColumnDefinitions, Value::CreateUnref (new ColumnDefinitionCollection ()));
	SetValue (Grid::RowDefinitions, Value::CreateUnref (new RowDefinitionCollection ()));
}

void
Grid::OnPropertyChanged (PropertyChangedEventArgs *args)
{
       if (args->property->type != Type::GRID) {
               Panel::OnPropertyChanged (args);
               return;
       }

       if (args->property == Grid::ShowGridLinesProperty){
               if (GetVisualParent () == NULL)
                       UpdateTransform ();
       }

       NotifyListenersOfPropertyChange (args);
}

void
Grid::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
       if (subobj_args->property == Canvas::TopProperty || subobj_args->property == Canvas::LeftProperty) {
               //
               // Technically the grid cares about Visuals, but we cant do much
               // with them, all the logic to relayout is in UIElement
               //
               if (!Type::Find (obj->GetObjectType ())->IsSubclassOf (Type::UIELEMENT)){
                       printf ("Grid: Child %s is not a UIELEMENT\n", dependency_object_get_name (obj));
                       return;
               }
               UIElement *ui = (UIElement *) obj;

               ui->UpdateTransform ();
       }
       else
               Panel::OnSubPropertyChanged (prop, obj, subobj_args);
}

void
Grid::OnCollectionChanged (Collection *col, CollectionChangeType type, DependencyObject *obj, PropertyChangedEventArgs *element_args)
{
       if (col == GetValue (Grid::ColumnDefinitions)->AsColumnDefinitionCollection () ||
           col == GetValue (Grid::RowDefinitions)->AsRowDefinitionCollection ()){

               //
               // Do something
               //
               fprintf (stderr, "Grid:OnCollectionChanged: do something\n");
       }
}

Grid *
grid_new (void)
{
       return new Grid ();
}

DependencyProperty *Grid::ColumnProperty;
DependencyProperty *Grid::ColumnSpanProperty;
DependencyProperty *Grid::RowProperty;
DependencyProperty *Grid::RowSpanProperty;
DependencyProperty *Grid::ShowGridLinesProperty;
DependencyProperty *Grid::RowDefinitions;
DependencyProperty *Grid::ColumnDefinitions;

DependencyProperty *RowDefinition::HeightProperty;
DependencyProperty *RowDefinition::MaxHeightProperty;
DependencyProperty *RowDefinition::MinHeightProperty;

DependencyProperty *ColumnDefinition::WidthProperty;
DependencyProperty *ColumnDefinition::MaxWidthProperty;
DependencyProperty *ColumnDefinition::MinWidthProperty;

ColumnDefinitionCollection *
column_definition_collection_new ()
{
       return new ColumnDefinitionCollection ();
}

RowDefinitionCollection
*row_definition_collection_new ()
{
       return new RowDefinitionCollection ();
}

ColumnDefinition *
column_definition_new ()
{
       return new ColumnDefinition ();
}

RowDefinition *
row_definition_new ()
{
       return new RowDefinition ();
}

double
row_definition_get_actual_height (RowDefinition *def)
{
       return def->actual;
}

double
column_definition_get_actual_width (ColumnDefinition *def)
{
       return def->actual;
}

void 
grid_init (void)
{
       // RowDefinition
       RowDefinition::HeightProperty = DependencyObject::Register (Type::ROWDEFINITION, "Height", Type::GRIDLENGTH);
       RowDefinition::MaxHeightProperty = DependencyObject::Register (Type::ROWDEFINITION, "MaxHeight", Type::DOUBLE);
       RowDefinition::MinHeightProperty = DependencyObject::Register (Type::ROWDEFINITION, "MinHeight", Type::DOUBLE);

       // ColumnDefinition
       ColumnDefinition::WidthProperty = DependencyObject::Register (Type::ROWDEFINITION, "Width", Type::GRIDLENGTH);
       ColumnDefinition::MaxWidthProperty = DependencyObject::Register (Type::ROWDEFINITION, "MaxWidth", Type::DOUBLE);
       ColumnDefinition::MinWidthProperty = DependencyObject::Register (Type::ROWDEFINITION, "MinWidth", Type::DOUBLE);

       // Grid
       Grid::ColumnProperty = DependencyObject::RegisterFull (Type::GRID, "Column", new Value (0), Type::INT32, true, false);
       Grid::ColumnSpanProperty = DependencyObject::RegisterFull (Type::GRID, "ColumnSpan", new Value (0), Type::INT32, true, false);
       Grid::RowProperty = DependencyObject::RegisterFull (Type::GRID, "Row", new Value (0), Type::INT32, true, false);
       Grid::RowSpanProperty = DependencyObject::RegisterFull (Type::GRID, "RowSpan", new Value (0), Type::INT32, true, false);
       Grid::ShowGridLinesProperty = DependencyObject::RegisterFull (Type::GRID, "ShowGridLines", new Value (false), Type::BOOL, true, false);

       Grid::ColumnDefinitions = DependencyObject::Register (Type::GRID, "ColumnDefinitions", Type::COLUMNDEFINITION_COLLECTION);
       Grid::RowDefinitions    = DependencyObject::Register (Type::GRID, "RowDefinitions", Type::ROWDEFINITION_COLLECTION);
}
