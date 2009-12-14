// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Collections;
using System.Collections.Generic; 
using System.Collections.ObjectModel; 
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis; 
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media; 
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Controls;
 
namespace System.Windows.Controlsb1
{
    [SuppressMessage("Microsoft.Maintainability", "CA1506:AvoidExcessiveClassCoupling")] 
    /// <summary>
    /// Displays data in a customizable grid.
    /// </summary> 
    [TemplatePart(Name = DataGrid.DATAGRID_elementRootName, Type = typeof(Grid))]
    [TemplatePart(Name = DataGrid.DATAGRID_elementCellsName, Type = typeof(Canvas))]
    [TemplatePart(Name = DataGrid.DATAGRID_elementColumnHeadersName, Type = typeof(Canvas))] 
    [TemplatePart(Name = DataGrid.DATAGRID_elementFocusVisualName, Type = typeof(FrameworkElement))] 
    //
 

    [TemplatePart(Name = DataGrid.DATAGRID_elementHorizontalScrollbarName, Type = typeof(ScrollBar))]
    [TemplatePart(Name = DataGrid.DATAGRID_elementRowHeadersName, Type = typeof(Canvas))] 
    //

    [TemplatePart(Name = DataGrid.DATAGRID_elementVerticalScrollbarName, Type = typeof(ScrollBar))] 
    // 

    [TemplatePart(Name = DataGrid.DATAGRID_stateNormalName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DataGrid.DATAGRID_stateUnfocusedName, Type = typeof(Storyboard))]
    public partial class DataGrid : Control
    { 
        #region Constants

        private const string DATAGRID_elementCellsName = "CellsPresenterElement"; 
        private const string DATAGRID_elementColumnHeadersName = "ColumnHeadersPresenterElement"; 
        private const string DATAGRID_elementFocusVisualName = "FocusVisualElement";
        // 

        private const string DATAGRID_elementHorizontalScrollbarName = "HorizontalScrollbarElement";
        private const string DATAGRID_elementRootName = "RootElement"; 
        private const string DATAGRID_elementRowHeadersName = "RowHeadersPresenterElement";
        private const string DATAGRID_elementTopLeftCornerHeaderName = "TopLeftCornerHeaderElement";
        private const string DATAGRID_elementTopRightCornerHeaderName = "TopRightCornerHeaderElement"; 
        // 

        private const string DATAGRID_elementVerticalScrollbarName = "VerticalScrollbarElement"; 

        private const string DATAGRID_partHeaderTemplate = "Part_HeaderTemplate";
 
        private const string DATAGRID_stateNormalName = "Normal State";
        private const string DATAGRID_stateUnfocusedName = "Unfocused State";
 
        // 
        private const bool DATAGRID_defaultCanUserResizeColumns = true;
        private const double DATAGRID_defaultColumnHeadersHeight = 22; 
        private const DataGridHeaders DATAGRID_defaultHeadersVisibility = DataGridHeaders.All;
        private const ScrollBarVisibility DATAGRID_defaultHorizontalScrollBarVisibility = ScrollBarVisibility.Auto;
        private const DataGridRowDetailsVisibility DATAGRID_defaultRowDetailsVisibility = DataGridRowDetailsVisibility.Collapsed; 
        private const double DATAGRID_defaultRowHeadersWidth = 40;
        private const DataGridSelectionMode DATAGRID_defaultSelectionMode = DataGridSelectionMode.ExtendedFullRow;
        private const ScrollBarVisibility DATAGRID_defaultVerticalScrollBarVisibility = ScrollBarVisibility.Auto; 
 
        //
 
        private const double DATAGRID_horizontalGridlinesThickness = 1;
        private const double DATAGRID_minimumRowHeadersWidth = 4;
        private const double DATAGRID_minimumColumnHeadersHeight = 4; 
        private const double DATAGRID_maxHeadersThickness = 32768;
        //
 
        private const double DATAGRID_verticalGridlinesThickness = 1; 

        internal const double DATAGRID_defaultColumnWidth = 100; 
        private const double DATAGRID_defaultRowHeight = 22;
        internal const double DATAGRID_defaultMinColumnWidth = 20;
 
        #endregion Constants

        #region Data 
 
        // DataGrid Template Parts
        private Canvas _cells; 
        private Canvas _columnHeaders;
        private ScrollBar _hScrollBar;
        // 
        private Canvas _rowHeaders;
        private ScrollBar _vScrollBar;
 
        // 
        private int _displayedVerticalGridlineCount;
        private DataGridCellCoordinates _currentCellCoordinates; 
        private FrameworkElement _currentCellFocusVisual;
        private List<DataGridCell> _editingBoundCells;
        private int _editingBoundElementGotFocusListeners; 
        private int _editingBoundElementLostFocusListeners; // Number of subscribers for the LostFocus event of the element of a edited cell in a bound column
        private int _editingColumnIndex;
        private DataGridRow _editingRow; 
        private DataGridEditingRowLocation _editingRowLocation; 
        private Control _editingTemplateControl; // Control that has focus inside a template column
        private DataGridEditingTriggerInfo _editingTriggerInfo; 
        private int _firstDisplayedRowIndex = -1;
        private DataTemplate _headerTemplate;
        // 


 
 
        private double _horizontalOffset;
        private bool _ignoreNextScrollBarsLayout; 
        private byte _horizontalScrollChangesIgnored;
        private int _lastDisplayedRowIndex = -1;
        private byte _layoutSuspended; 
        // prevents reentry into the VerticalScroll event handler
        private bool _makeFirstDisplayedCellCurrentCellPending;
        private double _minColumnWidth; 
        private double _minRowHeight; 
        private int? _mouseOverRowIndex;    // -1 is used for the 'new row'
        // the number of pixels of the firstDisplayedScrollingCol which are not displayed 
        private double _negHorizontalOffset;
        // the number of pixels of DisplayData.FirstDisplayedScrollingRow which are not displayed
        private double _negVerticalOffset; 
        //

        private DataGridNewRowLocation _newRowLocation; 
        private int _noSelectionChangeCount; 
        // Temporarily holds rows for which information is required, for example the exact row height.
        private List<DataGridRow> _prefetchedRows; 
        private List<DataGridRow> _preparedRows;
        private List<DataGridRow> _recyclableRows;
        private int _rowCount; 
        //
        private bool _selectionChanged;
        private IndexToValueTable<Visibility> _showDetailsTable; 
        private bool _temporarilyResetCurrentCell; 
        private ContentControl _topLeftCornerHeader;
        private ContentControl _topRightCornerHeader; 
        private object _uneditedValue; // Represents the original current cell value at the time it enters editing mode.
        //
 

        private byte _verticalScrollChangesIgnored;
 
        // An approximation of the sum of the heights in pixels of the scrolling rows preceding 
        // the first displayed scrolling row.  Since the scrolled off rows are discarded, the grid
        // does not know their actual height. The heights used for the approximation are the ones 
        // set as the rows were scrolled off.
        private double _verticalOffset;
 
        #endregion Data

        #region Events 
 
        public event EventHandler<DataGridAutoGeneratingColumnEventArgs> AutoGeneratingColumn;
        public event EventHandler<DataGridCellEditingCancelEventArgs> BeginningCellEdit; 
        //
        public event EventHandler<DataGridRowCancelEventArgs> CleaningRow;
        public event EventHandler<DataGridColumnEventArgs> ColumnDisplayIndexChanged; 
        public event EventHandler<DataGridCellEventArgs> CommitCellEdit;
        public event EventHandler<DataGridCellCancelEventArgs> CommittingCellEdit;
        public event EventHandler<DataGridRowCancelEventArgs> CommittingRowEdit; 
        public event EventHandler<EventArgs> CurrentCellChanged; 
        public event EventHandler<DataGridDataErrorEventArgs> DataError;
        // 
        public event EventHandler<DataGridPrepareCellEditEventArgs> PrepareCellEdit;
        public event EventHandler<DataGridRowEventArgs> PreparingRow;
        public event EventHandler<DataGridRowDetailsEventArgs> PreparingRowDetails; 
        public event EventHandler<DataGridRowDetailsEventArgs> CleaningRowDetails;
        public event EventHandler<EventArgs> SelectionChanged;
        // 
 
        #endregion Events
 
        /// <summary>
        /// Initializes a new instance of the DataGrid class.
        /// </summary> 
        [SuppressMessage("Microsoft.Performance", "CA1805:DoNotInitializeUnnecessarily", Justification="_minRowHeight should be 0.")]
        public DataGrid()
        { 
            this.TabNavigation = KeyboardNavigationMode.Once; 
            this.IsTabStop = true;
            this.KeyDown += new KeyEventHandler(DataGrid_KeyDown); 
            this.KeyUp += new KeyEventHandler(DataGrid_KeyUp);
            this.GotFocus += new RoutedEventHandler(DataGrid_GotFocus);
            this.LostFocus += new RoutedEventHandler(DataGrid_LostFocus); 

            this.SetValueNoCallback(GridlinesVisibilityProperty, DataGridGridlines.All);
 
            this._prefetchedRows = new List<DataGridRow>(); 
            this._preparedRows = new List<DataGridRow>();
            this._recyclableRows = new List<DataGridRow>(); 
            this._editingBoundCells = new List<DataGridCell>(2);
            this.SelectedItemsInternal = new DataGridSelectedItemsCollection(this);
            this.ColumnsInternal = CreateColumnsInstance(); 

            this.SetValueNoCallback(ColumnWidthProperty, DATAGRID_defaultColumnWidth);
 
            this.DisplayData = new DataGridDisplayData(); 

            this.SetValueNoCallback(RowHeightProperty, DATAGRID_defaultRowHeight); 

            this._minColumnWidth = DATAGRID_defaultMinColumnWidth;
            this._minRowHeight = DataGridRow.DATAGRIDROW_minMinHeight; 

            this.DataConnection = new DataGridDataConnection(this);
            this._editingRowLocation = DataGridEditingRowLocation.Inline; 
            this._newRowLocation = DataGridNewRowLocation.Inline; 
            this._showDetailsTable = new IndexToValueTable<Visibility>();
 
            this.SetValueNoCallback(RowDetailsVisibilityProperty, DATAGRID_defaultRowDetailsVisibility);

            this.AnchorRowIndex = -1; 
            this._editingColumnIndex = -1;
            this._mouseOverRowIndex = null;
            this.CurrentCellCoordinates = new DataGridCellCoordinates(-1, -1); 
 
            this.SetValueNoCallback(HeadersVisibilityProperty, DATAGRID_defaultHeadersVisibility);
            this.SetValueNoCallback(HorizontalScrollBarVisibilityProperty, DATAGRID_defaultHorizontalScrollBarVisibility); 
            this.SetValueNoCallback(VerticalScrollBarVisibilityProperty, DATAGRID_defaultVerticalScrollBarVisibility);
            this.SetValueNoCallback(ColumnHeadersHeightProperty, DATAGRID_defaultColumnHeadersHeight);
            this.SetValueNoCallback(RowHeadersWidthProperty, DATAGRID_defaultRowHeadersWidth); 
            this.SetValueNoCallback(SelectionModeProperty, DATAGRID_defaultSelectionMode);

            // 
 
            this.CanUserResizeColumns = DATAGRID_defaultCanUserResizeColumns;
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
            DataGrid dataGrid = d as DataGrid; 
            if (dataGrid != null)
            {
                // Go through the Displayed rows and update the background 
                for (int childIndex = 0; childIndex < dataGrid.DisplayedAndEditedRowCount; childIndex++)
                {
                    DataGridRow row = dataGrid._cells.Children[childIndex] as DataGridRow; 
                    row.EnsureBackground(); 
                }
                // Go through the Prefetched rows and update the background 
                foreach (DataGridRow row in dataGrid._prefetchedRows)
                {
                    row.EnsureBackground(); 
                }
                // Simply discard the Recyclable rows.  This operation happens less enough that discarding
                // the Recyclable rows here shoudln't be too bad 
                dataGrid._recyclableRows.Clear(); 
            }
        } 
        #endregion AlternatingRowBackground

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
            bool value = (bool)e.NewValue;
 
            if (value)
            {
                DataGrid dataGrid = d as DataGrid; 

                if (dataGrid != null)
                { 
                    dataGrid.RefreshRowsAndColumns(); 
                }
            } 
        }
        #endregion AutoGenerateColumns
 
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
 
        #region ColumnHeadersHeight 
        /// <summary>
        /// Gets or sets the suggested height of the grid's column headers. 
        /// </summary>
        public double ColumnHeadersHeight
        { 
            get { return (double)GetValue(ColumnHeadersHeightProperty); }
            set { SetValue(ColumnHeadersHeightProperty, value); }
        } 
 
        /// <summary>
        /// Identifies the ColumnHeadersHeight dependency property. 
        /// </summary>
        public static readonly DependencyProperty ColumnHeadersHeightProperty =
            DependencyProperty.Register( 
                "ColumnHeadersHeight",
                typeof(double),
                typeof(DataGrid), 
                new PropertyMetadata(OnColumnHeadersHeightPropertyChanged)); 

        /// <summary> 
        /// ColumnHeadersHeightProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its ColumnHeadersHeight.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnColumnHeadersHeightPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            DataGrid source = d as DataGrid; 
            Debug.Assert(source != null,
                "The source is not an instance of DataGrid!"); 

            Debug.Assert(typeof(double).IsInstanceOfType(e.NewValue),
                "The value is not an instance of double!"); 
            double value = (double)e.NewValue;

            if (!source.IsHandlerSuspended(e.Property)) 
            { 
                if (value < DATAGRID_minimumColumnHeadersHeight)
                { 
                    d.SetValueNoCallback(e.Property, e.OldValue);
                    throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "ColumnHeadersHeight", DATAGRID_minimumColumnHeadersHeight);
                } 
                if (value > DATAGRID_maxHeadersThickness)
                {
                    d.SetValueNoCallback(e.Property, e.OldValue); 
                    throw DataGridError.DataGrid.ValueMustBeLessThanOrEqualTo("value", "ColumnHeadersHeight", DATAGRID_maxHeadersThickness); 
                }
 
