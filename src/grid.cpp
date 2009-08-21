/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
	row_matrix = NULL;
	col_matrix = NULL;
}

Grid::~Grid ()
{
	DestroyMatrices ();
}

double
Grid::Clamp (double val, double min, double max)
{
	if (val < min)
		return min;
	else if (val > max)
		return max;
	return val;
}

void
Grid::OnPropertyChanged (PropertyChangedEventArgs *args, MoonError *error)
{
	if (args->GetProperty ()->GetOwnerType() != Type::GRID) {
		Panel::OnPropertyChanged (args, error);
		return;
	}

	if (args->GetId () == Grid::ShowGridLinesProperty){
		Invalidate ();
	}

	InvalidateMeasure ();

	NotifyListenersOfPropertyChange (args, error);
}

void
Grid::OnCollectionChanged (Collection *col, CollectionChangedEventArgs *args)
{
	if (col == GetColumnDefinitions () ||
	    col == GetRowDefinitions ()) {
		//InvalidateMeasure ();
	} else {
		Panel::OnCollectionChanged (col, args);
	}
	
	InvalidateMeasure ();
}

void
Grid::OnCollectionItemChanged (Collection *col, DependencyObject *obj, PropertyChangedEventArgs *args)
{
	if (col == GetChildren ()) {
		if (args->GetId () == Grid::ColumnProperty
		    || args->GetId () == Grid::RowProperty
		    || args->GetId () == Grid::ColumnSpanProperty
		    || args->GetId () == Grid::RowSpanProperty) {
			InvalidateMeasure ();
			return;
		}
	} else if (col == GetColumnDefinitions () || col == GetRowDefinitions ()) {
		if (args->GetId() != ColumnDefinition::ActualWidthProperty 
		    && args->GetId() != RowDefinition::ActualHeightProperty) {
			InvalidateMeasure ();
		}
		return;
	}
	
	Panel::OnCollectionItemChanged (col, obj, args);
}

