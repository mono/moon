// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Windows.Automation.Peers;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;

namespace System.Windows.Controls
{
    [SuppressMessage("Microsoft.Maintainability", "CA1506:AvoidExcessiveClassCoupling")]
    /// <summary>
    /// Displays data in a customizable grid.
    /// </summary>
    [TemplatePart(Name = DataGrid.DATAGRID_elementRowsPresenterName, Type = typeof(DataGridRowsPresenter))]
    [TemplatePart(Name = DataGrid.DATAGRID_elementColumnHeadersPresenterName, Type = typeof(DataGridColumnHeadersPresenter))]
    [TemplatePart(Name = DataGrid.DATAGRID_elementFrozenColumnScrollBarSpacerName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = DataGrid.DATAGRID_elementHorizontalScrollbarName, Type = typeof(ScrollBar))]
    [TemplatePart(Name = DataGrid.DATAGRID_elementVerticalScrollbarName, Type = typeof(ScrollBar))]
    public partial class DataGrid : Control
    {
        #region Constants

        private const string DATAGRID_elementRowsPresenterName = "RowsPresenter";
        private const string DATAGRID_elementColumnHeadersPresenterName = "ColumnHeadersPresenter";
        private const string DATAGRID_elementFrozenColumnScrollBarSpacerName = "FrozenColumnScrollBarSpacer";
        private const string DATAGRID_elementHorizontalScrollbarName = "HorizontalScrollbar";
        private const string DATAGRID_elementRowHeadersPresenterName = "RowHeadersPresenter";
        private const string DATAGRID_elementTopLeftCornerHeaderName = "TopLeftCornerHeader";
        private const string DATAGRID_elementTopRightCornerHeaderName = "TopRightCornerHeader";
        private const string DATAGRID_elementVerticalScrollbarName = "VerticalScrollbar";
        
        private const bool DATAGRID_defaultAutoGenerateColumns = true;
        internal const bool DATAGRID_defaultCanUserReorderColumns = true;
        internal const bool DATAGRID_defaultCanUserResizeColumns = true;
        internal const bool DATAGRID_defaultCanUserSortColumns = true;
        private const DataGridRowDetailsVisibilityMode DATAGRID_defaultRowDetailsVisibility = DataGridRowDetailsVisibilityMode.VisibleWhenSelected;
        private const DataGridSelectionMode DATAGRID_defaultSelectionMode = DataGridSelectionMode.Extended;

        private const double DATAGRID_horizontalGridLinesThickness = 1;
        private const double DATAGRID_minimumRowHeaderWidth = 4;
        private const double DATAGRID_minimumColumnHeaderHeight = 4;
        private const double DATAGRID_maxHeadersThickness = 32768;

        private const double DATAGRID_defaultRowHeight = 22;
        private const double DATAGRID_defaultMinColumnWidth = 20;
        private const double DATAGRID_defaultMaxColumnWidth = double.PositiveInfinity;

        #endregion Constants

        #region Data

        // DataGrid Template Parts
        private DataGridRowsPresenter _rowsPresenter;
        private DataGridColumnHeadersPresenter _columnHeadersPresenter;
        private ScrollBar _hScrollBar;
        private ScrollBar _vScrollBar;
        
        private List<object> _addedSelectedItems;
        private byte _autoGeneratingColumnOperationCount;
        private Popup _columnDropLocationIndicatorPopup;
        private Control _columnDropLocationIndicator;
        private DataGridCellCoordinates _currentCellCoordinates;
        private List<DataGridCell> _editingBoundCells;
        private int _editingElementGotFocusListeners;
        private int _editingElementLostFocusListeners; // Number of subscribers for the LostFocus event of the element of a edited cell in a column
        private int _editingColumnIndex;
        private DataGridRow _editingRow;
        private RoutedEventArgs _editingEventArgs;
        private Control _editingTemplateControl; // Control that has focus inside a template column
        private bool _focusEditingControl;
        private DataGridRow _focusedRow;
        private FrameworkElement _frozenColumnScrollBarSpacer;
        // the sum of the widths in pixels of the scrolling columns preceding 
        // the first displayed scrolling column
        private double _horizontalOffset;
        private bool _ignoreNextScrollBarsLayout;
        private byte _horizontalScrollChangesIgnored;
        // Nth row of rows 0..N that make up the RowHeightEstimate
        private int _lastEstimatedRow;
        private List<DataGridRow> _loadedRows;
        // prevents reentry into the VerticalScroll event handler
        private bool _makeFirstDisplayedCellCurrentCellPending;
        private bool _measured;
        private int? _mouseOverRowIndex;    // -1 is used for the 'new row'
        // the number of pixels of the firstDisplayedScrollingCol which are not displayed
        private double _negHorizontalOffset;
        // the number of pixels of DisplayData.FirstDisplayedScrollingRow which are not displayed
        // 


        private int _noSelectionChangeCount;
        private List<object> _removedSelectedItems;
        private double _rowHeaderDesiredWidth;
        private DataGridSelectedItemsCollection _selectedItems;
        private bool _selectionChanged;
        private IndexToValueTable<Visibility> _showDetailsTable;
        private bool _temporarilyResetCurrentCell;
        private ContentControl _topLeftCornerHeader; 
        private ContentControl _topRightCornerHeader;
        private object _uneditedValue; // Represents the original current cell value at the time it enters editing mode. 
        private byte _verticalScrollChangesIgnored;

        // An approximation of the sum of the heights in pixels of the scrolling rows preceding 
        // the first displayed scrolling row.  Since the scrolled off rows are discarded, the grid
        // does not know their actual height. The heights used for the approximation are the ones
        // set as the rows were scrolled off.
        private double _verticalOffset;

        #endregion Data

        #region Events

        public event EventHandler<DataGridAutoGeneratingColumnEventArgs> AutoGeneratingColumn;
        public event EventHandler<DataGridBeginningEditEventArgs> BeginningEdit;
        // 

        public event EventHandler<DataGridColumnEventArgs> ColumnDisplayIndexChanged;
        public event EventHandler<DragStartedEventArgs> ColumnHeaderDragStarted;
        public event EventHandler<DragDeltaEventArgs> ColumnHeaderDragDelta;
        public event EventHandler<DragCompletedEventArgs> ColumnHeaderDragCompleted;
        /// <summary>
        /// Raised when column reordering ends, to allow subscribers to clean up.
        /// </summary>
        public event EventHandler<DataGridColumnEventArgs> ColumnReordered;
        /// <summary>
        /// Raised when starting a column reordering action.  Subscribers to this event can
        /// set tooltip and caret UIElements, constrain tooltip position, indicate that
        /// a preview should be shown, or cancel reordering.
        /// </summary>
        public event EventHandler<DataGridColumnReorderingEventArgs> ColumnReordering;
        // 

        public event EventHandler<EventArgs> CurrentCellChanged;
        public event EventHandler<DataGridPreparingCellForEditEventArgs> PreparingCellForEdit;
        public event EventHandler<DataGridRowEventArgs> LoadingRow;
        public event EventHandler<DataGridRowDetailsEventArgs> LoadingRowDetails;
        public event EventHandler<DataGridRowEventArgs> UnloadingRow;
        public event EventHandler<DataGridRowDetailsEventArgs> UnloadingRowDetails;
        public event EventHandler<DataGridRowDetailsEventArgs> RowDetailsVisibilityChanged;
        public event SelectionChangedEventHandler SelectionChanged;

        #endregion Events

        /// <summary>
        /// Initializes a new instance of the DataGrid class.
        /// </summary>
        [SuppressMessage("Microsoft.Performance", "CA1805:DoNotInitializeUnnecessarily", Justification="_minRowHeight should be 0.")]
        public DataGrid()
        {
            this.TabNavigation = KeyboardNavigationMode.Once;
            this.KeyDown += new KeyEventHandler(DataGrid_KeyDown);
            this.KeyUp += new KeyEventHandler(DataGrid_KeyUp);
            this.GotFocus += new RoutedEventHandler(DataGrid_GotFocus);
            this.LostFocus += new RoutedEventHandler(DataGrid_LostFocus);

            this._loadedRows = new List<DataGridRow>();
            this._editingBoundCells = new List<DataGridCell>(2);
            this._selectedItems = new DataGridSelectedItemsCollection(this);
            this.SetValueNoCallback(SelectedIndexProperty, -1);

            this.DisplayData = new DataGridDisplayData();
            this.ColumnsInternal = CreateColumnsInstance();

            this.SetValueNoCallback(ColumnWidthProperty, DataGridLength.Auto);
            this.SetValueNoCallback(MaxColumnWidthProperty, DATAGRID_defaultMaxColumnWidth);
            this.SetValueNoCallback(MinColumnWidthProperty, DATAGRID_defaultMinColumnWidth);
            this.SetValueNoCallback(RowHeightProperty, double.NaN);
            this.RowHeightEstimate = DATAGRID_defaultRowHeight;
            this.RowDetailsHeightEstimate = 0;
            this._rowHeaderDesiredWidth = 0;

            this.DataConnection = new DataGridDataConnection(this);
            //this._newRowLocation = DataGridNewRowLocation.Inline;
            this._showDetailsTable = new IndexToValueTable<Visibility>();

            this.AnchorRowIndex = -1;
            this._lastEstimatedRow = -1;
            this._editingColumnIndex = -1;
            this._mouseOverRowIndex = null;
            this.CurrentCellCoordinates = new DataGridCellCoordinates(-1, -1);

            this.SetValueNoCallback(RowHeaderWidthProperty, double.NaN);
            this.SetValueNoCallback(ColumnHeaderHeightProperty, double.NaN);

            this._addedSelectedItems = new List<object>();
            this._removedSelectedItems = new List<object>();

            DefaultStyleKey = typeof(DataGrid);
        }

        #region Dependency Properties

        #region AlternatingRowBackground

        /// <summary>
        /// Gets or sets a brush that describes the background of odd-numbered rows in the grid.
        /// </summary>
        public Brush AlternatingRowBackground
        {
            get { return GetValue(AlternatingRowBackgroundProperty) as Brush; }
            set { SetValue(AlternatingRowBackgroundProperty, value); }
        }

        /// <summary>
        /// Identifies the AlternatingRowBackground dependency property.
        /// </summary>
        public static readonly DependencyProperty AlternatingRowBackgroundProperty = 
            DependencyProperty.Register(
                "AlternatingRowBackground", 
                typeof(Brush), 
                typeof(DataGrid), 
                new PropertyMetadata(OnAlternatingRowBackgroundPropertyChanged));

        private static void OnAlternatingRowBackgroundPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            if (dataGrid._rowsPresenter != null)
            {
                foreach (DataGridRow row in dataGrid._rowsPresenter.Children)
                {
                    row.EnsureBackground();
                }
            }
        }
        #endregion AlternatingRowBackground

        #region AreRowDetailsFrozen
        /// <summary>
        /// Gets or sets a value indicating whether the horizontal ScrollBar of the DataGrid affects the
        /// details section of a row.
        /// </summary>
        public bool AreRowDetailsFrozen
        {
            get { return (bool)GetValue(AreRowDetailsFrozenProperty); }
            set { SetValue(AreRowDetailsFrozenProperty, value); }
        }

        /// <summary>
        /// Identifies the AreRowDetailsFrozen dependency property.
        /// </summary>
        public static readonly DependencyProperty AreRowDetailsFrozenProperty =
            DependencyProperty.Register(
                "AreRowDetailsFrozen",
                typeof(bool),
                typeof(DataGrid),
                null);
        #endregion AreRowDetailsFrozen

        #region AutoGenerateColumns
        /// <summary>
        /// Gets or sets a value indicating whether columns are created automatically when the ItemsSource 
        /// property is set.
        /// </summary>
        public bool AutoGenerateColumns
        {
            get { return (bool)GetValue(AutoGenerateColumnsProperty); }
            set { SetValue(AutoGenerateColumnsProperty, value); }
        }

        /// <summary>
        /// Identifies the AutoGenerateColumns dependency property.
        /// </summary>
        public static readonly DependencyProperty AutoGenerateColumnsProperty =
            DependencyProperty.Register(
                "AutoGenerateColumns",
                typeof(bool),
                typeof(DataGrid),
                new PropertyMetadata(OnAutoGenerateColumnsPropertyChanged));

        private static void OnAutoGenerateColumnsPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            bool value = (bool)e.NewValue;
            if (value)
            {
                dataGrid.RefreshRowsAndColumns();
            }
            else
            {
                dataGrid.RemoveAutoGeneratedColumns();
            }
        }
        #endregion AutoGenerateColumns

        #region CanUserReorderColumns
        /// <summary>
        /// Gets or sets a value indicating whether users can reorder columns.
        /// </summary>
        public bool CanUserReorderColumns
        {
            get { return (bool)GetValue(CanUserReorderColumnsProperty); }
            set { SetValue(CanUserReorderColumnsProperty, value); }
        }

        /// <summary>
        /// Identifies the CanUserReorderColumns dependency property.
        /// </summary>
        public static readonly DependencyProperty CanUserReorderColumnsProperty =
            DependencyProperty.Register(
                "CanUserReorderColumns",
                typeof(bool),
                typeof(DataGrid),
                null);
        #endregion CanUserReorderColumns

        #region CanUserResizeColumns
        /// <summary>
        /// Gets or sets a value indicating whether users can resize columns.
        /// </summary>
        public bool CanUserResizeColumns
        {
            get { return (bool)GetValue(CanUserResizeColumnsProperty); }
            set { SetValue(CanUserResizeColumnsProperty, value); }
        }

        /// <summary>
        /// Identifies the CanUserResizeColumns dependency property.
        /// </summary>
        public static readonly DependencyProperty CanUserResizeColumnsProperty =
            DependencyProperty.Register(
                "CanUserResizeColumns",
                typeof(bool),
                typeof(DataGrid),
                null);
        #endregion CanUserResizeColumns

        #region CanUserSortColumns
        /// <summary>
        /// Gets or sets a value indicating whether users can sort columns.
        /// </summary>
        public bool CanUserSortColumns
        {
            get { return (bool)GetValue(CanUserSortColumnsProperty); }
            set { SetValue(CanUserSortColumnsProperty, value); }
        }

        /// <summary>
        /// Identifies the CanUserSortColumns dependency property.
        /// </summary>
        public static readonly DependencyProperty CanUserSortColumnsProperty =
            DependencyProperty.Register(
                "CanUserSortColumns",
                typeof(bool),
                typeof(DataGrid),
                null);
        #endregion CanUserSortColumns

        #region CellStyle
        /// <summary>
        /// Gets or sets the style used by cells when they are rendered.
        /// </summary>
        public Style CellStyle
        {
            get { return GetValue(CellStyleProperty) as Style; }
            set { SetValue(CellStyleProperty, value); }
        }

        public static readonly DependencyProperty CellStyleProperty =
            DependencyProperty.Register(
                "CellStyle",
                typeof(Style),
                typeof(DataGrid),
                new PropertyMetadata(OnCellStylePropertyChanged));

        private static void OnCellStylePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Style newStyle = e.NewValue as Style;
            if (newStyle != null)
            {
                DataGrid dataGrid = d as DataGrid;
                if (dataGrid != null && dataGrid._rowsPresenter != null)
                {
                    foreach (DataGridRow row in dataGrid._rowsPresenter.Children)
                    {
                        foreach (DataGridCell cell in row.Cells)
                        {
                            cell.EnsureCellStyle();
                        }
                    }
                    dataGrid.InvalidateRowHeightEstimate();
                }
            }
        }
        #endregion CellStyle

        #region ColumnHeaderHeight
        /// <summary>
        /// Gets or sets the suggested height of the grid's column headers.
        /// </summary>
        public double ColumnHeaderHeight
        {
            get { return (double)GetValue(ColumnHeaderHeightProperty); }
            set { SetValue(ColumnHeaderHeightProperty, value); }
        }

        /// <summary>
        /// Identifies the ColumnHeaderHeight dependency property.
        /// </summary>
        public static readonly DependencyProperty ColumnHeaderHeightProperty =
            DependencyProperty.Register(
                "ColumnHeaderHeight",
                typeof(double),
                typeof(DataGrid),
                new PropertyMetadata(OnColumnHeaderHeightPropertyChanged));

        /// <summary>
        /// ColumnHeaderHeightProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its ColumnHeaderHeight.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnColumnHeaderHeightPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            if (!dataGrid.IsHandlerSuspended(e.Property))
            {
                double value = (double)e.NewValue;
                if (value < DATAGRID_minimumColumnHeaderHeight)
                {
                    dataGrid.SetValueNoCallback(e.Property, e.OldValue);
                    throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "ColumnHeaderHeight", DATAGRID_minimumColumnHeaderHeight);
                }
                if (value > DATAGRID_maxHeadersThickness)
                {
                    dataGrid.SetValueNoCallback(e.Property, e.OldValue);
                    throw DataGridError.DataGrid.ValueMustBeLessThanOrEqualTo("value", "ColumnHeaderHeight", DATAGRID_maxHeadersThickness);
                }
                dataGrid.InvalidateMeasure();
            }
        }
        #endregion ColumnHeaderHeight

        #region ColumnHeaderStyle
        /// <summary>
        /// Gets or sets the style used by column headers when they are rendered. 
        /// </summary>
        public Style ColumnHeaderStyle
        {
            get { return GetValue(ColumnHeaderStyleProperty) as Style; }
            set { SetValue(ColumnHeaderStyleProperty, value); }
        }

        /// <summary>
        /// Identifies the ColumnHeaderStyle dependency property.
        /// </summary>
        public static readonly DependencyProperty ColumnHeaderStyleProperty = DependencyProperty.Register("ColumnHeaderStyle", typeof(Style), typeof(DataGrid), new PropertyMetadata(OnColumnHeaderStylePropertyChanged));

        private static void OnColumnHeaderStylePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            // 
            Style newStyle = e.NewValue as Style;
            if (newStyle != null)
            {
                DataGrid dataGrid = d as DataGrid;
                if (dataGrid != null)
                {
                    Style oldStyle = e.OldValue as Style;
                    foreach (DataGridColumn column in dataGrid.Columns)
                    {
                        EnsureColumnHeaderCellStyle(column, oldStyle, newStyle);
                    }
                    EnsureColumnHeaderCellStyle(dataGrid.ColumnsInternal.FillerColumn, oldStyle, newStyle);
                }
            }
        }
        #endregion ColumnHeaderStyle

        #region ColumnWidth
        /// <summary>
        /// Gets or sets the width of the grid's columns.
        /// </summary>
        public DataGridLength ColumnWidth
        {
            get { return (DataGridLength)GetValue(ColumnWidthProperty); }
            set { SetValue(ColumnWidthProperty, value); }
        }

        /// <summary>
        /// Identifies the ColumnWidth dependency property.
        /// </summary>
        public static readonly DependencyProperty ColumnWidthProperty =
            DependencyProperty.Register(
                "ColumnWidth",
                typeof(DataGridLength),
                typeof(DataGrid),
                new PropertyMetadata(OnColumnWidthPropertyChanged));

        /// <summary>
        /// ColumnWidthProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its ColumnWidth.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnColumnWidthPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;

            dataGrid.EnsureHorizontalLayout();
        }
        #endregion ColumnWidth

        #region DragIndicatorStyle
        /// <summary>
        /// Gets or sets the style used by rows when they are rendered.
        /// </summary>
        public Style DragIndicatorStyle
        {
            get { return GetValue(DragIndicatorStyleProperty) as Style; }
            set { SetValue(DragIndicatorStyleProperty, value); }
        }

        public static readonly DependencyProperty DragIndicatorStyleProperty =
            DependencyProperty.Register(
                "DragIndicatorStyle",
                typeof(Style),
                typeof(DataGrid),
                null);
        #endregion DragIndicatorStyle

        #region DropLocationIndicatorStyle
        /// <summary>
        /// Gets or sets the style used by rows when they are rendered.
        /// </summary>
        public Style DropLocationIndicatorStyle
        {
            get { return GetValue(DropLocationIndicatorStyleProperty) as Style; }
            set { SetValue(DropLocationIndicatorStyleProperty, value); }
        }

        public static readonly DependencyProperty DropLocationIndicatorStyleProperty =
            DependencyProperty.Register(
                "DropLocationIndicatorStyle",
                typeof(Style),
                typeof(DataGrid),
                null);
        #endregion DropLocationIndicatorStyle

        #region FrozenColumnCount

        public int FrozenColumnCount
        {
            get { return (int)GetValue(FrozenColumnCountProperty); }
            set { SetValue(FrozenColumnCountProperty, value); }
        }

        public static readonly DependencyProperty FrozenColumnCountProperty =
            DependencyProperty.Register(
                "FrozenColumnCount",
                typeof(int),
                typeof(DataGrid),
                new PropertyMetadata(OnFrozenColumnCountPropertyChanged));

        private static void OnFrozenColumnCountPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            ProcessFrozenColumnCount((DataGrid)d);
        }

        private static void ProcessFrozenColumnCount(DataGrid dataGrid)
        {
            dataGrid.CorrectColumnFrozenStates();
            dataGrid.ComputeScrollBarsLayout();

            dataGrid.InvalidateColumnHeadersArrange();
            dataGrid.InvalidateCellsArrange();
        }

        #endregion FrozenColumnCount

        #region GridLinesVisibility
        /// <summary>
        /// Gets or sets a value that indicates whether horizontal or vertical gridlines for 
        /// the inner cells should be displayed.
        /// </summary>
        public DataGridGridLinesVisibility GridLinesVisibility
        {
            get { return (DataGridGridLinesVisibility)GetValue(GridLinesVisibilityProperty); }
            set { SetValue(GridLinesVisibilityProperty, value); }
        }

        /// <summary>
        /// Identifies the GridLines dependency property.
        /// </summary>
        public static readonly DependencyProperty GridLinesVisibilityProperty =
            DependencyProperty.Register(
                "GridLinesVisibility",
                typeof(DataGridGridLinesVisibility),
                typeof(DataGrid),
                new PropertyMetadata(OnGridLinesVisibilityPropertyChanged));

        /// <summary>
        /// GridLinesProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its GridLines.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnGridLinesVisibilityPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            if (!dataGrid.IsHandlerSuspended(e.Property) && dataGrid._rowsPresenter != null)
            {
                foreach (DataGridRow row in dataGrid._rowsPresenter.Children)
                {
                    row.EnsureGridLines();
                    row.InvalidateHorizontalArrange();
                }
            }
        }
        #endregion GridLinesVisibility

        #region HeadersVisibility
        /// <summary>
        /// Gets or sets a value that indicates whether column or row headers should be displayed.
        /// </summary>
        public DataGridHeadersVisibility HeadersVisibility
        {
            get { return (DataGridHeadersVisibility)GetValue(HeadersVisibilityProperty); }
            set { SetValue(HeadersVisibilityProperty, value); }
        }

        /// <summary>
        /// Identifies the HeadersVisibility dependency property.
        /// </summary>
        public static readonly DependencyProperty HeadersVisibilityProperty =
            DependencyProperty.Register(
                "HeadersVisibility",
                typeof(DataGridHeadersVisibility),
                typeof(DataGrid),
                new PropertyMetadata(OnHeadersVisibilityPropertyChanged));

        /// <summary>
        /// HeadersVisibilityProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its HeadersVisibility.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnHeadersVisibilityPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            if (!dataGrid.IsHandlerSuspended(e.Property))
            {
                DataGridHeadersVisibility newValue = (DataGridHeadersVisibility)e.NewValue;
                DataGridHeadersVisibility oldValue = (DataGridHeadersVisibility)e.OldValue;

                Func<DataGridHeadersVisibility, DataGridHeadersVisibility, bool> hasFlags = (DataGridHeadersVisibility value, DataGridHeadersVisibility flags) => ((value & flags) == flags);

                bool newValueCols = hasFlags(newValue, DataGridHeadersVisibility.Column);
                bool newValueRows = hasFlags(newValue, DataGridHeadersVisibility.Row);
                bool oldValueCols = hasFlags(oldValue, DataGridHeadersVisibility.Column);
                bool oldValueRows = hasFlags(oldValue, DataGridHeadersVisibility.Row);

                // Columns
                if (newValueCols != oldValueCols)
                {
                    if (dataGrid._columnHeadersPresenter != null)
                    {
                        dataGrid.EnsureColumnHeadersVisibility();
                        if (!newValueCols)
                        {
                            dataGrid._columnHeadersPresenter.Measure(Size.Empty);
                        }
                        dataGrid.InvalidateMeasure();
                    }
                }

                // Rows
                if (newValueRows != oldValueRows)
                {
                    if (dataGrid._rowsPresenter != null)
                    {
                        foreach (DataGridRow row in dataGrid._rowsPresenter.Children)
                        {
                            row.EnsureHeaderStyleAndVisibility();
                            if (newValueRows)
                            {
                                row.ApplyState(false /*animate*/);
                                row.EnsureHeaderVisibility();
                            }
                        }
                        dataGrid.InvalidateRowHeightEstimate();
                        dataGrid.InvalidateRowsMeasure(true /*invalidateIndividualRows*/);
                    }
                }

                // 

                if (dataGrid._topLeftCornerHeader != null)
                {
                    dataGrid._topLeftCornerHeader.Visibility = newValueRows && newValueCols ? Visibility.Visible : Visibility.Collapsed;
                    if (dataGrid._topLeftCornerHeader.Visibility == Visibility.Collapsed)
                    {
                        dataGrid._topLeftCornerHeader.Measure(Size.Empty);
                    }
                }
            }
        }
        #endregion HeadersVisibility

        #region HorizontalGridLinesBrush
        /// <summary>
        /// Gets or sets a brush that describes the horizontal gridlines color.
        /// </summary>
        public Brush HorizontalGridLinesBrush
        {
            get { return GetValue(HorizontalGridLinesBrushProperty) as Brush; }
            set { SetValue(HorizontalGridLinesBrushProperty, value); }
        }

        /// <summary>
        /// Identifies the HorizontalGridLinesBrush dependency property.
        /// </summary>
        public static readonly DependencyProperty HorizontalGridLinesBrushProperty =
            DependencyProperty.Register(
                "HorizontalGridLinesBrush",
                typeof(Brush),
                typeof(DataGrid),
                new PropertyMetadata(OnHorizontalGridLinesBrushPropertyChanged));

        /// <summary>
        /// HorizontalGridLinesBrushProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its HorizontalGridLinesBrush.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnHorizontalGridLinesBrushPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            if (!dataGrid.IsHandlerSuspended(e.Property) && dataGrid._rowsPresenter != null)
            {
                foreach (DataGridRow row in dataGrid._rowsPresenter.Children)
                {
                    row.EnsureGridLines();
                }
            }
        }
        #endregion HorizontalGridLinesBrush

        #region HorizontalScrollBarVisibility
        /// <summary>
        /// Gets or sets a value that indicates whether a horizontal ScrollBar should be displayed.
        /// </summary>
        public ScrollBarVisibility HorizontalScrollBarVisibility
        {
            get { return (ScrollBarVisibility)GetValue(HorizontalScrollBarVisibilityProperty); }
            set { SetValue(HorizontalScrollBarVisibilityProperty, value); }
        }

        /// <summary>
        /// Identifies the HorizontalScrollBarVisibility dependency property.
        /// </summary>
        public static readonly DependencyProperty HorizontalScrollBarVisibilityProperty =
            DependencyProperty.Register(
                "HorizontalScrollBarVisibility",
                typeof(ScrollBarVisibility),
                typeof(DataGrid),
                new PropertyMetadata(OnHorizontalScrollBarVisibilityPropertyChanged));

        /// <summary>
        /// HorizontalScrollBarVisibilityProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its HorizontalScrollBarVisibility.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnHorizontalScrollBarVisibilityPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            if (!dataGrid.IsHandlerSuspended(e.Property) &&
                (ScrollBarVisibility)e.NewValue != (ScrollBarVisibility)e.OldValue &&
                dataGrid._hScrollBar != null)
            {
                dataGrid.InvalidateMeasure();
            }
        }
        #endregion HorizontalScrollBarVisibility

        #region IsReadOnly
        /// <summary>
        /// Gets or sets a value indicating whether the user can edit the cells of the DataGrid control.
        /// </summary>
        public bool IsReadOnly
        {
            get { return (bool)GetValue(IsReadOnlyProperty); }
            set { SetValue(IsReadOnlyProperty, value); }
        }

        /// <summary>
        /// Identifies the IsReadOnly dependency property.
        /// </summary>
        public static readonly DependencyProperty IsReadOnlyProperty =
            DependencyProperty.Register(
                "IsReadOnly",
                typeof(bool),
                typeof(DataGrid),
                new PropertyMetadata(OnIsReadOnlyPropertyChanged));

        /// <summary>
        /// IsReadOnlyProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its IsReadOnly.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnIsReadOnlyPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            if (!dataGrid.IsHandlerSuspended(e.Property))
            {
                bool value = (bool)e.NewValue;
                if (value && !dataGrid.CommitEdit(DataGridEditingUnit.Row, true /*exitEditing*/))
                {
                    d.SetValueNoCallback(e.Property, e.OldValue);
                    throw DataGridError.DataGrid.CommitFailedCannotCompleteOperation();
                }
            }
        }
        #endregion IsReadOnly

        #region ItemsSource
        /// <summary>
        /// Gets or sets a collection used to generate the content of the DataGrid.
        /// </summary>
        public IEnumerable ItemsSource
        {
            get { return GetValue(ItemsSourceProperty) as IEnumerable; }
            set { SetValue(ItemsSourceProperty, value); }
        }

        /// <summary>
        /// Identifies the ItemsSource dependency property.
        /// </summary>
        public static readonly DependencyProperty ItemsSourceProperty =
            DependencyProperty.Register(
                "ItemsSource",
                typeof(IEnumerable),
                typeof(DataGrid),
                new PropertyMetadata(OnItemsSourcePropertyChanged));

        /// <summary>
        /// ItemsSourceProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its ItemsSource.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnItemsSourcePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            if (!dataGrid.IsHandlerSuspended(e.Property))
            {
                Debug.Assert(dataGrid.DataConnection != null);

                if (dataGrid.LoadingOrUnloadingRow)
                {
                    dataGrid.SetValueNoCallback(ItemsSourceProperty, e.OldValue);
                    throw DataGridError.DataGrid.CannotChangeItemsWhenLoadingRows();
                }

                dataGrid.DataConnection.UnWireEvents(dataGrid.DataConnection.DataSource);
                dataGrid.DataConnection.ClearDataProperties();

                // Wrap an IList in a CollectionView if it's not already one
                IEnumerable newDataSource;
                IList tempList = e.NewValue as IList;
                if (tempList != null && !(e.NewValue is System.ComponentModel.ICollectionView))
                {
                    newDataSource = new ListCollectionView(tempList);
                }
                else
                {
                    newDataSource = (IEnumerable)e.NewValue;
                }
                dataGrid.DataConnection.DataSource = newDataSource;

                if (newDataSource != null)
                {
                    dataGrid.DataConnection.WireEvents(newDataSource);
                }

                // we always want to do this
                dataGrid.RefreshRowsAndColumns();
            }
        }

        #endregion ItemsSource

        #region MaxColumnWidth
        /// <summary>
        /// Gets or sets the width of the grid's columns.
        /// </summary>
        public double MaxColumnWidth
        {
            get { return (double)GetValue(MaxColumnWidthProperty); }
            set { SetValue(MaxColumnWidthProperty, value); }
        }

        /// <summary>
        /// Identifies the MaxColumnWidth dependency property.
        /// </summary>
        public static readonly DependencyProperty MaxColumnWidthProperty =
            DependencyProperty.Register(
                "MaxColumnWidth",
                typeof(double),
                typeof(DataGrid),
                new PropertyMetadata(OnMaxColumnWidthPropertyChanged));

        /// <summary>
        /// MaxColumnWidthProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its ColumnWidth.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnMaxColumnWidthPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            double newValue = (double)e.NewValue;

            if (double.IsNaN(newValue))
            {
                dataGrid.SetValueNoCallback(e.Property, e.OldValue);
                throw DataGridError.DataGrid.ValueCannotBeSetToNAN("MaxColumnWidth");
            }
            if (newValue < 0)
            {
                dataGrid.SetValueNoCallback(e.Property, e.OldValue);
                throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "MaxColumnWidth", 0);
            }
            if (dataGrid.MinColumnWidth > newValue)
            {
                dataGrid.SetValueNoCallback(e.Property, e.OldValue);
                throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "MaxColumnWidth", "MinColumnWidth");
            }

            dataGrid.ColumnsInternal.EnsureVisibleEdgedColumnsWidth();
            dataGrid.InvalidateColumnHeadersMeasure();
            dataGrid.InvalidateRowsMeasure(true);
        }
        #endregion MaxColumnWidth

        #region MinColumnWidth
        /// <summary>
        /// Gets or sets the width of the grid's columns.
        /// </summary>
        public double MinColumnWidth
        {
            get { return (double)GetValue(MinColumnWidthProperty); }
            set { SetValue(MinColumnWidthProperty, value); }
        }

        /// <summary>
        /// Identifies the MinColumnWidth dependency property.
        /// </summary>
        public static readonly DependencyProperty MinColumnWidthProperty =
            DependencyProperty.Register(
                "MinColumnWidth",
                typeof(double),
                typeof(DataGrid),
                new PropertyMetadata(OnMinColumnWidthPropertyChanged));

        /// <summary>
        /// MinColumnWidthProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its ColumnWidth.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnMinColumnWidthPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            double newValue = (double)e.NewValue;

            if (double.IsNaN(newValue))
            {
                dataGrid.SetValueNoCallback(e.Property, e.OldValue);
                throw DataGridError.DataGrid.ValueCannotBeSetToNAN("MinColumnWidth");
            }
            if (newValue < 0)
            {
                dataGrid.SetValueNoCallback(e.Property, e.OldValue);
                throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "MinColumnWidth", 0);
            }
            if (double.IsPositiveInfinity(newValue))
            {
                dataGrid.SetValueNoCallback(e.Property, e.OldValue);
                throw DataGridError.DataGrid.ValueCannotBeSetToInfinity("MinColumnWidth");
            }
            if (dataGrid.MaxColumnWidth < newValue)
            {
                dataGrid.SetValueNoCallback(e.Property, e.OldValue);
                throw DataGridError.DataGrid.ValueMustBeLessThanOrEqualTo("value", "MinColumnWidth", "MaxColumnWidth");
            }

            dataGrid.ColumnsInternal.EnsureVisibleEdgedColumnsWidth();
            dataGrid.InvalidateColumnHeadersMeasure();
            dataGrid.InvalidateRowsMeasure(true);
        }
        #endregion MinColumnWidth

        #region RowBackground
        /// <summary>
        /// Gets or sets a brush that describes the background of a row in the grid.
        /// </summary>
        public Brush RowBackground
        {
            get { return GetValue(RowBackgroundProperty) as Brush; }
            set { SetValue(RowBackgroundProperty, value); }
        }

        public static readonly DependencyProperty RowBackgroundProperty = DependencyProperty.Register("RowBackground", typeof(Brush), typeof(DataGrid), new PropertyMetadata(OnRowBackgroundPropertyChanged));

        private static void OnRowBackgroundPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;

            if (dataGrid._rowsPresenter != null)
            {
                // Go through the Displayed rows and update the background
                foreach (DataGridRow row in dataGrid._rowsPresenter.Children)
                {
                    row.EnsureBackground();
                }
            }
        }
        #endregion RowBackground

        #region RowDetailsTemplate
        /// <summary>
        /// Gets or sets the DataTemplate used to display the details section of a row.
        /// </summary>
        public DataTemplate RowDetailsTemplate
        {
            get { return GetValue(RowDetailsTemplateProperty) as DataTemplate; }
            set { SetValue(RowDetailsTemplateProperty, value); }
        }

        /// <summary>
        /// Identifies the RowDetailsTemplate dependency property.
        /// </summary>
        public static readonly DependencyProperty RowDetailsTemplateProperty =
            DependencyProperty.Register(
                "RowDetailsTemplate",
                typeof(DataTemplate),
                typeof(DataGrid),
                new PropertyMetadata(OnRowDetailsTemplatePropertyChanged));

        private static void OnRowDetailsTemplatePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;

            // Update the RowDetails templates if necessary
            if (dataGrid._rowsPresenter != null)
            {
                foreach (DataGridRow row in dataGrid._rowsPresenter.Children)
                {
                    if (dataGrid.GetRowDetailsVisibility(row.Index) == Visibility.Visible)
                    {
                        // DetailsPreferredHeight is initialized when the DetailsElement's size changes.
                        row.ApplyDetailsTemplate(false /*initializeDetailsPreferredHeight*/);
                    }
                }
            }

            dataGrid.UpdateRowDetailsHeightEstimate();
            dataGrid.InvalidateMeasure();
        }

        #endregion RowDetailsTemplate

        #region RowDetailsVisibilityMode
        /// <summary>
        /// Gets or sets a value that indicates when the details section of a row should be displayed.
        /// </summary>
        // 
        [SuppressMessage("Microsoft.Naming", "CA1721:PropertyNamesShouldNotMatchGetMethods")]
        public DataGridRowDetailsVisibilityMode RowDetailsVisibilityMode
        {
            get { return (DataGridRowDetailsVisibilityMode)GetValue(RowDetailsVisibilityModeProperty); }
            set { SetValue(RowDetailsVisibilityModeProperty, value); }
        }

        /// <summary>
        /// Identifies the RowDetailsVisibilityMode dependency property.
        /// </summary>
        public static readonly DependencyProperty RowDetailsVisibilityModeProperty =
            DependencyProperty.Register(
                "RowDetailsVisibilityMode",
                typeof(DataGridRowDetailsVisibilityMode),
                typeof(DataGrid),
                new PropertyMetadata(OnRowDetailsVisibilityModePropertyChanged));

        /// <summary>
        /// RowDetailsVisibilityModeProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its RowDetailsVisibilityMode.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnRowDetailsVisibilityModePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            if (dataGrid._rowsPresenter != null && dataGrid.RowCount > 0)
            {
                DataGridRowDetailsVisibilityMode newDetailsMode = (DataGridRowDetailsVisibilityMode)e.NewValue;
                Visibility newDetailsVisibility = Visibility.Collapsed;
                switch (newDetailsMode)
                {
                    case DataGridRowDetailsVisibilityMode.Visible:
                        newDetailsVisibility = Visibility.Visible;
                        dataGrid._showDetailsTable.AddValues(0, dataGrid.RowCount, Visibility.Visible);
                        break;
                    case DataGridRowDetailsVisibilityMode.Collapsed:
                        newDetailsVisibility = Visibility.Collapsed;
                        dataGrid._showDetailsTable.AddValues(0, dataGrid.RowCount, Visibility.Collapsed); 
                        break;
                    case DataGridRowDetailsVisibilityMode.VisibleWhenSelected:
                        dataGrid._showDetailsTable.Clear();
                        break;
                }

                bool updated = false;
                foreach (DataGridRow row in dataGrid._rowsPresenter.Children)
                {
                    Debug.Assert(row.Index != -1);
                    if (newDetailsMode == DataGridRowDetailsVisibilityMode.VisibleWhenSelected)
                    {
                        // For VisibleWhenSelected, we need to calculate the value for each individual row
                        newDetailsVisibility = dataGrid._selectedItems.Contains(row.Index) ? Visibility.Visible : Visibility.Collapsed;
                    }
                    if (row.DetailsVisibility != newDetailsVisibility)
                    {
                        updated = true;
                        row.SetDetailsVisibilityInternal(newDetailsVisibility, true /* raiseNotification */, false /* animate */);
                    }
                }
                if (updated)
                {
                    dataGrid.UpdateDisplayedRows(dataGrid.DisplayData.FirstDisplayedScrollingRow, dataGrid.CellsHeight);
                    dataGrid.InvalidateRowsMeasure(false /*invalidateIndividualRows*/);
                }
            }
        }
        #endregion RowDetailsVisibilityMode

        #region RowHeight
        /// <summary>
        /// Gets or sets the suggested height of the grid's rows.
        /// </summary>
        public double RowHeight
        {
            get { return (double)GetValue(RowHeightProperty); }
            set { SetValue(RowHeightProperty, value); }
        }

        /// <summary>
        /// Identifies the RowHeight dependency property.
        /// </summary>
        public static readonly DependencyProperty RowHeightProperty =
            DependencyProperty.Register(
                "RowHeight",
                typeof(double),
                typeof(DataGrid),
                new PropertyMetadata(OnRowHeightPropertyChanged));

        /// <summary>
        /// RowHeightProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its RowHeight.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        [SuppressMessage("Microsoft.Usage", "CA2208:InstantiateArgumentExceptionsCorrectly", Justification = "This parameter is exposed to the user as a 'RowHeight' dependency property.")]
        private static void OnRowHeightPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;

            if (!dataGrid.IsHandlerSuspended(e.Property))
            {
                double value = (double)e.NewValue;

                if (value < DataGridRow.DATAGRIDROW_minimumHeight)
                {
                    dataGrid.SetValueNoCallback(e.Property, e.OldValue);
                    throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "RowHeight", 0);
                }
                if (value > DataGridRow.DATAGRIDROW_maximumHeight)
                {
                    dataGrid.SetValueNoCallback(e.Property, e.OldValue);
                    throw DataGridError.DataGrid.ValueMustBeLessThanOrEqualTo("value", "RowHeight", DataGridRow.DATAGRIDROW_maximumHeight);
                }

                dataGrid.InvalidateRowHeightEstimate();
                // Re-measure all the rows due to the Height change
                dataGrid.InvalidateRowsMeasure(true);
                // DataGrid needs to update the layout information and the ScrollBars
                dataGrid.InvalidateMeasure();
            }
        }
        #endregion RowHeight

        #region RowHeaderWidth
        /// <summary>
        /// Gets or sets the width of the grid's row headers.
        /// </summary>
        public double RowHeaderWidth
        {
            get { return (double)GetValue(RowHeaderWidthProperty); }
            set { SetValue(RowHeaderWidthProperty, value); }
        }

        /// <summary>
        /// Identifies the RowHeaderWidth dependency property.
        /// </summary>
        public static readonly DependencyProperty RowHeaderWidthProperty =
            DependencyProperty.Register(
                "RowHeaderWidth",
                typeof(double),
                typeof(DataGrid),
                new PropertyMetadata(OnRowHeaderWidthPropertyChanged));

        /// <summary>
        /// RowHeaderWidthProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its RowHeaderWidth.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnRowHeaderWidthPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            if (!dataGrid.IsHandlerSuspended(e.Property))
            {
                double value = (double)e.NewValue;

                if (value < DATAGRID_minimumRowHeaderWidth)
                {
                    dataGrid.SetValueNoCallback(e.Property, e.OldValue);
                    throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "RowHeaderWidth", DATAGRID_minimumRowHeaderWidth);
                }
                if (value > DATAGRID_maxHeadersThickness)
                {
                    dataGrid.SetValueNoCallback(e.Property, e.OldValue);
                    throw DataGridError.DataGrid.ValueMustBeLessThanOrEqualTo("value", "RowHeaderWidth", DATAGRID_maxHeadersThickness);
                }
                dataGrid.EnsureRowHeaderWidth();
            }
        }
        #endregion RowHeaderWidth

        #region RowHeaderStyle
        /// <summary>
        /// Gets or sets the style used by row headers when they are rendered. 
        /// </summary>
        public Style RowHeaderStyle
        {
            get { return GetValue(RowHeaderStyleProperty) as Style; }
            set { SetValue(RowHeaderStyleProperty, value); }
        }

        public static readonly DependencyProperty RowHeaderStyleProperty = DependencyProperty.Register("RowHeaderStyle", typeof(Style), typeof(DataGrid), new PropertyMetadata(OnRowHeaderStylePropertyChanged));

        private static void OnRowHeaderStylePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Style newStyle = e.NewValue as Style;
            if (newStyle != null)
            {
                DataGrid dataGrid = d as DataGrid;
                if (dataGrid != null && dataGrid._rowsPresenter != null)
                {
                    // Set HeaderCell.Style for displayed rows if HeaderCell.Style is not already set
                    foreach (DataGridRow row in dataGrid._rowsPresenter.Children)
                    {
                        row.EnsureHeaderStyleAndVisibility();
                    }
                    dataGrid.InvalidateRowHeightEstimate();
                }
            }
        }
        #endregion RowHeaderStyle

        #region RowStyle
        /// <summary>
        /// Gets or sets the style used by rows when they are rendered.
        /// </summary>
        public Style RowStyle
        {
            get { return GetValue(RowStyleProperty) as Style; }
            set { SetValue(RowStyleProperty, value); }
        }

        public static readonly DependencyProperty RowStyleProperty =
            DependencyProperty.Register(
                "RowStyle",
                typeof(Style),
                typeof(DataGrid),
                new PropertyMetadata(OnRowStylePropertyChanged));

        private static void OnRowStylePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Style newStyle = e.NewValue as Style;
            if (newStyle != null)
            {
                DataGrid dataGrid = d as DataGrid;
                if (dataGrid != null)
                {
                    Style oldStyle = e.OldValue as Style;
                    if (dataGrid._rowsPresenter != null)
                    {
                        // Set the style for displayed rows if it has not already been set
                        foreach (DataGridRow row in dataGrid._rowsPresenter.Children)
                        {
                            EnsureRowStyle(row, oldStyle, newStyle);
                        }
                    }
                    dataGrid.InvalidateRowHeightEstimate();
                }
            }
        }
        #endregion RowStyle

        #region SelectionMode
        /// <summary>
        /// Gets or sets the selection behavior for a DataGrid.
        /// </summary>
        public DataGridSelectionMode SelectionMode
        {
            get { return (DataGridSelectionMode)GetValue(SelectionModeProperty); }
            set { SetValue(SelectionModeProperty, value); }
        }

        /// <summary>
        /// Identifies the SelectionMode dependency property.
        /// </summary>
        public static readonly DependencyProperty SelectionModeProperty =
            DependencyProperty.Register(
                "SelectionMode",
                typeof(DataGridSelectionMode),
                typeof(DataGrid),
                new PropertyMetadata(OnSelectionModePropertyChanged));

        /// <summary>
        /// SelectionModeProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its SelectionMode.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnSelectionModePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            if (!dataGrid.IsHandlerSuspended(e.Property))
            {
                dataGrid.ClearRowSelection(true /*resetAnchorRowIndex*/);
            }
        }
        #endregion SelectionMode

        #region SelectedIndex
        /// <summary>
        /// Gets or sets the index of the current selection or returns -1 if the selection is empty.
        /// </summary>
        public int SelectedIndex
        {
            get { return (int)GetValue(SelectedIndexProperty); }
            set { SetValue(SelectedIndexProperty, value); }
        }

        /// <summary>
        /// Identifies the SelectedIndex dependency property.
        /// </summary>
        public static readonly DependencyProperty SelectedIndexProperty =
            DependencyProperty.Register(
                "SelectedIndex",
                typeof(int),
                typeof(DataGrid),
                new PropertyMetadata(OnSelectedIndexPropertyChanged));

        /// <summary>
        /// SelectedIndexProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its SelectedIndex.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnSelectedIndexPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            if (!dataGrid.IsHandlerSuspended(e.Property))
            {
                int index = (int)e.NewValue;
                
                // GetDataItem returns null if index is >= Count, we do not check newValue 
                // against Count here to avoid enumerating through an Enumerable twice
                // Setting SelectedItem coerces the finally value of the SelectedIndex
                dataGrid.SelectedItem = (index < 0) ? null : dataGrid.DataConnection.GetDataItem(index);
            }
        }
        #endregion SelectedIndex

        #region SelectedItem
        /// <summary>
        /// Gets or sets the first item in the current selection or returns null if the selection is empty.
        /// </summary>
        public object SelectedItem
        {
            get { return GetValue(SelectedItemProperty) as object; }
            set { SetValue(SelectedItemProperty, value); }
        }

        /// <summary>
        /// Identifies the SelectedItem dependency property.
        /// </summary>
        public static readonly DependencyProperty SelectedItemProperty =
            DependencyProperty.Register(
                "SelectedItem",
                typeof(object),
                typeof(DataGrid),
                new PropertyMetadata(OnSelectedItemPropertyChanged));

        /// <summary>
        /// SelectedItemProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its SelectedItem.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnSelectedItemPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;

            if (!dataGrid.IsHandlerSuspended(e.Property))
            {
                int rowIndex = (e.NewValue == null) ? -1 : dataGrid.DataConnection.IndexOf(e.NewValue);
                if (rowIndex == -1)
                {
                    // If the Item is null or it's not found, clear the Selection
                    if (!dataGrid.CommitEdit(DataGridEditingUnit.Row, true /*exitEditing*/))
                    {
                        // Edited value couldn't be committed or aborted
                        d.SetValueNoCallback(e.Property, e.OldValue);
                        throw DataGridError.DataGrid.CommitFailedCannotCompleteOperation();
                    }

                    // Clear all row selections
                    dataGrid.ClearRowSelection(true /*resetAnchorRowIndex*/);
                }
                else
                {
                    dataGrid.SetValueNoCallback(DataGrid.SelectedIndexProperty, rowIndex);

                    if (rowIndex != dataGrid.CurrentRowIndex)
                    {
                        if (!dataGrid.CommitEdit(DataGridEditingUnit.Row, true /*exitEditing*/))
                        {
                            // Edited value couldn't be committed or aborted
                            d.SetValueNoCallback(e.Property, e.OldValue);
                            throw DataGridError.DataGrid.CommitFailedCannotCompleteOperation();
                        }
                        if (dataGrid.IsRowOutOfBounds(rowIndex))
                        {
                            return;
                        }
                    }

                    try
                    {
                        dataGrid._noSelectionChangeCount++;
                        dataGrid.ClearRowSelection(rowIndex /*rowIndexException*/, true /*resetAnchorRowIndex*/);
                        int columnIndex = dataGrid.CurrentColumnIndex;

                        if (columnIndex == -1)
                        {
                            columnIndex = dataGrid.ColumnsInternal.FirstVisibleColumn == null ? -1 : dataGrid.ColumnsInternal.FirstVisibleColumn.Index;
                        }

                        if (columnIndex == -1 || dataGrid.IsRowOutOfBounds(rowIndex))
                        {
                            return;
                        }
                        
                        bool success = dataGrid.SetCurrentCellCore(columnIndex, rowIndex/*, false, false*/);
                        Debug.Assert(success);
                    }
                    finally
                    {
                        dataGrid.NoSelectionChangeCount--;
                    }
                }
            }
        }
        #endregion SelectedItem

        #region VerticalGridLinesBrush
        /// <summary>
        /// Gets or sets a brush that describes the vertical gridlines color.
        /// </summary>
        public Brush VerticalGridLinesBrush
        {
            get { return GetValue(VerticalGridLinesBrushProperty) as Brush; }
            set { SetValue(VerticalGridLinesBrushProperty, value); }
        }

        /// <summary>
        /// Identifies the VerticalGridLinesBrush dependency property.
        /// </summary>
        public static readonly DependencyProperty VerticalGridLinesBrushProperty =
            DependencyProperty.Register(
                "VerticalGridLinesBrush",
                typeof(Brush),
                typeof(DataGrid),
                new PropertyMetadata(OnVerticalGridLinesBrushPropertyChanged));

        /// <summary>
        /// VerticalGridLinesBrushProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its VerticalGridLinesBrush.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnVerticalGridLinesBrushPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            if (!dataGrid.IsHandlerSuspended(e.Property) && dataGrid._rowsPresenter != null)
            {
                foreach (DataGridRow row in dataGrid._rowsPresenter.Children)
                {
                    row.EnsureGridLines();
                }
            }
        }
        #endregion VerticalGridLinesBrush

        #region VerticalScrollBarVisibility
        /// <summary>
        /// Gets or sets a value that indicates whether a vertical ScrollBar should be displayed.
        /// </summary>
        public ScrollBarVisibility VerticalScrollBarVisibility
        {
            get { return (ScrollBarVisibility)GetValue(VerticalScrollBarVisibilityProperty); }
            set { SetValue(VerticalScrollBarVisibilityProperty, value); }
        }

        /// <summary>
        /// Identifies the VerticalScrollBarVisibility dependency property.
        /// </summary>
        public static readonly DependencyProperty VerticalScrollBarVisibilityProperty =
            DependencyProperty.Register(
                "VerticalScrollBarVisibility",
                typeof(ScrollBarVisibility),
                typeof(DataGrid),
                new PropertyMetadata(OnVerticalScrollBarVisibilityPropertyChanged));

        /// <summary>
        /// VerticalScrollBarVisibilityProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its VerticalScrollBarVisibility.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnVerticalScrollBarVisibilityPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid dataGrid = (DataGrid)d;
            if (!dataGrid.IsHandlerSuspended(e.Property) &&
                (ScrollBarVisibility)e.NewValue != (ScrollBarVisibility)e.OldValue &&
                dataGrid._vScrollBar != null)
            {
                dataGrid.InvalidateMeasure();
            }
        }
        #endregion VerticalScrollBarVisibility

        #endregion Dependency Properties

        #region Public Properties    

        // 

        /// <summary>
        /// Gets the collection of columns currently present in the DataGrid.
        /// </summary>      
        public ObservableCollection<DataGridColumn> Columns
        {
            get
            {
                // we use a backing field here because the field's type
                // is a subclass of the property's
                return this.ColumnsInternal;
            }
        }

        /// <summary>
        /// Gets or sets the column that contains the cell that will go into 
        /// editing mode.
        /// </summary>
        public DataGridColumn CurrentColumn
        {
            get
            {
                if (this.CurrentColumnIndex == -1)
                {
                    return null;
                }
                Debug.Assert(this.CurrentColumnIndex < this.Columns.Count);
                return this.Columns[this.CurrentColumnIndex];
            }
            set
            {
                DataGridColumn dataGridColumn = value;
                if (dataGridColumn == null)
                {
                    throw DataGridError.DataGrid.ValueCannotBeSetToNull("value", "CurrentColumn");
                }
                if (this.CurrentColumn != dataGridColumn)
                {
                    if (dataGridColumn.OwningGrid != this)
                    {
                        // Provided column does not belong to this DataGrid
                        throw DataGridError.DataGrid.ColumnNotInThisDataGrid();
                    }
                    if (dataGridColumn.Visibility == Visibility.Collapsed)
                    {
                        // CurrentColumn cannot be set to an invisible column
                        throw DataGridError.DataGrid.ColumnCannotBeCollapsed();
                    }
                    if (this.CurrentRowIndex == -1)
                    {
                        // There is no current row so the current column cannot be set
                        throw DataGridError.DataGrid.NoCurrentRow();
                    }
                    bool beginEdit = this._editingColumnIndex != -1;
                    if (!EndCellEdit(true /*commitCellEdit*/, true /*exitEditingMode*/, this.ContainsFocus /*keepFocus*/))
                    {
                        // Edited value couldn't be committed or aborted
                        throw DataGridError.DataGrid.CommitFailedCannotCompleteOperation();
                    }
                    bool success = SetCurrentCellCore(dataGridColumn.Index, this.CurrentRowIndex);
                    Debug.Assert(success);
                    if (beginEdit && 
                        this._editingColumnIndex == -1 && 
                        this.CurrentRowIndex != -1 &&
                        this.CurrentColumnIndex != -1 && 
                        this.CurrentColumnIndex == dataGridColumn.Index &&
                        dataGridColumn.OwningGrid == this &&
                        !GetColumnEffectiveReadOnlyState(dataGridColumn))
                    {
                        // Returning to editing mode since the grid was in that mode prior to the EndCellEdit call above.
                        BeginCellEdit(new RoutedEventArgs());
                    }
                }
            }
        }

        /* 



















*/

        // 















        /* 



















*/

        /// <summary>
        /// Gets the currently selected items.
        /// </summary>
        public IList SelectedItems
        {
            get { return _selectedItems as IList; }
        }

        #endregion Public Properties

        #region Protected Properties

        /// <summary>
        /// Gets the item that will be edited during editing mode.
        /// </summary>
        protected object CurrentItem
        {
            get
            {
                if (this.CurrentRowIndex == -1 || this.ItemsSource /*this.DataConnection.DataSource*/ == null)
                {
                    return null;
                }
                return this.DataConnection.GetDataItem(this.CurrentRowIndex);
            }
            // 















































        }

        #endregion Protected Properties

        #region Internal Properties

        internal int AnchorRowIndex
        {
            get;
            private set;
        }

        internal double ActualRowHeaderWidth
        {
            get
            {
                if (!this.AreRowHeadersVisible)
                {
                    return 0;
                }
                else
                {
                    return !double.IsNaN(this.RowHeaderWidth) ? this.RowHeaderWidth : this.RowHeadersDesiredWidth;
                }
            }
        }

        internal bool AreColumnHeadersVisible
        {
            get
            {
                return (this.HeadersVisibility & DataGridHeadersVisibility.Column) == DataGridHeadersVisibility.Column;
            }
        }

        internal bool AreRowHeadersVisible
        {
            get
            {
                return (this.HeadersVisibility & DataGridHeadersVisibility.Row) == DataGridHeadersVisibility.Row;
            }
        }

        internal double AvailableRowRoom
        {
            get;
            set;
        }

        // 









        
        // Height currently available for cells this value is smaller.  This height is reduced by the existence of ColumnHeaders
        // or a horizontal scrollbar.  Layout is asynchronous so changes to the ColumnHeaders or the horizontal scrollbar are 
        // not reflected immediately.
        internal double CellsHeight
        {
            get
            {
                return this.RowsPresenterAvailableSize.Height;
            }
        }

        // Width currently available for cells this value is smaller.  This width is reduced by the existence of RowHeaders
        // or a vertical scrollbar.  Layout is asynchronous so changes to the RowHeaders or the vertical scrollbar are
        // not reflected immediately
        internal double CellsWidth
        {
            get
            {
                if (double.IsPositiveInfinity(this.RowsPresenterAvailableSize.Width))
                {
                    // If we're given infinite width, then the cells will just grow to be as big as the columns
                    return this.ColumnsInternal.VisibleEdgedColumnsWidth;
                }
                else
                {
                    return Math.Max(0, this.RowsPresenterAvailableSize.Width - this.ActualRowHeaderWidth);
                }
            }
        }

        internal DataGridColumnHeadersPresenter ColumnHeaders
        {
            get
            {
                return this._columnHeadersPresenter;
            }
        }

        // 
        internal DataGridColumnCollection ColumnsInternal
        {
            get;
            private set;
        }

        internal List<DataGridColumn> ColumnsItemsInternal
        {
            get
            {
                return this.ColumnsInternal.ItemsInternal;
            }
        }

        internal Popup ColumnDropLocationIndicatorPopup
        {
            get
            {
                if (this._columnDropLocationIndicatorPopup == null)
                {
                    this._columnDropLocationIndicatorPopup = new Popup
                    {
                        Child = this.ColumnDropLocationIndicator,
                        IsOpen = false
                    };
                }

                return this._columnDropLocationIndicatorPopup;
            }
        }

        internal Control ColumnDropLocationIndicator
        {
            get
            {
                // 



                if (this._columnDropLocationIndicator == null ||
                    this._columnDropLocationIndicator.Style != this.DropLocationIndicatorStyle)
                {
                    this._columnDropLocationIndicator = new ContentControl();
                    this._columnDropLocationIndicator.Style = this.DropLocationIndicatorStyle;
                }

                return this._columnDropLocationIndicator;
            }
        }

        internal bool ContainsFocus
        {
            get;
            private set;
        }

        internal int CurrentColumnIndex
        {
            get
            {
                return this.CurrentCellCoordinates.ColumnIndex;
            }

            private set
            {
                this.CurrentCellCoordinates.ColumnIndex = value;
            }
        }

        internal int CurrentRowIndex
        {
            get
            {
                return this.CurrentCellCoordinates.RowIndex;
            }

            private set
            {
                this.CurrentCellCoordinates.RowIndex = value;
            }
        }

        internal DataGridDataConnection DataConnection
        {
            get;
            private set;
        }

        internal DataGridDisplayData DisplayData
        {
            get;
            private set;
        }

        internal int EditingColumnIndex
        {
            get
            {
                return this._editingColumnIndex;
            }
        }

        internal int? EditingRowIndex
        {
            get
            {
                if (this._editingRow == null)
                {
                    return null;
                }
                else
                {
                    return this._editingRow.Index;
                }
            }
        }

        internal double FirstDisplayedScrollingColumnHiddenWidth
        {
            get
            {
                return this._negHorizontalOffset;
            }
        }

        internal static double HorizontalGridLinesThickness
        {
            get
            {
                return DATAGRID_horizontalGridLinesThickness;
            }
        }

        // the sum of the widths in pixels of the scrolling columns preceding 
        // the first displayed scrolling column
        internal double HorizontalOffset
        {
            get
            {
                return this._horizontalOffset;
            }
            set
            {
                if (value < 0)
                {
                    value = 0;
                }
                double widthNotVisible = Math.Max(0, this.ColumnsInternal.VisibleEdgedColumnsWidth - this.CellsWidth);
                if (value > widthNotVisible)
                {
                    value = widthNotVisible;
                }
                if (value == this._horizontalOffset)
                {
                    return;
                }

                if (this._hScrollBar != null && value != this._hScrollBar.Value)
                {
                    this._hScrollBar.Value = value;
                }
                this._horizontalOffset = value;

                this.DisplayData.FirstDisplayedScrollingCol = ComputeFirstVisibleScrollingColumn();
                // update the lastTotallyDisplayedScrollingCol
                ComputeDisplayedColumns();
            }
        }

        internal ScrollBar HorizontalScrollBar
        {
            get
            {
                return _hScrollBar;
            }
        }

        internal bool LoadingOrUnloadingRow
        {
            get;
            private set;
        }

        internal bool InDisplayIndexAdjustments
        {
            get;
            set;
        }

        internal int? MouseOverRowIndex
        {
            get
            {
                return this._mouseOverRowIndex;
            }
            set
            {
                if (this._mouseOverRowIndex != value)
                {
                    DataGridRow oldMouseOverRow = null;
                    if (this._mouseOverRowIndex.HasValue && this.DisplayData.IsRowDisplayed(this._mouseOverRowIndex.Value))
                    {
                        oldMouseOverRow = this.DisplayData.GetDisplayedRow(this._mouseOverRowIndex.Value);
                    }
                    this._mouseOverRowIndex = value;
                    if (oldMouseOverRow != null)
                    {
                        oldMouseOverRow.ApplyState(true /*animate*/);
                    }
                    if (this._mouseOverRowIndex.HasValue && this.DisplayData.IsRowDisplayed(this._mouseOverRowIndex.Value))
                    {
                        this.DisplayData.GetDisplayedRow(this._mouseOverRowIndex.Value).ApplyState(true /*animate*/);
                    }
                }
            }
        }

        internal double NegVerticalOffset
        {
            get;
            private set;
        }

        internal int RowCount
        {
            get;
            private set;
        }

        internal double RowDetailsHeightEstimate
        {
            get;
            private set;
        }

        internal double RowHeadersDesiredWidth
        {
            get
            {
                return _rowHeaderDesiredWidth;
            }
            set
            {
                // We only auto grow
                if (_rowHeaderDesiredWidth < value)
                {
                    double oldActualRowHeaderWidth = this.ActualRowHeaderWidth;
                    _rowHeaderDesiredWidth = value;
                    if (oldActualRowHeaderWidth != this.ActualRowHeaderWidth)
                    {
                        EnsureRowHeaderWidth();
                    }
                }
            }
        }

        internal double RowHeightEstimate
        {
            get;
            private set;
        }

        internal Size RowsPresenterAvailableSize
        {
            get;
            set;
        }

        internal ScrollBar VerticalScrollBar
        {
            get
            {
                return _vScrollBar;
            }
        }

        #endregion Internal Properties

        #region Private Properties

        private DataGridCellCoordinates CurrentCellCoordinates
        {
            get
            {
                return this._currentCellCoordinates;
            }

            set
            {
                this._currentCellCoordinates = value;
            }
        }

        private int FirstDisplayedColumnIndex
        {
            get
            {
                int firstDisplayedColumnIndex = -1;
                DataGridColumn dataGridColumn = this.ColumnsInternal.FirstVisibleColumn;
                if (dataGridColumn != null)
                {
                    if (dataGridColumn.IsFrozen)
                    {
                        firstDisplayedColumnIndex = dataGridColumn.Index;
                    }
                    else if (this.DisplayData.FirstDisplayedScrollingCol >= 0)
                    {
                        firstDisplayedColumnIndex = this.DisplayData.FirstDisplayedScrollingCol;
                    }
                }
                return firstDisplayedColumnIndex;
            }
        }

        // 












        private int NoSelectionChangeCount
        {
            get
            {
                return this._noSelectionChangeCount;
            }
            set
            {
                Debug.Assert(value >= 0);
                this._noSelectionChangeCount = value;
                if (value == 0)
                {
                    FlushSelectionChanged();
                }
            }
        }

        #endregion Private Properties

        #region Public Methods

        /// <summary>
        /// Enters editing mode for the current cell and current row (if they're not already in editing mode).
        /// </summary>
        /// <returns>True if operation was successful. False otherwise.</returns>
        public bool BeginEdit()
        {
            return BeginEdit(null);
        }

        /// <summary>
        /// Enters editing mode for the current cell and current row (if they're not already in editing mode).
        /// </summary>
        /// <param name="editingEventArgs">Provides information about the user gesture that caused the call to BeginEdit. Can be null.</param>
        /// <returns>True if operation was successful. False otherwise.</returns>
        public bool BeginEdit(RoutedEventArgs editingEventArgs)
        {
            // 
            if (this.CurrentColumnIndex == -1 || !GetRowSelection(this.CurrentRowIndex))
            {
                // 
                return false;
            }

            Debug.Assert(this.CurrentColumnIndex >= 0);
            Debug.Assert(this.CurrentColumnIndex < this.Columns.Count);
            Debug.Assert(this.CurrentRowIndex >= -1);
            Debug.Assert(this.CurrentRowIndex < this.RowCount);
            Debug.Assert(this._editingRow == null || this.EditingRowIndex == this.CurrentRowIndex);

            if (GetColumnEffectiveReadOnlyState(this.CurrentColumn))
            {
                // Current column is read-only
                // 
                return false;
            }
            // 
            return BeginCellEdit(editingEventArgs);
        }

        /// <summary>
        /// Cancels editing mode for the specified DataGridEditingUnit and restores its original value.
        /// </summary>
        /// <param name="editingUnit">Specifies whether to cancel edit for a Cell or Row.</param>
        /// <returns>True if operation was successful. False otherwise.</returns>
        public bool CancelEdit(DataGridEditingUnit editingUnit)
        {
            if (!EndCellEdit(false, true, this.ContainsFocus /*keepFocus*/))
            {
                return false;
            }
            if (editingUnit == DataGridEditingUnit.Row)
            {
                return EndRowEdit(false, true);
            }
            return true;
        }

        /// <summary>
        /// Commits editing mode for the specified DataGridEditingUnit and pushes changes to the backend.
        /// </summary>
        /// <param name="editingUnit">Specifies whether to commit edit for a Cell or Row.</param>
        /// <param name="exitEditingMode">Editing mode is left if True.</param>
        /// <returns>True if operation was successful. False otherwise.</returns>
        public bool CommitEdit(DataGridEditingUnit editingUnit, bool exitEditingMode)
        {
            if (!EndCellEdit(true, exitEditingMode, this.ContainsFocus /*keepFocus*/))
            {
                return false;
            }
            if (editingUnit == DataGridEditingUnit.Row)
            {
                return EndRowEdit(true, exitEditingMode);
            }
            return true;
        }

        /// <summary>
        /// Scrolls the specified item and/or column into view.
        /// If item is not null: scrolls the row representing the item into view;
        /// If column is not null: scrolls the column into view;
        /// If both item and column are null, the method returns without scrolling.
        /// </summary>
        /// <param name="item">an item from the DataGrid's items source</param>
        /// <param name="column">a column from the DataGrid's columns collection</param>
        public void ScrollIntoView(object item, DataGridColumn column)
        {
            if (item == null && column == null)
            {
                // no-op
                return;
            }

            // the row index will be set to -1 if the item is null or not in the list
            int rowIndex = this.DataConnection.IndexOf(item);
            if (item != null && rowIndex == -1)
            {
                return;
            }

            // the column, if non-null, must be owned by this grid
            if (column != null && column.OwningGrid != this)
            {
                // invalid
                return;
            }

            // dispatch
            if (item != null && column == null)
            {
                // scroll row into view
                this.ScrollIntoView(this.FirstDisplayedColumnIndex, rowIndex, true);
            }
            else if (item == null && column != null)
            {
                // scroll column into view
                this.ScrollIntoView(column.Index, this.DisplayData.FirstDisplayedScrollingRow, false);
            }
            else if (item != null && column != null)
            {
                // scroll cell into view
                this.ScrollIntoView(column.Index, rowIndex, true);
            }
        }

        #endregion Public Methods

        #region Protected Methods

        protected override Size ArrangeOverride(Size finalSize)
        {
            if (_makeFirstDisplayedCellCurrentCellPending)
            {
                MakeFirstDisplayedCellCurrentCell();
            }

            if (this.ActualWidth != finalSize.Width)
            {
                // If our final width has changed, we might need to update the filler
                InvalidateColumnHeadersArrange();
                InvalidateCellsArrange();
            }

            return base.ArrangeOverride(finalSize);
        }

        protected override Size MeasureOverride(Size availableSize)
        {
            // Delay column autogeneration till the after the inital measure to avoid generating columns if
            // the user does dataGrid.ItemsSource = myList; dataGrid.AutogenerateColumns = false;
            if (!_measured)
            {
                _measured = true;
                if (this.AutoGenerateColumns)
                {
                    RefreshRowsAndColumns();
                }

                // Update our estimates now that the DataGrid has all of the information necessary
                UpdateRowDetailsHeightEstimate();

                // Update frozen columns to account for columns added prior to loading or autogenerated columns
                if (this.FrozenColumnCount > 0)
                {
                    ProcessFrozenColumnCount(this);
                }
            }

            Size desiredSize;
            // This is a shortcut to skip layout if we don't have any columns
            if (this.ColumnsInternal.VisibleEdgedColumnsWidth == 0)
            {
                if (_hScrollBar != null && _hScrollBar.Visibility != Visibility.Collapsed)
                {
                    _hScrollBar.Visibility = Visibility.Collapsed;
                }
                if (_vScrollBar != null && _vScrollBar.Visibility != Visibility.Collapsed)
                {
                    _vScrollBar.Visibility = Visibility.Collapsed;
                }
                desiredSize = base.MeasureOverride(availableSize);
            }
            else
            {
                if (_rowsPresenter != null)
                {
                    _rowsPresenter.InvalidateMeasure();
                }

                InvalidateColumnHeadersMeasure();

                desiredSize = base.MeasureOverride(availableSize);

                // 
                ComputeScrollBarsLayout();
            }

            return desiredSize;
        }

        public override void OnApplyTemplate()
        {
            _columnHeadersPresenter = GetTemplateChild(DATAGRID_elementColumnHeadersPresenterName) as DataGridColumnHeadersPresenter;
            if (_columnHeadersPresenter != null)
            {
                _columnHeadersPresenter.OwningGrid = this;
                // Columns were added before before our Template was applied, add the ColumnHeaders now
                foreach (DataGridColumn column in this.ColumnsInternal)
                {
                    InsertDisplayedColumnHeader(column);
                }
            }
            _rowsPresenter = GetTemplateChild(DATAGRID_elementRowsPresenterName) as DataGridRowsPresenter;
            if (_rowsPresenter != null)
            {
                _rowsPresenter.OwningGrid = this;
                InvalidateRowHeightEstimate();
                UpdateRowDetailsHeightEstimate();
            }
            _frozenColumnScrollBarSpacer = GetTemplateChild(DATAGRID_elementFrozenColumnScrollBarSpacerName) as FrameworkElement;
            _hScrollBar = GetTemplateChild(DATAGRID_elementHorizontalScrollbarName) as ScrollBar;
            if (_hScrollBar != null)
            {
                _hScrollBar.IsTabStop = false;
                _hScrollBar.Maximum = 0.0;
                _hScrollBar.Orientation = Orientation.Horizontal;
                _hScrollBar.Visibility = Visibility.Collapsed;
                _hScrollBar.Scroll += new ScrollEventHandler(HorizontalScrollBar_Scroll);
            }
            _vScrollBar = GetTemplateChild(DATAGRID_elementVerticalScrollbarName) as ScrollBar;
            if (_vScrollBar != null)
            {
                _vScrollBar.IsTabStop = false;
                _vScrollBar.Maximum = 0.0;
                _vScrollBar.Orientation = Orientation.Vertical;
                _vScrollBar.Visibility = Visibility.Collapsed;
                _vScrollBar.Scroll += new ScrollEventHandler(VerticalScrollBar_Scroll);
            }

            _topLeftCornerHeader = GetTemplateChild(DATAGRID_elementTopLeftCornerHeaderName) as ContentControl;
            EnsureTopLeftCornerHeader(); // EnsureTopLeftCornerHeader checks for a null _topLeftCornerHeader;
            _topRightCornerHeader = GetTemplateChild(DATAGRID_elementTopRightCornerHeaderName) as ContentControl;
        }

        /// <summary>
        /// Raises the AutoGeneratingColumn event.
        /// </summary>
        protected virtual void OnAutoGeneratingColumn(DataGridAutoGeneratingColumnEventArgs e)
        {
            EventHandler<DataGridAutoGeneratingColumnEventArgs> handler = this.AutoGeneratingColumn;
            if (handler != null)
            {
                handler(this, e);
            }
        }

        /// <summary>
        /// Raises the BeginningEdit event.
        /// </summary>
        protected virtual void OnBeginningEdit(DataGridBeginningEditEventArgs e)
        {
            EventHandler<DataGridBeginningEditEventArgs> handler = this.BeginningEdit;
            if (handler != null)
            {
                handler(this, e);
            }
        }

        // 












        // 











        
        
        
        
        
        

        /// <summary>
        /// Creates AutomationPeer (<see cref="UIElement.OnCreateAutomationPeer"/>)
        /// </summary>
        protected override AutomationPeer OnCreateAutomationPeer()
        {
            return new DataGridAutomationPeer(this);
        }

        /// <summary>
        /// Raises the CurrentCellChanged event.
        /// </summary>
        protected virtual void OnCurrentCellChanged(EventArgs e)
        {
            EventHandler<EventArgs> handler = this.CurrentCellChanged;
            if (handler != null)
            {
                handler(this, e);
            }

            if (AutomationPeer.ListenerExists(AutomationEvents.SelectionItemPatternOnElementSelected))
            {
                DataGridAutomationPeer peer = DataGridAutomationPeer.FromElement(this) as DataGridAutomationPeer;
                if (peer != null)
                {
                    peer.RaiseAutomationCellSelectedEvent(this.CurrentRowIndex, this.CurrentColumnIndex);
                }
            }
        }

        /// <summary>
        /// Raises the PreparingCellForEdit event.
        /// </summary>
        protected virtual void OnPreparingCellForEdit(DataGridPreparingCellForEditEventArgs e)
        {
            EventHandler<DataGridPreparingCellForEditEventArgs> handler = this.PreparingCellForEdit;
            if (handler != null)
            {
                handler(this, e);
            }

            // Raise the automation invoke event for the cell that just began edit because now
            // its editable content has been loaded
            DataGridAutomationPeer peer = DataGridAutomationPeer.FromElement(this) as DataGridAutomationPeer;
            if (peer != null && AutomationPeer.ListenerExists(AutomationEvents.InvokePatternOnInvoked))
            {
                peer.RaiseAutomationInvokeEvents(DataGridEditingUnit.Cell, e.Column, e.Row);
            }
        }

        /// <summary>
        /// Raises the LoadingRow event for row preparation.
        /// </summary>
        protected virtual void OnLoadingRow(DataGridRowEventArgs e)
        {
            EventHandler<DataGridRowEventArgs> handler = this.LoadingRow;
            if (handler != null)
            {
                Debug.Assert(!this._loadedRows.Contains(e.Row));
                this._loadedRows.Add(e.Row);
                this.LoadingOrUnloadingRow = true;
                handler(this, e);
                this.LoadingOrUnloadingRow = false;
                Debug.Assert(this._loadedRows.Contains(e.Row));
                this._loadedRows.Remove(e.Row);
            }
        }

        /// <summary>
        /// Raises the LoadingRowDetails for row details preparation
        /// </summary>
        protected virtual void OnLoadingRowDetails(DataGridRowDetailsEventArgs e)
        {
            EventHandler<DataGridRowDetailsEventArgs> handler = this.LoadingRowDetails;
            if (handler != null)
            {
                this.LoadingOrUnloadingRow = true;
                handler(this, e);
                this.LoadingOrUnloadingRow = false;
            }
        }

        /// <summary>
        /// Raises the SelectionChanged event and clears the _selectionChanged.
        /// This event won't get raised again until after _selectionChanged is set back to true.
        /// </summary>
        protected virtual void OnSelectionChanged(SelectionChangedEventArgs e)
        {
            this.CoerceSelectedItem();

            this._selectionChanged = false;
            SelectionChangedEventHandler handler = this.SelectionChanged;
            if (handler != null)
            {
                handler(this, e);
            }

            if (AutomationPeer.ListenerExists(AutomationEvents.SelectionItemPatternOnElementSelected) ||
                AutomationPeer.ListenerExists(AutomationEvents.SelectionItemPatternOnElementAddedToSelection) ||
                AutomationPeer.ListenerExists(AutomationEvents.SelectionItemPatternOnElementRemovedFromSelection))
            {
                DataGridAutomationPeer peer = DataGridAutomationPeer.FromElement(this) as DataGridAutomationPeer;
                if (peer != null)
                {
                    peer.RaiseAutomationSelectionEvents(e);
                }
            }
        }

        /// <summary>
        /// Raises the UnloadingRow event for row recycling.
        /// </summary>
        protected virtual void OnUnloadingRow(DataGridRowEventArgs e)
        {
            EventHandler<DataGridRowEventArgs> handler = this.UnloadingRow;
            if (handler != null)
            {
                this.LoadingOrUnloadingRow = true;
                handler(this, e);
                this.LoadingOrUnloadingRow = false;
            }
        }

        /// <summary>
        /// Raises the UnloadingRowDetails event
        /// </summary>
        protected virtual void OnUnloadingRowDetails(DataGridRowDetailsEventArgs e)
        {
            EventHandler<DataGridRowDetailsEventArgs> handler = this.UnloadingRowDetails;
            if (handler != null)
            {
                this.LoadingOrUnloadingRow = true;
                handler(this, e);
                this.LoadingOrUnloadingRow = false;
            }
        }

        #endregion Protected Methods

        #region Internal Methods

        /// <summary>
        /// call when: selection changes or SelectedItems object changes
        /// </summary>
        internal void CoerceSelectedItem()
        {
            object selectedItem = null;

            if (this.SelectionMode == DataGridSelectionMode.Extended &&
                this.CurrentRowIndex != -1 &&
                _selectedItems.Contains(this.CurrentRowIndex))
            {
                selectedItem = this.CurrentItem;
            }
            else if (_selectedItems.Count > 0)
            {
                selectedItem = _selectedItems[0];
            }

            this.SetValueNoCallback(SelectedItemProperty, selectedItem);

            // Update the SelectedIndex
            int newIndex = -1;
            if (selectedItem != null)
            {
                newIndex = this.DataConnection.IndexOf(selectedItem);
            }
            this.SetValueNoCallback(SelectedIndexProperty, newIndex);
        }

        internal static DataGridCell GetOwningCell(FrameworkElement element)
        {
            Debug.Assert(element != null);
            DataGridCell cell = element as DataGridCell;
            while (element != null && cell == null)
            {
                element = element.Parent as FrameworkElement;
                cell = element as DataGridCell;
            }
            return cell;
        }

        // 

        internal void OnAddedSelectedItem(int rowIndex)
        {
            object item = this.DataConnection.GetDataItem(rowIndex);
            Debug.Assert(item != null);
            if (this._removedSelectedItems.Contains(item))
            {
                this._removedSelectedItems.Remove(item);
            }
            else if (!this._selectedItems.Contains(rowIndex) && !this._addedSelectedItems.Contains(item))
            {
                this._addedSelectedItems.Add(item);
            }
        }

        internal void OnRemovedSelectedItem(int rowIndex)
        {
            OnRemovedSelectedItem(rowIndex, this.DataConnection.GetDataItem(rowIndex));
        }

        internal void OnRemovedSelectedItem(int rowIndex, object item)
        {
            if (this._addedSelectedItems.Contains(item))
            {
                this._addedSelectedItems.Remove(item);
            }
            else if (this._selectedItems.Contains(rowIndex) && !this._removedSelectedItems.Contains(item))
            {
                this._removedSelectedItems.Add(item);
            }
        }

        internal void OnRowDetailsChanged()
        {
            // Update layout when RowDetails are expanded or collapsed, just updating the vertical scroll bar is not enough 
            // since rows could be added or removed
            InvalidateMeasure();
        }

        internal bool ProcessDownKey()
        {
            bool moved, shift, ctrl;
            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);
            return ProcessDownKeyInternal(shift, ctrl, out moved);
        }

        internal bool ProcessEndKey()
        {
            bool ctrl;
            bool shift;

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);

            DataGridColumn dataGridColumn = this.ColumnsInternal.LastVisibleColumn;
            int lastVisibleColumnIndex = (dataGridColumn == null) ? -1 : dataGridColumn.Index;
            int firstRowIndex = this.FirstRowIndex;
            int lastRowIndex = this.LastRowIndex;
            if (lastVisibleColumnIndex == -1 || firstRowIndex == -1)
            {
                return false;
            }

            this._noSelectionChangeCount++;
            try
            {
                if (!ctrl /**/)
                {
                    return ProcessRightMost(lastVisibleColumnIndex, firstRowIndex);
                }
                else
                {
                    if (!PrepareForScroll(lastVisibleColumnIndex, lastRowIndex, true))
                    {
                        return false;
                    }
                    if (shift && this.SelectionMode == DataGridSelectionMode.Extended && this.AnchorRowIndex >= 0)
                    {
                        ClearRowSelection(lastRowIndex, false /*resetAnchorRowIndex*/);
                        SetRowsSelection(this.AnchorRowIndex, lastRowIndex);
                    }
                    else
                    {
                        ClearRowSelection(lastRowIndex, true /*resetAnchorRowIndex*/);
                    }
                    // Scrolling needs to be done after Selection is updated
                    if (!ScrollIntoView(lastVisibleColumnIndex, lastRowIndex, true))
                    {
                        return true;
                    }
                    if (IsInnerCellOutOfBounds(lastVisibleColumnIndex, lastRowIndex))
                    {
                        return true;
                    }
                    bool success = SetCurrentCellCore(lastVisibleColumnIndex, lastRowIndex/*, false, false*/);
                    Debug.Assert(success);
                }
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }
            return true;
        }

        internal bool ProcessEnterKey()
        {
            bool ctrl, shift, moved = false, ret = true, endRowEdit = true;

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);

            if (!ctrl)
            {
                // If Enter was used by a TextBox, we shouldn't handle the key
                TextBox focusedTextBox = FocusManager.GetFocusedElement() as TextBox;
                if (focusedTextBox != null && focusedTextBox.AcceptsReturn)
                {
                    return false;
                }

                // Enter behaves like down arrow - it commits the potential editing and goes down one cell.
                endRowEdit = false;
                ret = ProcessDownKeyInternal(false, ctrl, out moved);
            }

            // Try to commit the potential editing
            if (!moved && EndCellEdit(true /*commitCellEdit*/, true /*exitEditingMode*/, true /*keepFocus*/) && endRowEdit && this._editingRow != null)
            {
                EndRowEdit(true /*commitRowEdit*/, true /*exitEditingMode*/);
            }

            return ret;
        }

        internal bool ProcessHomeKey()
        {
            bool ctrl;
            bool shift;

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);

            DataGridColumn dataGridColumn = this.ColumnsInternal.FirstVisibleColumn;
            int firstVisibleColumnIndex = (dataGridColumn == null) ? -1 : dataGridColumn.Index;
            int firstRowIndex = this.FirstRowIndex;
            if (firstVisibleColumnIndex == -1 || firstRowIndex == -1)
            {
                return false;
            }
            this._noSelectionChangeCount++;
            try
            {
                if (!ctrl /* */)
                {
                    return ProcessLeftMost(firstVisibleColumnIndex, firstRowIndex);
                }
                else
                {
                    if (!ScrollIntoView(firstVisibleColumnIndex, firstRowIndex, true /* forCurrentCellChange */))
                    {
                        return true;
                    }
                    if (IsInnerCellOutOfBounds(firstVisibleColumnIndex, firstRowIndex))
                    {
                        return true;
                    }
                    if (shift && this.SelectionMode == DataGridSelectionMode.Extended && this.AnchorRowIndex >= 0)
                    {
                        ClearRowSelection(firstRowIndex, false /*resetAnchorRowIndex*/);
                        SetRowsSelection(firstRowIndex, this.AnchorRowIndex);
                    }
                    else
                    {
                        ClearRowSelection(firstRowIndex, true /*resetAnchorRowIndex*/);
                    }
                    bool success = SetCurrentCellCore(firstVisibleColumnIndex, firstRowIndex/*, false, false*/);
                    Debug.Assert(success);
                }
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }
            return true;
        }

        internal void ProcessHorizontalScroll(ScrollEventType scrollEventType)
        {
            if (this._horizontalScrollChangesIgnored > 0)
            {
                return;
            }

            // If the user scrolls with the buttons, we need to update the new value of the scroll bar since we delay
            // this calculation.  If they scroll in another other way, the scroll bar's correct value has already been set
            double scrollBarValueDifference = 0;
            if (scrollEventType == ScrollEventType.SmallIncrement)
            {
                scrollBarValueDifference = GetHorizontalSmallScrollIncrease();
            }
            else if (scrollEventType == ScrollEventType.SmallDecrement)
            {
                scrollBarValueDifference = -GetHorizontalSmallScrollDecrease();
            }
            this._horizontalScrollChangesIgnored++;
            try
            {
                if (scrollBarValueDifference != 0)
                {
                    Debug.Assert(this._horizontalOffset + scrollBarValueDifference >= 0);
                    this._hScrollBar.Value = this._horizontalOffset + scrollBarValueDifference;
                }
                UpdateHorizontalOffset(this._hScrollBar.Value);
            }
            finally
            {
                this._horizontalScrollChangesIgnored--;
            }

            DataGridAutomationPeer peer = DataGridAutomationPeer.FromElement(this) as DataGridAutomationPeer;
            if (peer != null)
            {
                peer.RaiseAutomationScrollEvents();
            }
        }

        internal bool ProcessLeftKey()
        {
            bool success;
            bool ctrl;
            bool shift;

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);

            DataGridColumn dataGridColumn = this.ColumnsInternal.FirstVisibleColumn;
            int firstVisibleColumnIndex = (dataGridColumn == null) ? -1 : dataGridColumn.Index;
            int firstRowIndex = this.FirstRowIndex;
            if (firstVisibleColumnIndex == -1 || firstRowIndex == -1)
            {
                return false;
            }
            int previousVisibleColumnIndex = -1;
            if (this.CurrentColumnIndex != -1)
            {
                dataGridColumn = this.ColumnsInternal.GetPreviousVisibleColumn(this.Columns[this.CurrentColumnIndex]);
                if (dataGridColumn != null)
                {
                    previousVisibleColumnIndex = dataGridColumn.Index;
                }
            }

            this._noSelectionChangeCount++;
            try
            {
                if (ctrl)
                {
                    return ProcessLeftMost(firstVisibleColumnIndex, firstRowIndex);
                }
                else
                {
                    if (this.CurrentColumnIndex == -1)
                    {
                        Debug.Assert(_selectedItems.Count == 0);
                        SetRowSelection(firstRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                        success = ScrollIntoView(firstVisibleColumnIndex, firstRowIndex, false);
                        Debug.Assert(success);
                        if (IsInnerCellOutOfBounds(firstVisibleColumnIndex, firstRowIndex))
                        {
                            return true;
                        }
                        success = SetCurrentCellCore(firstVisibleColumnIndex, firstRowIndex/*, false, false*/);
                        Debug.Assert(success);
                    }
                    else
                    {
                        if (previousVisibleColumnIndex == -1)
                        {
                            return true;
                        }
                        if (!ScrollIntoView(previousVisibleColumnIndex, this.CurrentRowIndex, true))
                        {
                            return true;
                        }
                        if (IsRowOutOfBounds(this.CurrentRowIndex) || IsColumnOutOfBounds(previousVisibleColumnIndex))
                        {
                            return true;
                        }
                        success = SetCurrentCellCore(previousVisibleColumnIndex, this.CurrentRowIndex/*, false, false*/);
                        Debug.Assert(success);
                    }
                }
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }
            return true;
        }

        internal bool ProcessNextKey()
        {
            bool ctrl;
            bool shift;

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);

            DataGridColumn dataGridColumn = this.ColumnsInternal.FirstVisibleColumn;
            int firstVisibleColumnIndex = (dataGridColumn == null) ? -1 : dataGridColumn.Index;
            if (firstVisibleColumnIndex == -1)
            {
                return false;
            }
            int nextScreenRowIndexTmp, nextScreenRowIndex = -1, jumpRows = 0;
            if (this.CurrentRowIndex == -1)
            {
                nextScreenRowIndex = this.DisplayData.FirstDisplayedScrollingRow;
                if (nextScreenRowIndex == -1)
                {
                    return false;
                }
            }
            else
            {
                nextScreenRowIndex = this.CurrentRowIndex;
            }

            jumpRows += this.DisplayData.NumTotallyDisplayedScrollingRows;

            nextScreenRowIndexTmp = nextScreenRowIndex;
            Debug.Assert(nextScreenRowIndexTmp != -1);
            if (jumpRows == 0)
            {
                jumpRows = 1;
            }
            while (jumpRows > 0 && nextScreenRowIndexTmp != -1)
            {
                nextScreenRowIndexTmp = this.GetNextRow(nextScreenRowIndex);
                if (nextScreenRowIndexTmp != -1)
                {
                    nextScreenRowIndex = nextScreenRowIndexTmp;
                    jumpRows--;
                }
            }

            this._noSelectionChangeCount++;
            try
            {
                if (this.CurrentColumnIndex == -1)
                {
                    Debug.Assert(_selectedItems.Count == 0);
                    SetRowSelection(nextScreenRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                    ScrollIntoView(firstVisibleColumnIndex, nextScreenRowIndex, false);
                    if (IsInnerCellOutOfBounds(firstVisibleColumnIndex, nextScreenRowIndex))
                    {
                        return true;
                    }
                    SetCurrentCellCore(firstVisibleColumnIndex, nextScreenRowIndex/*, false, false*/);
                    return true;
                }

                if (!PrepareForScroll(this.CurrentColumnIndex, nextScreenRowIndex, true))
                {
                    return false;
                }
                
                if (shift && this.SelectionMode == DataGridSelectionMode.Extended)
                {
                    ClearRowSelection(nextScreenRowIndex, false /*resetAnchorRowIndex*/);
                    if (this.AnchorRowIndex == -1)
                    {
                        return true;
                    }
                    if (this.AnchorRowIndex < nextScreenRowIndex)
                    {
                        SetRowsSelection(this.AnchorRowIndex, nextScreenRowIndex);
                    }
                    else
                    {
                        SetRowsSelection(nextScreenRowIndex, this.AnchorRowIndex);
                    }
                }
                else
                {
                    ClearRowSelection(nextScreenRowIndex, true /*resetAnchorRowIndex*/);
                }
                // Scrolling needs to be done after Selection is updated
                if (!ScrollIntoView(this.CurrentColumnIndex, nextScreenRowIndex, true))
                {
                    return true;
                }
                if (this.CurrentColumnIndex == -1 || IsRowOutOfBounds(nextScreenRowIndex))
                {
                    return true;
                }
                bool success = SetCurrentCellCore(this.CurrentColumnIndex, nextScreenRowIndex/*, false, false*/);
                Debug.Assert(success);
                return true;
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }
        }

        internal bool ProcessPriorKey()
        {
            bool ctrl;
            bool shift;
            bool success;

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);

            DataGridColumn dataGridColumn = this.ColumnsInternal.FirstVisibleColumn;
            int firstVisibleColumnIndex = (dataGridColumn == null) ? -1 : dataGridColumn.Index;
            if (firstVisibleColumnIndex == -1)
            {
                return false;
            }
            int previousScreenRowIndexTmp, previousScreenRowIndex = -1;
            if (this.CurrentRowIndex == -1)
            {
                previousScreenRowIndex = this.DisplayData.FirstDisplayedScrollingRow;
                if (previousScreenRowIndex == -1)
                {
                    return false;
                }
            }
            else
            {
                previousScreenRowIndex = this.CurrentRowIndex;
            }

            int jumpRows = this.DisplayData.NumTotallyDisplayedScrollingRows;
            if (jumpRows == 0)
            {
                jumpRows = 1;
            }
            previousScreenRowIndexTmp = previousScreenRowIndex;
            Debug.Assert(previousScreenRowIndexTmp != -1);
            while (jumpRows > 0 && previousScreenRowIndexTmp != -1)
            {
                previousScreenRowIndexTmp = GetPreviousRow(previousScreenRowIndex);
                if (previousScreenRowIndexTmp != -1)
                {
                    previousScreenRowIndex = previousScreenRowIndexTmp;
                }
                jumpRows--;
            }

            Debug.Assert(previousScreenRowIndex != -1);

            this._noSelectionChangeCount++;
            try
            {
                if (this.CurrentColumnIndex == -1)
                {
                    Debug.Assert(_selectedItems.Count == 0);
                    SetRowSelection(previousScreenRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                    success = ScrollIntoView(firstVisibleColumnIndex, previousScreenRowIndex, false);
                    Debug.Assert(success);
                    if (IsInnerCellOutOfBounds(firstVisibleColumnIndex, previousScreenRowIndex))
                    {
                        return true;
                    }
                    success = SetCurrentCellCore(firstVisibleColumnIndex, previousScreenRowIndex/*, false, false*/);
                    // 
                    return true;
                }

                if (!ScrollIntoView(this.CurrentColumnIndex, previousScreenRowIndex, true))
                {
                    return true;
                }
                if (this.CurrentColumnIndex == -1 || IsRowOutOfBounds(previousScreenRowIndex))
                {
                    return true;
                }
                if (shift && this.SelectionMode == DataGridSelectionMode.Extended)
                {
                    ClearRowSelection(previousScreenRowIndex, false /*resetAnchorRowIndex*/);
                    if (this.AnchorRowIndex == -1)
                    {
                        return true;
                    }
                    if (this.AnchorRowIndex < previousScreenRowIndex)
                    {
                        SetRowsSelection(this.AnchorRowIndex, previousScreenRowIndex);
                    }
                    else
                    {
                        SetRowsSelection(previousScreenRowIndex, this.AnchorRowIndex);
                    }
                }
                else
                {
                    ClearRowSelection(previousScreenRowIndex, true /*resetAnchorRowIndex*/);
                }
                success = SetCurrentCellCore(this.CurrentColumnIndex, previousScreenRowIndex/*, false, false*/);
                Debug.Assert(success);
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }
            return true;
        }

        internal bool ProcessRightKey()
        {
            bool success;
            bool ctrl;
            bool shift;

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);

            DataGridColumn dataGridColumn = this.ColumnsInternal.LastVisibleColumn;
            int lastVisibleColumnIndex = (dataGridColumn == null) ? -1 : dataGridColumn.Index;
            int firstRowIndex = this.FirstRowIndex;
            if (lastVisibleColumnIndex == -1 || firstRowIndex == -1)
            {
                return false;
            }
            int nextVisibleColumnIndex = -1;
            if (this.CurrentColumnIndex != -1)
            {
                dataGridColumn = this.ColumnsInternal.GetNextVisibleColumn(this.Columns[this.CurrentColumnIndex]);
                if (dataGridColumn != null)
                {
                    nextVisibleColumnIndex = dataGridColumn.Index;
                }
            }
            this._noSelectionChangeCount++;
            try
            {
                if (ctrl)
                {
                    return ProcessRightMost(lastVisibleColumnIndex, firstRowIndex);
                }
                else
                {
                    if (this.CurrentColumnIndex == -1)
                    {
                        Debug.Assert(_selectedItems.Count == 0);
                        SetRowSelection(firstRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                        success = ScrollIntoView(lastVisibleColumnIndex, firstRowIndex, false);
                        Debug.Assert(success);
                        if (IsInnerCellOutOfBounds(lastVisibleColumnIndex, firstRowIndex))
                        {
                            return true;
                        }
                        success = SetCurrentCellCore(lastVisibleColumnIndex, firstRowIndex/*, false, false*/);
                        Debug.Assert(success);
                    }
                    else
                    {
                        if (nextVisibleColumnIndex == -1)
                        {
                            return true;
                        }
                        if (!ScrollIntoView(nextVisibleColumnIndex, this.CurrentRowIndex, true))
                        {
                            return true;
                        }
                        if (IsRowOutOfBounds(this.CurrentRowIndex) || IsColumnOutOfBounds(nextVisibleColumnIndex))
                        {
                            return true;
                        }
                        success = SetCurrentCellCore(nextVisibleColumnIndex, this.CurrentRowIndex/*, false, false*/);
                        Debug.Assert(success);
                    }
                }
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }
            return true;
        }

        [SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
        internal bool ProcessUpKey()
        {
            bool ctrl;
            bool shift;
            bool success;

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);

            DataGridColumn dataGridColumn = this.ColumnsInternal.FirstVisibleColumn;
            int firstVisibleColumnIndex = (dataGridColumn == null) ? -1 : dataGridColumn.Index;
            int firstRowIndex = this.FirstRowIndex;
            if (firstVisibleColumnIndex == -1 || firstRowIndex == -1)
            {
                return false;
            }
            int previousRowIndex = -1;
            if (this.CurrentRowIndex != -1)
            {
                previousRowIndex = GetPreviousRow(this.CurrentRowIndex);
                // 








            }
            // 

















            this._noSelectionChangeCount++;

            try
            {
                if (ctrl)
                {
                    if (shift)
                    {
                        if (this.CurrentColumnIndex == -1)
                        {
                            Debug.Assert(_selectedItems.Count == 0);
                            SetRowSelection(firstRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                            success = ScrollIntoView(firstVisibleColumnIndex, firstRowIndex, false);
                            Debug.Assert(success);
                            if (IsInnerCellOutOfBounds(firstVisibleColumnIndex, firstRowIndex))
                            {
                                return true;
                            }
                            success = SetCurrentCellCore(firstVisibleColumnIndex, firstRowIndex/*, false, false*/);
                            Debug.Assert(success);
                        }
                        else
                        {
                            if (this.SelectionMode == DataGridSelectionMode.Extended)
                            {
                                if (!ScrollIntoView(this.CurrentColumnIndex, firstRowIndex, true))
                                {
                                    return true;
                                }
                                if (/*this.ptAnchorCell.X == -1 ||*/ this.CurrentColumnIndex == -1 ||
                                    IsRowOutOfBounds(firstRowIndex))
                                {
                                    return true;
                                }
                                ClearRowSelection(firstRowIndex, false /*resetAnchorRowIndex*/);
                                Debug.Assert(this.AnchorRowIndex >= 0);
                                SetRowsSelection(firstRowIndex, this.AnchorRowIndex);
                                success = SetCurrentCellCore(this.CurrentColumnIndex, firstRowIndex/*, false, false*/);
                                Debug.Assert(success);
                            }
                            else
                            {
                                if (!ScrollIntoView(this.CurrentColumnIndex, firstRowIndex, true))
                                {
                                    return true;
                                }
                                if (this.CurrentColumnIndex == -1 || IsRowOutOfBounds(firstRowIndex))
                                {
                                    return true;
                                }
                                if (firstRowIndex != this.SelectedIndex)
                                {
                                    SetRowSelection(this.CurrentRowIndex, false /*isSelected*/, false /*setAnchorRowIndex*/);
                                    SetRowSelection(firstRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                                }
                                success = SetCurrentCellCore(this.CurrentColumnIndex, firstRowIndex/*, false, false*/);
                                Debug.Assert(success);
                            }
                        }
                    }
                    else
                    {
                        if (this.CurrentColumnIndex == -1)
                        {
                            Debug.Assert(_selectedItems.Count == 0);
                            SetRowSelection(firstRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                            success = ScrollIntoView(firstVisibleColumnIndex, firstRowIndex, false);
                            Debug.Assert(success);
                            if (IsInnerCellOutOfBounds(firstVisibleColumnIndex, firstRowIndex))
                            {
                                return true;
                            }
                            success = SetCurrentCellCore(firstVisibleColumnIndex, firstRowIndex/*, false, false*/);
                            // 
                        }
                        else
                        {
                            if (!ScrollIntoView(this.CurrentColumnIndex, firstRowIndex, true))
                            {
                                return true;
                            }
                            if (this.CurrentColumnIndex == -1 || IsRowOutOfBounds(firstRowIndex))
                            {
                                return true;
                            }
                            ClearRowSelection(firstRowIndex, true /*resetAnchorRowIndex*/);
                            success = SetCurrentCellCore(this.CurrentColumnIndex, firstRowIndex/*, false, false*/);
                            Debug.Assert(success);
                        }
                    }
                }
                else
                {
                    if (shift)
                    {
                        if (this.CurrentColumnIndex == -1)
                        {
                            Debug.Assert(_selectedItems.Count == 0);
                            SetRowSelection(firstRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                            success = ScrollIntoView(firstVisibleColumnIndex, firstRowIndex, false);
                            Debug.Assert(success);
                            if (IsInnerCellOutOfBounds(firstVisibleColumnIndex, firstRowIndex))
                            {
                                return true;
                            }
                            success = SetCurrentCellCore(firstVisibleColumnIndex, firstRowIndex/*, false, false*/);
                            Debug.Assert(success);
                        }
                        else
                        {
                            if (previousRowIndex == -1)
                            {
                                return true;
                            }
                            if (!ScrollIntoView(this.CurrentColumnIndex, previousRowIndex, true))
                            {
                                return true;
                            }
                            if (this.CurrentColumnIndex == -1 || IsRowOutOfBounds(previousRowIndex))
                            {
                                return true;
                            }
                            ClearRowSelection(false /*resetAnchorRowIndex*/);
                            if (this.SelectionMode == DataGridSelectionMode.Extended)
                            {
                                if (this.AnchorRowIndex == -1)
                                {
                                    return true;
                                }
                                if (this.AnchorRowIndex >= previousRowIndex)
                                {
                                    SetRowsSelection(previousRowIndex, this.AnchorRowIndex);
                                }
                                else
                                {
                                    SetRowsSelection(this.AnchorRowIndex, previousRowIndex);
                                }
                            }
                            else
                            {
                                SetRowSelection(previousRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                            }
                            success = this.SetCurrentCellCore(this.CurrentColumnIndex, previousRowIndex/*, false, false*/);
                            Debug.Assert(success);
                        }
                    }
                    else
                    {
                        if (this.CurrentColumnIndex == -1)
                        {
                            Debug.Assert(_selectedItems.Count == 0);
                            SetRowSelection(firstRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                            success = ScrollIntoView(firstVisibleColumnIndex, firstRowIndex, false);
                            Debug.Assert(success);
                            if (IsInnerCellOutOfBounds(firstVisibleColumnIndex, firstRowIndex))
                            {
                                return true;
                            }
                            success = SetCurrentCellCore(firstVisibleColumnIndex, firstRowIndex/*, false, false*/);
                            // 
                        }
                        else
                        {
                            // 











                            if (previousRowIndex == -1)
                            {
                                return true;
                            }
                            // 
                            if (previousRowIndex != -1)
                            {
                                if (!ScrollIntoView(this.CurrentColumnIndex, previousRowIndex, true))
                                {
                                    return true;
                                }
                            }
                            if (this.CurrentColumnIndex == -1 || IsRowOutOfBounds(previousRowIndex))
                            {
                                return true;
                            }
                            ClearRowSelection(previousRowIndex, true /*resetAnchorRowIndex*/);
                            success = SetCurrentCellCore(this.CurrentColumnIndex, previousRowIndex/*, false, false*/);
                            Debug.Assert(success);
                        }
                    }
                }
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }
            return true;
        }

        internal void ProcessVerticalScroll(ScrollEventType scrollEventType)
        {
            if (this._verticalScrollChangesIgnored > 0)
            {
                return;
            }
            // 
            Debug.Assert(DoubleUtil.LessThanOrClose(this._vScrollBar.Value, this._vScrollBar.Maximum));

            this._verticalScrollChangesIgnored++;
            try
            {
                Debug.Assert(this._vScrollBar != null);
                if (scrollEventType == ScrollEventType.SmallIncrement)
                {
                    this.DisplayData.PendingVerticalScrollHeight = GetVerticalSmallScrollIncrease();
                    double newVerticalOffset = this._verticalOffset + this.DisplayData.PendingVerticalScrollHeight;
                    if (newVerticalOffset > this._vScrollBar.Maximum)
                    {
                        this.DisplayData.PendingVerticalScrollHeight -= newVerticalOffset - this._vScrollBar.Maximum;
                    }
                }
                else if (scrollEventType == ScrollEventType.SmallDecrement)
                {
                    if (DoubleUtil.GreaterThan(this.NegVerticalOffset, 0))
                    {
                        this.DisplayData.PendingVerticalScrollHeight -= this.NegVerticalOffset;
                    }
                    else
                    {
                        if (this.DisplayData.FirstDisplayedScrollingRow > 0)
                        {
                            ScrollRowIntoView(this.DisplayData.FirstDisplayedScrollingRow - 1);
                        }
                        return;
                    }
                }
                else
                {
                    this.DisplayData.PendingVerticalScrollHeight = this._vScrollBar.Value - this._verticalOffset;
                }

                if (!DoubleUtil.IsZero(this.DisplayData.PendingVerticalScrollHeight))
                {
                    // Invalidate so the scroll happens on idle
                    InvalidateRowsMeasure(false /*invalidateIndividualRows*/);
                }
                // 
            }
            finally
            {
                this._verticalScrollChangesIgnored--;
            }
        }

        internal void RefreshRowsAndColumns()
        {
            ClearRows(false);
            if (this.AutoGenerateColumns)
            {
                //Column auto-generation refreshes the rows too
                AutoGenerateColumnsPrivate();
            }
            foreach (DataGridColumn column in this.ColumnsItemsInternal)
            {
                //We don't need to refresh the state of AutoGenerated column headers because they're up-to-date
                if (!column.IsAutoGenerated && column.HasHeaderCell)
                {
                    column.HeaderCell.ApplyState();
                }
            }

            RefreshRows(false);

            if (this.Columns.Count > 0 && this.CurrentColumnIndex == -1)
            {
                MakeFirstDisplayedCellCurrentCell();
            }
        }

        //
        internal bool ScrollIntoView(int columnIndex, int rowIndex, bool forCurrentCellChange)
        {
            Debug.Assert(columnIndex >= 0 && columnIndex < this.Columns.Count);
            Debug.Assert(this.DisplayData.FirstDisplayedScrollingCol >= -1 && this.DisplayData.FirstDisplayedScrollingCol < this.Columns.Count);
            Debug.Assert(this.DisplayData.LastTotallyDisplayedScrollingCol >= -1 && this.DisplayData.LastTotallyDisplayedScrollingCol < this.Columns.Count);
            Debug.Assert(!IsRowOutOfBounds(rowIndex));
            Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow >= -1 && this.DisplayData.FirstDisplayedScrollingRow < this.RowCount);
            Debug.Assert(this.Columns[columnIndex].Visibility == Visibility.Visible);

            if (!PrepareForScroll(columnIndex, rowIndex, forCurrentCellChange))
            {
                return false;
            }

            //scroll horizontally
            if (!ScrollColumnIntoView(columnIndex))
            {
                return false;
            }

            if (IsInnerCellOutOfBounds(columnIndex, rowIndex))
            {
                return false;
            }

            //scroll vertically
            if (!ScrollRowIntoView(rowIndex))
            {
                return false;
            }

            DataGridAutomationPeer peer = DataGridAutomationPeer.FromElement(this) as DataGridAutomationPeer;
            if (peer != null)
            {
                peer.RaiseAutomationScrollEvents();
            }
            return true;
        }

        // Convenient overload that commits the current edit.
        internal bool SetCurrentCellCore(int columnIndex, int rowIndex)
        {
            return SetCurrentCellCore(columnIndex, rowIndex, true /*commitEdit*/);
        }

        internal void UpdateHorizontalOffset(double newValue)
        {
            if (this.HorizontalOffset != newValue)
            {
                this.HorizontalOffset = newValue;

                InvalidateColumnHeadersMeasure();
                InvalidateRowsMeasure(true);
            }
        }

        [SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
        internal bool UpdateStateOnMouseLeftButtonDown(MouseButtonEventArgs mouseButtonEventArgs, int columnIndex, int rowIndex)
        {
            bool ctrl, shift, beginEdit;

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);

            Debug.Assert(rowIndex >= 0);

            // Before changing selection, check if the current cell needs to be committed, and
            // check if the current row needs to be committed. If any of those two operations are required and fail, 
            // do no change selection, and do not change current cell.

            bool wasInEdit = this.EditingColumnIndex != -1;

            if (this.CurrentRowIndex != rowIndex && !CommitEdit(DataGridEditingUnit.Row, true /*exitEditing*/))
            {
                // Edited value couldn't be committed or aborted
                return true;
            }
            if (IsRowOutOfBounds(rowIndex))
            {
                return true;
            }

            try
            {
                this._noSelectionChangeCount++;
                if (this.SelectionMode == DataGridSelectionMode.Extended && shift && this.AnchorRowIndex != -1)
                {
                    // Shift select multiple rows
                    int anchorRowIndex = this.AnchorRowIndex;
                    ClearRowSelection(rowIndex /*rowIndexException*/, false /*resetAnchorRowIndex*/);
                    if (rowIndex <= anchorRowIndex)
                    {
                        SetRowsSelection(rowIndex, anchorRowIndex);
                    }
                    else
                    {
                        SetRowsSelection(anchorRowIndex, rowIndex);
                    }
                }
                else if (GetRowSelection(rowIndex))  // Unselecting single row or Selecting a previously multi-selected row
                {
                    if (!ctrl && this.SelectionMode == DataGridSelectionMode.Extended && _selectedItems.Count != 0)
                    {
                        // Unselect everything except the row that was clicked on
                        ClearRowSelection(rowIndex /*rowIndexException*/, true /*setAnchorRowIndex*/);
                    }
                    else if (ctrl)
                    {
                        if (!CommitEdit(DataGridEditingUnit.Row, true /*exitEditing*/))
                        {
                            // Edited value couldn't be committed or aborted
                            return true;
                        }
                        SetRowSelection(rowIndex, false /*isSelected*/, false /*setAnchorRowIndex*/);
                    }
                }
                else // Selecting a single row or multi-selecting with Ctrl
                {
                    if (this.SelectionMode == DataGridSelectionMode.Single || !ctrl)
                    {
                        // Unselect the currectly selected rows except the new selected row
                        ClearRowSelection(rowIndex /*rowIndexException*/, true /*setAnchorRowIndex*/);
                    }
                    else
                    {
                        SetRowSelection(rowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                    }
                }

                if (IsRowOutOfBounds(rowIndex) || (columnIndex != -1 && IsColumnOutOfBounds(columnIndex)))
                {
                    return true;
                }

                beginEdit = this.CurrentRowIndex == rowIndex &&
                            columnIndex != -1 &&
                            (wasInEdit || this.CurrentColumnIndex == columnIndex) &&
                            !GetColumnEffectiveReadOnlyState(this.Columns[columnIndex]);

                if (this.CurrentRowIndex != rowIndex ||
                    (this.CurrentColumnIndex != columnIndex && columnIndex != -1))
                {
                    if (columnIndex == -1)
                    {
                        if (this.CurrentColumnIndex != -1)
                        {
                            columnIndex = this.CurrentColumnIndex;
                        }
                        else
                        {
                            DataGridColumn firstVisibleColumn = this.ColumnsInternal.FirstVisibleColumn;
                            if (firstVisibleColumn != null)
                            {
                                columnIndex = firstVisibleColumn.Index;
                            }
                        }
                    }
                    if (columnIndex != -1)
                    {
                        bool success = SetCurrentCellCore(columnIndex, rowIndex);
                        Debug.Assert(success);
                        success = ScrollRowIntoView(rowIndex);
                        Debug.Assert(success);
                    }
                }
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }

            if (beginEdit && BeginCellEdit(mouseButtonEventArgs))
            {
                FocusEditingCell(true /*setFocus*/);
            }

            return true;
        }

        internal void UpdateVerticalScrollBar()
        {
            if (this._vScrollBar != null && this._vScrollBar.Visibility == Visibility.Visible)
            {
                UpdateVerticalScrollBar(true /*needVertScrollbar*/, false /*forceVertScrollbar*/, this.EdgedRowsHeightCalculated, this.CellsHeight);
            }
        }

        #endregion Internal Methods

        #region Private Methods

        private void AddNewCellPrivate(DataGridRow row, DataGridColumn column)
        {
            DataGridCell newCell = new DataGridCell();
            PopulateCellContent(true /*forceTemplating*/, false /*isCellEdited*/, column, row, newCell);
            if (row.OwningGrid != null)
            {
                newCell.OwningColumn = column;
                newCell.Visibility = column.Visibility;
            }
            newCell.EnsureCellStyle();
            row.Cells.Insert(column.Index, newCell);
        }

        // 





















        private bool BeginCellEdit(RoutedEventArgs editingEventArgs)
        {
            // 
            if (this.CurrentColumnIndex == -1 || !GetRowSelection(this.CurrentRowIndex))
            {
                // 
                return false;
            }

            Debug.Assert(this.CurrentColumnIndex >= 0);
            Debug.Assert(this.CurrentColumnIndex < this.Columns.Count);
            Debug.Assert(this.CurrentRowIndex >= -1);
            Debug.Assert(this.CurrentRowIndex < this.RowCount);
            Debug.Assert(this._editingRow == null || this.EditingRowIndex == this.CurrentRowIndex);
            Debug.Assert(!GetColumnEffectiveReadOnlyState(this.CurrentColumn));
            Debug.Assert(this.CurrentColumn.Visibility == Visibility.Visible);

            if (this._editingColumnIndex != -1)
            {
                // Current cell is already in edit mode
                Debug.Assert(this._editingColumnIndex == this.CurrentColumnIndex);
                // 
                return true;
            }

            DataGridRow dataGridRow = this._editingRow;
            if (dataGridRow == null)
            {
                if (this.DisplayData.IsRowDisplayed(this.CurrentRowIndex))
                {
                    dataGridRow = this.DisplayData.GetDisplayedRow(this.CurrentRowIndex);
                }
                else
                {
                    dataGridRow = GenerateRow(this.CurrentRowIndex);
                }
            }
            Debug.Assert(dataGridRow != null);
            DataGridCell dataGridCell = dataGridRow.Cells[this.CurrentColumnIndex];
            DataGridBeginningEditEventArgs e = new DataGridBeginningEditEventArgs(this.CurrentColumn, dataGridRow, editingEventArgs);
            OnBeginningEdit(e);
            if (e.Cancel)
            {
                // 
                return false;
            }

            if (this._editingRow == null && !BeginRowEdit(dataGridRow))
            {
                // 
                return false;
            }
            Debug.Assert(this._editingRow != null);
            Debug.Assert(this.EditingRowIndex == this.CurrentRowIndex);

            this._editingColumnIndex = this.CurrentColumnIndex;
            this._editingEventArgs = editingEventArgs;
            this._editingRow.Cells[this.CurrentColumnIndex].ApplyCellState(true /*animate*/);
            PopulateCellContent(false /*forceTemplating*/, true /*isCellEdited*/, this.CurrentColumn, dataGridRow, dataGridCell);
            // 
            return true;
        }

        private bool BeginRowEdit(DataGridRow dataGridRow)
        {
            // 
            Debug.Assert(this._editingRow == null);
            Debug.Assert(dataGridRow != null);

            Debug.Assert(this.CurrentRowIndex >= -1);
            Debug.Assert(this.CurrentRowIndex < this.RowCount);

            if (this.DataConnection.BeginEdit(dataGridRow.DataContext))
            {
                this._editingRow = dataGridRow;
                this._editingRow.ApplyState(true /*animate*/);
                // 
                return true;
             }
            // 
            return false;
        }

        private void CancelCellEdit(bool exitEditingMode)
        {
            if (this._editingColumnIndex == -1)
            {
                return;
            }

            Debug.Assert(this._editingColumnIndex >= 0);
            Debug.Assert(this._editingColumnIndex < this.ColumnsItemsInternal.Count);
            Debug.Assert(this._editingRow != null);
            Debug.Assert(this.CurrentColumn != null);

            // 








            // Write the old cell value back into the cell content.
            DataGridCell editingCell = this._editingRow.Cells[this._editingColumnIndex];
            Debug.Assert(editingCell != null);
            FrameworkElement editingElement = editingCell.Content as FrameworkElement;
            DataGridTemplateColumn templateColumn = this.CurrentColumn as DataGridTemplateColumn;
            if (templateColumn != null)
            {
                PopulateCellContent(true /*forceTemplating*/, !exitEditingMode /*isCellEdited*/, templateColumn, this._editingRow, editingCell);
            }
            else
            {
                this.CurrentColumn.CancelCellEditInternal(editingElement, this._uneditedValue);
            }
        }

        private void CancelRowEdit(bool exitEditingMode)
        {
            if (this._editingRow == null)
            {
                return;
            }
            Debug.Assert(this.EditingRowIndex >= -1);
            Debug.Assert(this.EditingRowIndex < this.RowCount);
            Debug.Assert(this.CurrentColumn != null);

            object dataItem = this._editingRow.DataContext;
            if (!this.DataConnection.CancelEdit(dataItem))
            {
                return;
            }
            // 

            foreach (DataGridColumn column in this.Columns)
            {
                if (!exitEditingMode && column.Index == this._editingColumnIndex && column is DataGridBoundColumn)
                {
                    continue;
                }
                PopulateCellContent(true /*forceTemplating*/, !exitEditingMode && column.Index == this._editingColumnIndex /*isCellEdited*/, column, this._editingRow, this._editingRow.Cells[column.Index]);
            }
        }

        private bool CommitEditForOperation(int columnIndex, int rowIndex, bool forCurrentCellChange)
        {
            if (forCurrentCellChange)
            {
                if (!EndCellEdit(true /*commitCellEdit*/, true /*exitEditingMode*/, true /*keepFocus*/))
                {
                    return false;
                }
                if (this.CurrentRowIndex != rowIndex &&
                    !EndRowEdit(true /*commitRowEdit*/, true /*exitEditingMode*/))
                {
                    return false;
                }
            }
            // 

            if (IsColumnOutOfBounds(columnIndex))
            {
                return false;
            }
            if (rowIndex >= this.RowCount)
            {
                // Current cell was reset because the commit deleted row(s).
                // Since the user wants to change the current cell, we don't
                // want to end up with no current cell. We pick the last row 
                // in the grid which may be the 'new row'.
                int lastRowIndex = this.LastRowIndex;
                if (forCurrentCellChange &&
                    this.CurrentColumnIndex == -1 &&
                    lastRowIndex != -1)
                {
                    bool success = SetAndSelectCurrentCell(columnIndex,
                                                           lastRowIndex,
                                                           false /*forceCurrentCellSelection (unused here)*/);
                    Debug.Assert(success);
                }
                // Interrupt operation because it has become invalid.
                return false;
            }
            return true;
        }

        [SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes")]
        private void CommitRowEdit(bool exitEditingMode)
        {
            if (this._editingRow == null)
            {
                return;
            }
            Debug.Assert(this.EditingRowIndex >= -1);
            Debug.Assert(this.EditingRowIndex < this.RowCount);

            // 



            this.DataConnection.EndEdit(this._editingRow.DataContext);

            if (!exitEditingMode)
            {
                this.DataConnection.BeginEdit(this._editingRow.DataContext);
            }

            this.DataConnection.EndEdit(this._editingRow.DataContext);
        }

        private void CompleteCellsCollection(DataGridRow dataGridRow)
        {
            Debug.Assert(dataGridRow != null);
            int cellsInCollection = dataGridRow.Cells.Count;
            if (this.ColumnsItemsInternal.Count > cellsInCollection)
            {
                for (int columnIndex = cellsInCollection; columnIndex < this.ColumnsItemsInternal.Count; columnIndex++)
                {
                    AddNewCellPrivate(dataGridRow, this.ColumnsItemsInternal[columnIndex]);
                }
            }
        }

        [SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
        private void ComputeScrollBarsLayout()
        {
            if (this._ignoreNextScrollBarsLayout)
            {
                this._ignoreNextScrollBarsLayout = false;
                // 


            }
            double cellsWidth = this.CellsWidth;
            double cellsHeight = this.CellsHeight;

            bool allowHorizScrollbar = false;
            bool forceHorizScrollbar = false;
            double horizScrollBarHeight = 0;
            if (_hScrollBar != null)
            {
                forceHorizScrollbar = this.HorizontalScrollBarVisibility == ScrollBarVisibility.Visible;
                allowHorizScrollbar = forceHorizScrollbar || (this.ColumnsInternal.VisibleColumnCount > 0 &&
                    this.HorizontalScrollBarVisibility != ScrollBarVisibility.Disabled &&
                    this.HorizontalScrollBarVisibility != ScrollBarVisibility.Hidden);
                // Compensate if the horizontal scrollbar is already taking up space
                if (_hScrollBar.Visibility == Visibility.Visible)
                {
                    cellsHeight += this._hScrollBar.DesiredSize.Height;
                }
                horizScrollBarHeight = _hScrollBar.Height;
            }
            bool allowVertScrollbar = false;
            bool forceVertScrollbar = false;
            double vertScrollBarWidth = 0;
            if (_vScrollBar != null)
            {
                forceVertScrollbar = this.VerticalScrollBarVisibility == ScrollBarVisibility.Visible;
                allowVertScrollbar = forceVertScrollbar || (this.ColumnsItemsInternal.Count > 0 &&
                    this.VerticalScrollBarVisibility != ScrollBarVisibility.Disabled &&
                    this.VerticalScrollBarVisibility != ScrollBarVisibility.Hidden);
                // Compensate if the vertical scrollbar is already taking up space
                if (_vScrollBar.Visibility == Visibility.Visible)
                {
                    cellsWidth += _vScrollBar.DesiredSize.Width;
                }
                vertScrollBarWidth = _vScrollBar.Width;
            }

            // Now cellsWidth is the width potentially available for displaying data cells.
            // Now cellsHeight is the height potentially available for displaying data cells.

            bool needHorizScrollbar = false;
            bool needVertScrollbar = false;

            double totalVisibleWidth = this.ColumnsInternal.VisibleEdgedColumnsWidth;
            double totalVisibleFrozenWidth = this.ColumnsInternal.GetVisibleFrozenEdgedColumnsWidth();

            UpdateDisplayedRows(this.DisplayData.FirstDisplayedScrollingRow, this.CellsHeight);
            double totalVisibleHeight = this.EdgedRowsHeightCalculated;

            if (!forceHorizScrollbar && !forceVertScrollbar)
            {
                bool needHorizScrollbarWithoutVertScrollbar = false;

                if (allowHorizScrollbar &&
                    DoubleUtil.GreaterThan(totalVisibleWidth, cellsWidth) &&
                    DoubleUtil.LessThan(totalVisibleFrozenWidth, cellsWidth) &&
                    DoubleUtil.LessThanOrClose(horizScrollBarHeight, cellsHeight))
                {
                    double oldDataHeight = cellsHeight;
                    cellsHeight -= horizScrollBarHeight;
                    Debug.Assert(cellsHeight >= 0);
                    needHorizScrollbarWithoutVertScrollbar = needHorizScrollbar = true;
                    if (allowVertScrollbar && (DoubleUtil.LessThanOrClose(totalVisibleWidth - cellsWidth, vertScrollBarWidth) ||
                        DoubleUtil.LessThanOrClose(cellsWidth - totalVisibleFrozenWidth, vertScrollBarWidth)))
                    {
                        // Would we still need a horizontal scrollbar without the vertical one?
                        UpdateDisplayedRows(this.DisplayData.FirstDisplayedScrollingRow, cellsHeight);
                        if (this.DisplayData.NumTotallyDisplayedScrollingRows != this.RowCount)
                        {
                            needHorizScrollbar = DoubleUtil.LessThan(totalVisibleFrozenWidth, cellsWidth - vertScrollBarWidth);
                        }
                    }

                    if (!needHorizScrollbar)
                    {
                        // Restore old data height because turns out a horizontal scroll bar wouldn't make sense
                        cellsHeight = oldDataHeight;
                    }
                }

                UpdateDisplayedRows(this.DisplayData.FirstDisplayedScrollingRow, cellsHeight);
                if (allowVertScrollbar &&
                    DoubleUtil.GreaterThan(cellsHeight, 0) &&
                    DoubleUtil.LessThanOrClose(vertScrollBarWidth, cellsWidth) &&
                    this.DisplayData.NumTotallyDisplayedScrollingRows != this.RowCount)
                {
                    cellsWidth -= vertScrollBarWidth;
                    Debug.Assert(cellsWidth >= 0);
                    needVertScrollbar = true;
                }

                this.DisplayData.FirstDisplayedScrollingCol = ComputeFirstVisibleScrollingColumn();
                // we compute the number of visible columns only after we set up the vertical scroll bar.
                ComputeDisplayedColumns();

                if (allowHorizScrollbar &&
                    needVertScrollbar && !needHorizScrollbar &&
                    DoubleUtil.GreaterThan(totalVisibleWidth, cellsWidth) &&
                    DoubleUtil.LessThan(totalVisibleFrozenWidth, cellsWidth) &&
                    DoubleUtil.LessThanOrClose(horizScrollBarHeight, cellsHeight))
                {
                    cellsWidth += vertScrollBarWidth;
                    cellsHeight -= horizScrollBarHeight;
                    Debug.Assert(cellsHeight >= 0);
                    needVertScrollbar = false;

                    UpdateDisplayedRows(this.DisplayData.FirstDisplayedScrollingRow, cellsHeight);
                    if (cellsHeight > 0 &&
                        vertScrollBarWidth <= cellsWidth &&
                        this.DisplayData.NumTotallyDisplayedScrollingRows != this.RowCount)
                    {
                        cellsWidth -= vertScrollBarWidth;
                        Debug.Assert(cellsWidth >= 0);
                        needVertScrollbar = true;
                    }
                    if (needVertScrollbar)
                    {
                        needHorizScrollbar = true;
                    }
                    else
                    {
                        needHorizScrollbar = needHorizScrollbarWithoutVertScrollbar;
                    }
                }
            }
            else if (forceHorizScrollbar && !forceVertScrollbar)
            {
                if (allowVertScrollbar)
                {
                    if (cellsHeight > 0 &&
                        DoubleUtil.LessThanOrClose(vertScrollBarWidth, cellsWidth) &&
                        this.DisplayData.NumTotallyDisplayedScrollingRows != this.RowCount)
                    {
                        cellsWidth -= vertScrollBarWidth;
                        Debug.Assert(cellsWidth >= 0);
                        needVertScrollbar = true;
                    }
                    this.DisplayData.FirstDisplayedScrollingCol = ComputeFirstVisibleScrollingColumn();
                    ComputeDisplayedColumns();
                }
                needHorizScrollbar = totalVisibleWidth > cellsWidth && totalVisibleFrozenWidth < cellsWidth;
            }
            else if (!forceHorizScrollbar && forceVertScrollbar)
            {
                if (allowHorizScrollbar)
                {
                    if (cellsWidth > 0 &&
                        DoubleUtil.LessThanOrClose(horizScrollBarHeight, cellsHeight) &&
                        DoubleUtil.GreaterThan(totalVisibleWidth, cellsWidth) &&
                        DoubleUtil.LessThan(totalVisibleFrozenWidth, cellsWidth))
                    {
                        cellsHeight -= horizScrollBarHeight;
                        Debug.Assert(cellsHeight >= 0);
                        needHorizScrollbar = true;
                        UpdateDisplayedRows(this.DisplayData.FirstDisplayedScrollingRow, cellsHeight);
                    }
                    this.DisplayData.FirstDisplayedScrollingCol = ComputeFirstVisibleScrollingColumn();
                    ComputeDisplayedColumns();
                }
                needVertScrollbar = this.DisplayData.NumTotallyDisplayedScrollingRows != this.RowCount;
            }
            else
            {
                Debug.Assert(forceHorizScrollbar && forceVertScrollbar);
                Debug.Assert(allowHorizScrollbar && allowVertScrollbar);
                this.DisplayData.FirstDisplayedScrollingCol = ComputeFirstVisibleScrollingColumn();
                ComputeDisplayedColumns();
                needVertScrollbar = this.DisplayData.NumTotallyDisplayedScrollingRows != this.RowCount;
                needHorizScrollbar = totalVisibleWidth > cellsWidth && totalVisibleFrozenWidth < cellsWidth;
            }

            UpdateHorizontalScrollBar(needHorizScrollbar, forceHorizScrollbar, totalVisibleWidth, totalVisibleFrozenWidth, cellsWidth);
            UpdateVerticalScrollBar(needVertScrollbar, forceVertScrollbar, totalVisibleHeight, cellsHeight);

            if (this._topRightCornerHeader != null)
            {
                // Show the TopRightHeaderCell based on vertical ScrollBar visibility
                if (this.AreColumnHeadersVisible &&
                    this._vScrollBar != null && this._vScrollBar.Visibility == Visibility.Visible)
                {
                    this._topRightCornerHeader.Visibility = Visibility.Visible;
                }
                else
                {
                    this._topRightCornerHeader.Visibility = Visibility.Collapsed;
                }
            }
            this.DisplayData.FullyRecycleRows();
        }

        // Makes sure horizontal layout is updated to reflect any changes that affect it
        private void EnsureHorizontalLayout()
        {
            this.ColumnsInternal.EnsureVisibleEdgedColumnsWidth();
            InvalidateColumnHeadersMeasure();
            InvalidateRowsMeasure(true);
            InvalidateMeasure();
        }

        private void EnsureRowHeaderWidth()
        {
            if (this.AreRowHeadersVisible)
            {
                if (this.AreColumnHeadersVisible)
                {
                    EnsureTopLeftCornerHeader();
                }

                bool updated = false;
                if (_rowsPresenter != null)
                {
                    foreach (DataGridRow row in _rowsPresenter.Children)
                    {
                        // If the RowHeader resulted in a different width the last time it was measured, we need
                        // to re-measure it
                        if (row.HeaderCell != null && row.HeaderCell.DesiredSize.Width != this.ActualRowHeaderWidth)
                        {
                            row.HeaderCell.InvalidateMeasure();
                            updated = true;
                        }
                    }
                }
                if (updated)
                {
                    // We need to update the width of the horizontal scrollbar if the rowHeaders' width actually changed
                    InvalidateMeasure();
                }
            }
        }

        private void EnsureRowsPresenterVisibility()
        {
            if (_rowsPresenter != null)
            {
                // RowCount doesn't need to be considered, doing so might cause extra Visibility changes
                _rowsPresenter.Visibility = this.ColumnsInternal.FirstVisibleColumn == null ? Visibility.Collapsed : Visibility.Visible;
            }
        }

        private void EnsureTopLeftCornerHeader()
        {
            if (_topLeftCornerHeader != null)
            {
                _topLeftCornerHeader.Visibility = this.HeadersVisibility == DataGridHeadersVisibility.All ? Visibility.Visible : Visibility.Collapsed;

                if (_topLeftCornerHeader.Visibility == Visibility.Visible)
                {
                    if (!double.IsNaN(this.RowHeaderWidth))
                    {
                        // RowHeaderWidth is set explicitly so we should use that
                        _topLeftCornerHeader.Width = this.RowHeaderWidth;
                    }
                    else if (this.RowCount > 0)
                    {
                        // RowHeaders AutoSize and we have at least 1 row so take the desired width
                        _topLeftCornerHeader.Width = this.RowHeadersDesiredWidth;
                    }
                }
            }
        }

        private void InvalidateCellsArrange()
        {
            if (_rowsPresenter != null)
            {
                foreach (DataGridRow row in _rowsPresenter.Children)
                {
                    row.InvalidateHorizontalArrange();
                }
            }
        }
        
        private void InvalidateColumnHeadersArrange()
        {
            if (_columnHeadersPresenter != null)
            {
                _columnHeadersPresenter.InvalidateArrange();
            }
        }

        private void InvalidateColumnHeadersMeasure()
        {
            if (_columnHeadersPresenter != null)
            {
                EnsureColumnHeadersVisibility();
                _columnHeadersPresenter.InvalidateMeasure();
            }
        }
        
        private void InvalidateRowsArrange()
        {
            if (_rowsPresenter != null)
            {
                _rowsPresenter.InvalidateArrange();
            }
        }

        private void InvalidateRowsMeasure(bool invalidateIndividualRows)
        {
            if (_rowsPresenter != null)
            {
                _rowsPresenter.InvalidateMeasure();

                if (invalidateIndividualRows)
                {
                    foreach (UIElement child in _rowsPresenter.Children)
                    {
                        child.InvalidateMeasure();
                    }
                }
            }
        }

        private void DataGrid_GotFocus(object sender, RoutedEventArgs e)
        {
            if (!this.ContainsFocus)
            {
                // 
                this._focusedRow = null;
                this.ContainsFocus = true;
                Control focusedControl = e.OriginalSource as Control;
                if (focusedControl != null && focusedControl != this)
                {
                    DataGridCell dataGridCell = GetOwningCell(focusedControl);
                    if (dataGridCell != null && dataGridCell.OwningColumn is DataGridTemplateColumn)
                    {
                        this._editingTemplateControl = focusedControl;
                    }
                }
                ApplyDisplayedRowsState(this.DisplayData.FirstDisplayedScrollingRow, this.DisplayData.LastDisplayedScrollingRow);
                if (this.CurrentColumnIndex != -1 && this.DisplayData.IsRowDisplayed(this.CurrentRowIndex))
                {
                    this.DisplayData.GetDisplayedRow(this.CurrentRowIndex).Cells[this.CurrentColumnIndex].ApplyCellState(true /*animate*/);
                }
            }

            // Keep track of which row contains the newly focused element
            DataGridRow focusedRow = null;
            DependencyObject focusedElement = e.OriginalSource as DependencyObject;
            while (focusedElement != null)
            {
                focusedRow = focusedElement as DataGridRow;
                if (focusedRow != null && focusedRow.OwningGrid == this)
                {
                    _focusedRow = focusedRow;
                    break;
                }
                focusedElement = VisualTreeHelper.GetParent(focusedElement);
            }
        }

        private void DataGrid_KeyDown(object sender, KeyEventArgs e)
        {
            // 
            if (!e.Handled)
            {
                e.Handled = ProcessDataGridKey(e);
            }
        }

        private void DataGrid_KeyUp(object sender, KeyEventArgs e)
        {
            // 
            if (e.Key == Key.Tab && this.CurrentColumnIndex != -1 && e.OriginalSource == this)
            {
                bool success = ScrollIntoView(this.CurrentColumnIndex, this.CurrentRowIndex, false /*forCurrentCellChange*/);
                Debug.Assert(success);
                if (this.CurrentColumnIndex != -1 && this.SelectedItem == null)
                {
                    SetRowSelection(this.CurrentRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                }
            }
        }

        private void DataGrid_LostFocus(object sender, RoutedEventArgs e)
        {
            if (this.ContainsFocus)
            {
                this.ContainsFocus = false;
                ApplyDisplayedRowsState(this.DisplayData.FirstDisplayedScrollingRow, this.DisplayData.LastDisplayedScrollingRow);
                if (this.CurrentColumnIndex != -1 && this.DisplayData.IsRowDisplayed(this.CurrentRowIndex))
                {
                    this.DisplayData.GetDisplayedRow(this.CurrentRowIndex).Cells[this.CurrentColumnIndex].ApplyCellState(true /*animate*/);
                }
            }
        }

        /// <summary>
        /// Called when the edited element of a column gains focus
        /// </summary>
        private void EditingElement_GotFocus(object sender, RoutedEventArgs e)
        {
            // 
            FrameworkElement element = sender as FrameworkElement;
            if (element != null)
            {
                // No longer interested in the GotFocus event
                element.GotFocus -= new RoutedEventHandler(EditingElement_GotFocus);
                this._editingElementGotFocusListeners--;
                Debug.Assert(this._editingElementGotFocusListeners >= 0);
                // 

                // but need to know when the element loses focus
                element.LostFocus += new RoutedEventHandler(EditingElement_LostFocus);
                this._editingElementLostFocusListeners++;
                // 
            }
        }

        /// <summary>
        /// Called when the edited element of a column loses focus
        /// </summary>
        private void EditingElement_LostFocus(object sender, RoutedEventArgs e)
        {
            // 
            FrameworkElement element = sender as FrameworkElement;
            if (element != null)
            {
                // No longer interested in the LostFocus event
                element.LostFocus -= new RoutedEventHandler(EditingElement_LostFocus);
                this._editingElementLostFocusListeners--;
                Debug.Assert(this._editingElementLostFocusListeners >= 0);
                // 

                // An element outside the DataGrid may have received focus. We need to know
                // when the edited element receives it back, if ever.
                element.GotFocus += new RoutedEventHandler(EditingElement_GotFocus);
                Debug.Assert(this._editingElementGotFocusListeners >= 0);
                this._editingElementGotFocusListeners++;
                // 

                if (this.CurrentColumn != null && this.CurrentColumn is DataGridBoundColumn)
                {
                    // The edited element may need to repopulate its content asynchronously
                    this.Dispatcher.BeginInvoke(new Action(PopulateUneditedBoundCellContent), null);
                }

                // End editing if focus leaves the DataGrid entirely
                bool isDataGridChild = false;
                DependencyObject focusedElement = FocusManager.GetFocusedElement() as DependencyObject;
                while (focusedElement != null)
                {
                    if (focusedElement == this._editingRow || focusedElement == this)
                    {
                        isDataGridChild = true;
                        break;
                    }
                    focusedElement = VisualTreeHelper.GetParent(focusedElement);
                }
                if (!isDataGridChild)
                {
                    CommitEdit(DataGridEditingUnit.Row, true /*exitEditingMode*/);
                }
            }
        }

        private void EditingElement_Loaded(object sender, RoutedEventArgs e)
        {
            // 
            FrameworkElement element = sender as FrameworkElement;
            if (element != null)
            {
                // 
                element.Loaded -= new RoutedEventHandler(EditingElement_Loaded);
            }
            PreparingCellForEditPrivate(element);
        }

        [SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
        private bool EndCellEdit(bool commitCellEdit, bool exitEditingMode, bool keepFocus)
        {
            // 
            if (this._editingColumnIndex == -1)
            {
                // 
                return true;
            }

            Debug.Assert(this._editingRow != null);
            Debug.Assert(this._editingColumnIndex >= 0);
            Debug.Assert(this._editingColumnIndex < this.ColumnsItemsInternal.Count);
            Debug.Assert(this._editingColumnIndex == this.CurrentColumnIndex);
            Debug.Assert(this.EditingRowIndex == this.CurrentRowIndex);

            int curRowIndex = this.CurrentRowIndex;
            int curColIndex = this.CurrentColumnIndex;

            // 

            if (!commitCellEdit)
            {
                CancelCellEdit(exitEditingMode);
            }

            if (this._editingColumnIndex == -1 ||
                curRowIndex != this.CurrentRowIndex || 
                curColIndex != this.CurrentColumnIndex)
            {
                // 
                return true;
            }

            Debug.Assert(this._editingRow != null);
            Debug.Assert(this.EditingRowIndex == curRowIndex);
            Debug.Assert(this._editingColumnIndex != -1);
            Debug.Assert(this._editingColumnIndex == this.CurrentColumnIndex);

            if (exitEditingMode)
            {
                this._editingColumnIndex = -1;
                this._editingTemplateControl = null;
                this._editingRow.Cells[this.CurrentColumnIndex].ApplyCellState(true /*animate*/);
                //
                this.IsTabStop = true;
                if (keepFocus)
                {
                    bool success = Focus();
                    Debug.Assert(success);
                }
            }

            if (exitEditingMode || commitCellEdit)
            {
                DataGridCell cell = this._editingRow.Cells[this.CurrentColumnIndex];
                if (!(this.CurrentColumn is DataGridBoundColumn))
                {
                    // 
                    PopulateCellContent(false /*forceTemplating*/, !exitEditingMode /*isCellEdited*/, this.CurrentColumn, this._editingRow, cell);
                }
                else if (this._editingBoundCells.Count > 0)
                {
                    Debug.Assert(this._editingBoundCells.Contains(cell));
                    
                    this._editingBoundCells.Remove(cell);

                    PopulateCellContent(false /*forceTemplating*/, !exitEditingMode /*isCellEdited*/, this.CurrentColumn, this._editingRow, cell);
                }
                if (this._editingElementLostFocusListeners == 0 && this._editingElementGotFocusListeners > 0)
                {
                    FrameworkElement element = cell.Content as FrameworkElement;
                    if (element != null)
                    {
                        element.GotFocus -= new RoutedEventHandler(EditingElement_GotFocus);
                        this._editingElementGotFocusListeners--;
                    }
                }
            }

            return true;
        }

        private bool EndRowEdit(bool commitRowEdit, bool exitEditingMode)
        {
            if (this._editingRow == null)
            {
                return true;
            }

            if (commitRowEdit)
            {
                CommitRowEdit(exitEditingMode);
            }
            else
            {
                CancelRowEdit(exitEditingMode);
            }

            if (exitEditingMode)
            {
                Debug.Assert(this._editingRow != null);
                DataGridRow editingRow = this._editingRow;
                this._editingRow = null;
                if (!this.DisplayData.IsRowDisplayed(editingRow.Index) && this._rowsPresenter != null && editingRow != this._focusedRow)
                {
                    // 
                    this._rowsPresenter.Children.Remove(editingRow);
                }
                else
                {
                    editingRow.ApplyState(true /*animate*/);
                }
            }

            return true;
        }

        // Applies the given Style to the column's HeaderCell if the HeaderCell does not already
        // have a Style applied
        private static void EnsureColumnHeaderCellStyle(DataGridColumn column, Style oldDataGridStyle, Style newDataGridStyle)
        {
            Debug.Assert(column != null);

            // 

            if (column != null && (column.HeaderCell.Style == null || column.HeaderCell.Style == oldDataGridStyle))
            {
                column.HeaderCell.Style = newDataGridStyle;
            }
        }

        private void EnsureColumnHeadersVisibility()
        {
            if (_columnHeadersPresenter != null)
            {
                _columnHeadersPresenter.Visibility = this.AreColumnHeadersVisible ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        private static void EnsureRowStyle(DataGridRow row, Style oldDataGridRowStyle, Style newDataGridRowStyle)
        {
            Debug.Assert(row != null);

            if (newDataGridRowStyle != null)
            {
                // 

                if (row != null && (row.Style == null || row.Style == oldDataGridRowStyle))
                {
                    row.Style = newDataGridRowStyle;
                }
            }
        }

        private void EnsureVerticalGridLines()
        {
            double totalColumnsWidth = 0;
            foreach (DataGridColumn column in this.ColumnsInternal)
            {
                totalColumnsWidth += column.ActualWidth;

                if (this.AreColumnHeadersVisible)
                {
                    column.HeaderCell.SeparatorVisibility = (column != this.ColumnsInternal.LastVisibleColumn || totalColumnsWidth < this.CellsWidth) ?
                        Visibility.Visible : Visibility.Collapsed;
                }
            }

            foreach (DataGridRow row in this._rowsPresenter.Children)
            {
                foreach (DataGridCell cell in row.Cells)
                {
                    cell.EnsureGridLine(this.ColumnsInternal.LastVisibleColumn);
                }
            }
        }

        /// <summary>
        /// Exits editing mode without trying to commit or revert the editing, and 
        /// without repopulating the edited row's cell.
        /// </summary>
        private void ExitEdit(bool keepFocus)
        {
            if (this._editingRow == null)
            {
                Debug.Assert(this._editingColumnIndex == -1);
                return;
            }

            if (this._editingColumnIndex != -1)
            {
                Debug.Assert(this._editingColumnIndex >= 0);
                Debug.Assert(this._editingColumnIndex < this.ColumnsItemsInternal.Count);
                Debug.Assert(this._editingColumnIndex == this.CurrentColumnIndex);
                Debug.Assert(this.EditingRowIndex == this.CurrentRowIndex);

                this._editingColumnIndex = -1;
                this._editingRow.Cells[this.CurrentColumnIndex].ApplyCellState(false /*animate*/);
            }
            //
            this.IsTabStop = true;
            if (!this.DisplayData.IsRowDisplayed(this._editingRow.Index) && (this._rowsPresenter != null))
            {
                _rowsPresenter.Children.Remove(this._editingRow);
            }
            else
            {
                this._editingRow.ApplyState(true /*animate*/);
            }
            this._editingRow = null;
            if (keepFocus)
            {
                bool success = Focus();
                Debug.Assert(success);
            }
        }

        private void FlushSelectionChanged()
        {
            if (this._selectionChanged)
            {
                SelectionChangedEventArgs eventArgs = new SelectionChangedEventArgs(this._removedSelectedItems, this._addedSelectedItems);
                OnSelectionChanged(eventArgs);

                this._addedSelectedItems.Clear();
                this._removedSelectedItems.Clear();
            }
        }

        private bool FocusEditingCell(bool setFocus)
        {
            Debug.Assert(this.CurrentColumnIndex >= 0);
            Debug.Assert(this.CurrentColumnIndex < this.Columns.Count);
            Debug.Assert(this.CurrentRowIndex >= -1);
            Debug.Assert(this.CurrentRowIndex < this.RowCount);
            Debug.Assert(this.EditingRowIndex == this.CurrentRowIndex);
            Debug.Assert(this._editingColumnIndex != -1);

            //

            this.IsTabStop = false;
            this._focusEditingControl = false;

            DataGridCell dataGridCell = this._editingRow.Cells[this._editingColumnIndex];
            if (dataGridCell.OwningColumn is DataGridBoundColumn)
            {
                Control editingControl = dataGridCell.Content as Control;
                if (editingControl != null)
                {
                    editingControl.IsTabStop = true;
                    editingControl.TabIndex = this.TabIndex;
                    if (setFocus)
                    {
                        this._focusEditingControl = !editingControl.Focus();
                    }
                }
            }
            else if (setFocus && this._editingTemplateControl != null && this._editingTemplateControl.IsTabStop)
            {
                return this._editingTemplateControl.Focus();
            }
            return false;
        }

        // Calculates the amount to scroll for the ScrollLeft button
        // This is a method rather than a property to emphasize a calculation
        private double GetHorizontalSmallScrollDecrease()
        {
            // If the first column is covered up, scroll to the start of it when the user clicks the left button
            if (_negHorizontalOffset > 0)
            {
                return _negHorizontalOffset;
            }
            else
            {
                // The entire first column is displayed, show the entire previous column when the user clicks
                // the left button
                DataGridColumn previousColumn = this.ColumnsInternal.GetPreviousVisibleScrollingColumn(
                    this.ColumnsInternal[DisplayData.FirstDisplayedScrollingCol]);
                if (previousColumn != null)
                {
                    return GetEdgedColumnWidth(previousColumn);
                }
                else
                {
                    // There's no previous column so don't move
                    return 0;
                }
            }
        }

        // Calculates the amount to scroll for the ScrollRight button
        // This is a method rather than a property to emphasize a calculation
        private double GetHorizontalSmallScrollIncrease()
        {
            if (this.DisplayData.FirstDisplayedScrollingCol >= 0)
            {
                return GetEdgedColumnWidth(this.ColumnsInternal[DisplayData.FirstDisplayedScrollingCol]) - _negHorizontalOffset;
            }
            return 0;
        }

        // Calculates the amount the ScrollDown button should scroll
        // This is a method rather than a property to emphasize that calculations are taking place
        private double GetVerticalSmallScrollIncrease()
        {
            if (this.DisplayData.FirstDisplayedScrollingRow >= 0)
            {
                return GetEdgedExactRowHeight(this.DisplayData.FirstDisplayedScrollingRow) - this.NegVerticalOffset;
            }
            return 0;
        }

        private void HorizontalScrollBar_Scroll(object sender, ScrollEventArgs e)
        {
            ProcessHorizontalScroll(e.ScrollEventType);
        }    

        private bool IsColumnOutOfBounds(int columnIndex)
        {
            return columnIndex >= this.ColumnsItemsInternal.Count || columnIndex < 0;
        }

        private bool IsInnerCellOutOfBounds(int columnIndex, int rowIndex)
        {
            return IsColumnOutOfBounds(columnIndex) || IsRowOutOfBounds(rowIndex);
        }

        // 





        private bool IsRowOutOfBounds(int rowIndex)
        {
            return rowIndex >= this.RowCount || rowIndex < -1 /**/;
        }

        private void MakeFirstDisplayedCellCurrentCell()
        {
            if (this.CurrentColumnIndex != -1)
            {
                this._makeFirstDisplayedCellCurrentCellPending = false;
                return;
            }
            
            // No current cell, therefore no selection either - try to set the first displayed cell to be the current one.
            int firstDisplayedColumnIndex = this.FirstDisplayedColumnIndex;
            int rowIndex = (this.SelectedIndex == -1) ? this.DisplayData.FirstDisplayedScrollingRow : this.SelectedIndex;
            if (firstDisplayedColumnIndex != -1 && rowIndex != -1)
            {
                SetAndSelectCurrentCell(firstDisplayedColumnIndex,
                                        rowIndex,
                                        false /*forceCurrentCellSelection (unused here)*/);
                this.AnchorRowIndex = rowIndex;
                this._makeFirstDisplayedCellCurrentCellPending = false;
            }
            else
            {
                this._makeFirstDisplayedCellCurrentCellPending = true;
            }
        }

        private void PopulateCellContent(bool forceTemplating, bool isCellEdited,
                                         DataGridColumn dataGridColumn,
                                         DataGridRow dataGridRow,
                                         DataGridCell dataGridCell)
        {
            // 
            Debug.Assert(dataGridColumn != null);
            Debug.Assert(dataGridRow != null);
            Debug.Assert(dataGridRow.DataContext != null, "dataGridRow.DataContext is null");
            Debug.Assert(dataGridCell != null);

            FrameworkElement element = null;

            // 
            DataGridTemplateColumn dataGridTemplateColumn = dataGridColumn as DataGridTemplateColumn;
            if (dataGridTemplateColumn != null)
            {
                // 
                if (forceTemplating || dataGridTemplateColumn.HasDistinctTemplates)
                {
                    // 
                    DataTemplate cellTemplate = null;
                    // 
                    if (isCellEdited)
                    {
                        cellTemplate = dataGridTemplateColumn.CellEditingTemplate;
                        if (cellTemplate == null)
                        {
                            cellTemplate = dataGridTemplateColumn.CellTemplate;
                        }
                    }
                    else
                    {
                        cellTemplate = dataGridTemplateColumn.CellTemplate;
                        if (cellTemplate == null)
                        {
                            cellTemplate = dataGridTemplateColumn.CellEditingTemplate;
                        }
                    }
                    if (cellTemplate == null)
                    {
                        // No cell template nor cell editing template for DataGridTemplateColumn
                        throw DataGridError.DataGridTemplateColumn.MissingTemplateForType(typeof(DataGridTemplateColumn));
                    }
                    element = cellTemplate.LoadContent() as FrameworkElement;
                }
                else if (isCellEdited)
                {
                    PreparingCellForEditPrivate(dataGridCell.Content as FrameworkElement);
                    return;
                }
            }
            else // if the column is not a template column
            {
                DataGridBoundColumn dataGridBoundColumn = dataGridColumn as DataGridBoundColumn;

                // 
                if (isCellEdited)
                {
                    element = dataGridColumn.GenerateEditingElementInternal(dataGridCell, dataGridRow.DataContext);
                    if (element != null)
                    {
                        // 
                        if (dataGridBoundColumn != null && dataGridBoundColumn.EditingElementStyle != null && element.Style == null)
                        {
                            element.Style = dataGridBoundColumn.EditingElementStyle;
                        }

                        // Remember which cell needs to repopulate its content asynchronously.
                        this._editingBoundCells.Add(dataGridCell);
                    }
                }
                else
                {
                    // Generate Element and apply column style if available
                    element = dataGridColumn.GenerateElementInternal(dataGridCell, dataGridRow.DataContext);
                    // 
                    if (element != null && dataGridBoundColumn != null && dataGridBoundColumn.ElementStyle != null && element.Style == null)
                    {
                        element.Style = dataGridBoundColumn.ElementStyle;
                    }
                }
            }

            if (isCellEdited && element != null)
            {
                // 
                element.Loaded += new RoutedEventHandler(EditingElement_Loaded);

                // Subscribe to the GotFocus event so that the non-editing element can be generated 
                // asynchronously after the edited value is committed.
                element.GotFocus += new RoutedEventHandler(EditingElement_GotFocus);
                this._editingElementGotFocusListeners++;
                // 
            }

            dataGridCell.Content = element;
        }

        /// <summary>
        /// Repopulates a cell that was previously edited with the non-editing element.
        /// This method is invoked asynchronously after the editing element loses focus and
        /// the back-end property was updated with a new value, or when the current cell
        /// leaves the editing mode before the Loaded event has a chance to be raised.
        /// </summary>
        private void PopulateUneditedBoundCellContent()
        {
            // 
            if (this._editingBoundCells.Count > 0)
            {
                DataGridCell dataGridCell = this._editingBoundCells[0];
                Debug.Assert(dataGridCell != null);                
                if (dataGridCell.OwningGrid == this)
                {
                    Debug.Assert(dataGridCell.OwningColumn is DataGridBoundColumn);
                    if (dataGridCell.OwningColumn.Index != this._editingColumnIndex ||
                        dataGridCell.OwningRow != this._editingRow)
                    {
                        if (this._editingElementGotFocusListeners > 0)
                        {
                            // No longer interested in the GotFocus event
                            FrameworkElement element = dataGridCell.Content as FrameworkElement;
                            if (element != null)
                            {
                                element.GotFocus -= new RoutedEventHandler(EditingElement_GotFocus);
                                this._editingElementGotFocusListeners--;
                                Debug.Assert(this._editingElementGotFocusListeners >= 0);
                                // 
                            }
                        }
                        this._editingBoundCells.RemoveAt(0); // 0 = the dataGridCell we were just inspecting
                        
                        // Repopulate the cell with the non-editing element
                        PopulateCellContent(false /*forceTemplating*/, false /*isCellEdited*/,
                                            dataGridCell.OwningColumn, dataGridCell.OwningRow, dataGridCell);
                    }
                }
                else
                {
                    this._editingBoundCells.RemoveAt(0); // 0 = the dataGridCell we were just inspecting
                }
            }
        }

        // Returns False if we couldn't CommitEdit or if the rowIndex, columnIndex are out of bounds
        private bool PrepareForScroll(int columnIndex, int rowIndex, bool forCurrentCellChange)
        {
            if (this.CurrentColumnIndex >= 0 &&
                (this.CurrentColumnIndex != columnIndex || this.CurrentRowIndex != rowIndex))
            {
                if (!CommitEditForOperation(columnIndex, rowIndex, forCurrentCellChange))
                {
                    return false;
                }
                if (IsInnerCellOutOfBounds(columnIndex, rowIndex))
                {
                    return false;
                }
            }
            return true;
        }

        private void PreparingCellForEditPrivate(FrameworkElement editingElement)
        {
            // 

            if (this._editingColumnIndex == -1 || 
                this.CurrentColumnIndex == -1 ||
                this._editingRow.Cells[this.CurrentColumnIndex].Content != editingElement)
            {
                // The current cell has changed since the call to BeginCellEdit. The cell needs
                // to get back into non-editing mode.
                DataGridCell dataGridCell = GetOwningCell(editingElement);
                if (dataGridCell != null)
                {
                    Debug.Assert(!(dataGridCell.OwningColumn is DataGridTemplateColumn));
                    if (this._editingBoundCells.Count > 0 && dataGridCell == this._editingBoundCells[0])
                    {
                        Debug.Assert(dataGridCell.OwningColumn is DataGridBoundColumn);
                        PopulateUneditedBoundCellContent();
                    }
                    // 





                }
                // 
                return;
            }

            Debug.Assert(this._editingRow != null);
            Debug.Assert(this._editingColumnIndex >= 0);
            Debug.Assert(this._editingColumnIndex < this.ColumnsItemsInternal.Count);
            Debug.Assert(this._editingColumnIndex == this.CurrentColumnIndex);
            Debug.Assert(this.EditingRowIndex == this.CurrentRowIndex);

            FocusEditingCell(this.ContainsFocus || this._focusEditingControl /*setFocus*/);

            // Prepare the cell for editing and raise the PreparingCellForEdit event for all columns
            DataGridColumn dataGridColumn = this.CurrentColumn;
            this._uneditedValue = dataGridColumn.PrepareCellForEditInternal(editingElement, this._editingEventArgs);
            OnPreparingCellForEdit(new DataGridPreparingCellForEditEventArgs(dataGridColumn, this._editingRow, this._editingEventArgs, editingElement));

            // 
        }

        private bool ProcessAKey()
        {
            bool ctrl, shift, alt;

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift, out alt);

            if (ctrl && !shift && !alt && this.SelectionMode == DataGridSelectionMode.Extended)
            {
                SelectAll();
                return true;
            }
            return false;
        }

        private bool ProcessDataGridKey(KeyEventArgs e)
        {
            bool focusDataGrid = false;
            switch (e.Key)
            {
                case Key.Tab:
                    return ProcessTabKey(e);

                case Key.Up:
                    focusDataGrid = ProcessUpKey();
                    break;

                case Key.Down:
                    focusDataGrid = ProcessDownKey();
                    break;

                case Key.PageDown:
                    focusDataGrid = ProcessNextKey();
                    break;

                case Key.PageUp:
                    focusDataGrid = ProcessPriorKey();
                    break;

                case Key.Left:
                    focusDataGrid = ProcessLeftKey();
                    break;

                case Key.Right:
                    focusDataGrid = ProcessRightKey();
                    break;

                case Key.F2:
                    return ProcessF2Key(e);

                case Key.Home:
                    focusDataGrid = ProcessHomeKey();
                    break;

                // 



                case Key.End:
                    focusDataGrid = ProcessEndKey();
                    break;

                case Key.Enter:
                    focusDataGrid = ProcessEnterKey();
                    break;

                case Key.Escape:
                    return ProcessEscapeKey();

                case Key.A:
                    return ProcessAKey();

                // 





                // 



            }
            if (focusDataGrid && this.IsTabStop)
            {
                this.Focus();
            }
            return focusDataGrid;
        }

        [SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
        private bool ProcessDownKeyInternal(bool shift, bool ctrl, out bool moved)
        {
            bool success;

            DataGridColumn dataGridColumn = this.ColumnsInternal.FirstVisibleColumn;
            int firstVisibleColumnIndex = (dataGridColumn == null) ? -1 : dataGridColumn.Index;
            int lastRowIndex = this.LastRowIndex;
            if (firstVisibleColumnIndex == -1 || lastRowIndex == -1)
            {
                moved = false;
                return false;
            }
            int nextRowIndex = -1;
            if (this.CurrentRowIndex != -1)
            {
                nextRowIndex = this.GetNextRow(this.CurrentRowIndex);
                // 









            }
            // 

















            moved = true;

            _noSelectionChangeCount++;
            try
            {
                if (ctrl)
                {
                    if (shift)
                    {
                        if (this.CurrentColumnIndex == -1)
                        {
                            Debug.Assert(_selectedItems.Count == 0);
                            SetRowSelection(lastRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                            success = ScrollIntoView(firstVisibleColumnIndex, lastRowIndex, false);
                            Debug.Assert(success);
                            if (IsInnerCellOutOfBounds(firstVisibleColumnIndex, lastRowIndex))
                            {
                                moved = false;
                                return true;
                            }
                            success = SetCurrentCellCore(firstVisibleColumnIndex, lastRowIndex/*, false, false*/);
                            if (!success)
                            {
                                moved = false;
                            }
                        }
                        else
                        {
                            if (this.SelectionMode == DataGridSelectionMode.Extended)
                            {
                                if (!PrepareForScroll(this.CurrentColumnIndex, lastRowIndex, true))
                                {
                                    return false;
                                }

                                if (this.AnchorRowIndex == -1 || this.CurrentColumnIndex == -1 ||
                                    IsRowOutOfBounds(lastRowIndex))
                                {
                                    moved = false;
                                    return true;
                                }
                                ClearRowSelection(lastRowIndex, false /*resetAnchorRowIndex*/);
                                Debug.Assert(this.AnchorRowIndex >= 0);
                                SetRowsSelection(this.AnchorRowIndex, lastRowIndex);
                                // Scrolling needs to be done after Selection is updated
                                if (!ScrollIntoView(this.CurrentColumnIndex, lastRowIndex, true))
                                {
                                    return true;
                                }
                                success = SetCurrentCellCore(this.CurrentColumnIndex, lastRowIndex/*, false, false*/);
                                if (!success)
                                {
                                    moved = false;
                                }
                            }
                            else
                            {
                                if (!ScrollIntoView(this.CurrentColumnIndex, lastRowIndex, true))
                                {
                                    return true;
                                }
                                if (this.CurrentColumnIndex == -1 || IsRowOutOfBounds(lastRowIndex))
                                {
                                    moved = false;
                                    return true;
                                }
                                if (lastRowIndex != this.SelectedIndex)
                                {
                                    SetRowSelection(this.CurrentRowIndex, false /*isSelected*/, false /*setAnchorRowIndex*/);
                                    SetRowSelection(lastRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                                }
                                success = SetCurrentCellCore(this.CurrentColumnIndex, lastRowIndex/*, false, false*/);
                                if (!success)
                                {
                                    moved = false;
                                }
                            }
                        }
                    }
                    else
                    {
                        // Ctrl without Shift
                        if (this.CurrentColumnIndex == -1)
                        {
                            Debug.Assert(_selectedItems.Count == 0);
                            SetRowSelection(lastRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                            success = ScrollIntoView(firstVisibleColumnIndex, lastRowIndex, false);
                            Debug.Assert(success);
                            if (IsInnerCellOutOfBounds(firstVisibleColumnIndex, lastRowIndex))
                            {
                                moved = false;
                                return true;
                            }
                            success = SetCurrentCellCore(firstVisibleColumnIndex, lastRowIndex/*, false, false*/);
                            if (!success)
                            {
                                moved = false;
                            }
                        }
                        else
                        {
                            if (!ScrollIntoView(this.CurrentColumnIndex, lastRowIndex, true))
                            {
                                return true;
                            }
                            if (this.CurrentColumnIndex == -1 || IsRowOutOfBounds(lastRowIndex))
                            {
                                moved = false;
                                return true;
                            }
                            ClearRowSelection(lastRowIndex, true /*resetAnchorRowIndex*/);
                            success = SetCurrentCellCore(this.CurrentColumnIndex, lastRowIndex/*, false, false*/);
                            if (!success)
                            {
                                moved = false;
                            }
                        }
                    }
                }
                else
                {
                    // No Ctrl
                    if (shift)
                    {
                        if (this.CurrentColumnIndex == -1)
                        {
                            Debug.Assert(_selectedItems.Count == 0);
                            SetRowSelection(lastRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                            success = ScrollIntoView(firstVisibleColumnIndex, lastRowIndex, false);
                            Debug.Assert(success);
                            if (IsInnerCellOutOfBounds(firstVisibleColumnIndex, lastRowIndex))
                            {
                                moved = false;
                                return true;
                            }
                            success = SetCurrentCellCore(firstVisibleColumnIndex, lastRowIndex/*, false, false*/);
                            if (!success)
                            {
                                moved = false;
                            }
                        }
                        else
                        {
                            if (nextRowIndex == -1)
                            {
                                moved = false;
                                return true;
                            }
                            if (!ScrollIntoView(this.CurrentColumnIndex, nextRowIndex, true))
                            {
                                return true;
                            }
                            if (this.CurrentColumnIndex == -1 || IsRowOutOfBounds(nextRowIndex))
                            {
                                moved = false;
                                return true;
                            }
                            ClearRowSelection(false /*resetAnchorRowIndex*/);
                            if (this.SelectionMode == DataGridSelectionMode.Extended)
                            {
                                if (nextRowIndex >= this.AnchorRowIndex)
                                {
                                    SetRowsSelection(this.AnchorRowIndex, nextRowIndex);
                                }
                                else
                                {
                                    SetRowsSelection(nextRowIndex, this.AnchorRowIndex);
                                }
                            }
                            else
                            {
                                SetRowSelection(nextRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                            }
                            success = SetCurrentCellCore(this.CurrentColumnIndex, nextRowIndex/*, false, false*/);
                            if (!success)
                            {
                                moved = false;
                            }
                        }
                    }
                    else
                    {
                        if (this.CurrentColumnIndex == -1)
                        {
                            Debug.Assert(_selectedItems.Count == 0);
                            SetRowSelection(lastRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                            success = ScrollIntoView(firstVisibleColumnIndex, lastRowIndex, false);
                            Debug.Assert(success);
                            if (IsInnerCellOutOfBounds(firstVisibleColumnIndex, lastRowIndex))
                            {
                                moved = false;
                                return true;
                            }
                            success = SetCurrentCellCore(firstVisibleColumnIndex, lastRowIndex/*, false, false*/);
                            if (!success)
                            {
                                moved = false;
                            }
                        }
                        else
                        {
                            // 











                            if (nextRowIndex == -1)
                            {
                                moved = false;
                                return true;
                            }
                            // 
                            if (nextRowIndex != -1)
                            {
                                if (!ScrollIntoView(this.CurrentColumnIndex, nextRowIndex, true))
                                {
                                    return true;
                                }
                            }
                            if (this.CurrentColumnIndex == -1 || IsRowOutOfBounds(nextRowIndex))
                            {
                                moved = false;
                                return true;
                            }
                            ClearRowSelection(true /*resetAnchorRowIndex*/);
                            if (nextRowIndex != -1)
                            {
                                SetRowSelection(nextRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                            }
                            success = SetCurrentCellCore(this.CurrentColumnIndex, nextRowIndex/*, false, false*/);
                            if (!success)
                            {
                                moved = false;
                            }
                        }
                    }
                }
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }
            return true;
        }

        private bool ProcessEscapeKey()
        {
            if (this._editingColumnIndex != -1)
            {
                // Revert the potential cell editing and exit cell editing.
                EndCellEdit(false /*commitCellEdit*/, true /*exitEditingMode*/, true /*keepFocus*/);
                return true;
            }
            else if (this._editingRow != null)
            {
                // Revert the potential row editing and exit row editing.
                EndRowEdit(false /*commitRowEdit*/, true /*exitEditingMode*/);
                return true;
            }
            return false;
        }

        private bool ProcessF2Key(KeyEventArgs e)
        {
            bool ctrl, shift;
            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);

            if (!shift && !ctrl &&
                this._editingColumnIndex == -1 && this.CurrentColumnIndex != -1 && GetRowSelection(this.CurrentRowIndex) &&
                !GetColumnEffectiveReadOnlyState(this.CurrentColumn))
            {
                if (ScrollIntoView(this.CurrentColumnIndex, this.CurrentRowIndex, false))
                {
                    BeginCellEdit(e);
                }
                return true;
            }

            return false;
        }

        // Ctrl Left <==> Home
        private bool ProcessLeftMost(int firstVisibleColumnIndex, int firstRowIndex)
        {
            bool success;

            this._noSelectionChangeCount++;
            try
            {
                if (this.CurrentColumnIndex == -1)
                {
                    Debug.Assert(_selectedItems.Count == 0);
                    SetRowSelection(firstRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                    success = ScrollIntoView(firstVisibleColumnIndex, firstRowIndex, false);
                    Debug.Assert(success);
                    if (IsInnerCellOutOfBounds(firstVisibleColumnIndex, firstRowIndex))
                    {
                        return true;
                    }
                    success = SetCurrentCellCore(firstVisibleColumnIndex, firstRowIndex/*, false, false*/);
                    Debug.Assert(success);
                }
                else
                {
                    if (!ScrollIntoView(firstVisibleColumnIndex, this.CurrentRowIndex, true))
                    {
                        return true;
                    }
                    if (IsRowOutOfBounds(this.CurrentRowIndex) || IsColumnOutOfBounds(firstVisibleColumnIndex))
                    {
                        return true;
                    }
                    success = SetCurrentCellCore(firstVisibleColumnIndex, this.CurrentRowIndex/*, false, false*/);
                    Debug.Assert(success);
                }
                return true;
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }
        }

        // Ctrl Right <==> End
        private bool ProcessRightMost(int lastVisibleColumnIndex, int firstRowIndex)
        {
            bool success;

            this._noSelectionChangeCount++;
            try
            {
                if (this.CurrentColumnIndex == -1)
                {
                    Debug.Assert(_selectedItems.Count == 0);
                    SetRowSelection(firstRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                    success = ScrollIntoView(lastVisibleColumnIndex, firstRowIndex, false);
                    Debug.Assert(success);
                    if (IsInnerCellOutOfBounds(lastVisibleColumnIndex, firstRowIndex))
                    {
                        return true;
                    }
                    success = SetCurrentCellCore(lastVisibleColumnIndex, firstRowIndex/*, false, false*/);
                    Debug.Assert(success);
                }
                else
                {
                    if (!ScrollIntoView(lastVisibleColumnIndex, this.CurrentRowIndex, true))
                    {
                        return true;
                    }
                    if (IsRowOutOfBounds(this.CurrentRowIndex) || IsColumnOutOfBounds(lastVisibleColumnIndex))
                    {
                        return true;
                    }
                    success = SetCurrentCellCore(lastVisibleColumnIndex, this.CurrentRowIndex/*, false, false*/);
                    Debug.Assert(success);
                }
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }
            return true;
        }

        private bool ProcessTabKey(KeyEventArgs e)
        {
            bool ctrl, shift;
            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);

            if (ctrl || this._editingColumnIndex == -1 || this.IsReadOnly)
            {
                //Go to the next/previous control on the page when 
                // - Ctrl key is used
                // - Potential current cell is not edited, or the datagrid is read-only. 
                return false;
            }

            // Try to locate a writable cell before/after the current cell
            Debug.Assert(this.CurrentColumnIndex != -1);
            Debug.Assert(this.CurrentRowIndex != -1);

            int neighborVisibleWritableColumnIndex, neighborRowIndex;
            DataGridColumn dataGridColumn;
            if (shift)
            {
                dataGridColumn = this.ColumnsInternal.GetPreviousVisibleWritableColumn(this.Columns[this.CurrentColumnIndex]);
                neighborRowIndex = GetPreviousRow(this.CurrentRowIndex);
            }
            else
            {
                dataGridColumn = this.ColumnsInternal.GetNextVisibleWritableColumn(this.Columns[this.CurrentColumnIndex]);
                neighborRowIndex = GetNextRow(this.CurrentRowIndex);
            }
            neighborVisibleWritableColumnIndex = (dataGridColumn == null) ? -1 : dataGridColumn.Index;

            if (neighborVisibleWritableColumnIndex == -1 && neighborRowIndex == -1)
            {
                // There is no previous/next row and no previous/next writable cell on the current row
                return false;
            }

            int targetRowIndex = -1, targetColumnIndex = -1;
            bool success;

            this._noSelectionChangeCount++;
            try
            {
                if (neighborVisibleWritableColumnIndex == -1)
                {
                    targetRowIndex = neighborRowIndex;
                    if (shift)
                    {
                        Debug.Assert(this.ColumnsInternal.LastVisibleWritableColumn != null);
                        targetColumnIndex = this.ColumnsInternal.LastVisibleWritableColumn.Index;
                    }
                    else
                    {
                        Debug.Assert(this.ColumnsInternal.FirstVisibleWritableColumn != null);
                        targetColumnIndex = this.ColumnsInternal.FirstVisibleWritableColumn.Index;
                    }
                }
                else
                {
                    targetRowIndex = this.CurrentRowIndex;
                    targetColumnIndex = neighborVisibleWritableColumnIndex;
                }
                if (!ScrollIntoView(targetColumnIndex, targetRowIndex, true /*forCurrentCellChange*/))
                {
                    return true;
                }

                if (targetRowIndex != this.CurrentRowIndex || (this.SelectionMode == DataGridSelectionMode.Extended))
                {
                    if (IsRowOutOfBounds(targetRowIndex))
                    {
                        return true;
                    }
                    ClearRowSelection(targetRowIndex /*rowIndexException*/, true /*setAnchorRowIndex*/);
                }
                if (IsInnerCellOutOfBounds(targetColumnIndex, targetRowIndex))
                {
                    return true;
                }
                success = SetCurrentCellCore(targetColumnIndex, targetRowIndex/*, false, false*/);
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }

            if (success)
            {
                Debug.Assert(this.ContainsFocus);
                success = BeginCellEdit(e);
            }
            return true;
        }

        private void RemoveDisplayedColumnHeader(DataGridColumn dataGridColumn)
        {
            if (this.AreColumnHeadersVisible && _columnHeadersPresenter != null)
            {
                _columnHeadersPresenter.Children.RemoveAt(dataGridColumn.Index);
            }
        }

        private void RemoveDisplayedColumnHeaders()
        {
            if (_columnHeadersPresenter != null)
            {
                _columnHeadersPresenter.Children.Clear();
            }
            this.ColumnsInternal.FillerColumn.IsRepresented = false;
        }

        private void ResetCurrentCellCore()
        {
            if (this.CurrentColumnIndex != -1 &&
                !SetCurrentCellCore(-1, -1))
            {
                // Edited value couldn't be committed or aborted
                throw DataGridError.DataGrid.CommitFailedCannotCompleteOperation();
            }
        }

        private void SelectAll()
        {
            SetRowsSelection(0, this.RowCount - 1);
        }

        private bool SetAndSelectCurrentCell(int columnIndex,
                                             int rowIndex,
                                             bool forceCurrentCellSelection)
        {
            try
            {
                this._noSelectionChangeCount++;
                if (!SetCurrentCellCore(columnIndex, rowIndex))
                {
                    return false;
                }
                if (IsInnerCellOutOfBounds(columnIndex, rowIndex))
                {
                    return false;
                }
                if (!GetRowSelection(rowIndex))
                {
                    if (forceCurrentCellSelection)
                    {
                        SelectRow(rowIndex, true /*isSelected*/);
                        this._selectionChanged = true;
                    }
                    else
                    {
                        if (this.SelectionMode == DataGridSelectionMode.Extended && this.SelectedItems.Count > 1)
                        {
                            return true;   // Do not discard the multi-selection
                        }
                        if (this.SelectedItems.Count == 1)
                        {
                            int selectedIndex = this.DataConnection.IndexOf(this.SelectedItem);
                            if (selectedIndex != rowIndex)
                            {
                                return true;  // Do not change a single selection that does not match the new current cell
                            }
                        }
                        SelectRow(rowIndex, true /*isSelected*/);
                        this._selectionChanged = true;
                    }
                }
            }
            finally
            {
                this.NoSelectionChangeCount--;
            }
            return true;
        }

        // columnIndex = 2, rowIndex = -1 --> current cell belongs to the 'new row'.
        // columnIndex = 2, rowIndex = 2 --> current cell is an inner cell
        // columnIndex = -1, rowIndex = -1 --> current cell is reset
        // columnIndex = -1, rowIndex = 2 --> Unexpected
        [SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
        private bool SetCurrentCellCore(int columnIndex, int rowIndex, bool commitEdit)
        {
            Debug.Assert(!(columnIndex == -1 && rowIndex > -1));
            Debug.Assert(columnIndex < this.ColumnsItemsInternal.Count);
            Debug.Assert(rowIndex < this.RowCount);
            Debug.Assert(columnIndex == -1 || this.ColumnsItemsInternal[columnIndex].Visibility == Visibility.Visible);

            Debug.Assert(!(columnIndex > -1 && rowIndex == -1)); // 

            if (columnIndex == this.CurrentColumnIndex &&
                rowIndex == this.CurrentRowIndex)
            {
                Debug.Assert(this._editingColumnIndex == -1 || this._editingColumnIndex == this.CurrentColumnIndex);
                Debug.Assert(this._editingRow == null || this.EditingRowIndex == this.CurrentRowIndex);
                return true;
            }

            DataGridRow oldDisplayedCurrentRow = null;
            DataGridCellCoordinates oldCurrentCell = new DataGridCellCoordinates(this.CurrentCellCoordinates);

            if (this.CurrentColumnIndex > -1)
            {
                Debug.Assert(this.CurrentRowIndex > -1 /**/);
                Debug.Assert(this.CurrentColumnIndex < this.ColumnsItemsInternal.Count);
                Debug.Assert(this.CurrentRowIndex < this.RowCount);

                if (!this._temporarilyResetCurrentCell)
                {
                    bool keepFocus = this.ContainsFocus;
                    if (commitEdit)
                    {
                        if (!EndCellEdit(true /*commitCellEdit*/, true /*exitEditingMode*/, keepFocus))
                        {
                            return false;
                        }
                        // Resetting the current cell: setting it to (-1, -1) is not considered setting it out of bounds
                        if ((columnIndex != -1 && rowIndex != -1 && IsInnerCellOutOfBounds(columnIndex, rowIndex)) ||
                            IsInnerCellOutOfBounds(oldCurrentCell.ColumnIndex, oldCurrentCell.RowIndex))
                        {
                            return false;
                        }
                        if (oldCurrentCell.RowIndex != rowIndex && !EndRowEdit(true /*commitRowEdit*/, true /*exitEditingMode*/))
                        {
                            return false;
                        }
                    }
                    else
                    {
                        ExitEdit(keepFocus);
                    }
                }

                if (!IsInnerCellOutOfBounds(oldCurrentCell.ColumnIndex, oldCurrentCell.RowIndex) &&
                    this.DisplayData.IsRowDisplayed(oldCurrentCell.RowIndex))
                {
                    oldDisplayedCurrentRow = this.DisplayData.GetDisplayedRow(oldCurrentCell.RowIndex);
                }
            }

            this.CurrentColumnIndex = columnIndex;
            this.CurrentRowIndex = rowIndex;

            if (this._temporarilyResetCurrentCell)
            {
                if (columnIndex != -1)
                {
                    this._temporarilyResetCurrentCell = false;
                }
            }
            if (!this._temporarilyResetCurrentCell && this._editingColumnIndex != -1)
            {
                this._editingColumnIndex = columnIndex;
            }

            if (oldDisplayedCurrentRow != null)
            {
                if (this.AreRowHeadersVisible)
                {
                    oldDisplayedCurrentRow.ApplyHeaderStatus(true /*animate*/);
                }
                DataGridCell cell = oldDisplayedCurrentRow.Cells[oldCurrentCell.ColumnIndex];
                if (!(this._temporarilyResetCurrentCell && oldDisplayedCurrentRow.IsEditing && this._editingColumnIndex == cell.ColumnIndex))
                {
                    // Don't reset the state of the current cell if we're editing it because that would put it in an invalid state
                    cell.ApplyCellState(true /*animate*/);
                }
            }

            if (this.CurrentColumnIndex > -1)
            {
                Debug.Assert(this.CurrentRowIndex > -1 /**/);
                Debug.Assert(this.CurrentColumnIndex < this.ColumnsItemsInternal.Count);
                Debug.Assert(this.CurrentRowIndex < this.RowCount);
                if (this.DisplayData.IsRowDisplayed(this.CurrentRowIndex))
                {
                    DataGridRow dataGridRow = this.DisplayData.GetDisplayedRow(this.CurrentRowIndex);
                    dataGridRow.Cells[this.CurrentColumnIndex].ApplyCellState(true /*animate*/);
                    if (this.AreRowHeadersVisible)
                    {
                        dataGridRow.ApplyHeaderStatus(true /*animate*/);
                    }
                }
            }

            OnCurrentCellChanged(EventArgs.Empty);

            return true;
        }

        private void SetVerticalOffset(double newVerticalOffset)
        {
            _verticalOffset = newVerticalOffset;
            if (_vScrollBar != null && !DoubleUtil.AreClose(newVerticalOffset, _vScrollBar.Value))
            {
                _vScrollBar.Value = _verticalOffset;
            }
        }

        private void UpdateHorizontalScrollBar(bool needHorizScrollbar, bool forceHorizScrollbar, double totalVisibleWidth, double totalVisibleFrozenWidth, double cellsWidth)
        {
            if (this._hScrollBar != null)
            {
                if (needHorizScrollbar || forceHorizScrollbar)
                {
                    //          viewportSize
                    //        v---v
                    //|<|_____|###|>|
                    //  ^     ^
                    //  min   max

                    // we want to make the relative size of the thumb reflect the relative size of the viewing area
                    // viewportSize / (max + viewportSize) = cellsWidth / max
                    // -> viewportSize = max * cellsWidth / (max - cellsWidth)

                    // always zero
                    this._hScrollBar.Minimum = 0;
                    if (needHorizScrollbar)
                    {
                        // maximum travel distance -- not the total width
                        this._hScrollBar.Maximum = totalVisibleWidth - cellsWidth;
                        Debug.Assert(totalVisibleFrozenWidth >= 0);
                        if (this._frozenColumnScrollBarSpacer != null)
                        {
                            this._frozenColumnScrollBarSpacer.Width = totalVisibleFrozenWidth;
                        }
                        Debug.Assert(this._hScrollBar.Maximum >= 0);

                        // width of the scrollable viewing area
                        double viewPortSize = Math.Max(0, cellsWidth - totalVisibleFrozenWidth);
                        this._hScrollBar.ViewportSize = viewPortSize;
                        this._hScrollBar.LargeChange = viewPortSize;
                        // The ScrollBar should be in sync with HorizontalOffset at this point.  There's a resize case
                        // where the ScrollBar will coerce an old value here, but we don't want that
                        if (this._hScrollBar.Value != this._horizontalOffset)
                        {
                            this._hScrollBar.Value = this._horizontalOffset;
                        }
                    }
                    else
                    {
                        //
                        this._hScrollBar.Maximum = 0;
                        this._hScrollBar.ViewportSize = 0;
                    }

                    if (this._hScrollBar.Visibility != Visibility.Visible)
                    {
                        // This will trigger a call to this method via Cells_SizeChanged for
                        this._ignoreNextScrollBarsLayout = true;
                        // which no processing is needed.
                        this._hScrollBar.Visibility = Visibility.Visible;
                        if (this._hScrollBar.DesiredSize.Height == 0)
                        {
                            // We need to know the height for the rest of layout to work correctly so measure it now
                            this._hScrollBar.Measure(new Size(double.PositiveInfinity, double.PositiveInfinity));
                        }
                    }
                }
                else
                {
                    this._hScrollBar.Maximum = 0;
                    if (this._hScrollBar.Visibility != Visibility.Collapsed)
                    {
                        // This will trigger a call to this method via Cells_SizeChanged for 
                        // which no processing is needed.
                        this._hScrollBar.Visibility = Visibility.Collapsed;
                        this._ignoreNextScrollBarsLayout = true;
                    }
                }

                DataGridAutomationPeer peer = DataGridAutomationPeer.FromElement(this) as DataGridAutomationPeer;
                if (peer != null)
                {
                    peer.RaiseAutomationScrollEvents();
                }
            }
        }

        private void UpdateVerticalScrollBar(bool needVertScrollbar, bool forceVertScrollbar, double totalVisibleHeight, double cellsHeight)
        {
            if (this._vScrollBar != null)
            {
                if (needVertScrollbar || forceVertScrollbar)
                {
                    //          viewportSize
                    //        v---v
                    //|<|_____|###|>|
                    //  ^     ^
                    //  min   max

                    // we want to make the relative size of the thumb reflect the relative size of the viewing area
                    // viewportSize / (max + viewportSize) = cellsWidth / max
                    // -> viewportSize = max * cellsHeight / (totalVisibleHeight - cellsHeight)
                    // ->              = max * cellsHeight / (totalVisibleHeight - cellsHeight)
                    // ->              = max * cellsHeight / max
                    // ->              = cellsHeight

                    // always zero
                    this._vScrollBar.Minimum = 0;
                    if (needVertScrollbar)
                    {
                        // maximum travel distance -- not the total height
                        this._vScrollBar.Maximum = totalVisibleHeight - cellsHeight;
                        Debug.Assert(this._vScrollBar.Maximum >= 0);

                        // total height of the display area
                        this._vScrollBar.ViewportSize = cellsHeight;
                        this._vScrollBar.LargeChange = cellsHeight;
                    }
                    else
                    {
                        //
                        this._vScrollBar.Maximum = 0;
                        this._vScrollBar.ViewportSize = 0;
                    }

                    if (this._vScrollBar.Visibility != Visibility.Visible)
                    {
                        // This will trigger a call to this method via Cells_SizeChanged for 
                        // which no processing is needed.
                        this._vScrollBar.Visibility = Visibility.Visible;
                        if (this._vScrollBar.DesiredSize.Width == 0)
                        {
                            // We need to know the width for the rest of layout to work correctly so measure it now
                            this._vScrollBar.Measure(new Size(double.PositiveInfinity, double.PositiveInfinity));
                        }
                        this._ignoreNextScrollBarsLayout = true;
                    }
                }
                else
                {
                    this._vScrollBar.Maximum = 0;
                    if (this._vScrollBar.Visibility != Visibility.Collapsed)
                    {
                        // This will trigger a call to this method via Cells_SizeChanged for 
                        // which no processing is needed.
                        this._vScrollBar.Visibility = Visibility.Collapsed;
                        this._ignoreNextScrollBarsLayout = true;
                    }
                }

                DataGridAutomationPeer peer = DataGridAutomationPeer.FromElement(this) as DataGridAutomationPeer;
                if (peer != null)
                {
                    peer.RaiseAutomationScrollEvents();
                }
            }
        }

        private void VerticalScrollBar_Scroll(object sender, ScrollEventArgs e)
        {
            ProcessVerticalScroll(e.ScrollEventType);
        }

        #endregion Private Methods
    }
}
