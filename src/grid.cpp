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
		InvalidateMeasure ();
	} else {
		Panel::OnCollectionChanged (col, args);
	}
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

			// SL invalidates the measure on the child when these properties change.
			// Tested by MeasureAutoRows2 and ChangingGridPropertiesInvalidates
			((UIElement *) obj)->InvalidateMeasure ();
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
	Size totalSize = availableSize;

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
		if (height->type == GridUnitTypeStar) {
			row_matrix [i][i].stars = height->val;
			total_stars.height += height->val;
		}
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
		if (width->type == GridUnitTypeStar) {
			col_matrix [i][i].stars = width->val;
			total_stars.width += width->val;
		}
	}

	List sizes;
	GridNode *node;
	GridNode *separator = new GridNode (NULL, 0, 0, 0);
	sizes.Append (separator);
	
	// Pre-process the grid children so that we know what types of elements we have so
	// we can apply our special measuring rules.
	GridWalker grid_walker (this, row_matrix, row_matrix_dim, col_matrix, col_matrix_dim);
	for (int i = 0; i < 6; i++) {
		// These bools tell us which grid element type we should be measuring. i.e.
		// 'star/auto' means we should measure elements with a star row and auto col
		bool auto_auto = i == 0;
		bool star_auto = i == 1;
		bool auto_star = i == 2;
		bool star_auto_again = i == 3;
		bool non_star = i == 4;
		bool remaining_star = i == 5;
		
		VisualTreeWalker walker = VisualTreeWalker (this);
		while (UIElement *child = walker.Step ()) {
			gint32 col, row;
			gint32 colspan, rowspan;
			Size child_size = Size (0,0);
			bool star_col = false;
			bool star_row = false;
			bool auto_col = false;
			bool auto_row = false;

			col = MIN (Grid::GetColumn (child), col_count - 1);
			row = MIN (Grid::GetRow (child), row_count - 1);
			colspan = MIN (Grid::GetColumnSpan (child), col_count - col);
			rowspan = MIN (Grid::GetRowSpan (child), row_count - row);

			for (int r = row; r < row + rowspan; r++) {
				star_row |= row_matrix [r][r].type == GridUnitTypeStar;
				auto_row |= row_matrix [r][r].type == GridUnitTypeAuto;
			}
			for (int c = col; c < col + colspan; c++) {
				star_col |= col_matrix [c][c].type == GridUnitTypeStar;
				auto_col |= col_matrix [c][c].type == GridUnitTypeAuto;
			}

			// This series of if statements checks whether or not we should measure
			// the current element and also if we need to override the sizes
			// passed to the Measure call. 
			
			// If the element has Auto rows and Auto columns and does not span Star
			// rows/cols it should only be measured in the auto_auto phase.
			// There are similar rules governing auto/star and star/auto elements.
			// NOTE: star/auto elements are measured twice. The first time with
			// an override for height, the second time without it.
			if (auto_row && auto_col && !star_row && !star_col) {
				if (!auto_auto)
					continue;
				child_size.width = INFINITY;
				child_size.height = INFINITY;
			}
			else if (star_row && auto_col && !star_col) {
				if (!(star_auto || star_auto_again))
					continue;

				if (star_auto && grid_walker.HasAutoStar ())
					child_size.height = INFINITY;
				child_size.width = INFINITY;
			} else if (auto_row && star_col && !star_row) {
				if (!auto_star)
					continue;
				
				child_size.height = INFINITY;
			} else if ((auto_row || auto_col) && !(star_row || star_col)) {
				if (!non_star)
					continue;
				if (auto_row)
					child_size.height = INFINITY;
				if (auto_col)
					child_size.width = INFINITY;
			} else if (!(star_row || star_col)) {
				if (!non_star)
					continue;
			} else {
				if (!remaining_star)
					continue;
			}

			for (int r = row; r < row + rowspan; r++) {
				if (row_matrix [r][r].type == GridUnitTypeStar) {
					double v = availableSize.height * row_matrix [r][r].stars / total_stars.height;
					child_size.height += Grid::Clamp (v, row_matrix [r][r].min, row_matrix [r][r].max);
				} else {
					child_size.height += row_matrix [r][r].size;
				}
			}
			for (int c = col; c < col + colspan; c++) {
				if (col_matrix [c][c].type == GridUnitTypeStar) {
					double v = availableSize.width * col_matrix [c][c].stars / total_stars.width;
					child_size.width += Grid::Clamp (v, col_matrix [c][c].min, col_matrix [c][c].max);
				} else {
					child_size.width += col_matrix [c][c].size;
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
			if (!star_auto) {
				node = new GridNode (row_matrix, row + rowspan - 1, row, desired.height);
				sizes.InsertBefore (node, node->row == node->col ? separator->next : separator);
			}
			node = new GridNode (col_matrix, col + colspan  - 1, col, desired.width);
			sizes.InsertBefore (node, node->row == node->col ? separator->next : separator);
		}
		
		sizes.Unlink (separator);

		while (GridNode *node= (GridNode *) sizes.Last ()) {
			node->matrix [node->row][node->col].size = MAX (node->matrix [node->row][node->col].size, node->size);
			AllocateGridSegments (row_count, col_count);
			sizes.Remove (node);
		}

		// Calculate how much unused space we have so the next round of
		// measurements uses the right sizes for Star segments
		availableSize = totalSize;
		for (int r = 0; r < row_matrix_dim; r++)
			if (row_matrix [r][r].type != GridUnitTypeStar)
				availableSize.height = MAX (availableSize.height - row_matrix [r][r].size, 0);

		for (int c = 0; c < col_matrix_dim; c++)
			if (col_matrix [c][c].type != GridUnitTypeStar)
				availableSize.width = MAX (availableSize.width - col_matrix [c][c].size, 0);

		sizes.Append (separator);
	}
	
	// Once we have measured and distributed all sizes, we have to store
	// the results. Every time we want to expand the rows/cols, this will
	// be used as the baseline.
	SaveMeasureResults ();
	
	sizes.Remove (separator);

	// We have to calulate the desired grid size before expanding
	// star segments to consume available space.
	Size grid_size = Size (0, 0);
	for (int c = 0; c < col_count; c ++)
		grid_size.width += col_matrix [c][c].size;
	for (int r = 0; r < row_count; r ++)
		grid_size.height += row_matrix [r][r].size;


	// This is where we do the final expansion of star rows. Right now the value
	// of 'size' for star segments in row_matrix and col_matrix contains the
	// size that the segments would naturally take up.

	bool hasChildren = GetChildren ()->GetCount () > 0;
	if (totalSize.width != INFINITY && hasChildren)
		ExpandStarCols (totalSize);
	if (totalSize.height != INFINITY && hasChildren)
		ExpandStarRows (totalSize);

	if (free_col) {
		columns->unref ();
	}
	if (free_row) {
		rows->unref ();
	}
	// now choose whichever is smaller, our chosen size or the availableSize.
	return grid_size;
}

void
Grid::ExpandStarRows (Size availableSize)
{
	RowDefinitionCollection *rows = GetRowDefinitions ();

	// When expanding star rows, we need to zero out their height before
	// calling AssignSize. AssignSize takes care of distributing the 
	// available size when there are Mins and Maxs applied.
	for (int i = 0; i < row_matrix_dim; i++) {
		if (row_matrix [i][i].type == GridUnitTypeStar)
			row_matrix [i][i].size = 0.0;
		else
			availableSize.height = MAX (availableSize.height - row_matrix [i][i].size, 0);
	}

	AssignSize (row_matrix, 0, row_matrix_dim - 1, &availableSize.height, GridUnitTypeStar);
	if (rows->GetCount () > 0) {
		for (int i = 0; i < row_matrix_dim; i++)
			if (row_matrix [i][i].type == GridUnitTypeStar)
				rows->GetValueAt (i)->AsRowDefinition ()->SetActualHeight (row_matrix [i][i].size);
	}
}

void
Grid::ExpandStarCols (Size availableSize)
{
	ColumnDefinitionCollection *columns = GetColumnDefinitions ();

	for (int i = 0; i < col_matrix_dim; i++) {
		if (col_matrix [i][i].type == GridUnitTypeStar)
			col_matrix [i][i].size = 0;
		else
			availableSize.width = MAX (availableSize.width - col_matrix [i][i].size, 0);
	}

	AssignSize (col_matrix, 0, col_matrix_dim - 1, &availableSize.width, GridUnitTypeStar);
		
	if (columns->GetCount () > 0) {
		for (int i = 0; i < col_matrix_dim; i++)
			if (col_matrix [i][i].type == GridUnitTypeStar)
				columns->GetValueAt (i)->AsColumnDefinition ()->SetActualWidth (col_matrix [i][i].size);
	}
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
					if (spans_star) {
						AssignSize (matrix, col, row, &additional, GridUnitTypeStar);
					} else {
						AssignSize (matrix, col, row, &additional, GridUnitTypePixel);
						AssignSize (matrix, col, row, &additional, GridUnitTypeAuto);
					}
				}
			}
		}
	}
}