Size
Grid::MeasureOverride (Size availableSize)
{
	Size results = availableSize;

	ColumnDefinitionCollection *columns = GetColumnDefinitions ();
	RowDefinitionCollection *rows = GetRowDefinitions ();
	bool free_col = false;
	bool free_row = false;

	int col_count = columns->GetCount ();
	int row_count = rows->GetCount ();
	Size total_stars = Size (0,0);

	if (col_count == 0) {
		columns = new ColumnDefinitionCollection ();
		ColumnDefinition *coldef = new ColumnDefinition ();
		columns->Add (coldef);
		coldef->unref ();
		free_col = true;
		col_count = 1;
	}

	if (row_count == 0) {
		rows = new RowDefinitionCollection ();
		RowDefinition *rowdef = new RowDefinition ();
		rows->Add (rowdef);
		rowdef->unref ();
		free_row = true;
		row_count = 1;
	}

	CreateMatrices (row_count, col_count);

	for (int i = 0; i < row_count; i ++) {
		RowDefinition *rowdef = rows->GetValueAt (i)->AsRowDefinition ();
		GridLength* height = rowdef->GetHeight();

		rowdef->SetActualHeight (INFINITY);
		row_matrix [i][i] = Segment (0.0, rowdef->GetMinHeight (), rowdef->GetMaxHeight (), height->type);

		if (height->type == GridUnitTypePixel) {
			row_matrix [i][i].size = Grid::Clamp (height->val, row_matrix [i][i].min, row_matrix [i][i].max);
			rowdef->SetActualHeight (row_matrix [i][i].size);
		}
		if (height->type == GridUnitTypeStar)
			total_stars.height += height->val;
	}

	for (int i = 0; i < col_count; i ++) {
		ColumnDefinition *coldef = columns->GetValueAt (i)->AsColumnDefinition ();
		GridLength *width = coldef->GetWidth ();

		coldef->SetActualWidth (INFINITY);
		col_matrix [i][i] = Segment (0.0, coldef->GetMinWidth (), coldef->GetMaxWidth (), width->type);

		if (width->type == GridUnitTypePixel) {
			col_matrix [i][i].size = Grid::Clamp (width->val, col_matrix [i][i].min, col_matrix [i][i].max);
			coldef->SetActualWidth (col_matrix [i][i].size);
		}
		if (width->type == GridUnitTypeStar)
			total_stars.width += width->val;
	}

	List sizes;
	GridNode *node;
	GridNode *separator = new GridNode (NULL, 0, 0, 0);
	sizes.Append (separator);

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		if (child->GetVisibility () != VisibilityVisible)
			continue;

		gint32 col, row;
		gint32 colspan, rowspan;

		col = MIN (Grid::GetColumn (child), col_count - 1);
		row = MIN (Grid::GetRow (child), row_count - 1);
		colspan = MIN (Grid::GetColumnSpan (child), col_count - col);
		rowspan = MIN (Grid::GetRowSpan (child), row_count - row);
		Size pixels = Size ();
		Size stars = Size ();
		Size automatic = Size ();


		Size child_size = Size (0,0);
		double value = 0.0;
		for (int r = row; r < row + rowspan; r++) {
			RowDefinition *rowdef = rows->GetValueAt (r)->AsRowDefinition ();
			GridLength* height = rowdef->GetHeight();

			switch (height->type) {
			case GridUnitTypePixel:
				value = Grid::Clamp (height->val, rowdef->GetMinHeight (), rowdef->GetMaxHeight ());
				child_size.height += value;
				pixels.height += height->val;
				break;
			case GridUnitTypeAuto:
				automatic.height += 1;
				child_size.height = INFINITY;
				break;
			case GridUnitTypeStar:
				stars.height += height->val;
				child_size.height += availableSize.height * height->val / total_stars.height;
				break;
			}
		}

		for (int c = col; c < col + colspan; c++) {
			ColumnDefinition *coldef = columns->GetValueAt (c)->AsColumnDefinition ();
			GridLength* width = coldef->GetWidth();
			
			switch (width->type) {
			case GridUnitTypePixel:
				value = Grid::Clamp (width->val, coldef->GetMinWidth (), coldef->GetMaxWidth ());
				child_size.width += value;
				pixels.width += width->val;
				break;
			case GridUnitTypeAuto:
				automatic.width += 1;
				child_size.width = INFINITY;
				break;
			case GridUnitTypeStar:
				stars.width += width->val;
				child_size.width += availableSize.width * width->val / total_stars.width;
				break;
			}
		}

		child->Measure (child_size);
		Size desired = child->GetDesiredSize();

		// Elements distribute their height based on two rules:
		// 1) Elements with rowspan/colspan == 1 distribute their height first
		// 2) Everything else distributes in a LIFO manner.
		// As such, add all UIElements with rowspan/colspan == 1 after the separator in
		// the list and everything else before it. Then to process, just keep popping
		// elements off the end of the list.
		node = new GridNode (row_matrix, row + rowspan - 1, row, desired.height);
		sizes.InsertBefore (node, node->row == node->col ? separator->next : separator);
		
		node = new GridNode (col_matrix, col + colspan  - 1, col, desired.width);
		sizes.InsertBefore (node, node->row == node->col ? separator->next : separator);
	}
	
	sizes.Remove (separator);

	while (GridNode *node= (GridNode *) sizes.Last ()) {
		node->matrix [node->row][node->col].size = MAX (node->matrix [node->row][node->col].size, node->size);
		AllocateGridSegments (row_count, col_count);
		sizes.Remove (node);
	}

	Size grid_size;
	for (int r = 0; r < row_count; r ++)
		grid_size.height += row_matrix [r][r].size;

	for (int c = 0; c < col_count; c ++)
		grid_size.width += col_matrix [c][c].size;

	grid_size = grid_size.Max (GetWidth (), GetHeight ());
	results = results.Min (grid_size);

	if (free_col) {
		columns->unref ();
	}

	if (free_row) {
		rows->unref ();
	}
	// now choose whichever is smaller, our chosen size or the availableSize.
	return results;
}

