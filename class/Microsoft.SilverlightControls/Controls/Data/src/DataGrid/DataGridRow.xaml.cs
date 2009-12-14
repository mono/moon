// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.ComponentModel;
using System.Diagnostics; 
using System.Diagnostics.CodeAnalysis; 
using System.Windows.Input;
using System.Windows.Media; 
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Controls;
 
namespace System.Windows.Controlsb1
{
    [TemplatePart(Name = DATAGRIDROW_elementCells, Type = typeof(Canvas))] 
    [TemplatePart(Name = DATAGRIDROW_elementDetails, Type = typeof(Canvas))] 
    [TemplatePart(Name = DATAGRIDROW_elementRoot, Type = typeof(Panel))]
 
    [TemplatePart(Name = DATAGRIDROW_stateNormal, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDROW_stateAlternate, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDROW_stateDetailsVisible, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DATAGRIDROW_stateNormalEditing, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDROW_stateNormalEditingFocused, Type = typeof(Storyboard))]
 
    [TemplatePart(Name = DATAGRIDROW_stateSelected, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DATAGRIDROW_stateSelectedFocused, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDROW_stateMouseOver, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DATAGRIDROW_stateMouseOverEditing, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDROW_stateMouseOverEditingFocused, Type = typeof(Storyboard))]
 
    [TemplatePart(Name = DATAGRIDROW_stateMouseOverSelected, Type = typeof(Storyboard))]
    [TemplatePart(Name = DATAGRIDROW_stateMouseOverSelectedFocused, Type = typeof(Storyboard))]
    public class DataGridRow : Control 
    { 
        #region Constants
 
        private const byte DATAGRIDROW_defaultMinHeight = 0;
        internal const int DATAGRIDROW_maximumHeight = 65536;
        internal const byte DATAGRIDROW_minMinHeight = 2; 

        private const string DATAGRIDROW_elementCells = "CellsPresenterElement";
        private const string DATAGRIDROW_elementDetails = "DetailsPresenterElement"; 
        private const string DATAGRIDROW_elementRoot = "RootElement"; 

        private const string DATAGRIDROW_stateAlternate = "Normal AlternatingRow State"; 
        private const string DATAGRIDROW_stateDetailsVisible = "Normal DetailsVisible State";
        private const string DATAGRIDROW_stateMouseOver = "MouseOver State";
        private const string DATAGRIDROW_stateMouseOverEditing = "MouseOver Unfocused Editing State"; 
        private const string DATAGRIDROW_stateMouseOverEditingFocused = "MouseOver Editing State";
        private const string DATAGRIDROW_stateMouseOverSelected = "MouseOver Unfocused Selected State";
        private const string DATAGRIDROW_stateMouseOverSelectedFocused = "MouseOver Selected State"; 
        private const string DATAGRIDROW_stateNormal = "Normal State"; 
        private const string DATAGRIDROW_stateNormalEditing = "Unfocused Editing State";
        private const string DATAGRIDROW_stateNormalEditingFocused = "Normal Editing State"; 
        private const string DATAGRIDROW_stateSelected = "Unfocused Selected State";
        private const string DATAGRIDROW_stateSelectedFocused = "Normal Selected State";
 
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
 
        private const byte DATAGRIDROW_stateCount = 10; 

        #endregion Constants 

        #region Data
 
        // Track when we're applying the template because the DataGrid's children are locked at that time
        private bool _applyingTemplate;
        // Locally cache whether or not details are visible so we don't run redundant storyboards 
        private bool _areDetailsVisible; 
        // The Details Template that is actually applied to the Row
        private DataTemplate _appliedDetailsTemplate; 
        private Line _bottomGridline;
        private double _cellHeight;
        private Canvas _cellsElement; 
        // Optimal height of the details based on the Element created by the DataTemplate
        private double _detailsDesiredHeight;
        // 
        private Grid _detailsGrid; 
        private Storyboard _detailsVisibleStoryboard;
        private DoubleAnimation _detailsHeightAnimation; 
        private double? _detailsHeightAnimationToOverride;
        private Canvas _detailsElement;
        private DataGridCell _fillerCell; 
        private DataGridRowHeader _headerCell;
        private byte _layoutSuspended;
        private double _minHeight; 
        private int? _mouseOverColumnIndex;    // 
        private Duration?[] _storyboardDuration; //
 
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

        private delegate void VerticalGridlineLayoutDelegate(double leftEdge, DataGridColumnBase dataGridColumn, DataGridCell dataGridCell, bool isCellFrozen); 
 
