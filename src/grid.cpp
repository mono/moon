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
	if (col == GetColumnDefinitions () ||
	    col == GetRowDefinitions ()) {
               //
               // Do something
               //
               fprintf (stderr, "Grid:OnCollectionChanged: do something\n");
       }
}