void
Grid::AllocateGridSegments (int row_count, int col_count)
{
	// First allocate the heights of the RowDefinitions, then allocate
	// the widths of the ColumnDefinitions.
	for (int i = 0; i < 2; i ++) {
		Segment **matrix = i == 0 ? row_matrix : col_matrix;
		int count = i == 0 ? row_count : col_count;

		for (int row = count - 1; row >= 0; row--) {
			for (int col = row; col >= 0; col--) {
					bool spans_star = false;
					for (int j = row; j >= col; j --)
						spans_star |= matrix [j][j].type == GridUnitTypeStar;
			
				// This is the amount of pixels which must be available between the grid rows
				// at index 'col' and 'row'. i.e. if 'row' == 0 and 'col' == 2, there must
				// be at least 'matrix [row][col].size' pixels of height allocated between
				// all the rows in the range col -> row.
				double current = matrix [row][col].size;

				// Count how many pixels have already been allocated between the grid rows
				// in the range col -> row. The amount of pixels allocated to each grid row/column
				// is found on the diagonal of the matrix.
				double total_allocated = 0;
				for (int i = row; i >= col; i--)
					total_allocated += matrix [i][i].size;

				// If the size requirement has not been met, allocate the additional required
				// size between 'pixel' rows, then 'star' rows, finally 'auto' rows, until all
				// height has been assigned.
				if (total_allocated < current) {
					double additional = current - total_allocated;
					// Note that multiple passes may be required at each level depending on whether or not
					// the MaxHeight/MaxWidth value prevents the row/column from accepting the full contribution
					if (spans_star) {
						while (AssignSize (matrix, col, row, &additional, GridUnitTypeStar))  { }
					} else {
						while (AssignSize (matrix, col, row, &additional, GridUnitTypePixel)) { }
						while (AssignSize (matrix, col, row, &additional, GridUnitTypeAuto))  { }
					}
				}
			}
		}
	}
}

bool
Grid::AssignSize (Segment **matrix, int start, int end, double *size, GridUnitType type)
{
	bool assigned = false;
	int count = 0;
	double contribution = *size;
	
	for (int i = start; i <= end; i++) {
		if (matrix [i][i].type == type && matrix [i][i].size < matrix [i][i].max)
			count++;
	}
	
	contribution /= count;
	
	for (int i = start; i <= end; i++) {
		if (!(matrix [i][i].type == type && matrix [i][i].size < matrix [i][i].max))
			continue;
		double newsize = contribution + matrix [i][i].size;
		newsize = MIN (newsize, matrix [i][i].max);
		assigned |= newsize > matrix [i][i].size;
		*size -= newsize - matrix [i][i].size;
		matrix [i][i].size = newsize;
	}
	return assigned;
}

void
Grid::DestroyMatrices ()
{
	if (row_matrix != NULL) {
		for (int i = 0; i < row_matrix_dim; i++)
			delete [] row_matrix [i];
		delete [] row_matrix;
		row_matrix = NULL;
	}

	if (col_matrix != NULL) {
		for (int i = 0; i < col_matrix_dim; i++)
			delete [] col_matrix [i];
		delete [] col_matrix;
		col_matrix = NULL;
	}
}

void
Grid::CreateMatrices (int row_count, int col_count)
{
	DestroyMatrices ();

	row_matrix_dim = row_count;
	col_matrix_dim = col_count;

	row_matrix = new Segment *[row_count];
	for (int i = 0; i < row_count; i++) {
		row_matrix [i] = new Segment [row_count];
		for (int j = 0; j < row_count; j++)
			row_matrix [i][j] = Segment ();
	}

	col_matrix = new Segment *[col_count];
	for (int i = 0; i < col_count; i++) {
		col_matrix [i] = new Segment [col_count];
		for (int j = 0; j < col_count; j++)
			col_matrix [i][j] = Segment ();
	}
}