        //
        [SuppressMessage("Microsoft.Performance", "CA1805:DoNotInitializeUnnecessarily")] 
        [SuppressMessage("Microsoft.Usage", "CA2214:DoNotCallOverridableMethodsInConstructors")]
        public DataGridRow()
        { 
            this.MinHeight = DATAGRIDROW_defaultMinHeight;

            this.Index = -1; 
            this._areDetailsVisible = false; 
            this._storyboardDuration = new Duration?[DATAGRIDROW_stateCount];
            this._mouseOverColumnIndex = null; 
            this._detailsDesiredHeight = double.NaN;
            this._fillerCell = new DataGridCell();
            this._fillerCell.OwningRow = this; 
            this._fillerCell.Visibility = Visibility.Collapsed;
            this.Cells = new DataGridCellCollection(this);
            this.Cells.CellAdded += new EventHandler<DataGridCellEventArgs>(DataGridCellCollection_CellAdded); 
            this.Cells.CellRemoved += new EventHandler<DataGridCellEventArgs>(DataGridCellCollection_CellRemoved); 

            this.MouseLeftButtonDown += new MouseButtonEventHandler(DataGridRow_MouseLeftButtonDown); 
            this.MouseEnter += new MouseEventHandler(DataGridRow_MouseEnter);
            this.MouseLeave += new MouseEventHandler(DataGridRow_MouseLeave);
        } 

        #region Dependency Properties
 
        // 
        #region Background
        /// <summary> 
        /// Row's Background Brush
        /// </summary>
        public Brush Background 
        {
            get { return GetValue(BackgroundProperty) as Brush; }
            set { SetValue(BackgroundProperty, value); } 
        } 

        public static readonly DependencyProperty BackgroundProperty = 
            DependencyProperty.Register(
                "Background",
                typeof(Brush), 
                typeof(DataGridRow),
                new PropertyMetadata(OnBackgroundPropertyChanged));
 
        private static void OnBackgroundPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            DataGridRow source = d as DataGridRow; 
            if (source != null && source.RootElement != null && e.OldValue != e.NewValue)
            {
                source.RootElement.Background = e.NewValue as Brush; 
            }
        }
        #endregion Background 
 
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
 
        #region Header
        /// <summary>
        /// 
        /// </summary> 
        public object Header
        { 
            get
            {
                object value = GetValue(HeaderProperty) as object; 
                Debug.Assert(this.HeaderCell.Content == value, "DataGridRowHeader.Content desynchronized from DataGridRow");

                return value; 
            } 
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

            if (source.HeaderCell != null)
            { 
                source.HeaderCell.Content = e.NewValue; 
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
                if (row != null && row.HeaderCell != null && row.HeaderCell.Style != newStyle)
                { 
                    row.HeaderCell.Style = newStyle;
                }
            } 
        }
        #endregion HeaderStyle
 
        #endregion 

        #region Public Properties 

        /*
 


 
 

 


 


 
 

*/ 