void
Grid::AssignSize (Segment **matrix, int start, int end, double *size, GridUnitType type)
{
	double count = 0;
	bool assigned;
	
	// Count how many segments are of the correct type. If we're measuring Star rows/cols
	// we need to count the number of stars instead.
	for (int i = start; i <= end; i++)
		if (matrix [i][i].type == type && matrix [i][i].size < matrix [i][i].max)
			count += type == GridUnitTypeStar ? matrix [i][i].stars : 1;
	
	do {
		assigned = false;
		double contribution = *size / count;
		
		for (int i = start; i <= end; i++) {
			if (!(matrix [i][i].type == type && matrix [i][i].size < matrix [i][i].max))
				continue;
			double newsize = matrix [i][i].size;
			newsize += contribution * (type == GridUnitTypeStar ? matrix [i][i].stars : 1);
			newsize = MIN (newsize, matrix [i][i].max);
			assigned |= newsize > matrix [i][i].size;
			*size -= newsize - matrix [i][i].size;
			matrix [i][i].size = newsize;
		}
	} while (assigned);
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
		
		cairo_save (cr);
		RenderLayoutClip (cr);
		cairo_set_line_width(cr, 1.0);
		// Initially render a blue color
		cairo_set_dash (cr, &dash, 1, offset);
		cairo_set_source_rgb (cr, 0.4, 0.4, 1.0);
		cairo_new_path (cr);

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
		cairo_restore (cr);
	}		

	// Chain up in front_to_back mode since we've alread rendered content
	UIElement::PostRender (cr, region, true);
}

