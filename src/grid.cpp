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

		InvalidateMeasure ();
	}
}

Size
Grid::MeasureOverride (Size availableSize)
{
	Size results = Size (GetWidth (), GetHeight ());

	// if our width is not set, or is smaller than our configured MinWidth,
	// bump it up to the minimum.
	results = results.Max (GetMinWidth (), GetMinHeight ());

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

		row_heights[i] = MAX (row_heights[i], rowdef->GetMinHeight());
		row_heights[i] = MIN (row_heights[i], rowdef->GetMaxHeight());
	}

	for (int i = 0; i < col_count; i ++) {
		ColumnDefinition *coldef = columns->GetValueAt (i)->AsColumnDefinition ();
		GridLength* width = coldef->GetWidth();
		column_widths[i] = 0.0;
		if (width->type == GridUnitTypePixel)
			column_widths[i] = width->val;

		column_widths[i] = MAX (column_widths[i], coldef->GetMinWidth());
		column_widths[i] = MIN (column_widths[i], coldef->GetMaxWidth());
		printf ("column_widths[%d] specified as %g\n", i, column_widths[i]);
	}

	UIElementCollection *children = GetChildren ();
	for (int i = 0; i < children->GetCount(); i ++) {
		UIElement *child = children->GetValueAt(i)->AsUIElement ();
		Size child_size;

		if (child->Is(Type::FRAMEWORKELEMENT)) {
			FrameworkElement *fe = (FrameworkElement*)child;
			child_size = Size (fe->GetWidth(), fe->GetHeight());
			child->Measure (child_size);
		}
		else {
			child->Measure (Size (INFINITY, INFINITY));
			child_size = child->GetDesiredSize();
		}

		gint32 col, row;
		gint32 colspan, rowspan;

		col = Grid::GetColumn (child);
		row = Grid::GetRow (child);
		colspan = Grid::GetColumnSpan (child);
		rowspan = Grid::GetRowSpan (child);

		if (col_count) {
			double remaining_width = child_size.width;

			printf ("child_size.width = %g\n", child_size.width);

			for (int c = col; c < col + colspan; c++){
			  printf ("c = %d\n", c);
				ColumnDefinition *coldef = columns->GetValueAt (c)->AsColumnDefinition ();
				if (!coldef)
					break; // XXX what to do if col + colspan is more than the number of columns?
				GridLength* width = coldef->GetWidth();
				if (width->type != GridUnitTypePixel) {
				  printf ("column_widths[%d] = %g before, %g after\n",
					  col,
					  column_widths[col],
					  MAX(column_widths[col], remaining_width));
					column_widths[col] = MAX(column_widths[col], remaining_width);
				}
			}
		}

		if (row_count) {
			double remaining_height = child_size.height;

			for (int r = row; r < row + rowspan; r++){
				RowDefinition *rowdef = rows->GetValueAt (r)->AsRowDefinition ();
				if (!rowdef)
					break; // XXX what to do if row + rowspan is more than the number of rows?
				GridLength* height = rowdef->GetHeight();
				if (height->type != GridUnitTypePixel) {
					row_heights[col] = MAX(row_heights[col], remaining_height);
				}
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

	results = results.GrowBy (GetMargin());

	printf ("results = %g %g\n", results.width, results.height);

	// now choose whichever is smaller, our chosen size or the availableSize.
	return results.Min (availableSize);
}