        public Visibility DetailsVisibility
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
            set 
            {
                if (this.OwningGrid == null) 
                {
                    throw DataGridError.DataGrid.NoOwningGrid(this.GetType());
                } 
                if (this.Index == -1)
                {
                    throw DataGridError.DataGridRow.InvalidRowIndexCannotCompleteOperation(); 
                } 

                Visibility oldValue = this.OwningGrid.GetRowDetailsVisibility(this.Index); 
                this.OwningGrid.SetRowDetailsVisibility(this.Index, value);

                if (oldValue != value) 
                {
                    SetAreRowDetailsVisibleInternal(value == Visibility.Visible, true /* animate */);
                } 
            } 
        }
 
        // Height includes Cells only and does not include Details
        [SuppressMessage("Microsoft.Usage", "CA2208:InstantiateArgumentExceptionsCorrectly", Justification = "This parameter is exposed to the user as 'Height' property.")]
        public new double Height 
        {
            get
            { 
                return this.ActualCellHeight; 
            }
            set 
            {
                double minHeight = this.InheritedMinHeight;
                if (value < minHeight && value != 0) 
                {
                    value = minHeight;
                } 
                if (value > DATAGRIDROW_maximumHeight) 
                {
                    throw DataGridError.DataGrid.ValueMustBeLessThanOrEqualTo("value", "Height", DATAGRIDROW_maximumHeight); 
                }
                if (_cellHeight != value)
                { 
                    if (this.ActualCellHeight != value && this.OwningGrid != null)
                    {
                        // 
                        PerformLayout(false /*onlyLayoutGridlines*/); 
                    }
                    _cellHeight = value; 
                }
            }
        } 


        /// <summary> 
        /// Index of the row 
        /// </summary>
        public int Index 
        {
            get;
            internal set; 
        }

        [SuppressMessage("Microsoft.Usage", "CA2208:InstantiateArgumentExceptionsCorrectly", Justification = "This parameter is exposed to the user as 'MinHeight' property.")] 
        public new double MinHeight 
        {
            get 
            {
                return this._minHeight;
            } 
            set
            {
                if (this._minHeight != value) 
                { 
                    if (value < DATAGRIDROW_minMinHeight && value != 0)
                    { 
                        throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "MinHeight", DATAGRIDROW_minMinHeight);
                    }
 
                    double newEffectiveMinHeight;
                    if (value == 0)
                    { 
                        if (this.OwningGrid == null) 
                        {
                            newEffectiveMinHeight = DataGridRow.DATAGRIDROW_minMinHeight; 
                        }
                        else
                        { 
                            newEffectiveMinHeight = this.OwningGrid.MinRowHeight;
                        }
                    } 
                    else 
                    {
                        newEffectiveMinHeight = value; 
                    }
                    if (this.Height < newEffectiveMinHeight)
                    { 
                        this.Height = newEffectiveMinHeight;
                    }
                    this._minHeight = value; 
                    // 

 


                } 
            }
        }
 
        #endregion Public Properties 

        #region Protected Properties 
        #endregion Protected Properties

        #region Internal Properties 

        // This could be the value set by the user or Inherited from the Grid
        internal double ActualCellHeight 
        { 
            get
            { 
                if (_cellHeight == 0 && this.OwningGrid != null)
                {
                    return this.OwningGrid.RowHeight; 
                }
                else
                { 
                    return _cellHeight; 
                }
            } 
        }

        internal Line BottomGridline 
        {
            get
            { 
                if (this._bottomGridline == null) 
                {
                    EnsureGridline(); 
                }
                return this._bottomGridline;
            } 
            set
            {
                this._bottomGridline = value; 
            } 
        }
 
        internal DataGridCellCollection Cells
        {
            get; 
            private set;
        }
 
        // Width of the cells not including the FillerCell 
        internal double CellsWidth
        { 
            get;
            set;
        } 

        internal double DetailsHeight
        { 
            get 
            {
                if (_detailsElement != null && !Double.IsNaN(_detailsElement.Height)) // 
                {
                    return _detailsElement.Height;
                } 
                return 0;
            }
            // 
 

 


 


        } 
 
        internal double DisplayHeight
        { 
            get
            {
                return this.ActualCellHeight + this.DetailsHeight; 
            }
        }
 
        internal bool HasBottomGridline 
        {
            get 
            {
                return this._bottomGridline != null;
            } 
        }

        internal bool HasHeaderCell 
        { 
            get
            { 
                return this._headerCell != null;
            }
        } 

        internal DataGridRowHeader HeaderCell
        { 
            get 
            {
                if (_headerCell == null) 
                {
                    _headerCell = new DataGridRowHeader();
                    _headerCell.OwningRow = this; 
                }
                return _headerCell;
            } 
        } 

        internal double InheritedMinHeight 
        {
            get
            { 
                if (this._minHeight == 0 && this.OwningGrid != null)
                {
                    return this.OwningGrid.MinRowHeight; 
                } 
                return this._minHeight;
            } 
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
                Debug.Assert(this.OwningGrid != null);
                if (value != this.IsMouseOver)
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
                    if (this._mouseOverColumnIndex != null && this.OwningGrid.IsRowDisplayed(this.Index)) 
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
                    if (this._mouseOverColumnIndex != null && this.OwningGrid.IsRowDisplayed(this.Index)) 
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
 
        private Storyboard DetailsVisibleStoryboard 
        {
            get 
            {
                if (this._detailsVisibleStoryboard == null && this.RootElement != null)
                { 
                    this._detailsVisibleStoryboard = this.RootElement.FindName(DATAGRIDROW_stateDetailsVisible) as Storyboard;
                    if (this._detailsVisibleStoryboard != null &&
                        this._detailsVisibleStoryboard.Children.Count > 0) 
                    { 
                        // If the user set a To value for the animation, we want to respect
                        this._detailsHeightAnimation = this._detailsVisibleStoryboard.Children[0] as DoubleAnimation; 
                        if (this._detailsHeightAnimation != null)
                        {
                            this._detailsHeightAnimationToOverride = this._detailsHeightAnimation.To; 
                        }
                    }
                } 
                return this._detailsVisibleStoryboard; 
            }
        } 

        #endregion Private Properties
 
        #region Public Methods
        #endregion Public Methods
 
        #region Protected Methods 

        public override void OnApplyTemplate() 
        {
            //
            Debug.Assert(!this._applyingTemplate); 
            this._applyingTemplate = true;
            try
            { 
                this.RootElement = GetTemplateChild(DATAGRIDROW_elementRoot) as Panel; 
                //
                if (this.RootElement != null) 
                {
                    EnsureBackground();
                    ApplyBackgroundBrush(false /*animate*/); 
                }

                this._cellsElement = GetTemplateChild(DATAGRIDROW_elementCells) as Canvas; 
                if (this._cellsElement != null) 
                {
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
                    if (!this._cellsElement.Children.Contains(_fillerCell))
                    {
                        this._cellsElement.Children.Add(_fillerCell); 
                    }
                }
 
                _detailsElement = GetTemplateChild(DATAGRIDROW_elementDetails) as Canvas; 
                if (_detailsElement != null)
                { 
                    //
                    _detailsGrid = new Grid();
                    RowDefinition rowDefinition = new RowDefinition(); 
                    rowDefinition.Height = GridLength.Auto;
                    _detailsGrid.RowDefinitions.Add(rowDefinition);
                    _detailsElement.Children.Add(_detailsGrid); 
 
                    _detailsElement.SizeChanged += new SizeChangedEventHandler(DetailsElement_SizeChanged);
                    if (this.DetailsVisibility == Visibility.Visible && this.ActualDetailsTemplate != null && this._appliedDetailsTemplate == null) 
                    {
                        // Apply the DetailsTemplate now that the row template is applied.
                        SetAreRowDetailsVisibleInternal(true, false /* animate */); 
                    }
                }
 
                // Right before the row is displayed, we might need to apply the Row or DataGrid's RowHeaderStyle if 
                // the header doesn't already have a Style set
                EnsureHeaderStyle(); 

                if (this.IsLayoutDelayed)
                { 
                    this.IsLayoutDelayed = false;
                    PerformLayout(false /*onlyLayoutGridlines*/);
                    if (this.Index == this.OwningGrid.CurrentRowIndex) 
                    { 
                        this.OwningGrid.PerformCurrentCellFocusVisualLayout();
                    } 
                }
            }
            finally 
            {
                this._applyingTemplate = false;
            } 
        } 

        #endregion Protected Methods 

        #region Internal Methods
 
        /// <summary>
        /// Updates the background brush of the row, using a storyboard if available.
        /// </summary> 
        [SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes")] 
        internal void ApplyBackgroundBrush(bool animate)
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
 
                Storyboard storyboard; 
                do
                { 
                    string storyboardName;
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
                    storyboard = this.RootElement.FindName(storyboardName) as Storyboard;
                    if (storyboard == null) 
                    {
                        stateCode = _fallbackStateMapping[stateCode];
                    } 
                }
                while (storyboard == null && stateCode != DATAGRIDROW_stateNullCode);
 
                if (storyboard != null) 
                {
                    // 
                    if (!animate)
                    {
                        this._storyboardDuration[stateCode] = DataGrid.ResetStoryboardDuration(storyboard); 
                    }
                    else if (this._storyboardDuration[stateCode].HasValue)
                    { 
                        DataGrid.RestoreStoryboardDuration(storyboard, this._storyboardDuration[stateCode].Value); 
                    }
                    // 


                    try 
                    {
                        storyboard.Begin();
                    } 
                    catch (Exception) 
                    {
                    } 
                }

                if ((this.OwningGrid.HeadersVisibility & DataGridHeaders.Row) == DataGridHeaders.Row) 
                {
                    this.HeaderCell.ApplyRowStatus(animate);
                } 
            } 
        }
 
        internal void ApplyCellsState(bool animate)
        {
            foreach (DataGridCell dataGridCell in this.Cells) 
            {
                dataGridCell.ApplyCellState(animate);
            } 
        } 

        internal void ApplyDetailsTemplate(bool initializeDetailsPreferredHeight) 
        {
            if (_detailsElement != null && this.ActualDetailsTemplate != null && this.ActualDetailsTemplate != _appliedDetailsTemplate)
            { 
                if (_detailsGrid.Children.Count > 0)
                {
                    // Should only be 1 unless the user adds something by walking up the tree 
                    FrameworkElement detailsContent = _detailsGrid.Children[0] as FrameworkElement; 
                    if (detailsContent != null)
                    { 
                        // Detach the SizeChanged EventHandler we attached
                        detailsContent.SizeChanged -= new SizeChangedEventHandler(DetailsContent_SizeChanged);
                        this.OwningGrid.OnCleaningRowDetails(detailsContent); 
                        _detailsGrid.Children.Clear();
                    }
                } 
 
                FrameworkElement newDetailsContent = this.ActualDetailsTemplate.LoadContent() as FrameworkElement;
                _appliedDetailsTemplate = this.ActualDetailsTemplate; 

                if (newDetailsContent != null)
                { 
                    newDetailsContent.DataContext = this.DataContext;
                    _detailsGrid.Children.Add(newDetailsContent);
                    this.OwningGrid.OnPreparingRowDetails(newDetailsContent); 
 
                    if (initializeDetailsPreferredHeight && double.IsNaN(_detailsDesiredHeight))
                    { 
                        EnsureDetailsDesiredHeight(newDetailsContent);
                    }
 
                    // Attach a SizeChanged even to the Details will AutoSize
                    newDetailsContent.SizeChanged += new SizeChangedEventHandler(DetailsContent_SizeChanged);
                } 
            } 
        }
 
        // Make sure the row's background is set to its correct value.  It could be explicity set or inherit
        // DataGrid.RowBackground or DataGrid.AlternatingRowBackground
        internal void EnsureBackground() 
        {
            Debug.Assert(this.Index != -1);
            Debug.Assert(this.OwningGrid != null); 
 
            // Inherit the DataGrid's RowBackground properties only if this row doesn't explicity have a background set
            if (RootElement != null) 
            {
                Brush newBackground = null;
                if (this.Background == null) 
                {
                    if (this.OwningGrid != null)
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
 
        internal void EnsureGridline()
        {
            if (this._bottomGridline != null) 
            {
                return;
            } 
 
            Debug.Assert(this.OwningGrid != null);
            Debug.Assert(this.OwningGrid.GridlinesVisibility == DataGridGridlines.Horizontal || this.OwningGrid.GridlinesVisibility == DataGridGridlines.All); 
            Debug.Assert(this.OwningGrid.HorizontalGridlinesBrush != null);
            //
            this._bottomGridline = new Line(); 
            this._bottomGridline.Stroke = this.OwningGrid.HorizontalGridlinesBrush;
            this._bottomGridline.StrokeThickness = DataGrid.HorizontalGridlinesThickness;
            this._bottomGridline.SetValue(System.Windows.Controls.Canvas.ZIndexProperty, 2); 
            this.OwningGrid.AddDisplayedHorizontalGridline(this._bottomGridline); 
        }
 
        internal void PerformLayout(bool onlyLayoutGridlines)
        {
            if (_layoutSuspended > 0) 
            {
                return;
            } 
 
            if (this.OwningGrid != null)
            { 
                if (this.OwningGrid.DisplayData.Dirty)
                {
                    return; 
                }

                if (this.RootElement == null) 
                { 
                    // The template has not been loaded so we can't layout at this time.
                    // Delay the layout till when the template is applied 
                    this.IsLayoutDelayed = true;
                    return;
                } 
                else
                {
                    this.Width = this.CellsWidth + this.OwningGrid.ColumnsInternal.FillerColumn.Width; 
                    // If we skip RootLayout on a recycled row, we see scrolling artifacts 
                    if (!onlyLayoutGridlines || this.IsRecycled && this.RootElement.Width != this.Width)
                    { 
                        PerformRootLayout();
                    }
                    if (!onlyLayoutGridlines) 
                    {
                        PerformRowDetailsLayout();
                    } 
                    if (this._cellsElement != null) 
                    {
                        if (!onlyLayoutGridlines) 
                        {
                            this._cellsElement.Height = this.ActualCellHeight;
                            this._cellsElement.Width = this.Width; 
                            RectangleGeometry rg = new RectangleGeometry();
                            rg.Rect = new Rect(0, 0, this._cellsElement.Width, this._cellsElement.Height);
                            this._cellsElement.Clip = rg; 
 
                            PerformFillerCellLayout();
                        } 
                        double leftEdge = 0;
                        bool inFrozenZone = true;
                        DataGridColumnBase dataGridColumn = this.OwningGrid.ColumnsInternal.FirstColumn; 
                        Debug.Assert(dataGridColumn == null || dataGridColumn.DisplayIndex == 0);
                        while (dataGridColumn != null)
                        { 
                            DataGridCell cell = this.Cells[dataGridColumn.Index]; 
                            Debug.Assert(cell.OwningColumn == dataGridColumn);
                            if (dataGridColumn.Visibility == Visibility.Visible) 
                            {
                                if (dataGridColumn.IsFrozen)
                                { 
                                    Debug.Assert(inFrozenZone);
                                    if (!onlyLayoutGridlines)
                                    { 
                                        PerformCellLayout(leftEdge, dataGridColumn, cell, true /*isCellFrozen*/); 
                                    }
                                    PerformVerticalGridlineLayout(leftEdge + dataGridColumn.Width, dataGridColumn, cell, true /*isCellFrozen*/); 
                                    leftEdge += this.OwningGrid.GetEdgedColumnWidth(dataGridColumn);
                                }
                                else 
                                {
                                    if (inFrozenZone)
                                    { 
                                        leftEdge -= this.OwningGrid.FirstDisplayedScrollingColumnHiddenWidth; 
                                        inFrozenZone = false;
                                    } 
                                    if (this.OwningGrid.DisplayData.FirstDisplayedScrollingCol != -1 &&
                                        (this.OwningGrid.DisplayData.FirstDisplayedScrollingCol == dataGridColumn.Index ||
                                        this.OwningGrid.ColumnsInternal.DisplayInOrder(this.OwningGrid.DisplayData.FirstDisplayedScrollingCol, dataGridColumn.Index))) 
                                    {
                                        if (!onlyLayoutGridlines)
                                        { 
                                            PerformCellLayout(leftEdge, dataGridColumn, cell, false /*isCellFrozen*/); 
                                        }
                                        PerformVerticalGridlineLayout(leftEdge + dataGridColumn.Width, dataGridColumn, cell, false /*isCellFrozen*/); 
                                        leftEdge += this.OwningGrid.GetEdgedColumnWidth(dataGridColumn);
                                    }
                                    else 
                                    {
                                        cell.Visibility = Visibility.Collapsed;
                                        if (cell.HasRightGridline) 
                                        { 
                                            cell.RightGridline.Visibility = Visibility.Collapsed;
                                        } 
                                    }
                                }
                            } 
                            else
                            {
                                cell.Visibility = Visibility.Collapsed; 
                                if (cell.HasRightGridline) 
                                {
                                    cell.RightGridline.Visibility = Visibility.Collapsed; 
                                }
                            }
                            dataGridColumn = this.OwningGrid.ColumnsInternal.GetNextColumn(dataGridColumn); 
                        }
                    }
                } 
 
                Debug.Assert(Math.Abs(this.Width - this.OwningGrid.ColumnsInternal.GetVisibleEdgedColumnsWidth(true /*includeLastRightGridlineWhenPresent*/) - this.OwningGrid.ColumnsInternal.FillerColumn.Width + this.OwningGrid.HorizontalOffset) < 1);
            } 
        }

        // 
        internal void ReAttachHandlers()
        {
            if (_detailsElement != null) 
            { 
                // Temporary workaround.
                // 

                _detailsElement.SizeChanged -= new SizeChangedEventHandler(DetailsElement_SizeChanged);
                _detailsElement.SizeChanged += new SizeChangedEventHandler(DetailsElement_SizeChanged); 
            }
        }
 
        internal void Recycle() 
        {
            Debug.Assert(this.OwningGrid != null); 

            this.IsRecycled = true;
            RemoveDetailsTemplate(); 

            // Trash the HeaderCell if there's a unique Style applied to this row
            if (this.HeaderCell.Style != null && this.HeaderCell.Style != this.OwningGrid.RowHeaderStyle) 
            { 
                _headerCell = null;
            } 
        }

        internal void ResetGridline() 
        {
            this._bottomGridline = null;
        } 
 
        internal void ResumeLayout(bool updateLayout)
        { 
            Debug.Assert(this._layoutSuspended > 0);
            this._layoutSuspended--;
            if (this._layoutSuspended == 0 && updateLayout) 
            {
                PerformLayout(false /*onlyLayoutGridlines*/);
            } 
        } 

        // Sets AreDetailsVisible on the row and animates if necessary 
        internal void SetAreRowDetailsVisibleInternal(bool areVisible, bool animate)
        {
            Debug.Assert(this.OwningGrid != null); 
            Debug.Assert(this.Index != -1);

            if (areVisible != _areDetailsVisible) 
            { 
                if (_detailsElement != null)
                { 
                    _areDetailsVisible = areVisible;
                    if (this.DetailsVisibleStoryboard != null)
                    { 
                        this.DetailsVisibleStoryboard.Stop();
                    }
 
                    // Applies a new DetailsTemplate only if it has changed either here or at the DataGrid level 
                    ApplyDetailsTemplate(true /* initializeDetailsPreferredHeight */);
 
                    if (animate && this.DetailsVisibleStoryboard != null && _detailsHeightAnimation != null)
                    {
                        if (areVisible) 
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
                        _detailsElement.Height = areVisible ? _detailsDesiredHeight : 0;
                        PerformLayout(false);
                        OnRowDetailsChanged(); 
                    } 
                }
            } 
        }

        internal void SuspendLayout() 
        {
            this._layoutSuspended++;
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
            if (!e.Handled) 
            {
                this.IsMouseOver = true;
            } 
        } 

        private void DataGridRow_MouseLeave(object sender, MouseEventArgs e) 
        {
            if (!e.Handled)
            { 
                this.IsMouseOver = false;
            }
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
            double oldDesiredHeight = _detailsDesiredHeight; 
            _detailsDesiredHeight = e.NewSize.Height;

            if (oldDesiredHeight != _detailsDesiredHeight && this.DetailsVisibility == Visibility.Visible) 
            {
                _detailsElement.Height = _detailsDesiredHeight;
                PerformLayout(false); 
                this.OwningGrid.OnRowDetailsChanged(); 
            }
        } 

        private void DetailsElement_SizeChanged(object sender, SizeChangedEventArgs e)
        { 
            if (e.NewSize.Height != e.PreviousSize.Height)
            {
                OnRowDetailsChanged(); 
            } 
        }
 
        // Gets that height that the DetailsContent would like to be.  We need to measure it to know what
        // height we want to animate to.  Subsequently, we just update that height in response to SizeChanged
        private void EnsureDetailsDesiredHeight(FrameworkElement detailsContent) 
        {
            Debug.Assert(_detailsElement != null && detailsContent != null && _detailsGrid.Children.Contains(detailsContent));
            Debug.Assert(this.OwningGrid != null); 
 
            if (detailsContent != null)
            { 
                double contentWidth = this.OwningGrid.OverrideRowDetailsScrolling ?
                    this.OwningGrid.CellsWidth :
                    this.CellsWidth + this.OwningGrid.ColumnsInternal.FillerColumn.Width; 

                detailsContent.Measure(new Size(contentWidth, double.PositiveInfinity));
                _detailsDesiredHeight = detailsContent.DesiredSize.Height; 
            } 
            else
            { 
                _detailsDesiredHeight = 0;
            }
        } 

        // Set the proper style for the Header by walking up the Style hierarchy
        private void EnsureHeaderStyle() 
        { 
            if (this.HeaderCell.Style == null)
            { 
                if (this.HeaderStyle != null)
                {
                    // Set the Row's HeaderStyle if there is one 
                    this.HeaderCell.Style = this.HeaderStyle;
                }
                else if (this.OwningGrid != null && this.OwningGrid.RowHeaderStyle != null) 
                { 
                    // Set the DataGrid's RowHeaderSyle if there is one
                    this.HeaderCell.Style = this.OwningGrid.RowHeaderStyle; 
                }
            }
        } 

        private void RemoveDetailsTemplate()
        { 
            if (_detailsElement != null && _detailsGrid.Children.Count > 0) 
            {
                // Should only ever be 1 but just in case 
                FrameworkElement detailsContent = _detailsGrid.Children[0] as FrameworkElement;
                if (detailsContent != null)
                { 
                    // Detach the SizeChanged EventHandler we attached
                    detailsContent.SizeChanged -= new SizeChangedEventHandler(DetailsContent_SizeChanged);
                    this.OwningGrid.OnCleaningRowDetails(detailsContent); 
                } 

                _detailsGrid.Children.Clear(); 
                _detailsElement.Height = 0;

                this.SetValueNoCallback(DetailsTemplateProperty, null); 
            }
            _appliedDetailsTemplate = null;
            _detailsDesiredHeight = double.NaN; 
            _areDetailsVisible = false; 
        }
 
        private void OnRowDetailsChanged()
        {
            if (RootElement != null && _detailsElement != null && _appliedDetailsTemplate != null) 
            {
                if (_applyingTemplate)
                { 
                    // If we're applying the template, we need to delay Details expanding because that could cause a layout 
                    // which would add or remove rows from the DataGrid, but the DataGrid's children are locked at that time
 
                    // The Designer doesn't support BeginInvoke so don't call it if we're in DesignMode
                    if (Application.Current != null && Application.Current.RootVisual != null &&
                        !DesignerProperties.GetIsInDesignMode(Application.Current.RootVisual)) 
                    {
                        this.Dispatcher.BeginInvoke(new Action(OnRowDetailsChanged), null);
                    } 
                    return; 
                }
 
                PerformRootLayout();
                PerformRowDetailsLayout();
 
                // This will not re-layout any of the cells because there haven't changed
                // the width of our rows; We need this to do the vertical layout of the Rows
                this.OwningGrid.OnRowDetailsChanged(); 
            } 
        }
 
        private void PerformCellLayout(double leftEdge, DataGridColumnBase dataGridColumn, DataGridCell cell, bool isCellFrozen)
        {
            Debug.Assert(cell != null); 
            Debug.Assert(this._cellsElement != null);
            Debug.Assert(this.OwningGrid != null);
 
            if (leftEdge < this.OwningGrid.CellsWidth) 
            {
                if (isCellFrozen) 
                {
                    cell.Visibility = Visibility.Visible;
                    cell.SetValue(Canvas.LeftProperty, leftEdge); 
                    cell.SetValue(Canvas.ZIndexProperty, 1);
                    cell.Width = dataGridColumn.Width;
                    cell.Height = this._cellsElement.Height; 
                    RectangleGeometry rg = new RectangleGeometry(); 
                    rg.Rect = new Rect(0, 0, cell.Width, cell.Height);
                    cell.Clip = rg; 
                }
                else
                { 
                    if (this.OwningGrid.DisplayData.FirstDisplayedScrollingCol == dataGridColumn.Index &&
                        this.OwningGrid.FirstDisplayedScrollingColumnHiddenWidth > 0 &&
                        dataGridColumn.Width <= this.OwningGrid.FirstDisplayedScrollingColumnHiddenWidth) 
                    { 
                        cell.Visibility = Visibility.Collapsed;
                    } 
                    else
                    {
                        cell.Visibility = Visibility.Visible; 
                        cell.SetValue(Canvas.LeftProperty, leftEdge);
                        cell.Width = dataGridColumn.Width;
                        cell.Height = this._cellsElement.Height; 
                        RectangleGeometry rg = new RectangleGeometry(); 
                        if (this.OwningGrid.DisplayData.FirstDisplayedScrollingCol == dataGridColumn.Index && this.OwningGrid.FirstDisplayedScrollingColumnHiddenWidth > 0)
                        { 
                            rg.Rect = new Rect(this.OwningGrid.FirstDisplayedScrollingColumnHiddenWidth, 0, cell.Width - this.OwningGrid.FirstDisplayedScrollingColumnHiddenWidth, cell.Height);
                        }
                        else 
                        {
                            rg.Rect = new Rect(0, 0, cell.Width, cell.Height);
                        } 
                        cell.Clip = rg; 
                    }
                } 
            }
            else
            { 
                cell.Visibility = Visibility.Collapsed;
            }
        } 
 
        private void PerformFillerCellLayout()
        { 
            // Layout or hide the FillerCell as appropriate
            if (this.OwningGrid.ColumnsInternal.FillerColumn.IsActive)
            { 
                Debug.Assert(this.CellsWidth < this.Width);
                this._fillerCell.Width = this.Width - this.CellsWidth;
                this._fillerCell.Height = this.ActualCellHeight; 
                this._fillerCell.SetValue(Canvas.LeftProperty, this.CellsWidth); 
                this._fillerCell.Visibility = Visibility.Visible;
            } 
            else
            {
                this._fillerCell.Visibility = Visibility.Collapsed; 
            }
        }
 
        // Lays out the RootPart 
        private void PerformRootLayout()
        { 
            this.RootElement.Width = this.Width;
            this.RootElement.Height = this.DisplayHeight;
        } 

        // Performs layout for RowDetails and returns true if there were layout changes
        private void PerformRowDetailsLayout() 
        { 
            if (_detailsElement != null)
            { 
                if (this.DetailsVisibility == Visibility.Visible)
                {
                    _detailsGrid.Width = this.OwningGrid.OverrideRowDetailsScrolling ? this.OwningGrid.CellsWidth : this.Width + this.OwningGrid.HorizontalOffset; 
                    if (_detailsGrid.Children.Count > 0)
                    {
                        if (this.OwningGrid.OverrideRowDetailsScrolling) 
                        { 
                            // Details is frozen below the row and takes the Row's displayed width
                            _detailsGrid.SetValue(Canvas.LeftProperty, 0); 
                        }
                        else
                        { 
                            // Details expands the width of the row and scrolls with it
                            _detailsGrid.SetValue(Canvas.LeftProperty, -this.OwningGrid.HorizontalOffset);
                        } 
                    } 
                }
 
                // Clip the Details so it doesn't overlap the Cells when it's on Top
                if (!double.IsNaN(_detailsGrid.Width))
                { 
                    RectangleGeometry rg = new RectangleGeometry();
                    // The _detailsGrid is what actually contains the content so we clip that, but we clip
                    // the Height to _detailsElement.ActualHeight because the details storyboards animate 
                    // the the height of the _detailsElement 
                    rg.Rect = new Rect(0, 0, _detailsGrid.Width, _detailsElement.ActualHeight);
                    _detailsGrid.Clip = rg; 
                }
            }
        } 

        // Performs layout for the potential right edge of the provided cell
        private void PerformVerticalGridlineLayout(double leftEdge, DataGridColumnBase dataGridColumn, DataGridCell dataGridCell, bool isCellFrozen) 
        { 
            Debug.Assert(dataGridCell != null);
            Debug.Assert(this._cellsElement != null); 

            // Since we delay this when we're applying the Template, there's a chance this row is discarded
            if (this.OwningGrid == null) 
            {
                return;
            } 
 
            if (_applyingTemplate)
            { 
                //

                // Delay layout of VerticalGridlines until after the measure/arrange pass.  This is temporary 
                // until VerticalGridlines are moved to the Cells canvas
                // The Designer doesn't support BeginInvoke so don't call it if we're in DesignMode
                if (Application.Current != null && Application.Current.RootVisual != null && 
                    !DesignerProperties.GetIsInDesignMode(Application.Current.RootVisual)) 
                {
                    this.Dispatcher.BeginInvoke( 
                        new VerticalGridlineLayoutDelegate(PerformVerticalGridlineLayout),
                        leftEdge,
                        dataGridColumn, 
                        dataGridCell,
                        isCellFrozen);
                } 
                return; 
            }
 
            if (this.OwningGrid.ColumnRequiresRightGridline(dataGridColumn, true /*includeLastRightGridlineWhenPresent*/) && leftEdge < this.OwningGrid.CellsWidth)
            {
                double cutOff = 0; 
                double topEdge = (double)this.GetValue(Canvas.TopProperty);

                if (isCellFrozen) 
                { 
                    if (leftEdge + DataGrid.VerticalGridlinesThickness > this._cellsElement.Width)
                    { 
                        cutOff = leftEdge + DataGrid.VerticalGridlinesThickness - this._cellsElement.Width;
                    }
                    dataGridCell.RightGridline.Visibility = Visibility.Visible; 
                    dataGridCell.RightGridline.SetValue(Canvas.TopProperty, topEdge);
                    dataGridCell.RightGridline.SetValue(Canvas.LeftProperty, leftEdge);
                    dataGridCell.RightGridline.X1 = DataGrid.VerticalGridlinesThickness / 2; 
                    dataGridCell.RightGridline.X2 = DataGrid.VerticalGridlinesThickness / 2; 
                    dataGridCell.RightGridline.Y2 = this._cellsElement.Height;
                    RectangleGeometry rg = new RectangleGeometry(); 
                    rg.Rect = new Rect(0, 0, DataGrid.VerticalGridlinesThickness - cutOff, this._cellsElement.Height);
                    dataGridCell.RightGridline.Clip = rg;
                } 
                else
                {
                    double offset = 0; 
                    if (this.OwningGrid.DisplayData.FirstDisplayedScrollingCol == dataGridCell.ColumnIndex && 
                        this.OwningGrid.FirstDisplayedScrollingColumnHiddenWidth > DataGrid.VerticalGridlinesThickness + dataGridCell.Width)
                    { 
                        offset = this.OwningGrid.FirstDisplayedScrollingColumnHiddenWidth - DataGrid.VerticalGridlinesThickness - dataGridCell.Width;
                    }
                    if (leftEdge + DataGrid.VerticalGridlinesThickness > this._cellsElement.Width) 
                    {
                        cutOff = leftEdge + DataGrid.VerticalGridlinesThickness - this._cellsElement.Width;
                    } 
                    if (DataGrid.VerticalGridlinesThickness - offset - cutOff <= 0) 
                    {
                        if (dataGridCell.HasRightGridline) 
                        {
                            dataGridCell.RightGridline.Visibility = Visibility.Collapsed;
                        } 
                    }
                    else
                    { 
                        dataGridCell.RightGridline.Visibility = Visibility.Visible; 
                        dataGridCell.RightGridline.SetValue(Canvas.TopProperty, topEdge);
                        dataGridCell.RightGridline.SetValue(Canvas.LeftProperty, leftEdge); 
                        dataGridCell.RightGridline.X1 = DataGrid.VerticalGridlinesThickness / 2;
                        dataGridCell.RightGridline.X2 = DataGrid.VerticalGridlinesThickness / 2;
                        dataGridCell.RightGridline.Y2 = this._cellsElement.Height; 
                        RectangleGeometry rg = new RectangleGeometry();
                        rg.Rect = new Rect(offset, 0, DataGrid.VerticalGridlinesThickness - offset - cutOff, this._cellsElement.Height);
                        dataGridCell.RightGridline.Clip = rg; 
                    } 
                }
            } 
            else if (dataGridCell.HasRightGridline)
            {
                dataGridCell.RightGridline.Visibility = Visibility.Collapsed; 
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