Size
Grid::ArrangeOverride (Size finalSize)
{
	ColumnDefinitionCollection *columns = GetColumnDefinitions ();
	RowDefinitionCollection *rows = GetRowDefinitions ();

	int col_count = columns->GetCount ();
	int row_count = rows->GetCount ();

	RestoreMeasureResults ();

	Size total_consumed = Size (0, 0);
	for (int c = 0; c < col_matrix_dim; c++)
		total_consumed.width += col_matrix [c][c].size;
	for (int r = 0; r < row_matrix_dim; r++)
		total_consumed.height += row_matrix [r][r].size;

	if (total_consumed.width != finalSize.width)
		ExpandStarCols (finalSize);
	if (total_consumed.height != finalSize.height)
		ExpandStarRows (finalSize);

	for (int c = 0; c < col_count; c++)
		columns->GetValueAt (c)->AsColumnDefinition ()->SetActualWidth (col_matrix [c][c].size);
	for (int r = 0; r < row_count; r++)
		rows->GetValueAt (r)->AsRowDefinition ()->SetActualHeight (row_matrix [r][r].size);

	VisualTreeWalker walker = VisualTreeWalker (this);
	while (UIElement *child = walker.Step ()) {
		gint32 col = MIN (Grid::GetColumn (child), col_matrix_dim - 1);
		gint32 row = MIN (Grid::GetRow (child), row_matrix_dim - 1);
		gint32 colspan = MIN (Grid::GetColumnSpan (child), col_matrix_dim - col);
		gint32 rowspan = MIN (Grid::GetRowSpan (child), row_matrix_dim - row);

		Rect child_final = Rect (0, 0, 0, 0);
		for (int c = 0; c < col; c++)
			child_final.x += col_matrix [c][c].size;
		for (int c = col; c < col + colspan; c++)
			child_final.width += col_matrix [c][c].size;

		for (int r = 0; r < row; r++)
			child_final.y += row_matrix [r][r].size;
		for (int r = row; r < row + rowspan; r++)
			child_final.height += row_matrix [r][r].size;

		child->Arrange (child_final);
	}

	return finalSize;
}

void
Grid::SaveMeasureResults ()
{
	for (int i = 0; i < row_matrix_dim; i++)
		for (int j = 0; j < row_matrix_dim; j++)
			row_matrix [i][j].original_size = row_matrix [i][j].size;

	for (int i = 0; i < col_matrix_dim; i++)
		for (int j = 0; j < col_matrix_dim; j++)
			col_matrix [i][j].original_size = col_matrix [i][j].size;
}
void
Grid::RestoreMeasureResults ()
{
	for (int i = 0; i < row_matrix_dim; i++)
		for (int j = 0; j < row_matrix_dim; j++)
			row_matrix [i][j].size = row_matrix [i][j].original_size;

	for (int i = 0; i < col_matrix_dim; i++)
		for (int j = 0; j < col_matrix_dim; j++)
			col_matrix [i][j].size = col_matrix [i][j].original_size;
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
	this->stars = 0;
	this->type = type;
	
	this->size = Grid::Clamp (size, min, max);
	this->original_size = this->size;
}

GridWalker::GridWalker (Grid *grid, Segment **row_matrix, int row_count, Segment **col_matrix, int col_count)
{
	has_auto_auto = false;
	has_star_auto = false;
	has_auto_star = false;

	VisualTreeWalker walker (grid);
	while (UIElement *child = walker.Step ()) {
		bool star_col = false;
		bool star_row = false;
		bool auto_col = false;
		bool auto_row = false;
		
		gint32 col = MIN (Grid::GetColumn (child), col_count - 1);
		gint32 row = MIN (Grid::GetRow (child), row_count - 1);
		gint32 colspan = MIN (Grid::GetColumnSpan (child), col_count - col);
		gint32 rowspan = MIN (Grid::GetRowSpan (child), row_count - row);

		for (int r = row; r < row + rowspan; r++) {
			star_row |= row_matrix [r][r].type == GridUnitTypeStar;
			auto_row |= row_matrix [r][r].type == GridUnitTypeAuto;
		}
		for (int c = col; c < col + colspan; c++) {
			star_col |= col_matrix [c][c].type == GridUnitTypeStar;
			auto_col |= col_matrix [c][c].type == GridUnitTypeAuto;
		}

		has_auto_auto |= auto_row && auto_col && !star_row && !star_col;
		has_star_auto |= star_row && auto_col;
		has_auto_star |= auto_row && star_col;
	}
}
