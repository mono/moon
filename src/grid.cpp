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

#include <math.h>

#include "brush.h"
#include "rect.h"
#include "canvas.h"
#include "grid.h"
#include "runtime.h"
#include "namescope.h"
#include "collection.h"

Grid::Grid ()
{
	SetObjectType (Type::GRID);

	SetValue (Grid::ColumnDefinitionsProperty, Value::CreateUnref (new ColumnDefinitionCollection ()));
	SetValue (Grid::RowDefinitionsProperty, Value::CreateUnref (new RowDefinitionCollection ()));
}

Grid::~Grid ()
{
}

void
Grid::OnPropertyChanged (PropertyChangedEventArgs *args)
{
	if (args->property->GetOwnerType() != Type::GRID) {
		Panel::OnPropertyChanged (args);
		return;
	}

	if (args->property == Grid::ShowGridLinesProperty){
		Invalidate ();
	}

	InvalidateMeasure ();

	NotifyListenersOfPropertyChange (args);
}

void
Grid::OnSubPropertyChanged (DependencyProperty *prop, DependencyObject *obj, PropertyChangedEventArgs *subobj_args)
{
	if (subobj_args->property == Grid::ColumnProperty
	    || subobj_args->property == Grid::RowProperty
	    || subobj_args->property == Grid::ColumnSpanProperty
	    || subobj_args->property == Grid::RowSpanProperty) {
		InvalidateMeasure ();
	}
	
	Panel::OnSubPropertyChanged (prop, obj, subobj_args);
}

void
Grid::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (col == GetColumnDefinitions () ||
	    col == GetRowDefinitions ()) {

	} else {
		Panel::OnCollectionChanged (col, args);
	}
	
	InvalidateMeasure ();
}

Size
Grid::MeasureOverride (Size availableSize)
{
	Size results (0, 0);

	ColumnDefinitionCollection *columns = GetColumnDefinitions ();
	RowDefinitionCollection *rows = GetRowDefinitions ();

	int col_count = columns->GetCount ();
	int row_count = rows->GetCount ();

	if (col_count == 0) {
		columns = new ColumnDefinitionCollection ();
		columns->Add (new ColumnDefinition ());
		col_count = 1;
	}

	if (row_count == 0) {
		rows = new RowDefinitionCollection ();
		rows->Add (new RowDefinition ());
		row_count = 1;
	}

	for (int i = 0; i < row_count; i ++) {
		RowDefinition *rowdef = rows->GetValueAt (i)->AsRowDefinition ();
		GridLength* height = rowdef->GetHeight();

		rowdef->SetActualHeight (0.0);

		if (height->type == GridUnitTypePixel)
                       rowdef->SetActualHeight (height->val);
	}

	for (int i = 0; i < col_count; i ++) {
		ColumnDefinition *coldef = columns->GetValueAt (i)->AsColumnDefinition ();
		GridLength *width = coldef->GetWidth ();

		coldef->SetActualWidth (0.0);

		if (width->type == GridUnitTypePixel)
			coldef->SetActualWidth (width->val);

	}
	
	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		if (!child->GetRenderVisible ())
			continue;

		gint32 col, row;
		gint32 colspan, rowspan;

		col = MIN (Grid::GetColumn (child), col_count - 1);
		row = MIN (Grid::GetRow (child), row_count - 1);
		colspan = MIN (Grid::GetColumnSpan (child), col_count - col);
		rowspan = MIN (Grid::GetRowSpan (child), row_count - row);

		Size child_size = Size (0,0);
		Size min_size = Size (0,0);
		Size max_size = Size (0,0); 
		
		for (int r = row; r < row + rowspan; r++) {
			RowDefinition *rowdef = rows->GetValueAt (r)->AsRowDefinition ();
			GridLength* height = rowdef->GetHeight();

			if (height->type == GridUnitTypePixel)
				child_size.height += height->val;
			else
				child_size.height += rowdef->GetMaxHeight ();
			
			min_size.height += rowdef->GetMinHeight ();
			max_size.height += rowdef->GetMaxHeight ();
		}

		for (int c = col; c < col + colspan; c++) {
			ColumnDefinition *coldef = columns->GetValueAt (c)->AsColumnDefinition ();
			GridLength* width = coldef->GetWidth();

			if (width->type == GridUnitTypePixel)
				child_size.width += width->val;
			else
				child_size.width += coldef->GetMaxWidth ();

			min_size.width += coldef->GetMinWidth ();
			max_size.width += coldef->GetMaxWidth ();
		}
		
		
		child_size = child_size.Min (max_size);
		child_size = child_size.Max (min_size);

		child->Measure (child_size);
		Size desired = child->GetDesiredSize();
		
		Size remaining = desired;
		for (int c = col; c < col + colspan; c++){
			ColumnDefinition *coldef = columns->GetValueAt (c)->AsColumnDefinition ();
			GridLength *width = coldef->GetWidth ();

			double contribution = width->type != GridUnitTypePixel ? remaining.width : width->val;
			
			contribution = MAX (contribution, coldef->GetMinWidth ());
			contribution = MIN (contribution, coldef->GetMaxWidth ());
			
			coldef->SetActualWidth (MAX(coldef->GetActualWidth (), contribution));
			remaining.width -= contribution; 
		}

		for (int r = row; r < row + rowspan; r++){
			RowDefinition *rowdef = rows->GetValueAt (r)->AsRowDefinition ();
			GridLength *height = rowdef->GetHeight ();

			double contribution = height->type != GridUnitTypePixel ? remaining.height : height->val;
				
			contribution = MAX (contribution, rowdef->GetMinHeight ());
			contribution = MIN (contribution, rowdef->GetMaxHeight ());
			
			rowdef->SetActualHeight (MAX (rowdef->GetActualHeight (), contribution));
			remaining.height -= contribution;
		}
	}

	Size grid_size;
	for (int r = 0; r < row_count; r ++) {
		grid_size.height += rows->GetValueAt (r)->AsRowDefinition ()->GetActualHeight ();
	}

	for (int c = 0; c < col_count; c ++) {
		grid_size.width += columns->GetValueAt (c)->AsColumnDefinition ()->GetActualWidth ();
	}

	results = results.Max (grid_size);

	// now choose whichever is smaller, our chosen size or the availableSize.
	return results;
}

