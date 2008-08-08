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
	SetValue (Grid::ColumnDefinitionsProperty, Value::CreateUnref (new ColumnDefinitionCollection ()));
	SetValue (Grid::RowDefinitionsProperty, Value::CreateUnref (new RowDefinitionCollection ()));
}

void
Grid::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType() != Type::GRID) {
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
                       printf ("Grid: Child %s is not a UIELEMENT\n",obj ? obj->GetName () : NULL);
                       return;
               }
               UIElement *ui = (UIElement *) obj;

               ui->UpdateTransform ();
       }
       else
               Panel::OnSubPropertyChanged (prop, obj, subobj_args);
}

void
Grid::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
       if (col == GetValue (Grid::ColumnDefinitionsProperty)->AsColumnDefinitionCollection () ||
           col == GetValue (Grid::RowDefinitionsProperty)->AsRowDefinitionCollection ()) {
               //
               // Do something
               //
               fprintf (stderr, "Grid:OnCollectionChanged: do something\n");
       }
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
	// Don't register DPs here
	return;

       // RowDefinition
       RowDefinition::HeightProperty = DependencyProperty::Register (Type::ROWDEFINITION, "Height", Type::GRIDLENGTH);
       RowDefinition::MaxHeightProperty = DependencyProperty::Register (Type::ROWDEFINITION, "MaxHeight", Type::DOUBLE);
       RowDefinition::MinHeightProperty = DependencyProperty::Register (Type::ROWDEFINITION, "MinHeight", Type::DOUBLE);

       // ColumnDefinition
       ColumnDefinition::WidthProperty = DependencyProperty::Register (Type::COLUMNDEFINITION, "Width", Type::GRIDLENGTH);
       ColumnDefinition::MaxWidthProperty = DependencyProperty::Register (Type::COLUMNDEFINITION, "MaxWidth", Type::DOUBLE);
       ColumnDefinition::MinWidthProperty = DependencyProperty::Register (Type::COLUMNDEFINITION, "MinWidth", Type::DOUBLE);

       // Grid
       Grid::ColumnProperty = DependencyProperty::RegisterFull (Type::GRID, "Column", new Value (0), Type::INT32, true, false);
       Grid::ColumnSpanProperty = DependencyProperty::RegisterFull (Type::GRID, "ColumnSpan", new Value (0), Type::INT32, true, false);
       Grid::RowProperty = DependencyProperty::RegisterFull (Type::GRID, "Row", new Value (0), Type::INT32, true, false);
       Grid::RowSpanProperty = DependencyProperty::RegisterFull (Type::GRID, "RowSpan", new Value (0), Type::INT32, true, false);
       Grid::ShowGridLinesProperty = DependencyProperty::RegisterFull (Type::GRID, "ShowGridLines", new Value (false), Type::BOOL, true, false);

       Grid::ColumnDefinitionsProperty = DependencyProperty::Register (Type::GRID, "ColumnDefinitions", Type::COLUMNDEFINITION_COLLECTION);
       Grid::RowDefinitionsProperty    = DependencyProperty::Register (Type::GRID, "RowDefinitions", Type::ROWDEFINITION_COLLECTION);
}
