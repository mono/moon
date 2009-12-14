// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Windows.Automation.Peers;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace System.Windows.Controls
{
    [TemplatePart(Name = DataGridRow.DATAGRIDROW_elementBottomGridLine, Type = typeof(Rectangle))]
    [TemplatePart(Name = DataGridRow.DATAGRIDROW_elementCells, Type = typeof(DataGridCellsPresenter))]
    [TemplatePart(Name = DataGridRow.DATAGRIDROW_elementDetails, Type = typeof(DataGridDetailsPresenter))]
    [TemplatePart(Name = DataGridRow.DATAGRIDROW_elementRoot, Type = typeof(Panel))]
    [TemplatePart(Name = DataGridRow.DATAGRIDROW_elementRowHeader, Type = typeof(DataGridRowHeader))]

    [TemplateVisualState(Name = DataGridRow.DATAGRIDROW_stateNormal, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRow.DATAGRIDROW_stateAlternate, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRow.DATAGRIDROW_stateNormalEditing, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRow.DATAGRIDROW_stateNormalEditingFocused, GroupName = VisualStates.GroupCommon)]

    [TemplateVisualState(Name = DataGridRow.DATAGRIDROW_stateSelected, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRow.DATAGRIDROW_stateSelectedFocused, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRow.DATAGRIDROW_stateMouseOver, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRow.DATAGRIDROW_stateMouseOverEditing, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRow.DATAGRIDROW_stateMouseOverEditingFocused, GroupName = VisualStates.GroupCommon)]

    [TemplateVisualState(Name = DataGridRow.DATAGRIDROW_stateMouseOverSelected, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = DataGridRow.DATAGRIDROW_stateMouseOverSelectedFocused, GroupName = VisualStates.GroupCommon)]
    public partial class DataGridRow : Control
    {
        #region Constants

        private const byte DATAGRIDROW_defaultMinHeight = 0;
        internal const int DATAGRIDROW_maximumHeight = 65536;
        internal const double DATAGRIDROW_minimumHeight = 0;

        private const string DATAGRIDROW_detailsVisibleTransition = "DetailsVisibleTransition";

        private const string DATAGRIDROW_elementBottomGridLine = "BottomGridLine";
        private const string DATAGRIDROW_elementCells = "CellsPresenter";
        private const string DATAGRIDROW_elementDetails = "DetailsPresenter";
        private const string DATAGRIDROW_elementRoot = "Root";
        private const string DATAGRIDROW_elementRowHeader = "RowHeader";

        private const string DATAGRIDROW_stateAlternate = "Normal AlternatingRow";
        private const string DATAGRIDROW_stateMouseOver = "MouseOver";
        private const string DATAGRIDROW_stateMouseOverEditing = "MouseOver Unfocused Editing";
        private const string DATAGRIDROW_stateMouseOverEditingFocused = "MouseOver Editing";
        private const string DATAGRIDROW_stateMouseOverSelected = "MouseOver Unfocused Selected";
        private const string DATAGRIDROW_stateMouseOverSelectedFocused = "MouseOver Selected";
        private const string DATAGRIDROW_stateNormal = "Normal";
        private const string DATAGRIDROW_stateNormalEditing = "Unfocused Editing";
        private const string DATAGRIDROW_stateNormalEditingFocused = "Normal Editing";
        private const string DATAGRIDROW_stateSelected = "Unfocused Selected";
        private const string DATAGRIDROW_stateSelectedFocused = "Normal Selected";

        private const byte DATAGRIDROW_stateMouseOverCode = 0;
        private const byte DATAGRIDROW_stateMouseOverEditingCode = 1;
        private const byte DATAGRIDROW_stateMouseOverEditingFocusedCode = 2;
        private const byte DATAGRIDROW_stateMouseOverSelectedCode = 3;
        private const byte DATAGRIDROW_stateMouseOverSelectedFocusedCode = 4;
        private const byte DATAGRIDROW_stateNormalCode = 5;
        private const byte DATAGRIDROW_stateNormalEditingCode = 6;
        private const byte DATAGRIDROW_stateNormalEditingFocusedCode = 7;
        private const byte DATAGRIDROW_stateSelectedCode = 8;
        private const byte DATAGRIDROW_stateSelectedFocusedCode = 9;
        private const byte DATAGRIDROW_stateNullCode = 255;

        #endregion Constants

        #region Data

        // Locally cache whether or not details are visible so we don't run redundant storyboards
        // The Details Template that is actually applied to the Row
        private DataTemplate _appliedDetailsTemplate;
        private Visibility? _appliedDetailsVisibility;
        private Rectangle _bottomGridLine;
        private DataGridCellsPresenter _cellsElement;
        // Optimal height of the details based on the Element created by the DataTemplate
        private double _detailsDesiredHeight;
        private bool _detailsLoaded;
        private Storyboard _detailsVisibleStoryboard;
        private bool _detailsVisibilityNotificationPending;
        private DoubleAnimation _detailsHeightAnimation;
        private double? _detailsHeightAnimationToOverride;
        private FrameworkElement _detailsContent;
        private DataGridDetailsPresenter _detailsElement;
        private DataGridCell _fillerCell;
        private DataGridRowHeader _headerElement;
        private double _lastHorizontalOffset;
        private int? _mouseOverColumnIndex;    // 

        // Static arrays to handle state transitions:
        private static byte[] _idealStateMapping = new byte[] {
            DATAGRIDROW_stateNormalCode,
            DATAGRIDROW_stateNormalCode,
            DATAGRIDROW_stateMouseOverCode,
            DATAGRIDROW_stateMouseOverCode,
            DATAGRIDROW_stateNullCode,
            DATAGRIDROW_stateNullCode,
            DATAGRIDROW_stateNullCode,
            DATAGRIDROW_stateNullCode,
            DATAGRIDROW_stateSelectedCode,
            DATAGRIDROW_stateSelectedFocusedCode,
            DATAGRIDROW_stateMouseOverSelectedCode,
            DATAGRIDROW_stateMouseOverSelectedFocusedCode,
            DATAGRIDROW_stateNormalEditingCode,
            DATAGRIDROW_stateNormalEditingFocusedCode,
            DATAGRIDROW_stateMouseOverEditingCode,
            DATAGRIDROW_stateMouseOverEditingFocusedCode
        };

        private static byte[] _fallbackStateMapping = new byte[] {
            DATAGRIDROW_stateNormalCode, //DATAGRIDROW_stateMouseOverCode's fallback
            DATAGRIDROW_stateMouseOverEditingFocusedCode, //DATAGRIDROW_stateMouseOverEditingCode's fallback
            DATAGRIDROW_stateNormalEditingFocusedCode, //DATAGRIDROW_stateMouseOverEditingFocusedCode's fallback
            DATAGRIDROW_stateMouseOverSelectedFocusedCode, //DATAGRIDROW_stateMouseOverSelectedCode's fallback
            DATAGRIDROW_stateSelectedFocusedCode, //DATAGRIDROW_stateMouseOverSelectedFocusedCode's fallback
            DATAGRIDROW_stateNullCode, //DATAGRIDROW_stateNormalCode's fallback
            DATAGRIDROW_stateNormalEditingFocusedCode, //DATAGRIDROW_stateNormalEditingCode's fallback
            DATAGRIDROW_stateSelectedFocusedCode, //DATAGRIDROW_stateNormalEditingFocusedCode's fallback
            DATAGRIDROW_stateSelectedFocusedCode, //DATAGRIDROW_stateSelectedCode's fallback
            DATAGRIDROW_stateNormalCode //DATAGRIDROW_stateSelectedFocusedCode's fallback
        };

        private static string[] _stateNames = new string[] {
            DATAGRIDROW_stateMouseOver,
            DATAGRIDROW_stateMouseOverEditing,
            DATAGRIDROW_stateMouseOverEditingFocused,
            DATAGRIDROW_stateMouseOverSelected,
            DATAGRIDROW_stateMouseOverSelectedFocused,
            DATAGRIDROW_stateNormal,
            DATAGRIDROW_stateNormalEditing,
            DATAGRIDROW_stateNormalEditingFocused,
            DATAGRIDROW_stateSelected,
            DATAGRIDROW_stateSelectedFocused
        };

        #endregion Data

        // 

        private delegate void VerticalGridLineLayoutDelegate(double leftEdge, DataGridColumn dataGridColumn, DataGridCell dataGridCell, bool isCellFrozen);

        //
        [SuppressMessage("Microsoft.Performance", "CA1805:DoNotInitializeUnnecessarily")]
        [SuppressMessage("Microsoft.Usage", "CA2214:DoNotCallOverridableMethodsInConstructors")]
        public DataGridRow()
        {
            this.MinHeight = DATAGRIDROW_defaultMinHeight;

            this.Index = -1;
            this._mouseOverColumnIndex = null;
            this._detailsDesiredHeight = double.NaN;
            this._detailsLoaded = false;
            this.SetValueNoCallback(DetailsVisibilityProperty, Visibility.Collapsed);
            this._appliedDetailsVisibility = Visibility.Collapsed;
            this.Cells = new DataGridCellCollection(this);
            this.Cells.CellAdded += new EventHandler<DataGridCellEventArgs>(DataGridCellCollection_CellAdded);
            this.Cells.CellRemoved += new EventHandler<DataGridCellEventArgs>(DataGridCellCollection_CellRemoved);

            this.MouseLeftButtonDown += new MouseButtonEventHandler(DataGridRow_MouseLeftButtonDown);
            this.MouseEnter += new MouseEventHandler(DataGridRow_MouseEnter);
            this.MouseLeave += new MouseEventHandler(DataGridRow_MouseLeave);

            DefaultStyleKey = typeof(DataGridRow);
        }

        #region Dependency Properties

/*
        // 




























*/
        #region DetailsTemplate
        /// <summary>
        /// 
        /// </summary>
        public DataTemplate DetailsTemplate
        {
            get { return GetValue(DetailsTemplateProperty) as DataTemplate; }
            set { SetValue(DetailsTemplateProperty, value); }
        }

        /// <summary>
        /// Identifies the DetailsTemplate dependency property.
        /// </summary>
        public static readonly DependencyProperty DetailsTemplateProperty =
            DependencyProperty.Register(
                "DetailsTemplate",
                typeof(DataTemplate),
                typeof(DataGridRow),
                new PropertyMetadata(OnDetailsTemplatePropertyChanged));

        /// <summary>
        /// DetailsTemplateProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGridRow that changed its DetailsTemplate.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnDetailsTemplatePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGridRow source = d as DataGridRow;
            Debug.Assert(source != null,
                "The source is not an instance of DataGridRow!");

            Debug.Assert(typeof(DataTemplate).IsInstanceOfType(e.NewValue) || (e.NewValue == null),
                "The value is not an instance of DataTemplate!");
            DataTemplate newValue = (DataTemplate)e.NewValue;
            DataTemplate oldValue = (DataTemplate)e.OldValue;

            if (!source.IsHandlerSuspended(e.Property) && source.OwningGrid != null)
            {
                Func<DataTemplate, DataTemplate> actualDetailsTemplate = template => (template != null ? template : source.OwningGrid.RowDetailsTemplate);

                // We don't always want to apply the new Template because they might have set the same one
                // we inherited from the DataGrid
                if (actualDetailsTemplate(newValue) != actualDetailsTemplate(oldValue))
                {
                    source.ApplyDetailsTemplate(false /* initializeDetailsPreferredHeight */);
                }
            }
        }
        #endregion DetailsTemplate

        #region DetailsVisibility
        ///// <summary>
        ///// 
        ///// </summary>
        public Visibility DetailsVisibility
        {
            get { return (Visibility)GetValue(DetailsVisibilityProperty); }
            set { SetValue(DetailsVisibilityProperty, value); }
        }

        /// <summary>
        /// Identifies the DetailsTemplate dependency property.
        /// </summary>
        public static readonly DependencyProperty DetailsVisibilityProperty =
            DependencyProperty.Register(
                "DetailsVisibility",
                typeof(Visibility),
                typeof(DataGridRow),
                new PropertyMetadata(OnDetailsVisibilityPropertyChanged));

        /// <summary>
        /// DetailsVisibilityProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGridRow that changed its DetailsTemplate.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnDetailsVisibilityPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGridRow row = (DataGridRow)d;

            if (!row.IsHandlerSuspended(e.Property))
            {
                if (row.OwningGrid == null)
                {
                    throw DataGridError.DataGrid.NoOwningGrid(row.GetType());
                }
                if (row.Index == -1)
                {
                    throw DataGridError.DataGridRow.InvalidRowIndexCannotCompleteOperation();
                }

                Visibility newValue = (Visibility)e.NewValue;
                row.OwningGrid.OnRowDetailsVisibilityPropertyChanged(row.Index, newValue);
                row.SetDetailsVisibilityInternal(newValue, true /*raiseNotification*/, true /*animate*/);
            }
        }
        #endregion DetailsVisibility

        #region Header
        /// <summary>
        /// 
        /// </summary>
        public object Header
        {
            get { return GetValue(HeaderProperty); }
            set { SetValue(HeaderProperty, value); }
        }

        /// <summary>
        /// Identifies the Header dependency property.
        /// </summary>
        public static readonly DependencyProperty HeaderProperty =
            DependencyProperty.Register(
                "Header",
                typeof(object),
                typeof(DataGridRow),
                new PropertyMetadata(OnHeaderPropertyChanged));

        /// <summary>
        /// HeaderProperty property changed handler.
        /// </summary>
        /// <param name="d">DataGridRow that changed its Header.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnHeaderPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            DataGridRow source = d as DataGridRow;
            Debug.Assert(source != null,
                "The source is not an instance of DataGridRow!");

            if (source._headerElement != null)
            {
                source._headerElement.Content = e.NewValue;
            }
        }
        #endregion Header

        #region HeaderStyle
        /// <summary>
        /// 
        /// </summary>
        public Style HeaderStyle
        {
            get { return GetValue(HeaderStyleProperty) as Style; }
            set { SetValue(HeaderStyleProperty, value); }
        }

        public static readonly DependencyProperty HeaderStyleProperty = 
            DependencyProperty.Register(
                "HeaderStyle", 
                typeof(Style),
                typeof(DataGridRow), 
                new PropertyMetadata(OnHeaderStylePropertyChanged));

        private static void OnHeaderStylePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Style newStyle = e.NewValue as Style;
            if (newStyle != null)
            {
                DataGridRow row = d as DataGridRow;
                if (row != null && row._headerElement != null && row._headerElement.Style != newStyle)
                {
                    row._headerElement.Style = newStyle;
                }
            }
        }
        #endregion HeaderStyle

        #endregion

        #region Public Properties

        #endregion Public Properties

        #region Protected Properties
        #endregion Protected Properties

        #region Internal Properties

        internal double ActualBottomGridLineHeight
        {
            get
            {
                if (_bottomGridLine != null && this.OwningGrid != null && this.OwningGrid.AreRowBottomGridLinesRequired)
                {
                    // Unfortunately, _bottomGridLine has no size yet so we can't get its actualheight
                    return DataGrid.HorizontalGridLinesThickness;
                }
                return 0;
            }
        }

        internal DataGridCellCollection Cells
        {
            get;
            private set;
        }

        internal DataGridCell FillerCell
        {
            get
            {
                Debug.Assert(this.OwningGrid != null);

                if (_fillerCell == null)
                {
                    _fillerCell = new DataGridCell();
                    _fillerCell.Visibility = Visibility.Collapsed;
                    _fillerCell.OwningRow = this;
                    _cellsElement.Children.Add(_fillerCell);
                }
                return _fillerCell;
            }
        }

        internal bool HasBottomGridLine
        {
            get
            {
                return this._bottomGridLine != null;
            }
        }

        internal bool HasHeaderCell
        {
            get
            {
                return this._headerElement != null;
            }
        }

        internal DataGridRowHeader HeaderCell
        {
            get
            {
                return _headerElement;
            }
        }

        /// <summary>
        /// Index of the row
        /// </summary>
        internal int Index
        {
            get;
            set;
        }

        internal bool IsEditing
        {
            get
            {
                return this.OwningGrid != null && this.OwningGrid.EditingRowIndex == this.Index;
            }
        }

        /// <summary>
        /// Layout when template is applied
        /// </summary>
        internal bool IsLayoutDelayed
        {
            get;
            private set;
        }

        internal bool IsMouseOver
        {
            get
            {
                return this.OwningGrid != null && this.OwningGrid.MouseOverRowIndex == this.Index;
            }
            set
            {
                if (this.OwningGrid != null && value != this.IsMouseOver)
                {
                    if (value)
                    {
                        this.OwningGrid.MouseOverRowIndex = this.Index;
                    }
                    else
                    {
                        this.OwningGrid.MouseOverRowIndex = null;
                    }
                }
            }
        }

        internal bool IsRecycled
        {
            get;
            private set;
        }

        internal bool IsRecyclable
        {
            get
            {
                if (this.OwningGrid != null)
                {
                    return this.OwningGrid.IsRowRecyclable(this);
                }
                return true;
            }
        }

        internal bool IsSelected
        {
            get
            {
                if (this.OwningGrid == null)
                {
                    return false;
                }
                if (this.Index == -1)
                {
                    throw DataGridError.DataGridRow.InvalidRowIndexCannotCompleteOperation();
                }
                return this.OwningGrid.GetRowSelection(this.Index);
            }
            /* 












*/
        }

        internal int? MouseOverColumnIndex
        {
            get
            {
                return this._mouseOverColumnIndex;
            }
            set
            {
                if (this._mouseOverColumnIndex != value)
                {
                    DataGridCell oldMouseOverCell = null;
                    if (this._mouseOverColumnIndex != null && this.OwningGrid.DisplayData.IsRowDisplayed(this.Index))
                    {
                        if (this._mouseOverColumnIndex > -1)
                        {
                            oldMouseOverCell = this.Cells[(int)this._mouseOverColumnIndex];
                        }
                    }
                    this._mouseOverColumnIndex = value;
                    if (oldMouseOverCell != null)
                    {
                        oldMouseOverCell.ApplyCellState(true /*animate*/);
                    }
                    if (this._mouseOverColumnIndex != null && this.OwningGrid != null && this.OwningGrid.DisplayData.IsRowDisplayed(this.Index))
                    {
                        if (this._mouseOverColumnIndex > -1)
                        {
                            this.Cells[(int)this._mouseOverColumnIndex].ApplyCellState(true /*animate*/);
                        }
                    }
                }
            }
        }

        internal DataGrid OwningGrid
        {
            get;
            set;
        }

        internal Panel RootElement
        {
            get;
            private set;
        }

        // Height that the row will eventually end up at after a possible detalis animation has completed
        internal double TargetHeight
        {
            get
            {
                if (!double.IsNaN(this.Height))
                {
                    return this.Height;
                }
                else if (_detailsElement != null && _appliedDetailsVisibility == Visibility.Visible && _appliedDetailsTemplate != null)
                {
                    Debug.Assert(!double.IsNaN(_detailsElement.ContentHeight));
                    return this.DesiredSize.Height + _detailsDesiredHeight - _detailsElement.ContentHeight;
                }
                else
                {
                    return this.DesiredSize.Height;
                }
            }
        }

        #endregion Internal Properties

        #region Private Properties

        // Returns the actual template that should be sued for Details: either explicity set on this row 
        // or inherited from the DataGrid
        private DataTemplate ActualDetailsTemplate
        {
            get
            {
                Debug.Assert(this.OwningGrid != null);
                DataTemplate currentDetailsTemplate = this.DetailsTemplate;

                return currentDetailsTemplate != null ? currentDetailsTemplate : this.OwningGrid.RowDetailsTemplate;
            }
        }

        private Visibility ActualDetailsVisibility
        {
            get
            {
                if (this.OwningGrid == null)
                {
                    throw DataGridError.DataGrid.NoOwningGrid(this.GetType());
                }
                if (this.Index == -1)
                {
                    throw DataGridError.DataGridRow.InvalidRowIndexCannotCompleteOperation();
                }
                return this.OwningGrid.GetRowDetailsVisibility(this.Index);
            }
        }

        private bool AreDetailsVisible
        {
            get
            {
                return this.DetailsVisibility == Visibility.Visible;
            }
        }
        
        private Storyboard DetailsVisibleStoryboard
        {
            get
            {
                if (this._detailsVisibleStoryboard == null && this.RootElement != null)
                {
                    this._detailsVisibleStoryboard = this.RootElement.Resources[DATAGRIDROW_detailsVisibleTransition] as Storyboard;
                    if (this._detailsVisibleStoryboard != null)
                    {
                        this._detailsVisibleStoryboard.Completed += new EventHandler(DetailsVisibleStoryboard_Completed);
                        if (this._detailsVisibleStoryboard.Children.Count > 0)
                        {
                            // If the user set a To value for the animation, we want to respect
                            this._detailsHeightAnimation = this._detailsVisibleStoryboard.Children[0] as DoubleAnimation;
                            if (this._detailsHeightAnimation != null)
                            {
                                this._detailsHeightAnimationToOverride = this._detailsHeightAnimation.To;
                            }
                        }
                    }
                }
                return this._detailsVisibleStoryboard;
            }
        }

        #endregion Private Properties

        #region Public Methods

        [SuppressMessage("Microsoft.Design", "CA1024:UsePropertiesWhereAppropriate")]
        public int GetIndex()
        {
            return this.Index;
        }

        /// <summary>
        /// Returns the Row which contains the given element
        /// </summary>
        /// <param name="element">element contained in a row</param>
        /// <returns>Row that contains the element
        /// </returns>
        public static DataGridRow GetRowContainingElement(FrameworkElement element)
        {
            // Walk up the tree to find the DataGridRow that contains the element
            DependencyObject parent = element;
            DataGridRow row = parent as DataGridRow;
            while ((parent != null) && (row == null))
            {
                parent = VisualTreeHelper.GetParent(parent);
                row = parent as DataGridRow;
            }
            if (row != null)
            {
                return row;
            }
            else
            {
                throw DataGridError.DataGrid.InvalidRowElement("element");
            }
            
        }

        #endregion Public Methods

        #region Protected Methods

        protected override Size ArrangeOverride(Size finalSize)
        {
            if (this.OwningGrid == null)
            {
                return base.ArrangeOverride(finalSize);
            }

            // If the DataGrid was scrolled horizontally after our last Arrange, we need to make sure
            // the Cells and Details are Arranged again
            if (_lastHorizontalOffset != this.OwningGrid.HorizontalOffset)
            {
                _lastHorizontalOffset = this.OwningGrid.HorizontalOffset;
                InvalidateHorizontalArrange();
            }
        
            Size size = base.ArrangeOverride(finalSize);
            
            if (this.RootElement != null)
            {
                foreach (UIElement child in this.RootElement.Children)
                {
                    if (DataGridFrozenGrid.GetIsFrozen(child))
                    {
                        TranslateTransform transform = new TranslateTransform();
                        // Automatic layout rounding doesn't apply to transforms so we need to Round this
                        transform.X = Math.Round(this.OwningGrid.HorizontalOffset);
                        child.RenderTransform = transform;
                    }
                }
            }

            if (this._bottomGridLine != null)
            {
                RectangleGeometry gridlineClipGeometry = new RectangleGeometry();
                gridlineClipGeometry.Rect = new Rect(this.OwningGrid.HorizontalOffset, 0, Math.Max(0, this.DesiredSize.Width - this.OwningGrid.HorizontalOffset), this._bottomGridLine.DesiredSize.Height);
                this._bottomGridLine.Clip = gridlineClipGeometry;
            }

            return size;
        }

        protected override Size MeasureOverride(Size availableSize)
        {
            if (this.OwningGrid == null)
            {
                return base.MeasureOverride(availableSize);
            }

            // Allow the DataGrid specific componets to adjust themselves based on new values
            if (_headerElement != null)
            {
                _headerElement.InvalidateMeasure();
            }
            if (_cellsElement != null)
            {
                _cellsElement.InvalidateMeasure();
            }
            if (_detailsElement != null)
            {
                _detailsElement.InvalidateMeasure();
            }

            return base.MeasureOverride(availableSize);
        }

        public override void OnApplyTemplate()
        {
            // 
            this.RootElement = GetTemplateChild(DATAGRIDROW_elementRoot) as Panel;
            // 
            if (this.RootElement != null)
            {
                EnsureBackground();
                ApplyState(false /*animate*/);
            }

            bool updateVerticalScrollBar = false;
            if (this._cellsElement != null)
            {
                // If we're applying a new template, we  want to remove the cells from the previous _cellsElement
                this._cellsElement.Children.Clear();
                updateVerticalScrollBar = true;
            }
            
            this._cellsElement = GetTemplateChild(DATAGRIDROW_elementCells) as DataGridCellsPresenter;
            if (this._cellsElement != null)
            {
                this._cellsElement.OwningRow = this;
                // Cells that were already added before the Template was applied need to
                // be added to the Canvas
                if (this.Cells.Count > 0)
                {
                    foreach (DataGridCell cell in this.Cells)
                    {
                        this._cellsElement.Children.Add(cell);
                    }
                }
                // Add the FillerCell
                if (this.OwningGrid != null)
                {
                    EnsureFillerCellRepresentation(this.OwningGrid.ColumnsInternal.FillerColumn.IsRepresented);
                }
            }

            _detailsElement = GetTemplateChild(DATAGRIDROW_elementDetails) as DataGridDetailsPresenter;
            if (_detailsElement != null && this.OwningGrid != null)
            {
                _detailsElement.OwningRow = this;
                _detailsElement.SizeChanged += new SizeChangedEventHandler(DetailsElement_SizeChanged);
                if (this.ActualDetailsVisibility == Visibility.Visible && this.ActualDetailsTemplate != null && this._appliedDetailsTemplate == null)
                {
                    // Apply the DetailsTemplate now that the row template is applied.
                    SetDetailsVisibilityInternal(this.ActualDetailsVisibility, _detailsVisibilityNotificationPending /*raiseNotification*/, false /*animate*/);
                    _detailsVisibilityNotificationPending = false;
                }
            }

            _bottomGridLine = GetTemplateChild(DATAGRIDROW_elementBottomGridLine) as Rectangle;
            EnsureGridLines();

            _headerElement = GetTemplateChild(DATAGRIDROW_elementRowHeader) as DataGridRowHeader;
            if (_headerElement != null)
            {
                _headerElement.OwningRow = this;
                if (this.Header != null)
                {
                    _headerElement.Content = Header;
                }
                EnsureHeaderStyleAndVisibility();
            }

            // The height of this row might have changed after applying a new style, so fix the vertical scroll bar
            if (this.OwningGrid != null && updateVerticalScrollBar)
            {
                this.OwningGrid.UpdateVerticalScrollBar();
            }
        }

        /// <summary>
        /// Creates AutomationPeer (<see cref="UIElement.OnCreateAutomationPeer"/>)
        /// </summary>
        protected override AutomationPeer OnCreateAutomationPeer()
        {
            return new DataGridRowAutomationPeer(this);
        }

        #endregion Protected Methods

        #region Internal Methods

        internal void ApplyCellsState(bool animate)
        {
            foreach (DataGridCell dataGridCell in this.Cells)
            {
                dataGridCell.ApplyCellState(animate);
            }
        }

        internal void ApplyDetailsTemplate(bool initializeDetailsPreferredHeight)
        {
            if (_detailsElement != null && this.AreDetailsVisible)
            {
                if (this.ActualDetailsTemplate != null && this.ActualDetailsTemplate != _appliedDetailsTemplate)
                {
                    if (_detailsContent != null)
                    {
                        _detailsContent.SizeChanged -= new SizeChangedEventHandler(DetailsContent_SizeChanged);
                        if (_detailsLoaded)
                        {
                            this.OwningGrid.OnUnloadingRowDetails(this, _detailsContent);
                            _detailsLoaded = false;
                        }
                    }
                    _detailsElement.Children.Clear();

                    _detailsContent = this.ActualDetailsTemplate.LoadContent() as FrameworkElement;
                    _appliedDetailsTemplate = this.ActualDetailsTemplate;

                    if (_detailsContent != null)
                    {
                        _detailsContent.SizeChanged += new SizeChangedEventHandler(DetailsContent_SizeChanged);
                        _detailsElement.Children.Add(_detailsContent);
                    }
                }

                if (_detailsContent != null && !_detailsLoaded)
                {
                    _detailsLoaded = true;
                    _detailsContent.DataContext = this.DataContext;
                    this.OwningGrid.OnLoadingRowDetails(this, _detailsContent);
                }
                if (initializeDetailsPreferredHeight && double.IsNaN(_detailsDesiredHeight) &&
                    _appliedDetailsTemplate != null && _detailsElement.Children.Count > 0)
                {
                    EnsureDetailsDesiredHeight();
                }
            }
        }

        internal void ApplyHeaderStatus(bool animate)
        {
            if (_headerElement != null && this.OwningGrid.AreRowHeadersVisible)
            {
                _headerElement.ApplyRowStatus(animate);
            }
        }

        /// <summary>
        /// Updates the background brush of the row, using a storyboard if available.
        /// </summary>
        [SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes")]
        internal void ApplyState(bool animate)
        {
            if (this.RootElement != null && this.OwningGrid != null)
            {
                Debug.Assert(this.Index != -1);
                byte idealStateMappingIndex = 0;
                if (this.IsSelected)
                {
                    idealStateMappingIndex += 8;
                }
                if (this.IsEditing)
                {
                    idealStateMappingIndex += 4;
                }
                if (this.IsMouseOver)
                {
                    idealStateMappingIndex += 2;
                }
                if (this.OwningGrid.ContainsFocus)
                {
                    idealStateMappingIndex += 1;
                }

                byte stateCode = _idealStateMapping[idealStateMappingIndex];
                Debug.Assert(stateCode != DATAGRIDROW_stateNullCode);

                string storyboardName;
                while (stateCode != DATAGRIDROW_stateNullCode)
                {
                    if (stateCode == DATAGRIDROW_stateNormalCode)
                    {
                        if (this.Index % 2 == 1)
                        {
                            storyboardName = DATAGRIDROW_stateAlternate;
                        }
                        else
                        {
                            storyboardName = DATAGRIDROW_stateNormal;
                        }
                    }
                    else
                    {
                        storyboardName = _stateNames[stateCode];
                    }
                    if (VisualStateManager.GoToState(this, storyboardName, animate))
                    {
                        break;
                    }
                    else
                    {
                        // The state wasn't implemented so fall back to the next one
                        stateCode = _fallbackStateMapping[stateCode];
                    }
                }
                ApplyHeaderStatus(animate);
            }
        }

        internal void DetachFromDataGrid(bool recycle)
        {
            UnloadDetailsTemplate(recycle);

            if (recycle)
            {
                this.IsRecycled = true;
            }

            // 



        }

        // Make sure the row's background is set to its correct value.  It could be explicity set or inherit
        // DataGrid.RowBackground or DataGrid.AlternatingRowBackground
        internal void EnsureBackground()
        {
            // Inherit the DataGrid's RowBackground properties only if this row doesn't explicity have a background set
            if (RootElement != null && this.OwningGrid != null)
            {
                Debug.Assert(this.Index != -1);

                Brush newBackground = null;
                if (this.Background == null)
                {
                    if (this.Index % 2 == 0 || this.OwningGrid.AlternatingRowBackground == null)
                    {
                        // Use OwningGrid.RowBackground if the index is even or if the OwningGrid.AlternatingRowBackground is null
                        if (this.OwningGrid.RowBackground != null)
                        {
                            newBackground = this.OwningGrid.RowBackground;
                        }
                    }
                    else
                    {
                        // Alternate row
                        if (this.OwningGrid.AlternatingRowBackground != null)
                        {
                            newBackground = this.OwningGrid.AlternatingRowBackground;
                        }
                    }
                }
                else
                {
                    newBackground = this.Background;
                }

                if (RootElement.Background != newBackground)
                {
                    RootElement.Background = newBackground;
                }
            }
        }

        internal void EnsureFillerVisibility()
        {
            if (_cellsElement != null)
            {
                _cellsElement.EnsureFillerVisibility();
            }
        }

        internal void EnsureGridLines()
        {
            if (this.OwningGrid != null)
            {
                if (_bottomGridLine != null)
                {
                    // It looks like setting Visibility sometimes has side effects so make sure the value is actually
                    // diffferent before setting it
                    Visibility newVisibility = this.OwningGrid.GridLinesVisibility == DataGridGridLinesVisibility.Horizontal || this.OwningGrid.GridLinesVisibility == DataGridGridLinesVisibility.All
                        ? Visibility.Visible : Visibility.Collapsed;

                    if (newVisibility != _bottomGridLine.Visibility)
                    {
                        _bottomGridLine.Visibility = newVisibility;
                    }
                    _bottomGridLine.Fill = this.OwningGrid.HorizontalGridLinesBrush;
                }

                foreach (DataGridCell cell in this.Cells)
                {
                    cell.EnsureGridLine(null);
                }
            }
        }

        // Set the proper style for the Header by walking up the Style hierarchy
        internal void EnsureHeaderStyleAndVisibility()
        {
            if (_headerElement != null && this.OwningGrid != null)
            {
                if (this.OwningGrid.AreRowHeadersVisible)
                {
                    if (_headerElement.Style == null)
                    {
                        if (this.HeaderStyle != null)
                        {
                            // Set the Row's HeaderStyle if there is one
                            _headerElement.Style = this.HeaderStyle;
                        }
                        else if (this.OwningGrid.RowHeaderStyle != null)
                        {
                            // Set the DataGrid's RowHeaderSyle if there is one
                            _headerElement.Style = this.OwningGrid.RowHeaderStyle;
                        }
                    }
                    _headerElement.Visibility = Visibility.Visible;
                }
                else
                {
                    _headerElement.Visibility = Visibility.Collapsed;
                }
            }
        }

        internal void EnsureFillerCellRepresentation(bool needFillerCell)
        {
            // The FillerCell is added on the fly when it's used so it doesn't need to be created here
            if (_cellsElement != null && !needFillerCell && _fillerCell != null)
            {
                Debug.Assert(_cellsElement.Children.Contains(_fillerCell));
                _cellsElement.Children.Remove(_fillerCell);
                _fillerCell = null;
            }
        }

        internal void EnsureHeaderVisibility()
        {
            if (_headerElement != null && this.OwningGrid != null)
            {
                _headerElement.Visibility = this.OwningGrid.AreRowHeadersVisible ? Visibility.Visible : Visibility.Collapsed;
            }
        }

        internal void InvalidateHorizontalArrange()
        {
            if (_cellsElement != null)
            {
                _cellsElement.InvalidateArrange();
            }
            if (_detailsElement != null)
            {
                _detailsElement.InvalidateArrange();
            }
        }

        // 
        internal void ReAttachHandlers()
        {
            if (_detailsElement != null)
            {
                
                 

                _detailsElement.SizeChanged -= new SizeChangedEventHandler(DetailsElement_SizeChanged);
                _detailsElement.SizeChanged += new SizeChangedEventHandler(DetailsElement_SizeChanged);
            }
        }

        internal void ResetGridLine()
        {
            this._bottomGridLine = null;
        }

        // Sets AreDetailsVisible on the row and animates if necessary
        internal void SetDetailsVisibilityInternal(Visibility visibility, bool raiseNotification, bool animate)
        {
            Debug.Assert(this.OwningGrid != null);
            Debug.Assert(this.Index != -1);

            if (_appliedDetailsVisibility != visibility)
            {
                if (_detailsElement == null)
                {
                    if (raiseNotification)
                    {
                        _detailsVisibilityNotificationPending = true;
                    }
                    return;
                }

                _appliedDetailsVisibility = visibility;
                this.SetValueNoCallback(DetailsVisibilityProperty, visibility);

                // 





                // Applies a new DetailsTemplate only if it has changed either here or at the DataGrid level
                ApplyDetailsTemplate(true /* initializeDetailsPreferredHeight */);

                // no template to show
                if (_appliedDetailsTemplate == null)
                {
                    if (_detailsElement.ContentHeight > 0)
                    {
                        _detailsElement.ContentHeight = 0;
                    }
                    return;
                }

                if (animate && this.DetailsVisibleStoryboard != null && _detailsHeightAnimation != null)
                {
                    if (this.AreDetailsVisible)
                    {
                        // Expand
                        _detailsHeightAnimation.From = 0.0;
                        _detailsHeightAnimation.To = _detailsHeightAnimationToOverride.HasValue ?
                            _detailsHeightAnimationToOverride.Value :
                            _detailsDesiredHeight;
                    }
                    else
                    {
                        // Collapse
                        _detailsHeightAnimation.From = _detailsElement.ActualHeight;
                        _detailsHeightAnimation.To = 0.0;
                    }
                    this.DetailsVisibleStoryboard.Begin();
                }
                else
                {
                    // Set the details height directly
                    _detailsElement.ContentHeight = this.AreDetailsVisible ? _detailsDesiredHeight : 0;
                }

                if (raiseNotification)
                {
                    this.OwningGrid.OnRowDetailsVisibilityChanged(new DataGridRowDetailsEventArgs(this, _detailsContent));
                }
            }
        }
        

        #endregion Internal Methods

        #region Private Methods

        private void DataGridCellCollection_CellAdded(object sender, DataGridCellEventArgs e)
        {
            if (this._cellsElement != null)
            {
                this._cellsElement.Children.Add(e.Cell);
            }
        }

        private void DataGridCellCollection_CellRemoved(object sender, DataGridCellEventArgs e)
        {
            if (this._cellsElement != null)
            {
                this._cellsElement.Children.Remove(e.Cell);
            }
        }

        private void DataGridRow_MouseEnter(object sender, MouseEventArgs e)
        {
            this.IsMouseOver = true;
        }

        private void DataGridRow_MouseLeave(object sender, MouseEventArgs e)
        {
            this.IsMouseOver = false;
        }

        private void DataGridRow_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (!e.Handled && this.OwningGrid != null)
            {
                e.Handled = this.OwningGrid.UpdateStateOnMouseLeftButtonDown(e, -1, this.Index);
            }
        }

        private void DetailsContent_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (e.NewSize.Height != e.PreviousSize.Height && e.NewSize.Height != _detailsDesiredHeight)
            {
                // Update the new desired height for RowDetails
                _detailsDesiredHeight = e.NewSize.Height;

                if (this.AreDetailsVisible && _appliedDetailsTemplate != null)
                {
                    if (this.DetailsVisibleStoryboard != null)
                    {
                        // 




                    }
                    _detailsElement.ContentHeight = e.NewSize.Height;
                    // Calling this when details are not visible invalidates during layout when we have no work 
                    // to do.  In certain scenarios, this could cause a layout cycle
                    OnRowDetailsChanged();
                }
            }
        }

        private void DetailsElement_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (this.AreDetailsVisible && _appliedDetailsTemplate != null)
            {
                OnRowDetailsChanged();
            }
        }

        private void DetailsVisibleStoryboard_Completed(object sender, EventArgs e)
        {
            if (this.AreDetailsVisible && this.OwningGrid != null && this.OwningGrid.DisplayData.IsRowDisplayed(this.Index))
            {
                Debug.Assert(!double.IsNaN(_detailsDesiredHeight));
                // The height of the DetailsContents may have changed while we were animating its height
                _detailsElement.ContentHeight = _detailsDesiredHeight;
            }
        }

        // Makes sure the _detailsDesiredHeight is initialized.  We need to measure it to know what
        // height we want to animate to.  Subsequently, we just update that height in response to SizeChanged
        private void EnsureDetailsDesiredHeight()
        {
            Debug.Assert(_detailsElement != null && this.OwningGrid != null);
            
            if (_detailsContent != null)
            {
                Debug.Assert(_detailsElement.Children.Contains(_detailsContent));

                _detailsContent.Measure(new Size(double.PositiveInfinity, double.PositiveInfinity));
                _detailsDesiredHeight = _detailsContent.DesiredSize.Height;
            }
            else
            {
                _detailsDesiredHeight = 0;
            }
        }

        private void UnloadDetailsTemplate(bool recycle)
        {
            if (_detailsElement != null)
            {
                if (_detailsContent != null)
                {
                    if (_detailsLoaded)
                    {
                        this.OwningGrid.OnUnloadingRowDetails(this, _detailsContent);
                    }
                    _detailsContent.DataContext = null;
                    if (!recycle)
                    {
                        _detailsContent.SizeChanged -= new SizeChangedEventHandler(DetailsContent_SizeChanged);
                        _detailsContent = null;
                    }
                }

                if (!recycle)
                {
                    _detailsElement.Children.Clear();
                }
                _detailsElement.ContentHeight = 0;
            }
            if (!recycle)
            {
                _appliedDetailsTemplate = null;
                this.SetValueNoCallback(DetailsTemplateProperty, null);
            }

            _detailsLoaded = false;
            _appliedDetailsVisibility = null;
            this.SetValueNoCallback(DetailsVisibilityProperty, Visibility.Collapsed);
        }

        private void OnRowDetailsChanged()
        {
            if (this.OwningGrid != null)
            {
                this.OwningGrid.OnRowDetailsChanged();
            }
        }

        #endregion Private Methods

        #region Debugging Members

#if DEBUG
        [SuppressMessage("Microsoft.Naming", "CA1707:IdentifiersShouldNotContainUnderscores", Justification="This is a debug method.")]
        public int Debug_Index
        {
            get
            {
                return this.Index;
            }
        }
#endif

        #endregion Debugging Members
    }
}