Size
Grid::ArrangeOverride (Size finalSize)
{
	ColumnDefinitionCollection *columns = GetColumnDefinitions ();
	RowDefinitionCollection *rows = GetRowDefinitions ();

	int col_count = columns->GetCount ();
	int row_count = rows->GetCount ();

	if (col_count == 0) {
		columns = new ColumnDefinitionCollection ();
		columns->Add (new ColumnDefinition ());
		col_count = 1;
	}

	if (row_count == 0) {
		rows = new RowDefinitionCollection ();
		rows->Add (new RowDefinition ());
		row_count = 1;
	}

	for (int i = 0; i < col_count; i++)
		columns->GetValueAt (i)->AsColumnDefinition ()->SetActualWidth (0.0);

	for (int i = 0; i < row_count; i++)
		rows->GetValueAt (i)->AsRowDefinition ()->SetActualHeight (0.0);

	Size remaining = finalSize;

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		if (!child->GetRenderVisible ())
			continue;

		gint32 col = MIN (Grid::GetColumn (child), col_count - 1);
		gint32 row = MIN (Grid::GetRow (child), row_count - 1);
		//gint32 colspan = Grid::GetColumnSpan (child);
		//gint32 rowspan = Grid::GetRowSpan (child);
		
		Size desired = child->GetDesiredSize ();
		ColumnDefinition *coldef = columns->GetValueAt (col)->AsColumnDefinition ();
		coldef->SetActualWidth (MAX (coldef->GetActualWidth (), desired.width));

		RowDefinition *rowdef = rows->GetValueAt (row)->AsRowDefinition ();
		rowdef->SetActualHeight (MAX (rowdef->GetActualHeight (), desired.height));
	}

	double row_stars = 0.0;
	for (int i = 0; i < row_count; i ++) {
		RowDefinition *rowdef = rows->GetValueAt (i)->AsRowDefinition ();
		GridLength* height = rowdef->GetHeight();

		switch (height->type) {
		case GridUnitTypeStar:
			remaining.height -= rowdef->GetActualHeight ();
			row_stars += height->val;
			break;
		case GridUnitTypePixel:
			rowdef->SetActualHeight (height->val);
			remaining.height -= height->val;
			break;
		case GridUnitTypeAuto:
			remaining.height -= rowdef->GetActualHeight ();
			break;
		}
	}

	double col_stars = 0.0;
	for (int i = 0; i < col_count; i ++) {
		ColumnDefinition *coldef = columns->GetValueAt (i)->AsColumnDefinition ();
		GridLength* width = coldef->GetWidth();

		switch (width->type) {
		case GridUnitTypeStar:
			remaining.width -= coldef->GetActualWidth ();
			col_stars += width->val;
			break;
		case GridUnitTypePixel:
			coldef->SetActualWidth (width->val);
			remaining.width -= width->val;
			break;
		case GridUnitTypeAuto:
			remaining.width -= coldef->GetActualWidth ();
			break;
		}
	}

	for (int i = 0; i < row_count; i ++) {
		RowDefinition *rowdef = rows->GetValueAt (i)->AsRowDefinition ();
		GridLength* height = rowdef->GetHeight();

		if (height->type == GridUnitTypeStar)
			rowdef->SetActualHeight (rowdef->GetActualHeight () + (remaining.height * height->val / row_stars));
	}

	for (int i = 0; i < col_count; i ++) {
		ColumnDefinition *coldef = columns->GetValueAt (i)->AsColumnDefinition ();
		GridLength* width = coldef->GetWidth();

		if (width->type == GridUnitTypeStar)
			coldef->SetActualWidth (coldef->GetActualWidth () + (remaining.width * width->val / col_stars));
	}

	walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		if (!child->GetRenderVisible ())
			continue;

		gint32 col = MIN (Grid::GetColumn (child), col_count - 1);
		gint32 row = MIN (Grid::GetRow (child), row_count - 1);
		gint32 colspan = MIN (Grid::GetColumnSpan (child), col_count - col);
		gint32 rowspan = MIN (Grid::GetRowSpan (child), row_count - row);

		Rect child_final = Rect (0, 0, 0, 0);
		Size min_size;
		Size max_size;

		for (int r = 0; r < row + rowspan; r++) {
			RowDefinition *rowdef = rows->GetValueAt (r)->AsRowDefinition ();

			if (r < row) {
				child_final.y += rowdef->GetActualHeight ();
			} else {
				child_final.height += rowdef->GetActualHeight ();
				
				min_size.height += rowdef->GetMinHeight ();
				max_size.height += rowdef->GetMaxHeight ();
			}
		}

		for (int c = 0; c < col + colspan; c++) {
			ColumnDefinition *coldef = columns->GetValueAt (c)->AsColumnDefinition ();

			if (c < col) {
				child_final.x += coldef->GetActualWidth ();
			} else {
				child_final.width += coldef->GetActualWidth ();

				min_size.width += coldef->GetMinWidth ();
				max_size.width += coldef->GetMaxWidth ();
			}
		}

		child->Arrange (child_final);
		
		/*
		  Size arranged = child.GetRenderSize ();
		arranged = arranged.Max (min_size);
		arranged = arranged.Min (max_size);
		*/
	}

	return finalSize;
}

//
// ColumnDefinitionCollection
//

ColumnDefinitionCollection::ColumnDefinitionCollection ()
{
	SetObjectType (Type::COLUMNDEFINITION_COLLECTION);
}

ColumnDefinitionCollection::~ColumnDefinitionCollection ()
{
}


//
// ColumnDefinition
//

ColumnDefinition::ColumnDefinition ()
  : actual (0.0)
{
	SetObjectType (Type::COLUMNDEFINITION);
}

ColumnDefinition::~ColumnDefinition ()
{
}

//
// RowDefinitionCollection
//

RowDefinitionCollection::RowDefinitionCollection ()
{
	SetObjectType (Type::ROWDEFINITION_COLLECTION);
}

RowDefinitionCollection::~RowDefinitionCollection ()
{
}

//
// RowDefinition
//

RowDefinition::RowDefinition ()
  : actual (0.0)
{
	SetObjectType (Type::ROWDEFINITION);
}

RowDefinition::~RowDefinition ()
{
}