                source.PerformLayout();
            }
        } 
        #endregion ColumnHeadersHeight

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
                    foreach (DataGridColumnBase column in dataGrid.Columns) 
                    {
                        EnsureColumnHeaderCellStyle(column, e.OldValue as Style, newStyle);
                    } 
                } 
            }
        } 
        #endregion ColumnHeaderStyle

        #region ColumnWidth 
        /// <summary>
        /// Gets or sets the width of the grid's columns.
        /// </summary> 
        public double ColumnWidth 
        {
            get { return (double)GetValue(ColumnWidthProperty); } 
            set { SetValue(ColumnWidthProperty, value); }
        }
 
        /// <summary>
        /// Identifies the ColumnWidth dependency property.
        /// </summary> 
        public static readonly DependencyProperty ColumnWidthProperty = 
            DependencyProperty.Register(
                "ColumnWidth", 
                typeof(double),
                typeof(DataGrid),
                new PropertyMetadata(OnColumnWidthPropertyChanged)); 

        /// <summary>
        /// ColumnWidthProperty property changed handler. 
        /// </summary> 
        /// <param name="d">DataGrid that changed its ColumnWidth.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnColumnWidthPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid source = d as DataGrid; 
            Debug.Assert(source != null,
                "The source is not an instance of DataGrid!");
 
            Debug.Assert(typeof(double).IsInstanceOfType(e.NewValue), 
                "The value is not an instance of double!");
            double value = (double)e.NewValue; 

            if (!source.IsHandlerSuspended(e.Property))
            { 
                // coerce value
                if (value < source.MinColumnWidth)
                { 
                    source.SetValueNoCallback(ColumnWidthProperty, source.MinColumnWidth); 
                }
                if (value > DataGridColumnBase.DATAGRIDCOLUMN_maximumWidth) 
                {
                    d.SetValueNoCallback(e.Property, e.OldValue);
                    throw DataGridError.DataGrid.ValueMustBeLessThanOrEqualTo("value", "ColumnWidth", DataGridColumnBase.DATAGRIDCOLUMN_maximumWidth); 
                }

                // validated against inherited value if appropriate 
                bool inheritingGridColumnWidth = false; 
                foreach (DataGridColumnBase dataGridColumn in source.ColumnsItemsInternal)
                { 
                    if (dataGridColumn.LocalWidth == 0)
                    {
                        inheritingGridColumnWidth = true; 
                        if (value < dataGridColumn.InheritedMinWidth)
                        {
                            d.SetValueNoCallback(e.Property, e.OldValue); 
                            throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "ColumnWidth", dataGridColumn.InheritedMinWidth); 
                            //
                        } 
                    }
                }
 
                if (inheritingGridColumnWidth)
                {
                    // The inherited ColumnWidth has changed so layout needs to be updated 
                    source.PerformLayout(); 
                }
            } 
        }
        #endregion ColumnWidth
 
        //
        #region CornerHeaderStyle
        /// <summary> 
        /// 
        /// </summary>
        public Style CornerHeaderStyle 
        {
            get { return GetValue(CornerHeaderStyleProperty) as Style; }
            set { SetValue(CornerHeaderStyleProperty, value); } 
        }

        public static readonly DependencyProperty CornerHeaderStyleProperty = 
            DependencyProperty.Register( 
                "CornerHeaderStyle",
                typeof(Style), 
                typeof(DataGrid),
                new PropertyMetadata(OnCornerHeaderStylePropertyChanged));
 
        private static void OnCornerHeaderStylePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Style newStyle = e.NewValue as Style; 
            if (newStyle != null) 
            {
                DataGrid dataGrid = d as DataGrid; 
                if (dataGrid != null)
                {
                    if (dataGrid._topLeftCornerHeader != null) 
                    {
                        dataGrid._topLeftCornerHeader.Style = newStyle;
                    } 
                    if (dataGrid._topRightCornerHeader != null) 
                    {
                        dataGrid._topRightCornerHeader.Style = newStyle; 
                    }
                }
            } 
        }
        #endregion
 
        #region GridlinesVisibility 
        /// <summary>
        /// Gets or sets a value that indicates whether horizontal or vertical gridlines for 
        /// the inner cells should be displayed.
        /// </summary>
        public DataGridGridlines GridlinesVisibility 
        {
            get { return (DataGridGridlines)GetValue(GridlinesVisibilityProperty); }
            set { SetValue(GridlinesVisibilityProperty, value); } 
        } 

        /// <summary> 
        /// Identifies the Gridlines dependency property.
        /// </summary>
        public static readonly DependencyProperty GridlinesVisibilityProperty = 
            DependencyProperty.Register(
                "GridlinesVisibility",
                typeof(DataGridGridlines), 
                typeof(DataGrid), 
                new PropertyMetadata(OnGridlinesVisibilityPropertyChanged));
 
        /// <summary>
        /// GridlinesProperty property changed handler.
        /// </summary> 
        /// <param name="d">DataGrid that changed its Gridlines.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnGridlinesVisibilityPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        { 
            DataGrid source = d as DataGrid;
            Debug.Assert(source != null, 
                "The source is not an instance of DataGrid!");

            Debug.Assert(typeof(DataGridGridlines).IsInstanceOfType(e.NewValue) || (e.NewValue == null), 
                "The value is not an instance of DataGridGridlines!");

            if (!source.IsHandlerSuspended(e.Property)) 
            { 
                source.PerformLayout();
            } 
        }
        #endregion GridlinesVisibility
 
        #region HeadersVisibility
        /// <summary>
        /// Gets or sets a value that indicates whether column or row headers should be displayed. 
        /// </summary> 
        public DataGridHeaders HeadersVisibility
        { 
            get { return (DataGridHeaders)GetValue(HeadersVisibilityProperty); }
            set { SetValue(HeadersVisibilityProperty, value); }
        } 

        /// <summary>
        /// Identifies the HeadersVisibility dependency property. 
        /// </summary> 
        public static readonly DependencyProperty HeadersVisibilityProperty =
            DependencyProperty.Register( 
                "HeadersVisibility",
                typeof(DataGridHeaders),
                typeof(DataGrid), 
                new PropertyMetadata(OnHeadersVisibilityPropertyChanged));

        /// <summary> 
        /// HeadersVisibilityProperty property changed handler. 
        /// </summary>
        /// <param name="d">DataGrid that changed its HeadersVisibility.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnHeadersVisibilityPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            DataGrid source = d as DataGrid;
            Debug.Assert(source != null,
                "The source is not an instance of DataGrid!"); 
 
            Debug.Assert(typeof(DataGridHeaders).IsInstanceOfType(e.NewValue),
                "The value is not an instance of DataGridHeaders!"); 
            DataGridHeaders newValue = (DataGridHeaders)e.NewValue;
            DataGridHeaders oldValue = (DataGridHeaders)e.OldValue;
 
            Func<DataGridHeaders, DataGridHeaders, bool> hasFlags = (DataGridHeaders value, DataGridHeaders flags) => ((value & flags) == flags);

            bool newValueCols = hasFlags(newValue, DataGridHeaders.Column); 
            bool newValueRows = hasFlags(newValue, DataGridHeaders.Row); 
            bool oldValueCols = hasFlags(oldValue, DataGridHeaders.Column);
            bool oldValueRows = hasFlags(oldValue, DataGridHeaders.Row); 

            if (!source.IsHandlerSuspended(e.Property))
            { 
                // Columns
                if (newValueCols != oldValueCols)
                { 
                    if (source._columnHeaders != null) 
                    {
                        if (newValueCols) 
                        {
                            foreach (DataGridColumnBase dataGridColumn in source.Columns)
                            { 
                                source.InsertDisplayedColumnHeader(dataGridColumn);
                            }
                        } 
                        else 
                        {
                            source.RemoveDisplayedColumnHeaders(); 
                        }
                    }
                    source.PerformLayout(); 
                }

                // Rows 
                if (newValueRows != oldValueRows) 
                {
                    if (source._cells != null && source._rowHeaders != null) 
                    {
                        for (int childIndex = 0; childIndex < source.DisplayedAndEditedRowCount; childIndex++)
                        { 
                            DataGridRow dataGridRow = source._cells.Children[childIndex] as DataGridRow;
                            Debug.Assert(dataGridRow != null);
                            if (newValueRows) 
                            { 
                                source.InsertDisplayedRowHeader(dataGridRow, childIndex);
                                dataGridRow.ApplyBackgroundBrush(false /*animate*/); 
                                dataGridRow.HeaderCell.ApplyRowStatus(false /*animate*/);
                                if (source.RowHeaderStyle != null)
                                { 
                                    EnsureRowHeaderCellStyle(dataGridRow, null, source.RowHeaderStyle);
                                }
                            } 
                            else if (dataGridRow.HasHeaderCell) 
                            {
                                source.RemoveDisplayedRowHeader(0); 
                            }
                        }
                    } 
                    source.PerformLayout();
                }
            } 
        } 
        #endregion HeadersVisibility
 
        #region HorizontalGridlinesBrush
        /// <summary>
        /// Gets or sets a brush that describes the horizontal gridlines color. 
        /// </summary>
        public Brush HorizontalGridlinesBrush
        { 
            get { return GetValue(HorizontalGridlinesBrushProperty) as Brush; } 
            set { SetValue(HorizontalGridlinesBrushProperty, value); }
        } 

        /// <summary>
        /// Identifies the HorizontalGridlinesBrush dependency property. 
        /// </summary>
        public static readonly DependencyProperty HorizontalGridlinesBrushProperty =
            DependencyProperty.Register( 
                "HorizontalGridlinesBrush", 
                typeof(Brush),
                typeof(DataGrid), 
                new PropertyMetadata(OnHorizontalGridlinesBrushPropertyChanged));

        /// <summary> 
        /// HorizontalGridlinesBrushProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGrid that changed its HorizontalGridlinesBrush.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnHorizontalGridlinesBrushPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            DataGrid source = d as DataGrid;
            Debug.Assert(source != null,
                "The source is not an instance of DataGrid!"); 

            Debug.Assert(typeof(Brush).IsInstanceOfType(e.NewValue) || (e.NewValue == null),
                "The value is not an instance of Brush!"); 
 
            if (!source.IsHandlerSuspended(e.Property))
            { 
                source.RemoveDisplayedHorizontalGridlines();
                source.PerformLayout();
            } 
        }
        #endregion HorizontalGridlinesBrush
 
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
            DataGrid source = d as DataGrid; 
            Debug.Assert(source != null,
                "The source is not an instance of DataGrid!"); 
            Debug.Assert(typeof(ScrollBarVisibility).IsInstanceOfType(e.NewValue),
                "The value is not an instance of ScrollBarVisibility!");
 
            if (!source.IsHandlerSuspended(e.Property) &&
                (ScrollBarVisibility)e.NewValue != (ScrollBarVisibility)e.OldValue &&
                source._hScrollBar != null) 
            { 
                source.PerformLayout();
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
            DataGrid source = d as DataGrid;
            Debug.Assert(source != null,
                "The source is not an instance of DataGrid!"); 
 
            Debug.Assert(typeof(bool).IsInstanceOfType(e.NewValue),
                "The value is not an instance of bool!"); 
            bool value = (bool)e.NewValue;

            if (!source.IsHandlerSuspended(e.Property)) 
            {
                if (value && !source.EndEdit(true /*commitChanges*/, true /*exitEditingMode*/))
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
            DataGrid source = d as DataGrid;
            Debug.Assert(source != null, 
                "The source is not an instance of DataGrid!"); 

            Debug.Assert(typeof(IEnumerable).IsInstanceOfType(e.NewValue) || (e.NewValue == null), 
                "The value is not an instance of IEnumerable!");
            IEnumerable newValue = (IEnumerable)e.NewValue;
            IEnumerable oldValue = (IEnumerable)e.OldValue; 

            if (source.DataConnection != null)
            { 
                source.DataConnection.UnWireEvents(oldValue); 

                source.DataConnection.ClearDataProperties(); 

                if (newValue != null)
                { 
                    source.DataConnection.WireEvents(newValue);
                }
            } 
 
            // we always want to do this
            source.RefreshRowsAndColumns(); 
        }
        #endregion ItemsSource
 
        #region OverrideRowDetailsScrolling
        /// <summary>
        /// Gets or sets a value indicating whether the horizontal ScrollBar of the DataGrid affects the 
        /// details section of a row. 
        /// </summary>
        public bool OverrideRowDetailsScrolling 
        {
            get { return (bool)GetValue(OverrideRowDetailsScrollingProperty); }
            set { SetValue(OverrideRowDetailsScrollingProperty, value); } 
        }

        /// <summary> 
        /// Identifies the OverrideRowDetailsScrolling dependency property. 
        /// </summary>
        public static readonly DependencyProperty OverrideRowDetailsScrollingProperty = 
            DependencyProperty.Register(
                "OverrideRowDetailsScrolling",
                typeof(bool), 
                typeof(DataGrid),
                null);
        #endregion OverrideRowDetailsScrolling 
 
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
            DataGrid dataGrid = d as DataGrid;
            if (dataGrid != null)
            { 
                // Go through the Displayed rows and update the background 
                for (int childIndex = 0; childIndex < dataGrid.DisplayedAndEditedRowCount; childIndex++)
                { 
                    DataGridRow row = dataGrid._cells.Children[childIndex] as DataGridRow;
                    row.EnsureBackground();
                } 
                // Go through the Prefetched rows and update the background
                foreach (DataGridRow row in dataGrid._prefetchedRows)
                { 
                    row.EnsureBackground(); 
                }
                // Simply discard the Recyclable rows.  This operation happens less enough that discarding 
                // the Recyclable rows here shoudln't be too bad
                dataGrid._recyclableRows.Clear();
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
                null); 
        #endregion RowDetailsTemplate
 
        #region RowDetailsVisibility
        /// <summary>
        /// Gets or sets a value that indicates when the details section of a row should be displayed. 
        /// </summary>
        //
        [SuppressMessage("Microsoft.Naming", "CA1721:PropertyNamesShouldNotMatchGetMethods")] 
        public DataGridRowDetailsVisibility RowDetailsVisibility 
        {
            get { return (DataGridRowDetailsVisibility)GetValue(RowDetailsVisibilityProperty); } 
            set { SetValue(RowDetailsVisibilityProperty, value); }
        }
 
        /// <summary>
        /// Identifies the RowDetailsVisibility dependency property.
        /// </summary> 
        public static readonly DependencyProperty RowDetailsVisibilityProperty = 
            DependencyProperty.Register(
                "RowDetailsVisibility", 
                typeof(DataGridRowDetailsVisibility),
                typeof(DataGrid),
                new PropertyMetadata(OnRowDetailsVisibilityPropertyChanged)); 

        /// <summary>
        /// RowDetailsVisibilityProperty property changed handler. 
        /// </summary> 
        /// <param name="d">DataGrid that changed its RowDetailsVisibility.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnRowDetailsVisibilityPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid source = d as DataGrid; 
            Debug.Assert(source != null,
                "The source is not an instance of DataGrid!");
 
            Debug.Assert(typeof(DataGridRowDetailsVisibility).IsInstanceOfType(e.NewValue), 
                "The value is not an instance of DataGridRowDetailsVisibility!");
 
            DataGridRowDetailsVisibility newValue = (DataGridRowDetailsVisibility)e.NewValue;
            DataGridRowDetailsVisibility oldValue = (DataGridRowDetailsVisibility)e.OldValue;
 
            source._layoutSuspended++;
            try
            { 
                if (source._cells != null) 
                {
                    for (int childIndex = 0; childIndex < source.DisplayedAndEditedRowCount; childIndex++) 
                    {
                        DataGridRow row = source._cells.Children[childIndex] as DataGridRow;
                        Visibility oldVisible = source.GetRowDetailsVisibility(row.Index, oldValue); 
                        Visibility newVisible = source.GetRowDetailsVisibility(row.Index, newValue);
                        if (row != null && oldVisible != newVisible)
                        { 
                            row.SetAreRowDetailsVisibleInternal((newVisible == Visibility.Visible), false /* animate */); 
                        }
                    } 
                }
            }
            finally 
            {
                source.ResumeLayout(true);
            } 
        } 
        #endregion RowDetailsVisibility
 
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
            DataGrid source = d as DataGrid;
            Debug.Assert(source != null, 
                "The source is not an instance of DataGrid!");

            Debug.Assert(typeof(double).IsInstanceOfType(e.NewValue), 
                "The value is not an instance of double!"); 
            double value = (double)e.NewValue;
 
            if (!source.IsHandlerSuspended(e.Property))
            {
                if (value < source.MinRowHeight) 
                {
                    value = source.MinRowHeight;
                } 
                if (value > DataGridRow.DATAGRIDROW_maximumHeight) 
                {
                    d.SetValueNoCallback(e.Property, e.OldValue); 
                    throw DataGridError.DataGrid.ValueMustBeLessThanOrEqualTo("value", "RowHeight", DataGridRow.DATAGRIDROW_maximumHeight);
                }
 
                // Update the display rows
                source.SuspendLayout();
                try 
                { 
                    for (int childIndex = 0; childIndex < source.DisplayedAndEditedRowCount; childIndex++)
                    { 
                        DataGridRow row = source._cells.Children[childIndex] as DataGridRow;
                        row.Height = value;
                        // 


                    } 
                } 
                finally
                { 
                    source.ResumeLayout(false);
                }
                source.PerformLayout(true /* forceDataCellsHorizontalLayout */); 

                // Clear the prefetched and recycled rows; This needs to be done after updating the displayed rows
                // because the update might add recyclable rows that we want to discard 
                source._prefetchedRows.Clear(); 
                source._recyclableRows.Clear();
            } 
        }
        #endregion RowHeight
 
        #region RowHeadersWidth
        /// <summary>
        /// Gets or sets the width of the grid's row headers. 
        /// </summary> 
        public double RowHeadersWidth
        { 
            get { return (double)GetValue(RowHeadersWidthProperty); }
            set { SetValue(RowHeadersWidthProperty, value); }
        } 

        /// <summary>
        /// Identifies the RowHeadersWidth dependency property. 
        /// </summary> 
        public static readonly DependencyProperty RowHeadersWidthProperty =
            DependencyProperty.Register( 
                "RowHeadersWidth",
                typeof(double),
                typeof(DataGrid), 
                new PropertyMetadata(OnRowHeadersWidthPropertyChanged));

        /// <summary> 
        /// RowHeadersWidthProperty property changed handler. 
        /// </summary>
        /// <param name="d">DataGrid that changed its RowHeadersWidth.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnRowHeadersWidthPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            DataGrid source = d as DataGrid;
            Debug.Assert(source != null,
                "The source is not an instance of DataGrid!"); 
 
            Debug.Assert(typeof(double).IsInstanceOfType(e.NewValue),
                "The value is not an instance of double!"); 
            double value = (double)e.NewValue;

            if (!source.IsHandlerSuspended(e.Property)) 
            {
                if (value < DATAGRID_minimumRowHeadersWidth)
                { 
                    d.SetValueNoCallback(e.Property, e.OldValue); 
                    throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "RowHeaderWidth", DATAGRID_minimumRowHeadersWidth);
                } 
                if (value > DATAGRID_maxHeadersThickness)
                {
                    d.SetValueNoCallback(e.Property, e.OldValue); 
                    throw DataGridError.DataGrid.ValueMustBeLessThanOrEqualTo("value", "RowHeaderWidth", DATAGRID_maxHeadersThickness);
                }
 
                source.PerformLayout(); 
            }
        } 
        #endregion RowHeadersWidth

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
                if (dataGrid != null) 
                {
                    // Set HeaderCell.Style for displayed rows if HeaderCell.Style is not already set
                    for (int childIndex = 0; childIndex < dataGrid.DisplayedAndEditedRowCount; childIndex++) 
                    { 
                        EnsureRowHeaderCellStyle(dataGrid._cells.Children[childIndex] as DataGridRow, e.OldValue as Style, newStyle);
                    } 
                    // Set HeaderCell.Style for prefetched rows if HeaderCell.Style is not already set
                    foreach (DataGridRow row in dataGrid._prefetchedRows)
                    { 
                        EnsureRowHeaderCellStyle(row, e.OldValue as Style, newStyle);
                    }
                    // Simply discard the Recyclable rows.  This operation happens less enough that discarding 
                    // the Recyclable rows here shoudln't be too bad 
                    dataGrid._recyclableRows.Clear();
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
                    // Set the style for displayed rows if it has not already been set
                    for (int childIndex = 0; childIndex < dataGrid.DisplayedAndEditedRowCount; childIndex++)
                    { 
                        EnsureRowStyle(dataGrid._cells.Children[childIndex] as DataGridRow, oldStyle, newStyle); 
                    }
                    // Set the style for prefetched rows if it has not already been set 
                    foreach (DataGridRow row in dataGrid._prefetchedRows)
                    {
                        EnsureRowStyle(row, oldStyle, newStyle); 
                    }
                    // Simply discard the Recyclable rows.  This operation happens less enough that discarding
                    // the Recyclable rows here shoudln't be too bad 
                    dataGrid._recyclableRows.Clear(); 
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
            DataGrid source = d as DataGrid;
            Debug.Assert(source != null,
                "The source is not an instance of DataGrid!"); 
 
            Debug.Assert(typeof(DataGridSelectionMode).IsInstanceOfType(e.NewValue),
                "The value is not an instance of bool!"); 

            if (!source.IsHandlerSuspended(e.Property))
            { 
                source.ClearRowSelection(true /*resetAnchorRowIndex*/);
            }
        } 
        #endregion SelectionMode 

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
            DataGrid source = d as DataGrid; 
            Debug.Assert(source != null,
                "The source is not an instance of DataGrid!");
 
            Debug.Assert(typeof(object).IsInstanceOfType(e.NewValue) || (e.NewValue == null),
                "The value is not an instance of object!");
 
            if (!source.IsHandlerSuspended(e.Property)) 
            {
                if (e.NewValue == null) 
                {
                    if (!source.EndEdit(true /*commitChanges*/, true /*exitEditingMode*/))
                    { 
                        // Edited value couldn't be committed or aborted
                        d.SetValueNoCallback(e.Property, e.OldValue);
                        throw DataGridError.DataGrid.CommitFailedCannotCompleteOperation(); 
                    } 
                    // Clear all row selections
                    source.ClearRowSelection(true /*resetAnchorRowIndex*/); 
                }
                else
                { 
                    int rowIndex = source.DataConnection.IndexOf(e.NewValue);
                    if (rowIndex == -1)
                    { 
                        // Silently fail if the provided dataItem is not found. 
                        return;
                    } 
                    if (rowIndex != source.CurrentRowIndex)
                    {
                        if (!source.EndEdit(true /*commitChanges*/, true /*exitEditingMode*/)) 
                        {
                            // Edited value couldn't be committed or aborted
                            d.SetValueNoCallback(e.Property, e.OldValue); 
                            throw DataGridError.DataGrid.CommitFailedCannotCompleteOperation(); 
                        }
                        if (source.IsRowOutOfBounds(rowIndex)) 
                        {
                            return;
                        } 
                    }
                    try
                    { 
                        source._noSelectionChangeCount++; 
                        source.ClearRowSelection(rowIndex /*rowIndexException*/, true /*resetAnchorRowIndex*/);
                        int columnIndex = source.CurrentColumnIndex; 
                        if (columnIndex == -1)
                        {
                            columnIndex = source.ColumnsInternal.FirstVisibleColumn == null ? -1 : source.ColumnsInternal.FirstVisibleColumn.Index; 
                        }
                        if (columnIndex == -1 || source.IsRowOutOfBounds(rowIndex))
                        { 
                            return; 
                        }
                        bool success = source.SetCurrentCellCore(columnIndex, rowIndex/*, false, false*/); 
                        Debug.Assert(success);
                    }
                    finally 
                    {
                        source.NoSelectionChangeCount--;
                    } 
                } 
            }
        } 
        #endregion SelectedItem

        #region SelectedItems 
        /// <summary>
        /// Gets the currently selected items.
        /// </summary> 
        public IList SelectedItems 
        {
            get { return this.SelectedItemsInternal as IList; } 
        }

        /// <summary> 
        /// Identifies the SelectedItems dependency property.
        /// </summary>
        // 
        public static readonly DependencyProperty SelectedItemsProperty = 
            DependencyProperty.Register(
                "SelectedItems", 
                typeof(IList /*DataGridSelectedItemsCollection /*only*/),
                typeof(DataGrid),
                new PropertyMetadata(OnSelectedItemsPropertyChanged)); 

        /// <summary>
        /// SelectedItemsProperty property changed handler. 
        /// </summary> 
        /// <param name="d">DataGrid that changed its SelectedItems.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnSelectedItemsPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid source = d as DataGrid; 
            Debug.Assert(source != null,
                "The source is not an instance of DataGrid!");
 
            if (!typeof(DataGridSelectedItemsCollection).IsInstanceOfType(e.NewValue) || (e.NewValue == null)) 
            {
                d.SetValueNoCallback(e.Property, e.OldValue); 
                throw DataGridError.DataGrid.ValueIsNotAnInstanceOf("value", typeof(DataGridSelectedItemsCollection));
            }
            else if (e.OldValue != null || source != ((DataGridSelectedItemsCollection)e.NewValue).OwningGrid) 
            {
                d.SetValueNoCallback(e.Property, e.OldValue);
                throw DataGridError.DataGrid.ValueIsReadOnly("SelectedItems"); 
            } 
        }
 
        /// <summary>
        ///
        /// </summary> 
        internal DataGridSelectedItemsCollection SelectedItemsInternal
        {
            get { return GetValue(SelectedItemsProperty) as DataGridSelectedItemsCollection; } 
            private set { SetValue(SelectedItemsProperty, value); } 
        }
 
        #endregion SelectedItems

        #region VerticalGridlinesBrush 
        /// <summary>
        /// Gets or sets a brush that describes the vertical gridlines color.
        /// </summary> 
        public Brush VerticalGridlinesBrush 
        {
            get { return GetValue(VerticalGridlinesBrushProperty) as Brush; } 
            set { SetValue(VerticalGridlinesBrushProperty, value); }
        }
 
        /// <summary>
        /// Identifies the VerticalGridlinesBrush dependency property.
        /// </summary> 
        public static readonly DependencyProperty VerticalGridlinesBrushProperty = 
            DependencyProperty.Register(
                "VerticalGridlinesBrush", 
                typeof(Brush),
                typeof(DataGrid),
                new PropertyMetadata(OnVerticalGridlinesBrushPropertyChanged)); 

        /// <summary>
        /// VerticalGridlinesBrushProperty property changed handler. 
        /// </summary> 
        /// <param name="d">DataGrid that changed its VerticalGridlinesBrush.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnVerticalGridlinesBrushPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGrid source = d as DataGrid; 
            Debug.Assert(source != null,
                "The source is not an instance of DataGrid!");
 
            Debug.Assert(typeof(Brush).IsInstanceOfType(e.NewValue) || (e.NewValue == null), 
                "The value is not an instance of Brush!");
 
            if (!source.IsHandlerSuspended(e.Property))
            {
                source.RemoveDisplayedVerticalGridlines(); 
                source.PerformLayout();
            }
        } 
        #endregion VerticalGridlinesBrush 

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
            DataGrid source = d as DataGrid; 
            Debug.Assert(source != null,
                "The source is not an instance of DataGrid!");
            Debug.Assert(typeof(ScrollBarVisibility).IsInstanceOfType(e.NewValue), 
                "The value is not an instance of ScrollBarVisibility!");

            if (!source.IsHandlerSuspended(e.Property) && 
                (ScrollBarVisibility)e.NewValue != (ScrollBarVisibility)e.OldValue && 
                source._vScrollBar != null)
            { 
                source.PerformLayout();
            }
        } 
        #endregion VerticalScrollBarVisibility

        #endregion Dependency Properties 
 
        #region Public Properties
 
        /*

 


 
 

 


 


 
 

 


 


 
 

 


 

*/
 
        // 

        /// <summary> 
        /// Gets the collection of columns currently present in the DataGrid.
        /// </summary>
        public ObservableCollection<DataGridColumnBase> Columns 
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
        public DataGridColumnBase CurrentColumn
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
                DataGridColumnBase dataGridColumn = value; 
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
                        BeginCellEdit(new DataGridEditingTriggerInfo(this.ContainsFocus, ModifierKeys.None, null, null)); 
                    } 
                }
            } 
        }

        // 
        public DataTemplate HeaderTemplate
        {
            get 
            { 
                return this._headerTemplate;
            } 
            set
            {
                if (value == null) 
                {
                    throw DataGridError.DataGrid.ValueCannotBeSetToNull("value", "HeaderTemplate");
                } 
                if (this._headerTemplate != value) 
                {
                    this._headerTemplate = value; 
                }
            }
        } 

        /*
 
 

 


 


 
 

 


 


 
*/ 

        // 

        private DataGridNewRowLocation NewRowLocation
        { 
            get
            {
                return this._newRowLocation; 
            } 
            set
            { 
                if (this._newRowLocation != value)
                {
                    this._newRowLocation = value; 
                }
            }
        } 
 
        /*
 


 


 
 

 


 


 
 

 
*/

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
 
        //
 


 


 
 

 
        internal double CellsWidth
        {
            get 
            {
                return (this._cells == null) ? 0 : this._cells.ActualWidth;
            } 
        } 

        internal Canvas ColumnHeaders 
        {
            get
            { 
                return this._columnHeaders;
            }
        } 
 
        //
        internal DataGridColumnCollection ColumnsInternal 
        {
            get;
            private set; 
        }

        internal List<DataGridColumnBase> ColumnsItemsInternal 
        { 
            get
            { 
                return this.ColumnsInternal.ItemsInternal;
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
 
        internal static double HorizontalGridlinesThickness
        { 
            get
            {
                return DATAGRID_horizontalGridlinesThickness; 
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
                double widthNotVisible = Math.Max(0, this.ColumnsInternal.GetVisibleEdgedColumnsWidth(true /*includeLastRightGridlineWhenPresent*/) - this.CellsWidth); 
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

                this.DisplayData.Dirty = false;
            } 
        } 

        internal bool InDisplayIndexAdjustments 
        {
            get;
            set; 
        }

        internal double MinColumnWidth 
        { 
            get
            { 
                return this._minColumnWidth;
            }
            set 
            {
                if (this._minColumnWidth != value)
                { 
                    if (value < DataGridColumnBase.DATAGRIDCOLUMN_minMinWidth) 
                    {
                        throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "MinColumnWidth", DataGridColumnBase.DATAGRIDCOLUMN_minMinWidth); 
                    }
                    if (this.ColumnWidth < value)
                    { 
                        this.ColumnWidth = value;
                    }
                    this._minColumnWidth = value; 
                } 
            }
        } 

        internal double MinRowHeight
        { 
            get
            {
                return this._minRowHeight; 
            } 
            set
            { 
                if (this._minRowHeight != value)
                {
                    if (value < DataGridRow.DATAGRIDROW_minMinHeight) 
                    {
                        throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "MinRowHeight", DataGridRow.DATAGRIDROW_minMinHeight);
                    } 
                    // 

 


 


                    this._minRowHeight = value; 
                } 
            }
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
                    if (this._mouseOverRowIndex != null && IsRowDisplayed((int)this._mouseOverRowIndex))
                    {
                        oldMouseOverRow = GetDisplayedRow((int)this._mouseOverRowIndex); 
                    } 
                    this._mouseOverRowIndex = value;
                    if (oldMouseOverRow != null) 
                    {
                        oldMouseOverRow.ApplyBackgroundBrush(true /*animate*/);
                    } 
                    if (this._mouseOverRowIndex != null && IsRowDisplayed((int)this._mouseOverRowIndex))
                    {
                        GetDisplayedRow((int)this._mouseOverRowIndex).ApplyBackgroundBrush(true /*animate*/); 
                    } 
                }
            } 
        }

        internal Canvas RowHeaders 
        {
            get
            { 
                return this._rowHeaders; 
            }
        } 

        internal static double VerticalGridlinesThickness
        { 
            get
            {
                return DATAGRID_verticalGridlinesThickness; 
            } 
        }
 
        #endregion Internal Properties

        #region Private Properties 

        private bool AreColumnHeadersVisible
        { 
            get 
            {
                return (this.HeadersVisibility & DataGridHeaders.Column) == DataGridHeaders.Column; 
            }
        }
 
        private bool AreRowHeadersVisible
        {
            get 
            { 
                return (this.HeadersVisibility & DataGridHeaders.Row) == DataGridHeaders.Row;
            } 
        }

        private double CellsHeight 
        {
            get
            { 
                return (this._cells == null) ? 0 : this._cells.ActualHeight; 
            }
        } 

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

        private int DisplayedRowCount 
        { 
            get
            { 
                return this._firstDisplayedRowIndex == -1 ? 0 : (this._lastDisplayedRowIndex - this._firstDisplayedRowIndex + 1);
            }
        } 

        private int DisplayedAndEditedRowCount
        { 
            get 
            {
                switch (this._editingRowLocation) 
                {
                    case DataGridEditingRowLocation.Inline:
                        return this.DisplayedRowCount; 
                    default:
                        return this.DisplayedRowCount + 1;
                } 
            } 
        }
 
        private int FirstDisplayedColumnIndex
        {
            get 
            {
                int firstDisplayedColumnIndex = -1;
                DataGridColumnBase dataGridColumn = this.ColumnsInternal.FirstVisibleColumn; 
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
            return BeginEdit(new DataGridEditingTriggerInfo(this.ContainsFocus, ModifierKeys.None, null, null));
        } 

        /// <summary>
        /// Enters editing mode for the current cell and current row (if they're not already in editing mode). 
        /// </summary>
        /// <param name="editingTriggerInfo">Provides information about the user gesture that caused the call to BeginEdit. Can be null.</param>
        /// <returns>True if operation was successful. False otherwise.</returns> 
        public bool BeginEdit(DataGridEditingTriggerInfo editingTriggerInfo) 
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
            Debug.Assert(this.CurrentRowIndex < this._rowCount);
            Debug.Assert(this._editingRow == null || this.EditingRowIndex == this.CurrentRowIndex);
 
            if (GetColumnEffectiveReadOnlyState(this.CurrentColumn))
            {
                // Current column is read-only 
                // 
                return false;
            } 
            if (editingTriggerInfo != null)
            {
                editingTriggerInfo.ContainsFocus = this.ContainsFocus; 
            }
            //
            return BeginCellEdit(editingTriggerInfo); 
        } 

        /// <summary> 
        /// Common method for committing edits, cancelling edits, and exiting editing mode.
        /// </summary>
        /// <param name="commitChanges">Edits are pushed to the backend if True. Original cells values are restored if False.</param> 
        /// <param name="exitEditingMode">Editing mode is left if True.</param>
        /// <returns>True if operation was successful. False otherwise.</returns>
        public bool EndEdit(bool commitChanges, bool exitEditingMode) 
        { 
            if (!EndCellEdit(commitChanges, exitEditingMode, this.ContainsFocus /*keepFocus*/))
            { 
                return false;
            }
            return EndRowEdit(commitChanges, exitEditingMode); 
        }

        #endregion Public Methods 
 
        #region Protected Methods
 
        public override void OnApplyTemplate()
        {
            // 


 
 

            _columnHeaders = GetTemplateChild(DATAGRID_elementColumnHeadersName) as Canvas; 
            if (_columnHeaders != null && this.ColumnsInternal.Count > 0)
            {
                // Columns were added before before our Template was applied, add the ColumnHeaders now 
                foreach (DataGridColumnBase column in this.ColumnsInternal)
                {
                    InsertDisplayedColumnHeader(column); 
                } 
            }
            _rowHeaders = GetTemplateChild(DATAGRID_elementRowHeadersName) as Canvas; 
            _cells = GetTemplateChild(DATAGRID_elementCellsName) as Canvas;
            if (_cells != null)
            { 
                this.SetValueNoCallback(HorizontalGridlinesBrushProperty, this._cells.Resources["DefaultHorizontalGridlinesBrush"] as Brush);
                this.SetValueNoCallback(VerticalGridlinesBrushProperty, this._cells.Resources["DefaultVerticalGridlinesBrush"] as Brush);
 
                // 
                _cells.SizeChanged += new SizeChangedEventHandler(Cells_SizeChanged);
            } 
            _hScrollBar = GetTemplateChild(DATAGRID_elementHorizontalScrollbarName) as ScrollBar;
            if (_hScrollBar != null)
            { 
                _hScrollBar.IsTabStop = false;
                _hScrollBar.Orientation = Orientation.Horizontal;
                _hScrollBar.Visibility = Visibility.Collapsed;
                _hScrollBar.Scroll += new ScrollEventHandler(HorizontalScrollBar_Scroll); 
            }
            _vScrollBar = GetTemplateChild(DATAGRID_elementVerticalScrollbarName) as ScrollBar; 
            if (_vScrollBar != null)
            {
                _vScrollBar.IsTabStop = false; 
                _vScrollBar.Orientation = Orientation.Vertical;
                _vScrollBar.Visibility = Visibility.Collapsed;
                _vScrollBar.Scroll += new ScrollEventHandler(VerticalScrollBar_Scroll); 
            } 

            _topLeftCornerHeader = GetTemplateChild(DATAGRID_elementTopLeftCornerHeaderName) as ContentControl; 

            _topRightCornerHeader = GetTemplateChild(DATAGRID_elementTopRightCornerHeaderName) as ContentControl;
            if (_topRightCornerHeader != null && _vScrollBar != null) 
            {
                _topRightCornerHeader.Width = _vScrollBar.Width;
            } 
 
            this._currentCellFocusVisual = GetTemplateChild(DATAGRID_elementFocusVisualName) as FrameworkElement;
            ApplyCurrentCellFocusVisualState(); 
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
        /// Raises the BeginningCellEdit event. 
        /// </summary>
        protected virtual void OnBeginningCellEdit(DataGridCellEditingCancelEventArgs e) 
        {
            EventHandler<DataGridCellEditingCancelEventArgs> handler = this.BeginningCellEdit;
            if (handler != null) 
            {
                handler(this, e);
            } 
        } 

        // 


 


 
 

 
        /// <summary>
        /// Raises the CleaningRow event for row recycling.
        /// </summary> 
        protected virtual void OnCleaningRow(DataGridRowCancelEventArgs e)
        {
            EventHandler<DataGridRowCancelEventArgs> handler = this.CleaningRow; 
            if (handler != null) 
            {
                handler(this, e); 
            }
        }
 
        /// <summary>
        /// Raises the CleaningRowDetails event
        /// </summary> 
        protected virtual void OnCleaningRowDetails(DataGridRowDetailsEventArgs e) 
        {
            EventHandler<DataGridRowDetailsEventArgs> handler = this.CleaningRowDetails; 
            if (handler != null)
            {
                handler(this, e); 
            }
        }
 
        /// <summary> 
        /// Raises the CommitCellEdit event.
        /// </summary> 
        protected virtual void OnCommitCellEdit(DataGridCellEventArgs e)
        {
            EventHandler<DataGridCellEventArgs> handler = this.CommitCellEdit; 
            if (handler != null)
            {
                handler(this, e); 
            } 
        }
 
        /// <summary>
        /// Raises the CommittingCellEdit event.
        /// </summary> 
        protected virtual void OnCommittingCellEdit(DataGridCellCancelEventArgs e)
        {
            EventHandler<DataGridCellCancelEventArgs> handler = this.CommittingCellEdit; 
            if (handler != null) 
            {
                handler(this, e); 
            }
        }
 
        /// <summary>
        /// Raises the CommittingRowEdit event.
        /// </summary> 
        protected virtual void OnCommittingRowEdit(DataGridRowCancelEventArgs e) 
        {
            EventHandler<DataGridRowCancelEventArgs> handler = this.CommittingRowEdit; 
            if (handler != null)
            {
                handler(this, e); 
            }
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
        }
 
        /// <summary>
        /// Raises the DataError event.
        /// </summary> 
        protected virtual void OnDataError(DataGridDataErrorEventArgs e)
        {
            EventHandler<DataGridDataErrorEventArgs> handler = this.DataError; 
            if (handler != null) 
            {
                handler(this, e); 
            }
        }
 
        //

 
 

 


 

        /// <summary>
        /// Raises the PrepareCellEdit event. 
        /// </summary> 
        protected virtual void OnPrepareCellEdit(DataGridPrepareCellEditEventArgs e)
        { 
            EventHandler<DataGridPrepareCellEditEventArgs> handler = this.PrepareCellEdit;
            if (handler != null)
            { 
                handler(this, e);
            }
        } 
 
        /// <summary>
        /// Raises the PreparingRow event for row preparation. 
        /// </summary>
        protected virtual void OnPreparingRow(DataGridRowEventArgs e)
        { 
            EventHandler<DataGridRowEventArgs> handler = this.PreparingRow;
            if (handler != null)
            { 
                Debug.Assert(!this._preparedRows.Contains(e.Row)); 
                this._preparedRows.Add(e.Row);
                handler(this, e); 
                Debug.Assert(this._preparedRows.Contains(e.Row));
                this._preparedRows.Remove(e.Row);
            } 
        }

        /// <summary> 
        /// Raises the PreparingRowDetails for row details preparation 
        /// </summary>
        protected virtual void OnPreparingRowDetails(DataGridRowDetailsEventArgs e) 
        {
            EventHandler<DataGridRowDetailsEventArgs> handler = this.PreparingRowDetails;
            if (handler != null) 
            {
                handler(this, e);
            } 
        } 

        /// <summary> 
        /// Raises the SelectionChanged event and clears the _selectionChanged.
        /// This event won't get raised again until after _selectionChanged is set back to true.
        /// </summary> 
        protected virtual void OnSelectionChanged(EventArgs e)
        {
            this.CoerceSelectedItem(); 
 
            this._selectionChanged = false;
            EventHandler<EventArgs> handler = this.SelectionChanged; 
            if (handler != null)
            {
                handler(this, e); 
            }
        }
 
        // 

 


 


 
 
        #endregion Protected Methods
 
        #region Internal Methods

        internal void AddDisplayedHorizontalGridline(Line line) 
        {
            Debug.Assert(line != null);
            if (this._cells != null) 
            { 
                Debug.Assert(this.DisplayedRowCount >= 0);
                Debug.Assert(this._displayedVerticalGridlineCount >= 0); 
                Debug.Assert(!this._cells.Children.Contains(line));
                this._cells.Children.Insert(this.DisplayedAndEditedRowCount + this._displayedVerticalGridlineCount, line);
            } 
        }

        internal void AddDisplayedVerticalGridline(Line line) 
        { 
            Debug.Assert(line != null);
            if (this._cells != null) 
            {
                Debug.Assert(this.DisplayedRowCount >= 0);
                Debug.Assert(!this._cells.Children.Contains(line)); 
                this._cells.Children.Insert(this.DisplayedAndEditedRowCount, line);
                this._displayedVerticalGridlineCount++;
            } 
        } 

        /// <summary> 
        /// call when: selection changes or SelectedItems object changes
        /// </summary>
        internal void CoerceSelectedItem() 
        {
            object selectedItem = null;
 
            if (this.SelectionMode == DataGridSelectionMode.ExtendedFullRow && 
                this.CurrentRowIndex != -1 &&
                this.SelectedItemsInternal.Contains(this.CurrentRowIndex)) 
            {
                selectedItem = this.CurrentItem;
            } 
            else if (this.SelectedItemsInternal.Count > 0)
            {
                selectedItem = this.SelectedItemsInternal[0]; 
            } 

            this.SetValueNoCallback(SelectedItemProperty, selectedItem); 
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
 
        /// <summary>
        /// A cell value parsing or formatting failed 
        /// </summary>
        internal void OnDataConversionError(DataGridDataErrorEventArgs dataError)
        { 
            Debug.Assert(dataError != null);
            Debug.Assert(dataError.Exception != null);
            Debug.Assert(dataError.Column != null); 
 
            if (dataError.Column.OwningGrid == this &&
                (dataError.Row == null || dataError.Row.OwningGrid == this)) 
            {
                OnDataError(dataError);
                if (dataError.ThrowException) 
                {
                    throw dataError.Exception;
                } 
            } 
        }
 
        [SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
        internal void PerformColumnHeaderCellsLayout()
        { 
            bool displayColumnHeaders = this.AreColumnHeadersVisible && this._columnHeaders != null;

            double leftEdge = 0; 
            bool inFrozenZone = true; 
            DataGridColumnBase column = this.ColumnsInternal.FirstColumn;
            Debug.Assert(column == null || column.DisplayIndex == 0); 
            while (column != null)
            {
                Debug.Assert(column.OwningGrid == this); 
                DataGridColumnHeader headerCell = null;
                if (displayColumnHeaders)
                { 
                    headerCell = column.HeaderCell; 
                }
                if (column.Visibility == Visibility.Visible) 
                {
                    if (column.IsFrozen)
                    { 
                        Debug.Assert(inFrozenZone);
                        if (displayColumnHeaders)
                        { 
                            headerCell.Visibility = Visibility.Visible; 
                            headerCell.SetValue(Canvas.LeftProperty, leftEdge);
                            headerCell.Width = column.Width; 
                            headerCell.Height = this.ColumnHeadersHeight;
                            RectangleGeometry rg = new RectangleGeometry();
                            rg.Rect = new Rect(0, 0, headerCell.Width, this.ColumnHeadersHeight); 
                            headerCell.Clip = rg;
                        }
                        leftEdge += GetEdgedColumnWidth(column); 
                    } 
                    else
                    { 
                        if (inFrozenZone)
                        {
                            leftEdge -= this._negHorizontalOffset; 
                            inFrozenZone = false;
                        }
                        if (this.DisplayData.FirstDisplayedScrollingCol != -1 && 
                            (this.DisplayData.FirstDisplayedScrollingCol == column.Index || 
                            this.ColumnsInternal.DisplayInOrder(this.DisplayData.FirstDisplayedScrollingCol, column.Index)))
                        { 
                            if (displayColumnHeaders)
                            {
                                if (this.DisplayData.FirstDisplayedScrollingCol == column.Index && 
                                    this._negHorizontalOffset > 0 &&
                                    column.Width <= this._negHorizontalOffset)
                                { 
                                    headerCell.Visibility = Visibility.Collapsed; 
                                }
                                else 
                                {
                                    headerCell.Visibility = Visibility.Visible;
                                    headerCell.SetValue(Canvas.LeftProperty, leftEdge); 
                                    headerCell.Width = GetEdgedColumnWidth(column);
                                    headerCell.Height = this.ColumnHeadersHeight;
                                    RectangleGeometry rg = new RectangleGeometry(); 
                                    if (this.DisplayData.FirstDisplayedScrollingCol == column.Index && this._negHorizontalOffset > 0) 
                                    {
                                        rg.Rect = new Rect(this._negHorizontalOffset, 0, headerCell.Width - this._negHorizontalOffset, this.ColumnHeadersHeight); 
                                    }
                                    else
                                    { 
                                        rg.Rect = new Rect(0, 0, headerCell.Width, this.ColumnHeadersHeight);
                                    }
                                    headerCell.Clip = rg; 
                                } 
                            }
                            leftEdge += GetEdgedColumnWidth(column); 
                        }
                        else if (displayColumnHeaders)
                        { 
                            headerCell.Visibility = Visibility.Collapsed;
                        }
                    } 
                } 
                else if (displayColumnHeaders)
                { 
                    headerCell.Visibility = Visibility.Collapsed;
                }
                column = this.ColumnsInternal.GetNextColumn(column); 
            }

            // Show or hide the fillerColumn based on how much empty space we have 
            DataGridFillerColumn fillerColumn = this.ColumnsInternal.FillerColumn; 
            if (leftEdge < this.CellsWidth)
            { 
                // There is room for the filler column, mark it Active and add it to the Visual tree if we need to display it
                fillerColumn.IsActive = true;
                UpdateColumnHeadersSeparatorVisibility(); 
                if (displayColumnHeaders && !fillerColumn.IsRepresented)
                {
                    Debug.Assert(!this._columnHeaders.Children.Contains(fillerColumn.HeaderCell)); 
                    this._columnHeaders.Children.Insert(this.ColumnsInternal.Count, fillerColumn.HeaderCell); 
                    fillerColumn.IsRepresented = true;
                } 

                // Layout for the filler column
                double remainingWidth = this.CellsWidth - leftEdge; 
                fillerColumn.Width = remainingWidth;
                if (displayColumnHeaders)
                { 
                    fillerColumn.HeaderCell.Visibility = this.ColumnsInternal.Count > 0 ? Visibility.Visible : Visibility.Collapsed; 
                    fillerColumn.HeaderCell.Width = remainingWidth;
                    fillerColumn.HeaderCell.Height = this.ColumnHeadersHeight; 
                    fillerColumn.HeaderCell.SetValue(Canvas.LeftProperty, leftEdge);
                }
            } 
            else
            {
                // No room for the filler column 
                fillerColumn.IsActive = false; 
                UpdateColumnHeadersSeparatorVisibility();
                if (displayColumnHeaders && fillerColumn.IsRepresented) 
                {
                    Debug.Assert(this._columnHeaders.Children.Contains(fillerColumn.HeaderCell));
                    this._columnHeaders.Children.Remove(fillerColumn.HeaderCell); 
                    fillerColumn.IsRepresented = false;
                }
            } 
        } 

        internal void PerformCurrentCellFocusVisualLayout() 
        {
            if (this.DisplayData.Dirty || this._currentCellFocusVisual == null)
            { 
                return;
            }
            if (this.CurrentColumnIndex == -1 || this._cells == null || 
                (this.CurrentRowIndex != -1 && !IsRowDisplayed(this.CurrentRowIndex))) 
            {
                this._currentCellFocusVisual.Visibility = Visibility.Collapsed; 
                return;
            }
 
            DataGridRow dataGridCurrentRow = GetDisplayedRow(this.CurrentRowIndex);
            Debug.Assert(dataGridCurrentRow != null);
 
            DataGridCell dataGridCurrentCell = dataGridCurrentRow.Cells[this.CurrentColumnIndex]; 
            Debug.Assert(dataGridCurrentCell != null);
 
            if (dataGridCurrentRow.IsLayoutDelayed ||
                dataGridCurrentCell.Visibility == Visibility.Collapsed)
            { 
                this._currentCellFocusVisual.Visibility = Visibility.Collapsed;
                return;
            } 
 
            double currentCellY = (double)dataGridCurrentRow.GetValue(Canvas.TopProperty);
            if (currentCellY < -dataGridCurrentRow.ActualCellHeight) 
            {
                this._currentCellFocusVisual.Visibility = Visibility.Collapsed;
                return; 
            }

            this._currentCellFocusVisual.Visibility = Visibility.Visible; 
            this._currentCellFocusVisual.Height = dataGridCurrentRow.ActualCellHeight; 
            this._currentCellFocusVisual.Width = this.CurrentColumn.Width;
 
            double currentCellX = (double)dataGridCurrentCell.GetValue(Canvas.LeftProperty);

            this._currentCellFocusVisual.RenderTransform = new TranslateTransform { X = currentCellX, Y = currentCellY }; 
            RectangleGeometry currentCellClip = dataGridCurrentCell.Clip as RectangleGeometry;
            RectangleGeometry cellsClip = this._cells.Clip as RectangleGeometry;
            if (currentCellClip != null && cellsClip != null) 
            { 
                RectangleGeometry rg = new RectangleGeometry();
                if (currentCellX + this._currentCellFocusVisual.Width > cellsClip.Rect.Right) 
                {
                    if (currentCellY < 0)
                    { 
                        Debug.Assert(currentCellClip.Rect.Height + currentCellY >= 0);
                        rg.Rect = new Rect(currentCellClip.Rect.Left, -currentCellY,
                            cellsClip.Rect.Right - currentCellX, currentCellClip.Rect.Height + currentCellY); 
                    } 
                    else
                    { 
                        rg.Rect = new Rect(currentCellClip.Rect.Left, currentCellClip.Rect.Top,
                            cellsClip.Rect.Right - currentCellX, Math.Min(currentCellClip.Rect.Height, cellsClip.Rect.Bottom - currentCellY));
                    } 
                }
                else
                { 
                    if (currentCellY < 0) 
                    {
                        Debug.Assert(currentCellClip.Rect.Height + currentCellY >= 0); 
                        rg.Rect = new Rect(currentCellClip.Rect.Left, -currentCellY,
                            currentCellClip.Rect.Width, currentCellClip.Rect.Height + currentCellY);
                    } 
                    else
                    {
                        rg.Rect = new Rect(currentCellClip.Rect.Left, currentCellClip.Rect.Top, 
                            cellsClip.Rect.Width, Math.Min(currentCellClip.Rect.Height, cellsClip.Rect.Bottom - currentCellY)); 
                    }
                } 
                this._currentCellFocusVisual.Clip = rg;
            }
        } 

        internal void PerformDataCellsHorizontalLayout(bool forceRowsLayout)
        { 
            // The row won't layout if DisplayData is dirty so we shouldn't do anything here either 
            if (this.DisplayData.Dirty ||
                this.DisplayData.FirstDisplayedScrollingRow != this._firstDisplayedRowIndex || 
                this.DisplayData.LastDisplayedScrollingRow != this._lastDisplayedRowIndex)
            {
                return; 
            }
            else if (_cells != null)
            { 
                Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow == _firstDisplayedRowIndex); 
                Debug.Assert(this.DisplayData.LastDisplayedScrollingRow == _lastDisplayedRowIndex);
                double desiredWidth = this.ColumnsInternal.GetVisibleEdgedColumnsWidth(true /*includeLastRightGridlineWhenPresent*/) - _horizontalOffset; 
                for (int childIndex = 0; childIndex < this.DisplayedRowCount; childIndex++)
                {
                    DataGridRow row = GetDisplayedRow(this._firstDisplayedRowIndex + childIndex); 
                    // If the visible row width has changed or if we're rearranging columns we need
                    // the row to perform layout
                    if (forceRowsLayout || row.CellsWidth != desiredWidth) 
                    { 
                        row.CellsWidth = desiredWidth;
                        row.PerformLayout(false /*onlyLayoutGridlines*/); 
                    }
                    else if ((this.GridlinesVisibility == DataGridGridlines.Vertical || this.GridlinesVisibility == DataGridGridlines.All) && DataGrid.VerticalGridlinesThickness > 0 && this.VerticalGridlinesBrush != null)
                    { 
                        row.PerformLayout(true /*onlyLayoutGridlines*/);
                    }
                } 
            } 
        }
 
        //

 

        internal static Duration? ResetStoryboardDuration(Storyboard storyboard)
        { 
            Debug.Assert(storyboard != null); 
            if (storyboard.Children.Count > 0)
            { 
                Duration nullDuration = new Duration(new TimeSpan(0, 0, 0, 0));
                Duration duration = storyboard.Children[0].Duration;
                foreach (Timeline timeline in storyboard.Children) 
                {
                    timeline.Duration = nullDuration;
                } 
                return duration; 
            }
            return null; 
        }

        // 


        internal static void RestoreStoryboardDuration(Storyboard storyboard, Duration? duration) 
        { 
            Debug.Assert(storyboard != null);
            if (duration != null) 
            {
                foreach (Timeline timeline in storyboard.Children)
                { 
                    timeline.Duration = (Duration)duration;
                }
            } 
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
                //
 
                PerformColumnHeaderCellsLayout();
                //
                PerformDataCellsHorizontalLayout(this.InDisplayIndexAdjustments /*forceRowsLayout*/); 
                // 
                PerformCurrentCellFocusVisualLayout();
 
                //

 
                PerformHorizontalGridlinesLayout();

                // 
 
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
 
            if (this.CurrentRowIndex != rowIndex &&
                !EndEdit(true /*commitChanges*/, true /*exitEditingMode*/)) 
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
                if (this.SelectionMode == DataGridSelectionMode.ExtendedFullRow && shift && this.AnchorRowIndex != -1)
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
                    if (!ctrl && this.SelectionMode == DataGridSelectionMode.ExtendedFullRow && this.SelectedItemsInternal.Count != 0)
                    { 
                        // Unselect everything except the row that was clicked on
                        ClearRowSelection(rowIndex /*rowIndexException*/, true /*setAnchorRowIndex*/);
                    } 
                    else if (ctrl) 
                    {
                        if (!EndEdit(true /*commitChanges*/, true /*exitEditingMode*/)) 
                        {
                            // Edited value couldn't be committed or aborted
                            return true; 
                        }
                        SetRowSelection(rowIndex, false /*isSelected*/, false /*setAnchorRowIndex*/);
                    } 
                } 
                else // Selecting a single row or multi-selecting with Ctrl
                { 
                    if (this.SelectionMode == DataGridSelectionMode.SingleFullRow || !ctrl)
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
                            DataGridColumnBase firstVisibleColumn = this.ColumnsInternal.FirstVisibleColumn; 
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

            if (beginEdit && BeginCellEdit(new DataGridEditingTriggerInfo(true /*containsFocus*/, Keyboard.Modifiers, null, mouseButtonEventArgs))) 
            {
                FocusEditingCell(true /*setFocus*/);
            } 

            return true;
        } 
 
        #endregion Internal Methods
 
        #region Private Methods

        /// <summary> 
        /// Adds the provided Line as the last child of the column headers' container.
        /// </summary>
        private void AddDisplayedHorizontalCellsPresenterSeparator(Line line) 
        { 
            Debug.Assert(line != null);
            if (this.AreColumnHeadersVisible && this._columnHeaders != null) 
            {
                this._columnHeaders.Children.Add(line);
            } 
        }

        private void AddNewCellPrivate(DataGridRow row, DataGridColumnBase column) 
        { 
            DataGridCell newCell = new DataGridCell();
            PopulateCellContent(true /*forceTemplating*/, false /*isCellEdited*/, column, row, newCell); 
            if (row.OwningGrid != null)
            {
                newCell.OwningColumn = column; 
            }
            row.Cells.Insert(column.Index, newCell);
        } 
 
        //
 


 


 
 

 


 


 
 

 


        [SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes")] 
        private void ApplyCurrentCellFocusVisualState()
        {
            if (this._currentCellFocusVisual == null) 
            { 
                return;
            } 
            Storyboard storyboard;
            if (!this.ContainsFocus || this.CurrentColumnIndex == -1)
            { 
                storyboard = GetTemplateChild(DATAGRID_stateUnfocusedName) as Storyboard;
            }
            else 
            { 
                storyboard = GetTemplateChild(DATAGRID_stateNormalName) as Storyboard;
            } 
            if (storyboard != null)
            {
                // 


                try 
                { 
                    storyboard.Begin();
                } 
                catch (Exception)
                {
                } 
            }
        }
 
        private bool BeginCellEdit(DataGridEditingTriggerInfo editingTriggerInfo) 
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
            Debug.Assert(this.CurrentRowIndex < this._rowCount);
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

            //
 
 

 


 
            DataGridRow dataGridRow = this._editingRow;
            if (dataGridRow == null)
            { 
                if (IsRowDisplayed(this.CurrentRowIndex)) 
                {
                    dataGridRow = GetDisplayedRow(this.CurrentRowIndex); 
                }
                else
                { 
                    dataGridRow = GenerateRow(this.CurrentRowIndex);
                }
            } 
            Debug.Assert(dataGridRow != null); 
            DataGridCell dataGridCell = dataGridRow.Cells[this.CurrentColumnIndex];
            DataGridCellEditingCancelEventArgs e = new DataGridCellEditingCancelEventArgs(this.CurrentColumn, dataGridRow, dataGridCell.Content as FrameworkElement, editingTriggerInfo); 
            OnBeginningCellEdit(e);
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
            this._editingTriggerInfo = editingTriggerInfo;
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
            Debug.Assert(this.CurrentRowIndex < this._rowCount);

            // 
 

 


 


 
 
            try
            { 
                if (this.DataConnection.BeginEdit(dataGridRow.DataContext))
                {
                    this._editingRow = dataGridRow; 
                    if (this.AreRowHeadersVisible)
                    {
                        this._editingRow.HeaderCell.ApplyRowStatus(true /*animate*/); 
                    } 
                    this._editingRow.ApplyBackgroundBrush(true /*animate*/);
                    // 
                    return true;
                }
                // 
                return false;
            }
            catch (Exception ex) 
            { 
                DataGridDataErrorEventArgs dataError = new DataGridDataErrorEventArgs(ex, this.CurrentColumn, this._editingRow);
                OnDataError(dataError); 
                if (dataError.ThrowException)
                {
                    throw dataError.Exception; 
                }
            }
            // 
            return false; 
        }
 
        private void Cells_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            PerformLayout(true /*forceDataCellsHorizontalLayout*/); 
            if (e.PreviousSize.Height == 0 && e.PreviousSize.Width == 0 &&
                this._makeFirstDisplayedCellCurrentCellPending &&
                this.Columns.Count > 0 && this.CurrentColumnIndex == -1) 
            { 
                MakeFirstDisplayedCellCurrentCell();
            } 
        }

        [SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes")] 
        private DataGridDataErrorEventArgs CancelCellEdit(bool exitEditingMode)
        {
            if (this._editingColumnIndex == -1) 
            { 
                return null;
            } 

            Debug.Assert(this._editingColumnIndex >= 0);
            Debug.Assert(this._editingColumnIndex < this.ColumnsItemsInternal.Count); 
            Debug.Assert(this._editingRow != null);

            DataGridDataErrorEventArgs dataError = null; 
            DataGridColumnBase dataGridColumn = this.ColumnsItemsInternal[this._editingColumnIndex]; 
            DataGridCell dataGridCell = this._editingRow.Cells[this._editingColumnIndex];
            // Write the old cell value back into the cell content. 
            DataGridBoundColumnBase dataGridBoundColumn = dataGridColumn as DataGridBoundColumnBase;
            try
            { 
                if (dataGridBoundColumn != null)
                {
                    dataGridBoundColumn.CancelCellEdit(this._uneditedValue); 
                } 
                else
                { 
                    DataGridTemplateColumn dataGridTemplateColumn = dataGridColumn as DataGridTemplateColumn;
                    if (dataGridTemplateColumn != null)
                    { 
                        PopulateCellContent(true /*forceTemplating*/, !exitEditingMode /*isCellEdited*/, dataGridTemplateColumn, this._editingRow, dataGridCell);
                    }
                } 
            } 
            catch (Exception ex)
            { 
                dataError = new DataGridDataErrorEventArgs(ex, this.CurrentColumn, this._editingRow);
                OnDataError(dataError);
                return dataError; 
            }

            return null; 
        } 

        [SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes")] 
        private DataGridDataErrorEventArgs CancelRowEdit(bool exitEditingMode)
        {
            if (this._editingRow == null) 
            {
                return null;
            } 
            Debug.Assert(this.EditingRowIndex >= -1); 
            Debug.Assert(this.EditingRowIndex < this._rowCount);
 
            DataGridDataErrorEventArgs dataError = null;
            try
            { 
                object dataItem = this._editingRow.DataContext;
                if (!this.DataConnection.CancelEdit(dataItem))
                { 
                    dataError = new DataGridDataErrorEventArgs(null, this.CurrentColumn, this._editingRow); 
                    return dataError;
                } 
                //

                foreach (DataGridColumnBase dataGridColumn in this.Columns) 
                {
                    if (!exitEditingMode && dataGridColumn.Index == this._editingColumnIndex && dataGridColumn is DataGridBoundColumnBase)
                    { 
                        continue; 
                    }
                    PopulateCellContent(true /*forceTemplating*/, !exitEditingMode && dataGridColumn.Index == this._editingColumnIndex /*isCellEdited*/, dataGridColumn, this._editingRow, this._editingRow.Cells[dataGridColumn.Index]); 
                }
                if (!exitEditingMode && !this.DataConnection.BeginEdit(dataItem))
                { 
                    dataError = new DataGridDataErrorEventArgs(null, this.CurrentColumn, this._editingRow);
                    return dataError;
                } 
            } 
            catch (Exception ex)
            { 
                dataError = new DataGridDataErrorEventArgs(ex, this.CurrentColumn, this._editingRow);
                OnDataError(dataError);
                return dataError; 
            }

            return null; 
        } 

        [SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes")] 
        private DataGridDataErrorEventArgs CommitCellEditPrivate()
        {
            if (this._editingColumnIndex == -1) 
            {
                return null;
            } 
            Debug.Assert(this._editingColumnIndex >= 0); 
            Debug.Assert(this._editingColumnIndex < this.ColumnsItemsInternal.Count);
            Debug.Assert(this._editingRow != null); 

            DataGridDataErrorEventArgs dataError = null;
            DataGridColumnBase dataGridColumn = this.ColumnsItemsInternal[this._editingColumnIndex]; 
            FrameworkElement element = this._editingRow.Cells[this._editingColumnIndex].Content as FrameworkElement;
            DataGridCellCancelEventArgs e = new DataGridCellCancelEventArgs(dataGridColumn, this._editingRow, element);
            OnCommittingCellEdit(e); 
            if (e.Cancel) 
            {
                dataError = new DataGridDataErrorEventArgs(null, this.CurrentColumn, this._editingRow); 
                dataError.Cancel = true;
                return dataError;
            } 

            // Write the new cell value into the backend for non-bound cells.
            DataGridBoundColumnBase dataGridBoundColumn = dataGridColumn as DataGridBoundColumnBase; 
            try 
            {
                if (dataGridBoundColumn == null) 
                {
                    //
 
                    OnCommitCellEdit(new DataGridCellEventArgs(dataGridColumn, this._editingRow, element));
                }
            } 
            catch (Exception ex) 
            {
                dataError = new DataGridDataErrorEventArgs(ex, this.CurrentColumn, this._editingRow); 
                OnDataError(dataError);
                return dataError;
            } 
            return null;
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
            else 
            {
                DataGridDataErrorEventArgs dataError = CommitCellEditPrivate();
                if (null != dataError) 
                { 
                    if (dataError.ThrowException)
                    { 
                        throw dataError.Exception;
                    }
                    if (dataError.Cancel) 
                    {
                        return false;
                    } 
                } 
            }
 
            if (IsColumnOutOfBounds(columnIndex))
            {
                return false; 
            }
            if (rowIndex >= this._rowCount)
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
        private DataGridDataErrorEventArgs CommitRowEdit(bool exitEditingMode)
        {
            if (this._editingRow == null) 
            {
                return null;
            } 
            Debug.Assert(this.EditingRowIndex >= -1); 
            Debug.Assert(this.EditingRowIndex < this._rowCount);
 
            DataGridDataErrorEventArgs dataError = null;
            DataGridRowCancelEventArgs e = new DataGridRowCancelEventArgs(this._editingRow);
            OnCommittingRowEdit(e); 
            if (e.Cancel)
            {
                dataError = new DataGridDataErrorEventArgs(null, this.CurrentColumn, this._editingRow); 
                dataError.Cancel = true; 
                return dataError;
            } 

            try
            { 
                if (!this.DataConnection.EndEdit(this._editingRow.DataContext))
                {
                    dataError = new DataGridDataErrorEventArgs(null, this.CurrentColumn, this._editingRow); 
                    return dataError; 
                }
                if (!exitEditingMode && !this.DataConnection.BeginEdit(this._editingRow.DataContext)) 
                {
                    dataError = new DataGridDataErrorEventArgs(null, this.CurrentColumn, this._editingRow);
                    return dataError; 
                }
            }
            catch (Exception ex) 
            { 
                dataError = new DataGridDataErrorEventArgs(ex, this.CurrentColumn, this._editingRow);
                OnDataError(dataError); 
                return dataError;
            }
 
            return null;
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
 
            SuspendLayout();
            try
            { 
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
                    horizScrollBarHeight = _hScrollBar.Height;
                    if (this._hScrollBar.Visibility == Visibility.Visible && !forceHorizScrollbar)
                    { 
                        cellsHeight += horizScrollBarHeight;
                    }
                    else if (this._hScrollBar.Visibility == Visibility.Collapsed && forceHorizScrollbar) 
                    { 
                        cellsHeight -= horizScrollBarHeight;
                    } 
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
                    vertScrollBarWidth = _vScrollBar.Width;
                    if (this._vScrollBar.Visibility == Visibility.Visible && !forceVertScrollbar) 
                    {
                        cellsWidth += vertScrollBarWidth;
                    } 
                    else if (this._vScrollBar.Visibility == Visibility.Collapsed && forceVertScrollbar) 
                    {
                        cellsWidth -= vertScrollBarWidth; 
                    }
                }
 
                // Now cellsWidth is the width potentially available for displaying data cells.
                // Now cellsHeight is the height potentially available for displaying data cells.
 
                bool needHorizScrollbar = false; 
                bool needVertScrollbar = false;
 
                double totalVisibleWidth = this.ColumnsInternal.GetVisibleEdgedColumnsWidth(false /*includeLastRightGridlineWhenPresent*/);
                double totalVisibleFrozenWidth = this.ColumnsInternal.GetVisibleFrozenEdgedColumnsWidth(false /*includeLastRightGridlineWhenPresent*/);
 
                ComputeDisplayedRows(this.CellsHeight);
                double totalVisibleHeight = this.EdgedRowsHeight;
 
                if (!forceHorizScrollbar && !forceVertScrollbar) 
                {
                    int oldfirstDisplayedScrollingRow; 
                    bool needHorizScrollbarWithoutVertScrollbar = false;

                    if (allowHorizScrollbar && 
                        totalVisibleWidth > cellsWidth && totalVisibleFrozenWidth < cellsWidth &&
                        horizScrollBarHeight <= cellsHeight)
                    { 
                        double oldDataHeight = cellsHeight; 
                        cellsHeight -= horizScrollBarHeight;
                        Debug.Assert(cellsHeight >= 0); 
                        needHorizScrollbarWithoutVertScrollbar = needHorizScrollbar = true;
                        if (allowVertScrollbar && (totalVisibleWidth - cellsWidth <= vertScrollBarWidth ||
                            cellsWidth - totalVisibleFrozenWidth <= vertScrollBarWidth)) 
                        {
                            // Would we still need a horizontal scrollbar without the vertical one?
                            oldfirstDisplayedScrollingRow = this.DisplayData.FirstDisplayedScrollingRow; 
                            ComputeDisplayedRows(cellsHeight); 
                            if (this.DisplayData.NumTotallyDisplayedScrollingRows != this._rowCount /*
*/) 
                            {
                                needHorizScrollbar = (totalVisibleFrozenWidth < cellsWidth - vertScrollBarWidth);
                            } 
                            this.DisplayData.FirstDisplayedScrollingRow = oldfirstDisplayedScrollingRow;
                        }
 
                        if (!needHorizScrollbar) 
                        {
                            // Restore old data height because turns out a horizontal scroll bar wouldn't make sense 
                            cellsHeight = oldDataHeight;
                        }
                    } 

                    oldfirstDisplayedScrollingRow = this.DisplayData.FirstDisplayedScrollingRow;
                    ComputeDisplayedRows(cellsHeight); 
                    if (allowVertScrollbar && 
                        cellsHeight > 0 &&
                        vertScrollBarWidth <= cellsWidth && 
                        this.DisplayData.NumTotallyDisplayedScrollingRows != this._rowCount /*
*/)
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
                        totalVisibleWidth > cellsWidth && totalVisibleFrozenWidth < cellsWidth && 
                        horizScrollBarHeight <= cellsHeight) 
                    {
                        this.DisplayData.FirstDisplayedScrollingRow = oldfirstDisplayedScrollingRow; 
                        cellsWidth += vertScrollBarWidth;
                        cellsHeight -= horizScrollBarHeight;
                        Debug.Assert(cellsHeight >= 0); 
                        needVertScrollbar = false;

                        ComputeDisplayedRows(cellsHeight); 
                        if (cellsHeight > 0 && 
                            vertScrollBarWidth <= cellsWidth &&
                            this.DisplayData.NumTotallyDisplayedScrollingRows != this._rowCount /* 
*/)
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
                            vertScrollBarWidth <= cellsWidth &&
                            this.DisplayData.NumTotallyDisplayedScrollingRows != this._rowCount)
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
                            horizScrollBarHeight <= cellsHeight && 
                            totalVisibleWidth > cellsWidth &&
                            totalVisibleFrozenWidth < cellsWidth)
                        { 
                            cellsHeight -= horizScrollBarHeight; 
                            Debug.Assert(cellsHeight >= 0);
                            needHorizScrollbar = true; 
                            ComputeDisplayedRows(cellsHeight);
                        }
                        this.DisplayData.FirstDisplayedScrollingCol = ComputeFirstVisibleScrollingColumn(); 
                        ComputeDisplayedColumns();
                    }
                    needVertScrollbar = this.DisplayData.NumTotallyDisplayedScrollingRows != this._rowCount; 
                } 
                else
                { 
                    Debug.Assert(forceHorizScrollbar && forceVertScrollbar);
                    Debug.Assert(allowHorizScrollbar && allowVertScrollbar);
                    this.DisplayData.FirstDisplayedScrollingCol = ComputeFirstVisibleScrollingColumn(); 
                    ComputeDisplayedColumns();
                    needVertScrollbar = this.DisplayData.NumTotallyDisplayedScrollingRows != this._rowCount;
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
            } 
            finally
            { 
                ResumeLayout(false);
            }
        } 

        private void UpdateVerticalScrollBar()
        { 
            if (this._vScrollBar != null && this._vScrollBar.Visibility == Visibility.Visible) 
            {
                UpdateVerticalScrollBar(true /*needVertScrollbar*/, false /*forceVertScrollbar*/, this.EdgedRowsHeight, this.CellsHeight); 
            }
        }
 
        private void DataGrid_GotFocus(object sender, RoutedEventArgs e)
        {
            if (!this.ContainsFocus) 
            { 
                //
                this.ContainsFocus = true; 
                Control focusedControl = e.Source as Control;
                if (focusedControl != null && focusedControl != this)
                { 
                    DataGridCell dataGridCell = GetOwningCell(focusedControl);
                    if (dataGridCell != null && dataGridCell.OwningColumn is DataGridTemplateColumn)
                    { 
                        this._editingTemplateControl = focusedControl; 
                    }
                } 
                ApplyCurrentCellFocusVisualState();
                ApplyDisplayedRowsState(this._firstDisplayedRowIndex, this._lastDisplayedRowIndex);
                if (this.CurrentColumnIndex != -1 && IsRowDisplayed(this.CurrentRowIndex)) 
                {
                    GetDisplayedRow(this.CurrentRowIndex).Cells[this.CurrentColumnIndex].ApplyCellState(true /*animate*/);
                } 
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
            if (e.Key == Key.Tab && this.CurrentColumnIndex != -1 && e.Source == this)
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
                ApplyCurrentCellFocusVisualState();
                ApplyDisplayedRowsState(this._firstDisplayedRowIndex, this._lastDisplayedRowIndex); 
                if (this.CurrentColumnIndex != -1 && IsRowDisplayed(this.CurrentRowIndex))
                {
                    GetDisplayedRow(this.CurrentRowIndex).Cells[this.CurrentColumnIndex].ApplyCellState(true /*animate*/); 
                } 
            }
        } 

        /// <summary>
        /// Called when the edited element of a bound column gains focus 
        /// </summary>
        private void EditingBoundElement_GotFocus(object sender, RoutedEventArgs e)
        { 
            // 
            FrameworkElement element = sender as FrameworkElement;
            if (element != null) 
            {
                // No longer interested in the GotFocus event
                element.GotFocus -= new RoutedEventHandler(EditingBoundElement_GotFocus); 
                this._editingBoundElementGotFocusListeners--;
                Debug.Assert(this._editingBoundElementGotFocusListeners >= 0);
                // 
 
                // but need to know when the element loses focus
                element.LostFocus += new RoutedEventHandler(EditingBoundElement_LostFocus); 
                this._editingBoundElementLostFocusListeners++;
                //
            } 
        }

        /// <summary> 
        /// Called when the edited element of a bound column loses focus 
        /// </summary>
        private void EditingBoundElement_LostFocus(object sender, RoutedEventArgs e) 
        {
            //
            FrameworkElement element = sender as FrameworkElement; 
            if (element != null)
            {
                // No longer interested in the LostFocus event 
                element.LostFocus -= new RoutedEventHandler(EditingBoundElement_LostFocus); 
                this._editingBoundElementLostFocusListeners--;
                Debug.Assert(this._editingBoundElementLostFocusListeners >= 0); 
                //

                // An element outside the DataGrid may have received focus. We need to know 
                // when the edited element receives it back, if ever.
                element.GotFocus += new RoutedEventHandler(EditingBoundElement_GotFocus);
                Debug.Assert(this._editingBoundElementGotFocusListeners >= 0); 
                this._editingBoundElementGotFocusListeners++; 
                //
 
                // The edited element may need to repopulate its content asynchronously
                this.Dispatcher.BeginInvoke(new Action(PopulateUneditedBoundCellContent), null);
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
            PrepareCellEditPrivate(element); 
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
 
            DataGridDataErrorEventArgs dataError = null;
            if (commitCellEdit)
            { 
                dataError = CommitCellEditPrivate();
                if (dataError != null)
                { 
                    if (dataError.ThrowException) 
                    {
                        // 
                        throw dataError.Exception;
                    }
                    if (dataError.Cancel) 
                    {
                        //
                        return false; 
                    } 
                }
            } 

            if (!commitCellEdit || dataError != null)
            { 
                dataError = CancelCellEdit(exitEditingMode);
                if (null != dataError)
                { 
                    if (dataError.ThrowException) 
                    {
                        // 
                        throw dataError.Exception;
                    }
                    if (dataError.Cancel) 
                    {
                        //
                        return false; 
                    } 
                }
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
                if (!exitEditingMode && this._editingTriggerInfo != null && this._editingTriggerInfo.ContainsFocus)
                {
                    // Editing control should get focus based on the latest status of the DataGrid 
                    this._editingTriggerInfo.ContainsFocus = keepFocus; 
                }
                if (!(this.CurrentColumn is DataGridBoundColumnBase)) 
                {
                    //
                    PopulateCellContent(false /*forceTemplating*/, !exitEditingMode /*isCellEdited*/, this.CurrentColumn, this._editingRow, this._editingRow.Cells[this.CurrentColumnIndex]); 
                }
                else if (this._editingBoundCells.Count > 0 && this._editingBoundElementLostFocusListeners == 0)
                { 
                    // 
                    Debug.Assert(this._editingRow.Cells[this.CurrentColumnIndex] == this._editingBoundCells[0]);
                    if (this._editingBoundElementGotFocusListeners > 0) 
                    {
                        //
                        FrameworkElement element = this._editingBoundCells[0].Content as FrameworkElement; 
                        if (element != null)
                        {
                            element.GotFocus -= new RoutedEventHandler(EditingBoundElement_GotFocus); 
                            this._editingBoundElementGotFocusListeners--; 
                            Debug.Assert(this._editingBoundElementGotFocusListeners == 0);
                            // 
                        }
                    }
                    this._editingBoundCells.RemoveAt(0); 
                    PopulateCellContent(false /*forceTemplating*/, !exitEditingMode /*isCellEdited*/, this.CurrentColumn, this._editingRow, this._editingRow.Cells[this.CurrentColumnIndex]);
                }
            } 
 
            //
            return true; 
        }

        private bool EndRowEdit(bool commitRowEdit, bool exitEditingMode) 
        {
            if (this._editingRow == null)
            { 
                return true; 
            }
 
            DataGridDataErrorEventArgs dataError = null;
            if (commitRowEdit)
            { 
                dataError = CommitRowEdit(exitEditingMode);
                if (dataError != null)
                { 
                    if (dataError.ThrowException) 
                    {
                        throw dataError.Exception; 
                    }
                    if (dataError.Cancel)
                    { 
                        return false;
                    }
                } 
            } 

            if (!commitRowEdit || dataError != null) 
            {
                dataError = CancelRowEdit(exitEditingMode);
                if (null != dataError) 
                {
                    if (dataError.ThrowException)
                    { 
                        throw dataError.Exception; 
                    }
                    if (dataError.Cancel) 
                    {
                        return false;
                    } 
                }
            }
 
            if (exitEditingMode) 
            {
                DataGridRow editingRow = this._editingRow; 
                if (this._editingRowLocation == DataGridEditingRowLocation.Top ||
                    this._editingRowLocation == DataGridEditingRowLocation.Bottom)
                { 
                    RemoveRow(this._editingRow,
                              this._editingRowLocation == DataGridEditingRowLocation.Top ? 0 :
                              this._lastDisplayedRowIndex - this._firstDisplayedRowIndex + 1); 
                    this._editingRow = null; 
                    this._editingRowLocation = DataGridEditingRowLocation.Inline;
                } 
                else
                {
                    if (this.AreRowHeadersVisible) 
                    {
                        this._editingRow = null;
                        editingRow.HeaderCell.ApplyRowStatus(true /*animate*/); 
                    } 
                    else
                    { 
                        this._editingRow = null;
                    }
                    editingRow.ApplyBackgroundBrush(true /*animate*/); 
                }
            }
 
            return true; 
        }
 
        // Applies the given Style to the column's HeaderCell if the HeaderCell does not already
        // have a Style applied
        private static void EnsureColumnHeaderCellStyle(DataGridColumnBase column, Style oldDataGridStyle, Style newDataGridStyle) 
        {
            Debug.Assert(column != null);
 
            // 

            if (column != null && (column.HeaderCell.Style == null || column.HeaderCell.Style == oldDataGridStyle)) 
            {
                column.HeaderCell.Style = newDataGridStyle;
            } 
        }

        // Applies the given Style to the row's HeaderCell if the HeaderCell does not already 
        // have a Style applied 
        private static void EnsureRowHeaderCellStyle(DataGridRow row, Style oldDataGridStyle, Style newDataGridStyle)
        { 
            Debug.Assert(row != null);

            // 

            if (row != null && (row.HeaderCell.Style == null || row.HeaderCell.Style == oldDataGridStyle))
            { 
                row.HeaderCell.Style = newDataGridStyle; 
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
 
        /// <summary>
        /// Exits editing mode without trying to commit or revert the editing, and 
        /// without repopulating the edited row's cell.
        /// </summary>
        private void ExitEdit(bool keepFocus) 
        {
            if (this._editingColumnIndex == -1)
            { 
                return; 
            }
 
            Debug.Assert(this._editingRow != null);
            Debug.Assert(this._editingColumnIndex >= 0);
            Debug.Assert(this._editingColumnIndex < this.ColumnsItemsInternal.Count); 
            Debug.Assert(this._editingColumnIndex == this.CurrentColumnIndex);
            Debug.Assert(this.EditingRowIndex == this.CurrentRowIndex);
 
            this._editingColumnIndex = -1; 
            this._editingRow.Cells[this.CurrentColumnIndex].ApplyCellState(false /*animate*/);
            // 
            this.IsTabStop = true;
            DataGridRow editingRow = this._editingRow;
            if (this._editingRowLocation == DataGridEditingRowLocation.Top || 
                this._editingRowLocation == DataGridEditingRowLocation.Bottom)
            {
                RemoveRow(this._editingRow, 
                          this._editingRowLocation == DataGridEditingRowLocation.Top ? 0 : 
                          this._lastDisplayedRowIndex - this._firstDisplayedRowIndex + 1);
                this._editingRow = null; 
                this._editingRowLocation = DataGridEditingRowLocation.Inline;
            }
            else 
            {
                if (this.AreRowHeadersVisible)
                { 
                    this._editingRow = null; 
                    editingRow.HeaderCell.ApplyRowStatus(true /*animate*/);
                } 
                else
                {
                    this._editingRow = null; 
                }
                editingRow.ApplyBackgroundBrush(true /*animate*/);
            } 
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
                OnSelectionChanged(EventArgs.Empty); 
            }
        }
 
        private bool FocusEditingCell(bool setFocus)
        {
            Debug.Assert(this.CurrentColumnIndex >= 0); 
            Debug.Assert(this.CurrentColumnIndex < this.Columns.Count); 
            Debug.Assert(this.CurrentRowIndex >= -1);
            Debug.Assert(this.CurrentRowIndex < this._rowCount); 
            Debug.Assert(this.EditingRowIndex == this.CurrentRowIndex);
            Debug.Assert(this._editingColumnIndex != -1);
 
            //

            this.IsTabStop = false; 
 
            DataGridCell dataGridCell = this._editingRow.Cells[this._editingColumnIndex];
            if (dataGridCell.OwningColumn is DataGridBoundColumnBase) 
            {
                Control editingControl = dataGridCell.Content as Control;
                if (editingControl != null) 
                {
                    editingControl.IsTabStop = true;
                    editingControl.TabIndex = this.TabIndex; 
                    if (setFocus) 
                    {
                        return editingControl.Focus(); 
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
                DataGridColumnBase previousColumn = this.ColumnsInternal.GetPreviousVisibleScrollingColumn( 
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
 
        /*

 
 

 


 

*/
 
        // Calculates the amount the ScrollDown button should scroll 
        // This is a method rather than a property to emphasize that calculations are taking place
        private double GetVerticalSmallScrollIncrease() 
        {
            if (this.DisplayData.FirstDisplayedScrollingRow >= 0)
            { 
                return GetEdgedExactRowHeight(this.DisplayData.FirstDisplayedScrollingRow) - _negVerticalOffset;
            }
            return 0; 
        } 

        // Calculates the amount the ScrollUp button should scroll 
        // This is a method rather than a property to emphasize that calculations are taking place
        private double GetVerticalSmallScrollDecrease()
        { 

            // If the first row is covered up, scroll to the start of it when the user clicks the up button
            if (DoubleUtil.GreaterThan(_negVerticalOffset, 0)) 
            { 
                return _negVerticalOffset;
            } 
            else
            {
                // The entire first row is displayed, show the entire previous row when the user clicks the up button 
                int previousRowIndex = this.DisplayData.FirstDisplayedScrollingRow - 1;
                if (previousRowIndex >= 0)
                { 
                    return GetEdgedExactRowHeight(previousRowIndex); 
                }
                else 
                {
                    // There's no previous row so don't move
                    return 0; 
                }
            }
        }

        private void HorizontalScrollBar_Scroll(object sender, System.Windows.Controls.Primitives.ScrollEventArgs e)
        { 
            if (this._horizontalScrollChangesIgnored > 0)
            {
                return; 
            }

            // If the user scrolls with the buttons, we need to update the new value of the scroll bar since we delay 
            // this calculation.  If they scroll in another other way, the scroll bar's correct value has already been set 
            double scrollBarValueDifference = 0;
            if (e.ScrollEventType == System.Windows.Controls.Primitives.ScrollEventType.SmallIncrement) 
            {
                scrollBarValueDifference = GetHorizontalSmallScrollIncrease();
            }
            else if (e.ScrollEventType == System.Windows.Controls.Primitives.ScrollEventType.SmallDecrement)
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
            return rowIndex >= this._rowCount || rowIndex < -1 /**/; 
        } 

        private void MakeFirstDisplayedCellCurrentCell() 
        {
            Debug.Assert(this.CurrentColumnIndex == -1);
            // No current cell, therefore no selection either - try to set the first displayed cell to be the current one. 
            int firstDisplayedColumnIndex = this.FirstDisplayedColumnIndex;
            int firstDisplayedRowIndex = this.DisplayData.FirstDisplayedScrollingRow;
            if (firstDisplayedColumnIndex != -1 && firstDisplayedRowIndex != -1) 
            { 
                bool success = SetAndSelectCurrentCell(firstDisplayedColumnIndex,
                                                       firstDisplayedRowIndex, 
                                                       false /*forceCurrentCellSelection (unused here)*/);
                Debug.Assert(success);
                this._makeFirstDisplayedCellCurrentCellPending = false; 
            }
            else
            { 
                this._makeFirstDisplayedCellCurrentCellPending = true; 
            }
        } 

        /* Used for single vertical gridlines
        private void PerformVerticalGridlinesLayout() 
        {
            if (this._verticalGridlinesBrush == null || this._verticalGridlinesThickness == 0 || this.ColumnsItemsInternal.Count == 0)
            { 
                return; 
            }
 
            double cutOff = 0;
            double leftEdge = 0;
            double verticalLinesHeight = Math.Min(this._cells.ActualHeight, this.EdgedRowsHeight); 
            bool inFrozenZone = true;
            DataGridColumnBase dataGridColumn = this.ColumnsInternal.FirstColumn;
            Debug.Assert(dataGridColumn == null || dataGridColumn.DisplayIndex == 0); 
            while (dataGridColumn != null) 
            {
                Debug.Assert(dataGridColumn.OwningGrid == this); 
                if (dataGridColumn.IsVisible)
                {
                    if (dataGridColumn.IsFrozen) 
                    {
                        Debug.Assert(inFrozenZone);
                        if (this.ColumnsInternal.FirstVisibleFrozenColumn == dataGridColumn) 
                        { 
                            dataGridColumn.LeftGridline.Visibility = Visibility.Visible;
                            dataGridColumn.LeftGridline.SetValue(Canvas.TopProperty, 0); 
                            dataGridColumn.LeftGridline.SetValue(Canvas.LeftProperty, leftEdge);
                            dataGridColumn.LeftGridline.X1 = this._verticalGridlinesThickness / 2;
                            dataGridColumn.LeftGridline.X2 = this._verticalGridlinesThickness / 2; 
                            dataGridColumn.LeftGridline.Y2 = verticalLinesHeight;
                            RectangleGeometry rg = new RectangleGeometry();
                            if (leftEdge + this._verticalGridlinesThickness > this._cells.ActualWidth) 
                            { 
                                cutOff = leftEdge + this._verticalGridlinesThickness - this._cells.ActualWidth;
                            } 
                            rg.Rect = new Rect(0, 0, this._verticalGridlinesThickness - cutOff, verticalLinesHeight);
                            dataGridColumn.LeftGridline.Clip = rg;
                            leftEdge += this._verticalGridlinesThickness; 
                        }
                        else if (dataGridColumn.HasLeftGridline)
                        { 
                            dataGridColumn.LeftGridline.Visibility = Visibility.Collapsed; 
                        }
                        leftEdge += dataGridColumn.Width; 
                        if (leftEdge < this._cells.ActualWidth)
                        {
                            if (leftEdge + this._verticalGridlinesThickness > this._cells.ActualWidth) 
                            {
                                cutOff = leftEdge + this._verticalGridlinesThickness - this._cells.ActualWidth;
                            } 
                            dataGridColumn.RightGridline.Visibility = Visibility.Visible; 
                            dataGridColumn.RightGridline.SetValue(Canvas.TopProperty, 0);
                            dataGridColumn.RightGridline.SetValue(Canvas.LeftProperty, leftEdge); 
                            dataGridColumn.RightGridline.X1 = this._verticalGridlinesThickness / 2;
                            dataGridColumn.RightGridline.X2 = this._verticalGridlinesThickness / 2;
                            dataGridColumn.RightGridline.Y2 = verticalLinesHeight; 
                            RectangleGeometry rg = new RectangleGeometry();
                            rg.Rect = new Rect(0, 0, this._verticalGridlinesThickness - cutOff, verticalLinesHeight);
                            dataGridColumn.RightGridline.Clip = rg; 
                            leftEdge += this._verticalGridlinesThickness; 
                        }
                        else if (dataGridColumn.HasRightGridline) 
                        {
                            dataGridColumn.RightGridline.Visibility = Visibility.Collapsed;
                        } 
                    }
                    else
                    { 
                        if (inFrozenZone) 
                        {
                            leftEdge -= this._negHorizontalOffset; 
                            inFrozenZone = false;
                        }
                        if (this.DisplayData.FirstDisplayedScrollingCol != -1 && 
                            (this.DisplayData.FirstDisplayedScrollingCol == dataGridColumn.Index ||
                            this.ColumnsInternal.DisplayInOrder(this.DisplayData.FirstDisplayedScrollingCol, dataGridColumn.Index)))
                        { 
                            if (this.ColumnsInternal.FirstVisibleColumn == dataGridColumn && 
                                this.ColumnsInternal.FirstVisibleFrozenColumn == null)
                            { 
                                double offset = 0;
                                if (this.DisplayData.FirstDisplayedScrollingCol == dataGridColumn.Index)
                                { 
                                    offset = this._negHorizontalOffset;
                                }
                                if (leftEdge + this._verticalGridlinesThickness > this._cells.ActualWidth) 
                                { 
                                    cutOff = leftEdge + this._verticalGridlinesThickness - this._cells.ActualWidth;
                                } 
                                else
                                {
                                    cutOff = 0; 
                                }
                                if (this._verticalGridlinesThickness - offset - cutOff <= 0)
                                { 
                                    if (dataGridColumn.HasLeftGridline) 
                                    {
                                        dataGridColumn.LeftGridline.Visibility = Visibility.Collapsed; 
                                    }
                                }
                                else 
                                {
                                    dataGridColumn.LeftGridline.Visibility = Visibility.Visible;
                                    dataGridColumn.LeftGridline.SetValue(Canvas.TopProperty, 0); 
                                    dataGridColumn.LeftGridline.SetValue(Canvas.LeftProperty, leftEdge); 
                                    dataGridColumn.LeftGridline.X1 = this._verticalGridlinesThickness / 2;
                                    dataGridColumn.LeftGridline.X2 = this._verticalGridlinesThickness / 2; 
                                    dataGridColumn.LeftGridline.Y2 = verticalLinesHeight;
                                    RectangleGeometry rg = new RectangleGeometry();
                                    rg.Rect = new Rect(offset, 0, this._verticalGridlinesThickness - offset - cutOff, verticalLinesHeight); 
                                    dataGridColumn.LeftGridline.Clip = rg;
                                }
                                leftEdge += this._verticalGridlinesThickness; 
                            } 
                            else if (dataGridColumn.HasLeftGridline)
                            { 
                                dataGridColumn.LeftGridline.Visibility = Visibility.Collapsed;
                            }
                            leftEdge += dataGridColumn.Width; 
                            if (leftEdge < this._cells.ActualWidth)
                            {
                                double offset = 0; 
                                if (this.DisplayData.FirstDisplayedScrollingCol == dataGridColumn.Index && 
                                    this._negHorizontalOffset > this._verticalGridlinesThickness + dataGridColumn.Width)
                                { 
                                    offset = this._negHorizontalOffset - this._verticalGridlinesThickness - dataGridColumn.Width;
                                }
                                if (leftEdge + this._verticalGridlinesThickness > this._cells.ActualWidth) 
                                {
                                    cutOff = leftEdge + this._verticalGridlinesThickness - this._cells.ActualWidth;
                                } 
                                else 
                                {
                                    cutOff = 0; 
                                }
                                if (this._verticalGridlinesThickness - offset - cutOff <= 0)
                                { 
                                    if (dataGridColumn.HasRightGridline)
                                    {
                                        dataGridColumn.RightGridline.Visibility = Visibility.Collapsed; 
                                    } 
                                }
                                else 
                                {
                                    dataGridColumn.RightGridline.Visibility = Visibility.Visible;
                                    dataGridColumn.RightGridline.SetValue(Canvas.TopProperty, 0); 
                                    dataGridColumn.RightGridline.SetValue(Canvas.LeftProperty, leftEdge);
                                    dataGridColumn.RightGridline.X1 = this._verticalGridlinesThickness / 2;
                                    dataGridColumn.RightGridline.X2 = this._verticalGridlinesThickness / 2; 
                                    dataGridColumn.RightGridline.Y2 = verticalLinesHeight; 
                                    RectangleGeometry rg = new RectangleGeometry();
                                    rg.Rect = new Rect(offset, 0, this._verticalGridlinesThickness - offset - cutOff, verticalLinesHeight); 
                                    dataGridColumn.RightGridline.Clip = rg;
                                }
                                leftEdge += this._verticalGridlinesThickness; 
                            }
                            else if (dataGridColumn.HasRightGridline)
                            { 
                                dataGridColumn.RightGridline.Visibility = Visibility.Collapsed; 
                            }
                        } 
                        else
                        {
                            if (dataGridColumn.HasLeftGridline) 
                            {
                                dataGridColumn.LeftGridline.Visibility = Visibility.Collapsed;
                            } 
                            if (dataGridColumn.HasRightGridline) 
                            {
                                dataGridColumn.RightGridline.Visibility = Visibility.Collapsed; 
                            }
                        }
                    } 
                }
                else
                { 
                    if (dataGridColumn.HasLeftGridline) 
                    {
                        dataGridColumn.LeftGridline.Visibility = Visibility.Collapsed; 
                    }
                    if (dataGridColumn.HasRightGridline)
                    { 
                        dataGridColumn.RightGridline.Visibility = Visibility.Collapsed;
                    }
                } 
                dataGridColumn = this.ColumnsInternal.GetNextColumn(dataGridColumn); 
            }
        } */ 

        private void PerformGridLayout()
        { 
            RectangleGeometry rg;
            if (_cells != null)
            { 
                rg = new RectangleGeometry(); 
                rg.Rect = new Rect(0, 0, _cells.ActualWidth, _cells.ActualHeight);
                _cells.Clip = rg; 
            }
            if (this.AreRowHeadersVisible && _rowHeaders != null)
            { 
                rg = new RectangleGeometry();
                rg.Rect = new Rect(0, 0, _rowHeaders.ActualWidth, _rowHeaders.ActualHeight);
                _rowHeaders.Clip = rg; 
            } 
            if (this.AreColumnHeadersVisible && _columnHeaders != null)
            { 
                rg = new RectangleGeometry();
                rg.Rect = new Rect(0, 0, _columnHeaders.ActualWidth, _columnHeaders.ActualHeight);
                _columnHeaders.Clip = rg; 
            }
        }
 
        /* 

 


 


 
 

 


 


 
 

 


 


 
 

 


 
*/

        private void PerformHorizontalGridlinesLayout() 
        { 
            if (this._cells == null)
            { 
                return;
            }
 
            bool areRowBottomGridlinesRequired = this.AreRowBottomGridlinesRequired;
            double bottomEdge = this._cells.ActualHeight;
            double horizontalLinesWidth = this.ColumnsInternal.GetVisibleEdgedColumnsWidth(true /*includeLastRightGridlineWhenPresent*/); 
            double cutOff = 0; 
            double topEdge = -this._negVerticalOffset;
 
            if (this.ColumnsInternal.FillerColumn.IsActive)
            {
                horizontalLinesWidth += this.ColumnsInternal.FillerColumn.Width; 
            }

            for (int childIndex = 0; childIndex < this.DisplayedRowCount; childIndex++) 
            { 
                DataGridRow dataGridRow = GetDisplayedRow(this._firstDisplayedRowIndex + childIndex);
                topEdge += dataGridRow.DisplayHeight; 
                if (areRowBottomGridlinesRequired && topEdge < bottomEdge)
                {
                    cutOff = Math.Max(0, topEdge + DataGrid.HorizontalGridlinesThickness - bottomEdge); 
                    Debug.Assert(cutOff < DataGrid.HorizontalGridlinesThickness);
                    dataGridRow.BottomGridline.Visibility = Visibility.Visible;
                    // 
                    dataGridRow.BottomGridline.SetValue(Canvas.LeftProperty, -this._horizontalOffset); 
                    dataGridRow.BottomGridline.SetValue(Canvas.TopProperty, topEdge);
                    dataGridRow.BottomGridline.Y1 = DataGrid.HorizontalGridlinesThickness / 2; 
                    dataGridRow.BottomGridline.Y2 = DataGrid.HorizontalGridlinesThickness / 2;
                    dataGridRow.BottomGridline.X2 = horizontalLinesWidth;
 
                    RectangleGeometry rg = new RectangleGeometry();
                    rg.Rect = new Rect(0, 0, horizontalLinesWidth, DataGrid.HorizontalGridlinesThickness - cutOff);
                    dataGridRow.BottomGridline.Clip = rg; 
 
                    topEdge += DataGrid.HorizontalGridlinesThickness;
                } 
                else if (dataGridRow.HasBottomGridline)
                {
                    dataGridRow.BottomGridline.Visibility = Visibility.Collapsed; 
                }
            }
        } 
 
        private void PerformLayout()
        { 
            PerformLayout(this.InDisplayIndexAdjustments /*forceDataCellsHorizontalLayout*/);
        }
 
        private void PerformLayout(bool forceDataCellsHorizontalLayout)
        {
            if (this._layoutSuspended > 0) 
            { 
                return;
            } 

            // Sets the width of the RowHeaders and the height of the ColumnHeaders
            // This must be done before computing the ScrollBars 
            PerformRowHeaderAndColumnHeaderLayout();

            ComputeScrollBarsLayout(); 
 
            //
 
            // All prefetched rows need to be cleared after the scroll since their data may become obsolete.
            this._prefetchedRows.Clear();
 
            this.DisplayData.Dirty = false;

            CorrectDisplayedRowsIndexes(); 
            PerformColumnHeaderCellsLayout(); 
            //
 
            PerformRowHeaderAndDataCellsVerticalLayout();
            PerformRowHeaderCellsHorizontalLayout();
            PerformDataCellsHorizontalLayout(forceDataCellsHorizontalLayout); 
            //

            PerformGridLayout(); 
            PerformCurrentCellFocusVisualLayout(); 
            PerformHorizontalGridlinesLayout();
        } 

        private void PerformRowHeaderAndColumnHeaderLayout()
        { 
            double rowHeadersWidth = this.ColumnsItemsInternal.Count > 0 ? this.RowHeadersWidth : 0;
            double columnHeadersHeight = this.ColumnsItemsInternal.Count > 0 ? this.ColumnHeadersHeight : 0;
 
            // Set width of RowHeaders 
            if (this.AreRowHeadersVisible)
            { 
                if (this._rowHeaders != null)
                {
                    this._rowHeaders.Visibility = Visibility.Visible; 
                    this._rowHeaders.Width = rowHeadersWidth;
                }
            } 
            else 
            {
                if (this._rowHeaders != null) 
                {
                    this._rowHeaders.Visibility = Visibility.Collapsed;
                } 
            }

            // Set height of ColumnHeaders 
            if (this.AreColumnHeadersVisible) 
            {
                if (this._columnHeaders != null) 
                {
                    this._columnHeaders.Visibility = Visibility.Visible;
                    this._columnHeaders.Height = columnHeadersHeight; 
                }
            }
            else 
            { 
                if (this._columnHeaders != null)
                { 
                    this._columnHeaders.Visibility = Visibility.Collapsed;
                }
            } 

            if (this._topLeftCornerHeader != null)
            { 
                if (this.AreRowHeadersVisible && this.AreColumnHeadersVisible) 
                {
                    this._topLeftCornerHeader.Visibility = Visibility.Visible; 
                    this._topLeftCornerHeader.Width = rowHeadersWidth;
                    this._topLeftCornerHeader.Height = columnHeadersHeight;
                } 
                else
                {
                    this._topLeftCornerHeader.Visibility = Visibility.Collapsed; 
                } 
            }
 
            if (this._topRightCornerHeader != null)
            {
                if (this.AreColumnHeadersVisible && 
                    this._vScrollBar != null && this._vScrollBar.Visibility == Visibility.Visible)
                {
                    this._topRightCornerHeader.Visibility = Visibility.Visible; 
                    this._topRightCornerHeader.Height = columnHeadersHeight; 
                }
                else 
                {
                    this._topRightCornerHeader.Visibility = Visibility.Collapsed;
                } 
            }

            /* 
 

 
*/
        }
 
        private void PerformRowHeaderAndDataCellsVerticalLayout()
        {
            Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow == _firstDisplayedRowIndex); 
            Debug.Assert(this.DisplayData.LastDisplayedScrollingRow == _lastDisplayedRowIndex); 

            if (_cells != null) 
            {
                double topEdge = -this._negVerticalOffset;
                for (int childIndex = 0; childIndex < this.DisplayedRowCount; childIndex++) 
                {
                    DataGridRow row = GetDisplayedRow(this._firstDisplayedRowIndex + childIndex);
                    row.SetValue(Canvas.TopProperty, topEdge); 
                    double rowHeight = GetEdgedRowHeight(row.Index); 
                    if (this.AreRowHeadersVisible && _rowHeaders != null)
                    { 
                        row.HeaderCell.SetValue(Canvas.TopProperty, topEdge);
                        row.HeaderCell.Height = rowHeight;
                    } 
                    Debug.Assert(row.Index != -1); // A displayed row should always have its index
                    topEdge += rowHeight;
                } 
            } 
        }
 
        private void PerformRowHeaderCellsHorizontalLayout()
        {
            if (this.AreRowHeadersVisible && _rowHeaders != null && _cells != null) 
            {
                for (int childIndex = 0; childIndex < this.DisplayedRowCount; childIndex++)
                { 
                    DataGridRow row = GetDisplayedRow(this._firstDisplayedRowIndex + childIndex); 
                    Debug.Assert(row != null);
                    row.HeaderCell.Width = this.RowHeadersWidth; 
                }
            }
        } 

        /*
 
 

 


 


 
 

 


 


 
 

 


 


 
*/ 

        private void PopulateCellContent(bool forceTemplating, bool isCellEdited, 
                                         DataGridColumnBase dataGridColumn,
                                         DataGridRow dataGridRow,
                                         DataGridCell dataGridCell) 
        {
            //
            Debug.Assert(dataGridColumn != null); 
            Debug.Assert(dataGridRow != null); 
            Debug.Assert(dataGridCell != null);
 
            FrameworkElement element = null;
            DataGridBoundColumnBase dataGridBoundColumn = dataGridColumn as DataGridBoundColumnBase;
            if (dataGridBoundColumn != null) 
            {
                //
                if (isCellEdited) 
                { 
                    element = dataGridBoundColumn.GenerateEditingElementInternal();
                    if (element != null) 
                    {
                        //
                        if (dataGridBoundColumn.EditingElementStyle != null && element.Style == null) 
                        {
                            element.Style = dataGridBoundColumn.EditingElementStyle;
                        } 
                        // Subscribe to the GotFocus event so that the non-editing element can be generated 
                        // asynchronously after the edited value is committed.
                        element.GotFocus += new RoutedEventHandler(EditingBoundElement_GotFocus); 
                        this._editingBoundElementGotFocusListeners++;
                        //
 
                        // Remember which cell needs to repopulate its content asynchronously.
                        this._editingBoundCells.Add(dataGridCell);
                    } 
                } 
                else
                { 
                    // Generate Element and apply column style if available
                    element = dataGridBoundColumn.GenerateElementInternal();
                    // 
                    if (element != null && dataGridBoundColumn.ElementStyle != null && element.Style == null)
                    {
                        element.Style = dataGridBoundColumn.ElementStyle; 
                    } 
                }
            } 
            else
            {
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
                            throw DataGridError.DataGrid.MissingTemplateForType(typeof(DataGridTemplateColumn));
                        }
                        element = cellTemplate.LoadContent() as FrameworkElement; 
                    } 
                    else if (isCellEdited)
                    { 
                        PrepareCellEditPrivate(dataGridCell.Content as FrameworkElement);
                        return;
                    } 
                }
            }
 
            if (isCellEdited && element != null) 
            {
                // 
                element.Loaded += new RoutedEventHandler(EditingElement_Loaded);
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
                    Debug.Assert(dataGridCell.OwningColumn is DataGridBoundColumnBase);
                    if (dataGridCell.OwningColumn.Index != this._editingColumnIndex ||
                        dataGridCell.OwningRow != this._editingRow) 
                    { 
                        if (this._editingBoundElementGotFocusListeners > 0)
                        { 
                            // No longer interested in the GotFocus event
                            FrameworkElement element = dataGridCell.Content as FrameworkElement;
                            if (element != null) 
                            {
                                element.GotFocus -= new RoutedEventHandler(EditingBoundElement_GotFocus);
                                this._editingBoundElementGotFocusListeners--; 
                                Debug.Assert(this._editingBoundElementGotFocusListeners >= 0); 
                                //
                            } 
                        }
                        this._editingBoundCells.RemoveAt(0);
 
                        // Repopulate the cell with the non-editing element
                        PopulateCellContent(false /*forceTemplating*/, false /*isCellEdited*/,
                                            dataGridCell.OwningColumn, dataGridCell.OwningRow, dataGridCell); 
                    } 
                }
                else 
                {
                    this._editingBoundCells.RemoveAt(0);
                } 
            }
        }
 
        private void PrepareCellEditPrivate(FrameworkElement element) 
        {
            // 

            if (this._editingColumnIndex == -1 ||
                this.CurrentColumnIndex == -1 || 
                this._editingRow.Cells[this.CurrentColumnIndex].Content != element)
            {
                // The current cell has changed since the call to BeginCellEdit. The cell needs 
                // to get back into non-editing mode. 
                DataGridCell dataGridCell = GetOwningCell(element);
                if (dataGridCell != null) 
                {
                    Debug.Assert(!(dataGridCell.OwningColumn is DataGridTemplateColumn));
                    if (this._editingBoundCells.Count > 0 && dataGridCell == this._editingBoundCells[0]) 
                    {
                        Debug.Assert(dataGridCell.OwningColumn is DataGridBoundColumnBase);
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
 
            FocusEditingCell(this._editingTriggerInfo != null && this._editingTriggerInfo.ContainsFocus /*setFocus*/); 

            DataGridColumnBase dataGridColumn = this.CurrentColumn; 
            DataGridBoundColumnBase dataGridBoundColumn = dataGridColumn as DataGridBoundColumnBase;
            if (dataGridBoundColumn != null)
            { 
                this._uneditedValue = dataGridBoundColumn.PrepareCellEdit(this._editingTriggerInfo);
                DataGridValueConverter dataGridValueConverter = dataGridBoundColumn.DisplayMemberBinding.Converter as DataGridValueConverter;
                if (dataGridValueConverter != null) 
                { 
                    //
 

                    dataGridValueConverter.FallbackValue = this._uneditedValue;
                    dataGridValueConverter.Row = this._editingRow; 
                }
            }
            else 
            { 
                DataGridTemplateColumn dataGridTemplateColumn = dataGridColumn as DataGridTemplateColumn;
                if (dataGridTemplateColumn != null) 
                {
                    OnPrepareCellEdit(new DataGridPrepareCellEditEventArgs(dataGridColumn, this._editingRow, element, this._editingTriggerInfo));
                } 
            }
            //
        } 
 
        private bool ProcessAKey()
        { 
            bool ctrl, shift, alt;

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift, out alt); 

            if (ctrl && !shift && !alt && this.SelectionMode == DataGridSelectionMode.ExtendedFullRow)
            { 
                SelectAll(); 
                return true;
            } 
            return false;
        }
 
        private bool ProcessDataGridKey(KeyEventArgs e)
        {
            switch (e.Key) 
            { 
                case Key.Tab:
                    return ProcessTabKey(e); 

                case Key.Up:
                    return ProcessUpKey(); 

                case Key.Down:
                    return ProcessDownKey(); 
 
                case Key.PageDown:
                    return ProcessNextKey(); 

                case Key.PageUp:
                    return ProcessPriorKey(); 

                case Key.Left:
                    return ProcessLeftKey(); 
 
                case Key.Right:
                    return ProcessRightKey(); 

                case Key.F2:
                    return ProcessF2Key(e); 

                case Key.Home:
                    return ProcessHomeKey(); 
 
                //
 


                case Key.End: 
                    return ProcessEndKey();

                case Key.Enter: 
                    return ProcessEnterKey(); 

                case Key.Escape: 
                    return ProcessEscapeKey();

                case Key.A: 
                    return ProcessAKey();

                // 
 

 


                // 


 
            } 
            return false;
        } 

        private bool ProcessDownKey()
        { 
            bool moved, shift, ctrl;
            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);
            return ProcessDownKeyInternal(shift, ctrl, out moved); 
        } 

        [SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")] 
        private bool ProcessDownKeyInternal(bool shift, bool ctrl, out bool moved)
        {
            bool success; 

            DataGridColumnBase dataGridColumn = this.ColumnsInternal.FirstVisibleColumn;
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
                            Debug.Assert(this.SelectedItemsInternal.Count == 0); 
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
                            if (this.SelectionMode == DataGridSelectionMode.ExtendedFullRow)
                            { 
                                if (!ScrollIntoView(this.CurrentColumnIndex, lastRowIndex, true))
                                {
                                    return true; 
                                } 
                                if (this.AnchorRowIndex == -1 || this.CurrentColumnIndex == -1 ||
                                    IsRowOutOfBounds(lastRowIndex)) 
                                {
                                    moved = false;
                                    return true; 
                                }
                                ClearRowSelection(false /*resetAnchorRowIndex*/);
                                Debug.Assert(this.AnchorRowIndex >= 0); 
                                SetRowsSelection(this.AnchorRowIndex, lastRowIndex); 
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
                                SetRowSelection(this.CurrentRowIndex, false /*isSelected*/, false /*setAnchorRowIndex*/); 
                                SetRowSelection(lastRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
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
                            Debug.Assert(this.SelectedItemsInternal.Count == 0);
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
                            ClearRowSelection(true /*resetAnchorRowIndex*/); 
                            SetRowSelection(lastRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
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
                            Debug.Assert(this.SelectedItemsInternal.Count == 0);
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
                            if (this.SelectionMode == DataGridSelectionMode.ExtendedFullRow) 
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
                            Debug.Assert(this.SelectedItemsInternal.Count == 0); 
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

        private bool ProcessEndKey()
        { 
            bool ctrl;
            bool shift;
 
            KeyboardHelper.GetMetaKeyState(out ctrl, out shift); 

            DataGridColumnBase dataGridColumn = this.ColumnsInternal.LastVisibleColumn; 
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
                if (ctrl /**/) 
                {
                    return ProcessRightMost(lastVisibleColumnIndex, firstRowIndex);
                } 
                else 
                {
                    if (!ScrollIntoView(lastVisibleColumnIndex, lastRowIndex, true)) 
                    {
                        return true;
                    } 
                    if (IsInnerCellOutOfBounds(lastVisibleColumnIndex, lastRowIndex))
                    {
                        return true; 
                    } 
                    ClearRowSelection(false /*resetAnchorRowIndex*/);
                    if (shift) 
                    {
                        if (this.SelectionMode == DataGridSelectionMode.ExtendedFullRow && this.CurrentRowIndex >= 0)
                        { 
                            if (this.CurrentRowIndex == -1)
                            {
                                return true; 
                            } 
                            SetRowsSelection(this.CurrentRowIndex, lastRowIndex);
                        } 
                        else
                        {
                            SetRowSelection(lastRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/); 
                        }
                        bool success = SetCurrentCellCore(lastVisibleColumnIndex, lastRowIndex/*, false, false*/);
                        Debug.Assert(success); 
                    } 
                    else
                    { 
                        SetRowSelection(lastRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
                        bool success = SetCurrentCellCore(lastVisibleColumnIndex, lastRowIndex/*, false, false*/);
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

        private bool ProcessEnterKey()
        { 
            bool ctrl, shift, moved = false, ret = true, endRowEdit = true; 

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift); 

            if (!ctrl)
            { 
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
                    BeginCellEdit(new DataGridEditingTriggerInfo(this.ContainsFocus, Keyboard.Modifiers, e, null));
                }
                return true; 
            }

            return false; 
        } 

        private bool ProcessHomeKey() 
        {
            bool ctrl;
            bool shift; 

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);
 
            DataGridColumnBase dataGridColumn = this.ColumnsInternal.FirstVisibleColumn; 
            int firstVisibleColumnIndex = (dataGridColumn == null) ? -1 : dataGridColumn.Index;
            int firstRowIndex = this.FirstRowIndex; 
            if (firstVisibleColumnIndex == -1 || firstRowIndex == -1)
            {
                return false; 
            }
            this._noSelectionChangeCount++;
            try 
            { 
                if (ctrl /* */)
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
                    ClearRowSelection(true /*resetAnchorRowIndex*/);
                    SetRowSelection(firstRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
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
 
        private bool ProcessLeftKey()
        { 
            bool success;
            bool ctrl;
            bool shift; 

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);
 
            DataGridColumnBase dataGridColumn = this.ColumnsInternal.FirstVisibleColumn; 
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
                        Debug.Assert(this.SelectedItemsInternal.Count == 0);
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

        // Ctrl Left <==> Home 
        private bool ProcessLeftMost(int firstVisibleColumnIndex, int firstRowIndex)
        {
            bool success; 

            this._noSelectionChangeCount++;
            try 
            { 
                if (this.CurrentColumnIndex == -1)
                { 
                    Debug.Assert(this.SelectedItemsInternal.Count == 0);
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

        private bool ProcessNextKey() 
        { 
            bool ctrl;
            bool shift; 

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);
 
            DataGridColumnBase dataGridColumn = this.ColumnsInternal.FirstVisibleColumn;
            int firstVisibleColumnIndex = (dataGridColumn == null) ? -1 : dataGridColumn.Index;
            if (firstVisibleColumnIndex == -1) 
            { 
                return false;
            } 
            int nextScreenRowIndexTmp, nextScreenRowIndex = -1, jumpRows = 0;
            if (this.CurrentRowIndex == -1)
            { 
                nextScreenRowIndex = this._firstDisplayedRowIndex;
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
                    Debug.Assert(this.SelectedItemsInternal.Count == 0);
                    SetRowSelection(nextScreenRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/); 
                    ScrollIntoView(firstVisibleColumnIndex, nextScreenRowIndex, false);
                    if (IsInnerCellOutOfBounds(firstVisibleColumnIndex, nextScreenRowIndex))
                    { 
                        return true; 
                    }
                    SetCurrentCellCore(firstVisibleColumnIndex, nextScreenRowIndex/*, false, false*/); 
                    return true;
                }
 
                if (!ScrollIntoView(this.CurrentColumnIndex, nextScreenRowIndex, true))
                {
                    return true; 
                } 
                if (this.CurrentColumnIndex == -1 || IsRowOutOfBounds(nextScreenRowIndex))
                { 
                    return true;
                }
                ClearRowSelection(false /*resetAnchorRowIndex*/); 
                if (shift && this.SelectionMode == DataGridSelectionMode.ExtendedFullRow)
                {
                    Debug.Assert(this.AnchorRowIndex >= 0); 
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
                    SetRowSelection(nextScreenRowIndex, true /*isSelected*/, false /*setAnchorRowIndex*/);
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
 
        private bool ProcessPriorKey() 
        {
            bool ctrl; 
            bool shift;
            bool success;
 
            KeyboardHelper.GetMetaKeyState(out ctrl, out shift);

            DataGridColumnBase dataGridColumn = this.ColumnsInternal.FirstVisibleColumn; 
            int firstVisibleColumnIndex = (dataGridColumn == null) ? -1 : dataGridColumn.Index; 
            if (firstVisibleColumnIndex == -1)
            { 
                return false;
            }
            int previousScreenRowIndexTmp, previousScreenRowIndex = -1; 
            if (this.CurrentRowIndex == -1)
            {
                previousScreenRowIndex = this._firstDisplayedRowIndex; 
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
                    Debug.Assert(this.SelectedItemsInternal.Count == 0);
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
                ClearRowSelection(false /*resetAnchorRowIndex*/);
                if (shift && this.SelectionMode == DataGridSelectionMode.ExtendedFullRow) 
                {
                    Debug.Assert(this.AnchorRowIndex >= 0);
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
                    SetRowSelection(previousScreenRowIndex, true /*isSelected*/, false /*setAnchorRowIndex*/); 
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

        private bool ProcessRightKey() 
        {
            bool success;
            bool ctrl; 
            bool shift; 

            KeyboardHelper.GetMetaKeyState(out ctrl, out shift); 

            DataGridColumnBase dataGridColumn = this.ColumnsInternal.LastVisibleColumn;
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
                        Debug.Assert(this.SelectedItemsInternal.Count == 0);
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

        // Ctrl Right <==> End 
        private bool ProcessRightMost(int lastVisibleColumnIndex, int firstRowIndex)
        {
            bool success; 
 
            this._noSelectionChangeCount++;
            try 
            {
                if (this.CurrentColumnIndex == -1)
                { 
                    Debug.Assert(this.SelectedItemsInternal.Count == 0);
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

        [SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")] 
        private bool ProcessUpKey()
        {
            bool ctrl; 
            bool shift;
            bool success;
 
            KeyboardHelper.GetMetaKeyState(out ctrl, out shift); 

            DataGridColumnBase dataGridColumn = this.ColumnsInternal.FirstVisibleColumn; 
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
                            Debug.Assert(this.SelectedItemsInternal.Count == 0);
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
                            if (this.SelectionMode == DataGridSelectionMode.ExtendedFullRow)
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
                                ClearRowSelection(false /*resetAnchorRowIndex*/);
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
                                SetRowSelection(this.CurrentRowIndex, false /*isSelected*/, false /*setAnchorRowIndex*/);
                                SetRowSelection(firstRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/); 
                                success = SetCurrentCellCore(this.CurrentColumnIndex, firstRowIndex/*, false, false*/);
                                Debug.Assert(success);
                            } 
                        }
                    }
                    else 
                    { 
                        if (this.CurrentColumnIndex == -1)
                        { 
                            Debug.Assert(this.SelectedItemsInternal.Count == 0);
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
                            ClearRowSelection(true /*resetAnchorRowIndex*/);
                            SetRowSelection(firstRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
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
                            Debug.Assert(this.SelectedItemsInternal.Count == 0);
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
                            if (this.SelectionMode == DataGridSelectionMode.ExtendedFullRow) 
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
                            Debug.Assert(this.SelectedItemsInternal.Count == 0); 
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
                            ClearRowSelection(true /*resetAnchorRowIndex*/); 
                            SetRowSelection(previousRowIndex, true /*isSelected*/, true /*setAnchorRowIndex*/);
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
            DataGridColumnBase dataGridColumn; 
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
 
                if (targetRowIndex != this.CurrentRowIndex || (this.SelectionMode == DataGridSelectionMode.ExtendedFullRow))
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
                success = BeginCellEdit(new DataGridEditingTriggerInfo(true /*containsFocus*/, Keyboard.Modifiers, e, null));
            } 
            return true;
        }
 
        private void RefreshRowsAndColumns()
        {
            SuspendLayout(); 
            ClearRows(); 
            if (this.AutoGenerateColumns)
            { 
                //Column auto-generation refreshes the rows too
                AutoGenerateColumnsPrivate();
            } 
            else
            {
                RefreshRows(); 
            } 
            ResumeLayout(true);
            if (this.Columns.Count > 0 && this.CurrentColumnIndex == -1) 
            {
                MakeFirstDisplayedCellCurrentCell();
            } 
        }

        private void RemoveDisplayedColumnHeader(DataGridColumnBase dataGridColumn) 
        { 
            if (this.AreColumnHeadersVisible && _columnHeaders != null)
            { 
                _columnHeaders.Children.RemoveAt(dataGridColumn.Index);
            }
        } 

        private void RemoveDisplayedColumnHeaders()
        { 
            if (_columnHeaders != null) 
            {
                _columnHeaders.Children.Clear(); 
            }
            this.ColumnsInternal.FillerColumn.IsRepresented = false;
        } 

        /*
 
 

 


 


 
 

 


 


 
 

 


 


 
 

*/ 

        private void RemoveDisplayedHorizontalGridlines()
        { 
            if (this._cells != null && this.DisplayedAndEditedRowCount > 0)
            {
                for (int childIndex = 0; childIndex < this.DisplayedAndEditedRowCount; childIndex++) 
                { 
                    RemoveDisplayedHorizontalGridlines(this._cells.Children[childIndex] as DataGridRow);
                } 
            }
        }
 
        private void RemoveDisplayedHorizontalGridlines(DataGridRow dataGridRow)
        {
            Debug.Assert(this._cells != null); 
            Debug.Assert(dataGridRow != null); 
            if (dataGridRow.HasBottomGridline)
            { 
                Debug.Assert(this._cells.Children.Count > this.DisplayedAndEditedRowCount + this._displayedVerticalGridlineCount);
                Debug.Assert(this._cells.Children.Contains(dataGridRow.BottomGridline));
                this._cells.Children.Remove(dataGridRow.BottomGridline); 
                dataGridRow.ResetGridline();
            }
        } 
 
        private void RemoveDisplayedRowAt(int childIndex)
        { 
            Debug.Assert(this._cells != null);
            Debug.Assert(childIndex > -1);
            Debug.Assert(this._cells.Children[childIndex] is DataGridRow); 
            this._cells.Children.RemoveAt(childIndex);
        }
 
        private void RemoveDisplayedRowHeader(int childIndex) 
        {
            Debug.Assert(this._rowHeaders != null); 
            this._rowHeaders.Children.RemoveAt(childIndex);
        }
 
        private void RemoveDisplayedVerticalGridlines()
        {
            if (this._cells != null && this.DisplayedAndEditedRowCount > 0) 
            { 
                for (int childIndex = 0; childIndex < this.DisplayedAndEditedRowCount; childIndex++)
                { 
                    RemoveDisplayedVerticalGridlines(this._cells.Children[childIndex] as DataGridRow);
                }
            } 
        }

        private void RemoveDisplayedVerticalGridlines(DataGridColumnBase dataGridColumn) 
        { 
            Debug.Assert(this._cells != null);
            Debug.Assert(dataGridColumn != null); 
            if (this.DisplayedAndEditedRowCount > 0)
            {
                for (int childIndex = 0; childIndex < this.DisplayedAndEditedRowCount; childIndex++) 
                {
                    DataGridRow dataGridRow = this._cells.Children[childIndex] as DataGridRow;
                    RemoveDisplayedVerticalGridlines(dataGridRow.Cells[dataGridColumn.Index]); 
                } 
            }
        } 

        private void RemoveDisplayedVerticalGridlines(DataGridRow dataGridRow)
        { 
            Debug.Assert(this._cells != null);
            Debug.Assert(dataGridRow != null);
            foreach (DataGridCell dataGridCell in dataGridRow.Cells) 
            { 
                RemoveDisplayedVerticalGridlines(dataGridCell);
            } 
        }

        private void RemoveDisplayedVerticalGridlines(DataGridCell dataGridCell) 
        {
            Debug.Assert(this._cells != null);
            Debug.Assert(dataGridCell != null); 
            if (dataGridCell.HasRightGridline) 
            {
                Debug.Assert(this._displayedVerticalGridlineCount > 0); 
                Debug.Assert(this._cells.Children.Contains(dataGridCell.RightGridline));
                this._cells.Children.Remove(dataGridCell.RightGridline);
                this._displayedVerticalGridlineCount--; 
                dataGridCell.ResetGridline();
            }
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

        private void ResumeLayout(bool updateLayout) 
        {
            Debug.Assert(this._layoutSuspended > 0);
            this._layoutSuspended--; 
            if (this._layoutSuspended == 0 && updateLayout)
            {
                PerformLayout(); 
            } 
        }
 
        private bool ScrollIntoView(int columnIndex, int rowIndex, bool forCurrentCellChange)
        {
            Debug.Assert(columnIndex >= 0 && columnIndex < this.Columns.Count); 
            Debug.Assert(this.DisplayData.FirstDisplayedScrollingCol >= -1 && this.DisplayData.FirstDisplayedScrollingCol < this.Columns.Count);
            Debug.Assert(this.DisplayData.LastTotallyDisplayedScrollingCol >= -1 && this.DisplayData.LastTotallyDisplayedScrollingCol < this.Columns.Count);
            Debug.Assert(!IsRowOutOfBounds(rowIndex)); 
            Debug.Assert(this.DisplayData.FirstDisplayedScrollingRow >= -1 && this.DisplayData.FirstDisplayedScrollingRow < this._rowCount); 
            Debug.Assert(this.Columns[columnIndex].Visibility == Visibility.Visible);
 
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
            return ScrollRowIntoView(rowIndex);
        } 

        private void SelectAll()
        { 
            SetRowsSelection(0, this._rowCount - 1);
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
                        if (this.SelectionMode == DataGridSelectionMode.ExtendedFullRow && this.SelectedItems.Count > 1)
                        {
                            return true;   // Do not discard the multi-selection 
                        } 
                        if (this.SelectedItems.Count == 1)
                        { 
                            DataGridRow dataGridRow = GetRowFromItem(this.SelectedItem);
                            if (dataGridRow == null || dataGridRow.Index != rowIndex)
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
            Debug.Assert(rowIndex < this._rowCount);
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
                Debug.Assert(this.CurrentRowIndex < this._rowCount); 

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
                    IsRowDisplayed(oldCurrentCell.RowIndex))
                { 
                    oldDisplayedCurrentRow = GetDisplayedRow(oldCurrentCell.RowIndex); 
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
                    oldDisplayedCurrentRow.HeaderCell.ApplyRowStatus(true /*animate*/); 
                }
                oldDisplayedCurrentRow.Cells[oldCurrentCell.ColumnIndex].ApplyCellState(true /*animate*/);
            } 
 
            if (this.CurrentColumnIndex > -1)
            { 
                Debug.Assert(this.CurrentRowIndex > -1 /**/);
                Debug.Assert(this.CurrentColumnIndex < this.ColumnsItemsInternal.Count);
                Debug.Assert(this.CurrentRowIndex < this._rowCount); 
                if (IsRowDisplayed(this.CurrentRowIndex))
                {
                    DataGridRow dataGridRow = GetDisplayedRow(this.CurrentRowIndex); 
                    dataGridRow.Cells[this.CurrentColumnIndex].ApplyCellState(true /*animate*/); 
                    if (this.AreRowHeadersVisible)
                    { 
                        dataGridRow.HeaderCell.ApplyRowStatus(true /*animate*/);
                    }
                } 
            }

            PerformCurrentCellFocusVisualLayout(); 
            ApplyCurrentCellFocusVisualState(); 

            OnCurrentCellChanged(EventArgs.Empty); 

            return true;
        } 

        private void SuspendLayout()
        { 
            this._layoutSuspended++; 
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
                        double widthNotVisible = totalVisibleWidth - cellsWidth; 
                        // maximum travel distance -- not the total width
                        this._hScrollBar.Maximum = totalVisibleWidth - totalVisibleFrozenWidth - cellsWidth;
                        Debug.Assert(this._hScrollBar.Maximum > 0); 
 
                        // width of the scrollable viewing area
                        this._hScrollBar.ViewportSize = cellsWidth - totalVisibleFrozenWidth; 
                        this._hScrollBar.LargeChange = Math.Max(totalVisibleWidth - totalVisibleFrozenWidth - widthNotVisible, 0);
                    }
                    else 
                    {
                        //
                        this._hScrollBar.Maximum = 0; 
                        this._hScrollBar.ViewportSize = 0; 
                    }
                    this._hScrollBar.Width = cellsWidth - totalVisibleFrozenWidth; 
                    if (this._hScrollBar.Visibility != Visibility.Visible)
                    {
                        // This will trigger a call to this method via Cells_SizeChanged for 
                        // which no processing is needed.
                        this._hScrollBar.Visibility = Visibility.Visible;
                        this._ignoreNextScrollBarsLayout = true; 
                    } 
                }
                else if (this._hScrollBar.Visibility != Visibility.Collapsed) 
                {
                    // This will trigger a call to this method via Cells_SizeChanged for
                    // which no processing is needed. 
                    this._hScrollBar.Visibility = Visibility.Collapsed;
                    this._ignoreNextScrollBarsLayout = true;
                    // 
 

                } 
            }
        }
 
        /*

 
 

 


 


 
 

 


 


 
 

 


*/ 

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
                        Debug.Assert(this._vScrollBar.Maximum > 0); 

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
                    this._vScrollBar.Height = cellsHeight; 
                    if (this._vScrollBar.Visibility != Visibility.Visible) 
                    {
                        // This will trigger a call to this method via Cells_SizeChanged for 
                        // which no processing is needed.
                        this._vScrollBar.Visibility = Visibility.Visible;
                        this._ignoreNextScrollBarsLayout = true; 
                    }
                }
                else if (this._vScrollBar.Visibility != Visibility.Collapsed) 
                { 
                    // This will trigger a call to this method via Cells_SizeChanged for
                    // which no processing is needed. 
                    this._vScrollBar.Visibility = Visibility.Collapsed;
                    this._ignoreNextScrollBarsLayout = true;
                } 
            }
        }
 
        /* 

 


 


 
 

 


 


 
 

 


 


*/

        private void VerticalScrollBar_Scroll(object sender, System.Windows.Controls.Primitives.ScrollEventArgs e)
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
                double newVerticalOffset = this._vScrollBar.Value; 
                double heightDifference = 0;
                if (e.ScrollEventType == System.Windows.Controls.Primitives.ScrollEventType.SmallIncrement)
                { 
                    heightDifference = GetVerticalSmallScrollIncrease(); 
                    newVerticalOffset = this._verticalOffset + heightDifference;
                    if (newVerticalOffset > this._vScrollBar.Maximum) 
                    {
                        heightDifference -= newVerticalOffset - this._vScrollBar.Maximum;
                        newVerticalOffset = this._vScrollBar.Maximum; 
                        this._vScrollBar.Value = this._vScrollBar.Maximum;
                    }
                }
                else if (e.ScrollEventType == System.Windows.Controls.Primitives.ScrollEventType.SmallDecrement) 
                {
                    heightDifference = -GetVerticalSmallScrollDecrease(); 
                    newVerticalOffset = this._verticalOffset + heightDifference;
                }
                else 
                {
                    heightDifference = this._vScrollBar.Value - this._verticalOffset;
                } 
 
                if (!DoubleUtil.IsZero(heightDifference))
                { 
                    ScrollRowsByHeight(heightDifference, newVerticalOffset);
                }
 
                CorrectDisplayedRowsIndexes();
                // Unconditionally calling the full PerformLayout because ComputeScrollBarsLayout() needs to update the
                // vertical scrollbar ViewRange. Indeed this.EgdedRowsHeight may have become more precise thanks to this scroll. 
                PerformLayout(); 
            }
            finally 
            {
                this._verticalScrollChangesIgnored--;
            } 
        }

        #endregion Private Methods 
    } 
}