void
Grid::ComputeBounds ()
{
	Panel::ComputeBounds ();
	
	if (GetShowGridLines ()) {
		extents = Rect (0,0,GetActualWidth (),GetActualHeight ());
		bounds = IntersectBoundsWithClipPath (extents, false).Transform (&absolute_xform);
		bounds_with_children = bounds_with_children.Union (bounds);
	}
}
void
Grid::PostRender (cairo_t *cr, Region *region, bool front_to_back)
{
	// render our chidren if not in front to back mode
	if (!front_to_back) {
		VisualTreeWalker walker = VisualTreeWalker (this, ZForward);
		while (UIElement *child = walker.Step ())
			child->DoRender (cr, region);
	}
	
	if (GetShowGridLines ()) {
		double offset = 0;
		double dash = 4;
		ColumnDefinitionCollection *cols = GetColumnDefinitions ();
		RowDefinitionCollection *rows = GetRowDefinitions ();
		
		cairo_set_line_width(cr, 1.0);
		// Initially render a blue color
		cairo_set_dash (cr, &dash, 1, offset);
		cairo_set_source_rgb (cr, 0.4, 0.4, 1.0);
		
		// Draw gridlines between each pair of columns/rows
		for (int count = 0; count < 2; count++) {
			
			for (int i = 0, offset = 0; i < cols->GetCount () - 1; i++) {
				ColumnDefinition *def = cols->GetValueAt (i)->AsColumnDefinition ();
				offset += def->GetActualWidth ();
				cairo_move_to (cr, offset, 0);
				cairo_line_to (cr, offset, GetActualHeight ());
			}
			
			for (int i = 0, offset = 0; i < rows->GetCount () -1; i++) {
				RowDefinition *def = rows->GetValueAt (i)->AsRowDefinition ();
				offset += def->GetActualHeight ();
				cairo_move_to (cr, 0, offset);
				cairo_line_to (cr, GetActualWidth (), offset);
			}
			
			cairo_stroke (cr);
			
			// For the second pass render a yellow color in the gaps between the previous dashes
			cairo_set_dash (cr, &dash, 1, dash);
			cairo_set_source_rgb (cr, 1.0, 1.0, 0.3);
		}
	}		

	// Chain up in front_to_back mode since we've alread rendered content
	UIElement::PostRender (cr, region, true);
}

