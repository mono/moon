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

	double* row_heights = new double[row_count == 0 ? 1 : row_count];
	double* column_widths = new double[col_count == 0 ? 1 : col_count];

	row_heights[0] = 0.0;
	column_widths[0] = 0.0;

	for (int i = 0; i < row_count; i ++) {
		RowDefinition *rowdef = rows->GetValueAt (i)->AsRowDefinition ();
		GridLength* height = rowdef->GetHeight();
		row_heights[i] = 0.0;

		if (height->type == GridUnitTypePixel)
			row_heights[i] = height->val;
	}

	for (int i = 0; i < col_count; i ++) {
		ColumnDefinition *coldef = columns->GetValueAt (i)->AsColumnDefinition ();
		GridLength* width = coldef->GetWidth();
		column_widths[i] = 0.0;

		if (width->type == GridUnitTypePixel)
			column_widths[i] = width->val;

	}

	UIElementCollection *children = GetChildren ();
	for (int i = 0; i < children->GetCount(); i ++) {
		UIElement *child = children->GetValueAt(i)->AsUIElement ();
		gint32 col, row;
		gint32 colspan, rowspan;

		col = Grid::GetColumn (child);
		row = Grid::GetRow (child);
		colspan = Grid::GetColumnSpan (child);
		rowspan = Grid::GetRowSpan (child);

		Size child_size = Size (0,0);
		Size min_size = Size (0,0);
		Size max_size = Size (0,0); 
		
		for (int r = row; (r < row + rowspan) && (r < row_count); r++) {
			RowDefinition *rowdef = rows->GetValueAt (r)->AsRowDefinition ();

			if (rowdef->GetHeight ()->type == GridUnitTypePixel)
			        child_size.height += row_heights [r];
			else
				child_size.height += INFINITY;

			min_size.height += rowdef->GetMinHeight ();
			max_size.height += rowdef->GetMaxHeight ();
		}

		for (int c = col; (c < col + colspan) && (c < col_count); c++) {
			ColumnDefinition *coldef = columns->GetValueAt (c)->AsColumnDefinition ();

			if (coldef->GetWidth ()->type == GridUnitTypePixel)
				child_size.width += column_widths [c];
			else
				child_size.width += INFINITY;

			min_size.width += coldef->GetMinWidth ();
			max_size.width += coldef->GetMaxWidth ();
		}
		
		
		child_size = child_size.Min (max_size);
		child_size = child_size.Max (min_size);

		child->Measure (child_size);
		child_size = child->GetDesiredSize();
		
		child_size = child_size.Min (max_size);
		child_size = child_size.Max (min_size);

		if (col_count) {
			double remaining_width = child_size.width;

			for (int c = col; (c < col + colspan) && (c < col_count); c++){
				ColumnDefinition *coldef = columns->GetValueAt (c)->AsColumnDefinition ();
				if (!coldef)
					break; // XXX what to do if col + colspan is more than the number of columns?

				if (coldef->GetWidth ()->type != GridUnitTypePixel)
					column_widths[col] = MAX(column_widths[col], remaining_width);
			}
		}

		if (row_count) {
			double remaining_height = child_size.height;

			for (int r = row; (r < row + rowspan) && (r < row_count); r++){
				RowDefinition *rowdef = rows->GetValueAt (r)->AsRowDefinition ();
				if (!rowdef)
					break; // XXX what to do if row + rowspan is more than the number of rows?

				if (rowdef->GetHeight ()->type != GridUnitTypePixel)
					row_heights[col] = MAX(row_heights[col], remaining_height);
			}
		}
	}

	Size grid_size;
	for (int r = 0; r < row_count; r ++) {
		grid_size.height += row_heights[r];
	}

	for (int c = 0; c < col_count; c ++) {
		grid_size.width += column_widths[c];
	}

	results = results.Max (grid_size);

	//printf ("results = %g %g\n", results.width, results.height);

	delete [] row_heights;
	delete [] column_widths;

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

	double* row_heights = new double[row_count == 0 ? 1 : row_count];
	double* column_widths = new double[col_count == 0 ? 1 : col_count];

	Size remaining = finalSize;

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		gint32 col = Grid::GetColumn (child);
		gint32 row = Grid::GetRow (child);
		//gint32 colspan = Grid::GetColumnSpan (child);
		//gint32 rowspan = Grid::GetRowSpan (child);
		
		Size desired = child->GetDesiredSize ();
		if (col < col_count) {
			ColumnDefinition *coldef = columns->GetValueAt (col)->AsColumnDefinition ();
			GridLength *width = coldef->GetWidth ();

			if (width->type == GridUnitTypeAuto)
				column_widths[col] = MAX (column_widths[col], desired.width);
			if (width->type == GridUnitTypeStar)
				g_warning ("a star is a star");

		}

		if (row < row_count) {
			RowDefinition *rowdef = rows->GetValueAt (row)->AsRowDefinition ();
			GridLength *height = rowdef->GetHeight ();
			
			if (height->type == GridUnitTypeAuto)
				row_heights[row] = MAX (row_heights[row], desired.height);
			if (height->type == GridUnitTypeStar)
				g_warning ("a star is a star");
		}
	}

	double row_stars = 0.0;
	for (int i = 0; i < row_count; i ++) {
		RowDefinition *rowdef = rows->GetValueAt (i)->AsRowDefinition ();
		GridLength* height = rowdef->GetHeight();
		row_heights[i] = 0.0;

		if (height->type != GridUnitTypeStar) {
			row_heights[i] = height->val;
			remaining.height -= height->val;
		} else {
			row_stars += height->val;
		}
	}

	double col_stars = 0.0;
	for (int i = 0; i < col_count; i ++) {
		ColumnDefinition *coldef = columns->GetValueAt (i)->AsColumnDefinition ();
		GridLength* width = coldef->GetWidth();
		column_widths[i] = 0.0;

		if (width->type != GridUnitTypeStar) {
			column_widths[i] = width->val;
			remaining.width -= width->val;
		} else {
			col_stars += width->val;
		}
	}

	for (int i = 0; i < row_count; i ++) {
		RowDefinition *rowdef = rows->GetValueAt (i)->AsRowDefinition ();
		GridLength* height = rowdef->GetHeight();
		row_heights[i] = 0.0;

		if (height->type == GridUnitTypeStar) {
			row_heights[i] = remaining.height * height->val / row_stars;
		}
	}

	for (int i = 0; i < col_count; i ++) {
		ColumnDefinition *coldef = columns->GetValueAt (i)->AsColumnDefinition ();
		GridLength* width = coldef->GetWidth();
		column_widths[i] = 0.0;

		if (width->type == GridUnitTypeStar) {
			column_widths[i] = remaining.width * width->val / col_stars;
		} 
	}

	walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		gint32 col = Grid::GetColumn (child);
		gint32 row = Grid::GetRow (child);
		gint32 colspan = Grid::GetColumnSpan (child);
		gint32 rowspan = Grid::GetRowSpan (child);

		Rect child_final = Rect (0, 0, finalSize.width, finalSize.height);
		Size min_size;
		Size max_size;

		for (int r = 0; r < row && r < row_count; r++)
			child_final.y += row_heights [r];

		for (int r = row; (r < row + rowspan) && (r < row_count); r++) {
			RowDefinition *rowdef = rows->GetValueAt (r)->AsRowDefinition ();

			child_final.height += row_heights [r];
			
			min_size.height += rowdef->GetMinHeight ();
			max_size.height += rowdef->GetMaxHeight ();
		}


		for (int c = 0; c < col && c < col_count; c++)
			child_final.x += column_widths[c];

		for (int c = col; (c < col + colspan) && (c < col_count); c++) {
			ColumnDefinition *coldef = columns->GetValueAt (c)->AsColumnDefinition ();

			child_final.width += column_widths [c];

			min_size.width += coldef->GetMinWidth ();
			max_size.width += coldef->GetMaxWidth ();
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