Size
Grid::ArrangeOverride (Size finalSize)
{
	ColumnDefinitionCollection *columns = GetColumnDefinitions ();
	RowDefinitionCollection *rows = GetRowDefinitions ();
	bool free_col = false;
	bool free_row = false;

	int col_count = columns->GetCount ();
	int row_count = rows->GetCount ();

	if (col_count == 0) {
		columns = new ColumnDefinitionCollection ();
		ColumnDefinition *coldef = new ColumnDefinition ();
		columns->Add (coldef);
		coldef->unref ();
		free_col = true;
		col_count = 1;
	}

	if (row_count == 0) {
		rows = new RowDefinitionCollection ();
		RowDefinition *rowdef = new RowDefinition ();
		rows->Add (rowdef);
		rowdef->unref ();
		free_row = true;
		row_count = 1;
	}

	for (int i = 0; i < row_count; i++)
		rows->GetValueAt (i)->AsRowDefinition ()->SetActualHeight (row_matrix [i][i].size);

	for (int i = 0; i < col_count; i++)
		columns->GetValueAt (i)->AsColumnDefinition ()->SetActualWidth (col_matrix [i][i].size);

	Size requested = Size ();
	Size star_size = Size ();
	double row_stars = 0.0;
	HorizontalAlignment horiz = !isnan (GetWidth ()) ? HorizontalAlignmentStretch : GetHorizontalAlignment ();
	VerticalAlignment vert = !isnan (GetHeight ()) ? VerticalAlignmentStretch : GetVerticalAlignment ();

	for (int i = 0; i < row_count; i ++) {
		RowDefinition *rowdef = rows->GetValueAt (i)->AsRowDefinition ();
		GridLength* height = rowdef->GetHeight();

		switch (height->type) {
		case GridUnitTypeStar:
			// Star columns distribute evenly
			requested.height += rowdef->GetActualHeight ();
			star_size.height += rowdef->GetActualHeight ();
			//if (vert == VerticalAlignmentStretch)
			//	rowdef->SetActualHeight (0.0);

			row_stars += height->val;
			break;
		case GridUnitTypePixel:
			requested.height += rowdef->GetActualHeight ();
			break;
		case GridUnitTypeAuto:
			requested.height += rowdef->GetActualHeight ();
			break;
		}
	}

	double col_stars = 0.0;
	for (int i = 0; i < col_count; i ++) {
		ColumnDefinition *coldef = columns->GetValueAt (i)->AsColumnDefinition ();
		GridLength* width = coldef->GetWidth();

		switch (width->type) {
		case GridUnitTypeStar:
			// Star columns distribute evenly
			requested.width += coldef->GetActualWidth ();
			star_size.width += coldef->GetActualWidth ();
			//if (horiz == HorizontalAlignmentStretch)
			//	coldef->SetActualWidth (0.0);

			col_stars += width->val;
			break;
		case GridUnitTypePixel:
			requested.width += coldef->GetActualWidth ();
			break;
		case GridUnitTypeAuto:
			requested.width += coldef->GetActualWidth ();
			break;
		}
	}

	Size remaining = Size (finalSize.width - requested.width, finalSize.height - requested.height);

	if (horiz != HorizontalAlignmentStretch)
		remaining.width = MIN (remaining.width, 0);

	if (vert != VerticalAlignmentStretch)
		remaining.height = MIN (remaining.height, 0);

	if (remaining.height != 0) {     
		remaining.height += star_size.height;

		for (int i = 0; i < row_count; i ++) {
			RowDefinition *rowdef = rows->GetValueAt (i)->AsRowDefinition ();
			GridLength* height = rowdef->GetHeight();
			
			if (height->type == GridUnitTypeStar)
				rowdef->SetActualHeight (MAX ((remaining.height * height->val / row_stars), 0));
		}
	}
	
	if (remaining.width != 0) {
		remaining.width += star_size.width;

		for (int i = 0; i < col_count; i ++) {
			ColumnDefinition *coldef = columns->GetValueAt (i)->AsColumnDefinition ();
			GridLength* width = coldef->GetWidth();
			
			if (width->type == GridUnitTypeStar)
				coldef->SetActualWidth (MAX ((remaining.width * width->val / col_stars), 0));
		}
       	}

	bool first = true;
	Size arranged = finalSize;
	
	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		if (child->GetVisibility () != VisibilityVisible)
			continue;

		gint32 col = MIN (Grid::GetColumn (child), col_count - 1);
		gint32 row = MIN (Grid::GetRow (child), row_count - 1);
		gint32 colspan = MIN (Grid::GetColumnSpan (child), col_count - col);
		gint32 rowspan = MIN (Grid::GetRowSpan (child), row_count - row);

		Rect child_final = Rect (0, 0, 0, 0);
		Size min_size;
		Size max_size;

		if (first) {
			arranged = Size ();
			first = false;
		}

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
		Size child_arranged = child->GetRenderSize ();

		if (horiz == HorizontalAlignmentStretch)
			arranged.width = MAX (child_final.x + child_final.width, finalSize.width);
		else 
			arranged.width = MAX (child_final.x + child_final.width, arranged.width);
		    
		if (vert == VerticalAlignmentStretch)
			arranged.height = MAX (child_final.y + child_final.height, finalSize.height);
		else
			arranged.height = MAX (child_final.y + child_final.height, arranged.height);
	}

	if (free_col)
		columns->unref ();

	if (free_row)
		rows->unref ();

	return arranged;
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

bool
ColumnDefinitionCollection::AddedToCollection (Value *value, MoonError *error)
{
	if (Contains (value)) {
		MoonError::FillIn (error, MoonError::ARGUMENT, "ColumnDefinition is already a member of this collection.");
		return false;
	}
	return DependencyObjectCollection::AddedToCollection (value, error);
}

//
// ColumnDefinition
//

ColumnDefinition::ColumnDefinition ()
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

bool
RowDefinitionCollection::AddedToCollection (Value *value, MoonError *error)
{
	if (Contains (value)) {
		MoonError::FillIn (error, MoonError::ARGUMENT, "RowDefinition is already a member of this collection.");
		return false;
	}
	return DependencyObjectCollection::AddedToCollection (value, error);
}

//
// RowDefinition
//

RowDefinition::RowDefinition ()
{
	SetObjectType (Type::ROWDEFINITION);
}

RowDefinition::~RowDefinition ()
{
}

Segment::Segment ()
{
	Init (0.0, 0.0, INFINITY, GridUnitTypePixel);
}

Segment::Segment (double size, double min, double max, GridUnitType type)
{
	Init (size, min, max, type);
}

void
Segment::Init (double size, double min, double max, GridUnitType type)
{
	this->max = max;
	this->min = min;
	this->type = type;
	
	this->size = Grid::Clamp (size, min, max);
}
