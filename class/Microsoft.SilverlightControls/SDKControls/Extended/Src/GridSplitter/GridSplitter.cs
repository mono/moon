// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections;
using System.Diagnostics;
using System.Windows.Automation.Peers;
using System.Windows.Controls.Primitives;
using System.Windows.Input;

namespace System.Windows.Controls
{
    /// <summary>
    /// Represents the control that redistributes space between columns or rows of a Grid control
    /// </summary>
    [TemplatePart(Name = GridSplitter.ElementHorizontalTemplateName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = GridSplitter.ElementVerticalTemplateName, Type = typeof(FrameworkElement))]
    [TemplateVisualState(Name = VisualStates.StateNormal, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateMouseOver, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateDisabled, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateUnfocused, GroupName = VisualStates.GroupFocus)]
    [TemplateVisualState(Name = VisualStates.StateFocused, GroupName = VisualStates.GroupFocus)]
    public partial class GridSplitter : Control
    {
        #region TemplateParts

        internal const string ElementHorizontalTemplateName = "HorizontalTemplate";
        internal const string ElementVerticalTemplateName = "VerticalTemplate";

        internal FrameworkElement _elementHorizontalTemplateFrameworkElement;
        internal FrameworkElement _elementVerticalTemplateFrameworkElement;

        #endregion

        #region Dependency Properties

        /// <summary>
        /// Identifies the ShowsPreview dependency property
        /// </summary>
        public static readonly DependencyProperty ShowsPreviewProperty = DependencyProperty.Register("ShowsPreview", typeof(bool), typeof(GridSplitter), null);

        /// <summary>
        /// Identifies the PreviewStyle dependency property
        /// </summary>
        public static readonly DependencyProperty PreviewStyleProperty = DependencyProperty.Register("PreviewStyle", typeof(Style), typeof(GridSplitter), null);

        /// <summary>
        /// Called when the IsEnabled property changes.
        /// </summary>
        /// <param name="sender">Sender object</param>
        /// <param name="e">Property changed args</param>
        private void OnIsEnabledChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            Debug.Assert(e.NewValue is bool);
            bool isEnabled = (bool)e.NewValue;

            if (!isEnabled)
            {
                _isMouseOver = false;
            }
            ChangeVisualState();
        }

        #endregion

        internal ResizeData _resizeData; // is Null unless a resize operation is in progress
        private Canvas _previewLayer; // is Null until a resize operation is initiated with ShowsPreview == true, then it persists for the life of the GridSplitter
        private DragValidator _dragValidator; // is initialized in the constructor
        private GridResizeDirection _currentGridResizeDirection = GridResizeDirection.Auto;

        private bool _isMouseOver; // holds the state for whether the mouse is over the control or not

        // Default increment parameters
        private const double DragIncrement = 1.0;
        private const double KeyboardIncrement = 10.0;

        #region Control Instantiation

        /// <summary>
        /// Initializes a new instance of the <see cref="GridSplitter"/> class
        /// </summary>
        public GridSplitter()
        {
            this.IsEnabledChanged += new DependencyPropertyChangedEventHandler(OnIsEnabledChanged);
            this.KeyDown += new KeyEventHandler(GridSplitter_KeyDown);
            this.LayoutUpdated += delegate { UpdateTemplateOrientation(); };
            _dragValidator = new DragValidator(this);
            _dragValidator.DragStartedEvent += new EventHandler<DragStartedEventArgs>(DragValidator_DragStartedEvent);
            _dragValidator.DragDeltaEvent += new EventHandler<DragDeltaEventArgs>(DragValidator_DragDeltaEvent);
            _dragValidator.DragCompletedEvent += new EventHandler<DragCompletedEventArgs>(DragValidator_DragCompletedEvent);

            this.MouseEnter += delegate(object sender, MouseEventArgs e)
            {
                _isMouseOver = true;
                ChangeVisualState();
            };

            this.MouseLeave += delegate(object sender, MouseEventArgs e)
            {
                _isMouseOver = false;
                // Only change the visual state if we're not currently resizing, the visual state will get updated when the resize operation is completed
                if (_resizeData == null)
                {
                    ChangeVisualState();
                }
            };

            this.GotFocus += delegate(object sender, RoutedEventArgs e)
            {
                ChangeVisualState();
            };

            this.LostFocus += delegate(object sender, RoutedEventArgs e)
            {
                ChangeVisualState();
            };

            DefaultStyleKey = typeof(GridSplitter);
        }

        /// <summary>
        /// Called when template should be applied to the control
        /// </summary>
        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();

            _elementHorizontalTemplateFrameworkElement = this.GetTemplateChild(GridSplitter.ElementHorizontalTemplateName) as FrameworkElement;
            _elementVerticalTemplateFrameworkElement = this.GetTemplateChild(GridSplitter.ElementVerticalTemplateName) as FrameworkElement;

            // We need to recalculate the orientation, so set _currentGridResizeDirection back to Auto
            _currentGridResizeDirection = GridResizeDirection.Auto;

            UpdateTemplateOrientation();
            ChangeVisualState(false);
        }

        #endregion

        #region Public Members

        /// <summary>
        /// Gets or sets a value that indicates whether the GridSplitter control updates the column or row size as the user drags the control. This is a dependency property.
        /// </summary>
        public bool ShowsPreview
        {
            get
            {
                return (bool)GetValue(GridSplitter.ShowsPreviewProperty);
            }
            set
            {
                SetValue(GridSplitter.ShowsPreviewProperty, value);
            }
        }

        /// <summary>
        /// Gets or sets the style that customizes the appearance, effects, or other style characteristics for the GridSplitter control preview indicator that is displayed when the ShowsPreview property is set to true. This is a dependency property.
        /// </summary>
        public Style PreviewStyle
        {
            get
            {
                return (Style)GetValue(GridSplitter.PreviewStyleProperty);
            }
            set
            {
                SetValue(GridSplitter.PreviewStyleProperty, value);
            }
        }

        #endregion

        #region Protected Methods

        /// <summary>
        /// Creates AutomationPeer (<see cref="UIElement.OnCreateAutomationPeer"/>)
        /// </summary>
        protected override AutomationPeer OnCreateAutomationPeer()
        {
            return new GridSplitterAutomationPeer(this);
        }

        #endregion

        #region State Storyboard Support

        /// <summary>
        /// Method to change the visual state of the control
        /// </summary>
        private void ChangeVisualState()
        {
            ChangeVisualState(true);
        }

        /// <summary>
        /// Change to the correct visual state for the GridSplitter.
        /// </summary>
        /// <param name="useTransitions">
        /// true to use transitions when updating the visual state, false to
        /// snap directly to the new visual state.
        /// </param>
        private void ChangeVisualState(bool useTransitions)
        {
            if (!IsEnabled)
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateDisabled, VisualStates.StateNormal);
            }
            else if (_isMouseOver)
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateMouseOver, VisualStates.StateNormal);
            }
            else
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateNormal);
            }

            if (HasKeyboardFocus && this.IsEnabled)
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateFocused, VisualStates.StateUnfocused);
            }
            else
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateUnfocused);
            }

            if (GetEffectiveResizeDirection() == GridResizeDirection.Columns)
            {
                this.Cursor = Cursors.SizeWE;
            }
            else
            {
                this.Cursor = Cursors.SizeNS;
            }
        }

        #endregion

        #region Drag Support

        /// <summary>
        /// Handle the drag completed event to commit or cancel the resize operation in progress
        /// </summary>
        internal void DragValidator_DragCompletedEvent(object sender, DragCompletedEventArgs e)
        {
            if (_resizeData != null)
            {
                if (e.Canceled)
                {
                    CancelResize();
                }
                else
                {
                    if (_resizeData.ShowsPreview)
                    {
                        MoveSplitter(_resizeData.PreviewControl.OffsetX, _resizeData.PreviewControl.OffsetY);
                        RemovePreviewControl();
                    }
                }
                _resizeData = null;
            }
            ChangeVisualState();
        }

        /// <summary>
        /// Handle the drag delta event to update the UI for the resize operation in progress
        /// </summary>
        internal void DragValidator_DragDeltaEvent(object sender, DragDeltaEventArgs e)
        {
            if (_resizeData != null)
            {
                double horizontalChange = e.HorizontalChange;
                double verticalChange = e.VerticalChange;

                if (_resizeData.ShowsPreview)
                {
                    if (_resizeData.ResizeDirection == GridResizeDirection.Columns)
                    {
                        _resizeData.PreviewControl.OffsetX = Math.Min(Math.Max(horizontalChange, _resizeData.MinChange), _resizeData.MaxChange);
                    }
                    else
                    {
                        _resizeData.PreviewControl.OffsetY = Math.Min(Math.Max(verticalChange, _resizeData.MinChange), _resizeData.MaxChange);
                    }
                }
                else
                {
                    MoveSplitter(horizontalChange, verticalChange);
                }
            }
        }

        /// <summary>
        /// Handle the drag started event to start a resize operation if the control is enabled
        /// </summary>
        internal void DragValidator_DragStartedEvent(object sender, DragStartedEventArgs e)
        {
            if (this.IsEnabled)
            {
                Focus();
                InitializeData(this.ShowsPreview);
            }
        }

        #endregion

        #region Keyboard Support

        /// <summary>
        /// Handle the key down event to allow keyboard resizing or canceling a resize operation
        /// </summary>
        internal void GridSplitter_KeyDown(object sender, KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Left:
                    e.Handled = KeyboardMoveSplitter(-KeyboardIncrement, 0.0);
                    return;

                case Key.Up:
                    e.Handled = KeyboardMoveSplitter(0.0, -KeyboardIncrement);
                    return;

                case Key.Right:
                    e.Handled = KeyboardMoveSplitter(KeyboardIncrement, 0.0);
                    return;

                case Key.Down:
                    e.Handled = KeyboardMoveSplitter(0.0, KeyboardIncrement);
                    break;

                case Key.Escape:
                    if (_resizeData == null)
                    {
                        break;
                    }
                    CancelResize();
                    e.Handled = true;
                    return;

                default:
                    return;
            }
        }

        /// <summary>
        /// Gets whether or not the control has keyboard focus
        /// </summary>
        private bool HasKeyboardFocus
        {
            get
            {
                return FocusManager.GetFocusedElement() == this;
            }
        }

        /// <summary>
        /// Initialize the resize data and move the splitter by the specified amount
        /// </summary>
        /// <param name="horizontalChange">Horizontal amount to move the splitter</param>
        /// <param name="verticalChange">Vertical amount to move the splitter</param>
        /// <returns></returns>
        internal bool InitializeAndMoveSplitter(double horizontalChange, double verticalChange)
        {
            // resizing directly is not allowed if there is a mouse initiated resize operation in progress
            if (_resizeData != null)
            {
                return false;
            }

            InitializeData(false);
            if (_resizeData == null)
            {
                return false;
            }

            MoveSplitter(horizontalChange, verticalChange);
            _resizeData = null;
            return true;
        }

        /// <summary>
        /// Called by keyboard event handler to move the splitter if allowed
        /// </summary>
        /// <param name="horizontalChange">Horizontal amount to move the splitter</param>
        /// <param name="verticalChange">Vertical amount to move the splitter</param>
        /// <returns></returns>
        private bool KeyboardMoveSplitter(double horizontalChange, double verticalChange)
        {
            if (HasKeyboardFocus && this.IsEnabled)
            {
                return InitializeAndMoveSplitter(horizontalChange, verticalChange);
            }
            return false;
        }

        #endregion

        #region Preview Support

        /// <summary>
        /// Creates the preview layer and adds it to the parent grid
        /// </summary>
        /// <param name="parentGrid">Grid to add the preview layer to</param>
        private void CreatePreviewLayer(Grid parentGrid)
        {
            Debug.Assert(parentGrid != null);
            Debug.Assert(parentGrid.RowDefinitions != null);
            Debug.Assert(parentGrid.ColumnDefinitions != null);

            _previewLayer = new Canvas();
            // RowSpan and ColumnSpan default to 1 and should not be set to 0 in the case that a Grid
            // has been created without explicitly setting its ColumnDefinitions or RowDefinitions
            if (parentGrid.RowDefinitions.Count > 0)
            {
                _previewLayer.SetValue(Grid.RowSpanProperty, parentGrid.RowDefinitions.Count);
            }
            if (parentGrid.ColumnDefinitions.Count > 0)
            {
                _previewLayer.SetValue(Grid.ColumnSpanProperty, parentGrid.ColumnDefinitions.Count);
            }
            // 


            parentGrid.Children.Add(_previewLayer);
        }

        /// <summary>
        /// Add the preview layer to the Grid if it is not there already and then show the preview control
        /// </summary>
        private void SetupPreview()
        {
            if (_resizeData.ShowsPreview)
            {
                if (_previewLayer == null)
                {
                    CreatePreviewLayer(_resizeData.Grid);
                }

                _resizeData.PreviewControl = new PreviewControl();
                _resizeData.PreviewControl.Bind(this);
                _previewLayer.Children.Add(_resizeData.PreviewControl);
                double[] changeRange = GetDeltaConstraints();
                Debug.Assert(changeRange.Length == 2);
                _resizeData.MinChange = changeRange[0];
                _resizeData.MaxChange = changeRange[1];
            }
        }

        /// <summary>
        /// Remove the preview control from the preview layer if it exists
        /// </summary>
        private void RemovePreviewControl()
        {
            if ((_resizeData.PreviewControl != null) && (_previewLayer != null))
            {
                Debug.Assert(_previewLayer.Children.Contains(_resizeData.PreviewControl));
                _previewLayer.Children.Remove(_resizeData.PreviewControl);
            }
        }

        #endregion

        #region Resize Support

        /// <summary>
        /// Initialize the resizeData object to hold the information for the resize operation in progress
        /// </summary>
        /// <param name="showsPreview">Whether or not the preview should be shown</param>
        private void InitializeData(bool showsPreview)
        {
            Grid parent = base.Parent as Grid;
            if (parent != null)
            {
                _resizeData = new ResizeData();
                _resizeData.Grid = parent;
                _resizeData.ShowsPreview = showsPreview;
                _resizeData.ResizeDirection = GetEffectiveResizeDirection();
                _resizeData.ResizeBehavior = GetEffectiveResizeBehavior(_resizeData.ResizeDirection);
                _resizeData.SplitterLength = Math.Min(base.ActualWidth, base.ActualHeight);
                if (!SetupDefinitionsToResize())
                {
                    _resizeData = null;
                }
                else
                {
                    SetupPreview();
                }
            }
        }

        /// <summary>
        /// Move the splitter and resize the affected columns or rows
        /// </summary>
        /// <param name="horizontalChange">Amount to resize horizontally</param>
        /// <param name="verticalChange">Amount to resize vertically</param>
        /// <remarks>Only one of horizontalChange or verticalChange will be non-zero.</remarks>
        private void MoveSplitter(double horizontalChange, double verticalChange)
        {
            double resizeChange = (_resizeData.ResizeDirection == GridResizeDirection.Columns) ? horizontalChange : verticalChange;
            DefinitionAbstraction definition1 = _resizeData.Definition1;
            DefinitionAbstraction definition2 = _resizeData.Definition2;
            if ((definition1 != null) && (definition2 != null))
            {
                double definition1ActualLength = GetActualLength(definition1);
                double definition2ActualLength = GetActualLength(definition2);
                if ((_resizeData.SplitBehavior == SplitBehavior.Split) && !DoubleUtil.AreClose((double)(definition1ActualLength + definition2ActualLength), (double)(_resizeData.OriginalDefinition1ActualLength + _resizeData.OriginalDefinition2ActualLength)))
                {
                    this.CancelResize();
                }
                else
                {
                    double[] changeRange = GetDeltaConstraints();
                    Debug.Assert(changeRange.Length == 2);
                    double minDelta = changeRange[0];
                    double maxDelta = changeRange[1];

                    resizeChange = Math.Min(Math.Max(resizeChange, minDelta), maxDelta);
                    double newDefinition1Length = definition1ActualLength + resizeChange;
                    double newDefinition2Length = definition2ActualLength - resizeChange;
                    SetLengths(newDefinition1Length, newDefinition2Length);
                }
            }
        }

        /// <summary>
        /// Determine which adjacent column or row definitions need to be included in the resize operation and set up resizeData accordingly
        /// </summary>
        /// <returns>True if it is a valid resize operation</returns>
        private bool SetupDefinitionsToResize()
        {
            int spanAmount = (int)base.GetValue((_resizeData.ResizeDirection == GridResizeDirection.Columns) ? Grid.ColumnSpanProperty : Grid.RowSpanProperty);
            if (spanAmount == 1)
            {
                int definition1Index;
                int definition2Index;
                int splitterIndex = (int)base.GetValue((_resizeData.ResizeDirection == GridResizeDirection.Columns) ? Grid.ColumnProperty : Grid.RowProperty);
                switch (_resizeData.ResizeBehavior)
                {
                    case GridResizeBehavior.CurrentAndNext:
                        definition1Index = splitterIndex;
                        definition2Index = splitterIndex + 1;
                        break;

                    case GridResizeBehavior.PreviousAndCurrent:
                        definition1Index = splitterIndex - 1;
                        definition2Index = splitterIndex;
                        break;

                    default:
                        definition1Index = splitterIndex - 1;
                        definition2Index = splitterIndex + 1;
                        break;
                }
                int definitionCount = (_resizeData.ResizeDirection == GridResizeDirection.Columns) ? _resizeData.Grid.ColumnDefinitions.Count : _resizeData.Grid.RowDefinitions.Count;
                if ((definition1Index >= 0) && (definition2Index < definitionCount))
                {
                    _resizeData.SplitterIndex = splitterIndex;
                    _resizeData.Definition1Index = definition1Index;
                    _resizeData.Definition1 = GetGridDefinition(_resizeData.Grid, definition1Index, _resizeData.ResizeDirection);
                    _resizeData.OriginalDefinition1Length = _resizeData.Definition1.Size;
                    _resizeData.OriginalDefinition1ActualLength = GetActualLength(_resizeData.Definition1);
                    _resizeData.Definition2Index = definition2Index;
                    _resizeData.Definition2 = GetGridDefinition(_resizeData.Grid, definition2Index, _resizeData.ResizeDirection);
                    _resizeData.OriginalDefinition2Length = _resizeData.Definition2.Size;
                    _resizeData.OriginalDefinition2ActualLength = GetActualLength(_resizeData.Definition2);
                    bool isDefinition1Star = IsStar(_resizeData.Definition1);
                    bool isDefinition2Star = IsStar(_resizeData.Definition2);
                    if (isDefinition1Star && isDefinition2Star)
                    {
                        _resizeData.SplitBehavior = SplitBehavior.Split;
                    }
                    else
                    {
                        _resizeData.SplitBehavior = !isDefinition1Star ? SplitBehavior.ResizeDefinition1 : SplitBehavior.ResizeDefinition2;
                    }
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Cancel the resize operation in progress
        /// </summary>
        private void CancelResize()
        {
            if (_resizeData.ShowsPreview)
            {
                RemovePreviewControl();
            }
            else
            {
                SetLengths(_resizeData.OriginalDefinition1ActualLength, _resizeData.OriginalDefinition2ActualLength);
            }
            _resizeData = null;
        }

        /// <summary>
        /// Get the actual length of the given definition
        /// </summary>
        /// <param name="definition">Row or column definition to get the actual length for</param>
        /// <returns>Height of a row definition or width of a column definition</returns>
        private static double GetActualLength(DefinitionAbstraction definition)
        {
            if (definition.AsColumnDefinition != null)
            {
                return definition.AsColumnDefinition.ActualWidth;
            }
            return definition.AsRowDefinition.ActualHeight;
        }

        /// <summary>
        /// Determine the max and min that the two definitions can be resized
        /// </summary>
        /// <returns></returns>
        private double[] GetDeltaConstraints()
        {
            double definition1ActualLength = GetActualLength(_resizeData.Definition1);
            double definition1MinSize = _resizeData.Definition1.MinSize;
            double definition1MaxSize = _resizeData.Definition1.MaxSize;
            double definition2ActualLength = GetActualLength(_resizeData.Definition2);
            double definition2MinSize = _resizeData.Definition2.MinSize;
            double definition2MaxSize = _resizeData.Definition2.MaxSize;
            double minDelta, maxDelta;

            // Can't resize smaller than the size of the splitter control itself
            if (_resizeData.SplitterIndex == _resizeData.Definition1Index)
            {
                definition1MinSize = Math.Max(definition1MinSize, _resizeData.SplitterLength);
            }
            else if (_resizeData.SplitterIndex == _resizeData.Definition2Index)
            {
                definition2MinSize = Math.Max(definition2MinSize, _resizeData.SplitterLength);
            }

            if (_resizeData.SplitBehavior == SplitBehavior.Split)
            {
                minDelta = -Math.Min((double)(definition1ActualLength - definition1MinSize), (double)(definition2MaxSize - definition2ActualLength));
                maxDelta = Math.Min((double)(definition1MaxSize - definition1ActualLength), (double)(definition2ActualLength - definition2MinSize));
            }
            else if (_resizeData.SplitBehavior == SplitBehavior.ResizeDefinition1)
            {
                minDelta = definition1MinSize - definition1ActualLength;
                maxDelta = definition1MaxSize - definition1ActualLength;
            }
            else
            {
                minDelta = definition2ActualLength - definition2MaxSize;
                maxDelta = definition2ActualLength - definition2MinSize;
            }

            return new double[] { minDelta, maxDelta };
        }

        /// <summary>
        /// Determine the resize behavior based on the given direction and alignment
        /// </summary>
        private GridResizeBehavior GetEffectiveResizeBehavior(GridResizeDirection direction)
        {
            if (direction != GridResizeDirection.Columns)
            {
                switch (base.VerticalAlignment)
                {
                    case VerticalAlignment.Top:
                        return GridResizeBehavior.PreviousAndCurrent;

                    case VerticalAlignment.Bottom:
                        return GridResizeBehavior.CurrentAndNext;
                }
                return GridResizeBehavior.PreviousAndNext;
            }
            else
            {
                switch (base.HorizontalAlignment)
                {
                    case HorizontalAlignment.Left:
                        return GridResizeBehavior.PreviousAndCurrent;

                    case HorizontalAlignment.Right:
                        return GridResizeBehavior.CurrentAndNext;
                }
                return GridResizeBehavior.PreviousAndNext;
            }
        }

        /// <summary>
        /// Determine the resize direction based on the horizontal and vertical alignments
        /// </summary>
        private GridResizeDirection GetEffectiveResizeDirection()
        {
            if (base.HorizontalAlignment != HorizontalAlignment.Stretch)
            {
                return GridResizeDirection.Columns;
            }
            if ((base.VerticalAlignment == VerticalAlignment.Stretch) && (base.ActualWidth <= base.ActualHeight))
            {
                return GridResizeDirection.Columns;
            }
            return GridResizeDirection.Rows;
        }

        /// <summary>
        /// Create a DefinitionAbstraction instance for the given row or column index in the grid
        /// </summary>
        private static DefinitionAbstraction GetGridDefinition(Grid grid, int index, GridResizeDirection direction)
        {
            if (direction != GridResizeDirection.Columns)
            {
                return new DefinitionAbstraction(grid.RowDefinitions[index]);
            }
            return new DefinitionAbstraction(grid.ColumnDefinitions[index]);
        }

        /// <summary>
        /// Set the lengths of the two definitions depending on the split behavior
        /// </summary>
        private void SetLengths(double definition1Pixels, double definition2Pixels)
        {
            if (_resizeData.SplitBehavior == SplitBehavior.Split)
            {
                IEnumerable enumerable = (_resizeData.ResizeDirection == GridResizeDirection.Columns) ? ((IEnumerable)_resizeData.Grid.ColumnDefinitions) : ((IEnumerable)_resizeData.Grid.RowDefinitions);
                int definitionIndex = 0;
                DefinitionAbstraction definitionAbstraction;
                foreach (DependencyObject definition in enumerable)
                {
                    definitionAbstraction = new DefinitionAbstraction(definition);
                    if (definitionIndex == _resizeData.Definition1Index)
                    {
                        SetDefinitionLength(definitionAbstraction, new GridLength(definition1Pixels, GridUnitType.Star));
                    }
                    else if (definitionIndex == _resizeData.Definition2Index)
                    {
                        SetDefinitionLength(definitionAbstraction, new GridLength(definition2Pixels, GridUnitType.Star));
                    }
                    else if (IsStar(definitionAbstraction))
                    {
                        SetDefinitionLength(definitionAbstraction, new GridLength(GetActualLength(definitionAbstraction), GridUnitType.Star));
                    }
                    definitionIndex++;
                }
            }
            else if (_resizeData.SplitBehavior == SplitBehavior.ResizeDefinition1)
            {
                SetDefinitionLength(_resizeData.Definition1, new GridLength(definition1Pixels));
            }
            else
            {
                SetDefinitionLength(_resizeData.Definition2, new GridLength(definition2Pixels));
            }
        }

        /// <summary>
        /// Set the height/width of the given row/column
        /// </summary>
        private static void SetDefinitionLength(DefinitionAbstraction definition, GridLength length)
        {
            if (definition.AsColumnDefinition != null)
            {
                definition.AsColumnDefinition.SetValue(ColumnDefinition.WidthProperty, length);
            }
            else
            {
                definition.AsRowDefinition.SetValue(RowDefinition.HeightProperty, length);
            }
        }

        /// <summary>
        /// Determine if the given definition has its size set to the "*" value
        /// </summary>
        private static bool IsStar(DefinitionAbstraction definition)
        {
            if (definition.AsColumnDefinition != null)
            {
                return definition.AsColumnDefinition.Width.IsStar;
            }
            return definition.AsRowDefinition.Height.IsStar;
        }

        #endregion

        #region Template Orientation Support

        /// <summary>
        /// This code will run whenever the effective resize direction changes, to update the template
        /// being used to display this control.
        /// </summary>
        private void UpdateTemplateOrientation()
        {
            GridResizeDirection newGridResizeDirection = GetEffectiveResizeDirection();

            if (_currentGridResizeDirection != newGridResizeDirection)
            {
                if (newGridResizeDirection == GridResizeDirection.Columns)
                {
                    if (_elementHorizontalTemplateFrameworkElement != null)
                    {
                        _elementHorizontalTemplateFrameworkElement.Visibility = Visibility.Collapsed;
                    }
                    if (_elementVerticalTemplateFrameworkElement != null)
                    {
                        _elementVerticalTemplateFrameworkElement.Visibility = Visibility.Visible;
                    }
                }
                else
                {
                    if (_elementHorizontalTemplateFrameworkElement != null)
                    {
                        _elementHorizontalTemplateFrameworkElement.Visibility = Visibility.Visible;
                    }
                    if (_elementVerticalTemplateFrameworkElement != null)
                    {
                        _elementVerticalTemplateFrameworkElement.Visibility = Visibility.Collapsed;
                    }
                }
                _currentGridResizeDirection = newGridResizeDirection;
            }
        }

        #endregion

        #region DoubleUtil Nested Class

        /// <summary>
        /// A collection of helper methods for working with double data types
        /// </summary>
        internal static class DoubleUtil
        {
            /// <summary>
            /// Epsilon is the smallest value such that 1.0+epsilon != 1.0. It can be used to determine the 
            /// acceptable tolerance for rounding errors.
            /// </summary>
            /// <remarks>
            /// Epsilon is normally 2.2204460492503131E-16, but Silverlight 2 uses floats so the effective
            /// epsilon is really 1.192093E-07
            /// </remarks>
            private const double Epsilon = 1.192093E-07;

            private const double ScalarAdjustment = 10.0;

            /// <summary>
            /// Determine if the two doubles are effectively equal within tolerances
            /// </summary>
            public static bool AreClose(double value1, double value2)
            {
                if (value1 == value2)
                {
                    return true;
                }
                double num = ((Math.Abs(value1) + Math.Abs(value2)) + DoubleUtil.ScalarAdjustment) * DoubleUtil.Epsilon;
                double num2 = value1 - value2;
                return ((-num < num2) && (num > num2));
            }
        }

        #endregion

        #region ResizeData Nested Class

        /// <summary>
        /// Type to hold the data for the resize operation in progress
        /// </summary>
        internal class ResizeData
        {
            public PreviewControl PreviewControl;
            public DefinitionAbstraction Definition1;
            public int Definition1Index;
            public DefinitionAbstraction Definition2;
            public int Definition2Index;
            public Grid Grid;
            public double MaxChange;
            public double MinChange;
            public double OriginalDefinition1ActualLength;
            public GridLength OriginalDefinition1Length;
            public double OriginalDefinition2ActualLength;
            public GridLength OriginalDefinition2Length;
            public GridResizeBehavior ResizeBehavior;
            public GridResizeDirection ResizeDirection;
            public bool ShowsPreview;
            public GridSplitter.SplitBehavior SplitBehavior;
            public int SplitterIndex;
            public double SplitterLength;
        }

        #endregion

        #region DefinitionAbstraction Nested Class

        /// <summary>
        /// Pretends to be the base class for RowDefinition and ClassDefinition types so that objects of either type can be treated as one.
        /// </summary>
        internal class DefinitionAbstraction
        {

            /// <summary>
            /// Creates an instance of the DefinitionAbstraction class based on the given row or column definition
            /// </summary>
            /// <param name="definition">RowDefinition or ColumnDefinition instance</param>
            public DefinitionAbstraction(DependencyObject definition)
            {

                this.AsRowDefinition = definition as RowDefinition;
                if (this.AsRowDefinition == null)
                {
                    this.AsColumnDefinition = definition as ColumnDefinition;
                    Debug.Assert(this.AsColumnDefinition != null);
                }
            }

            /// <summary>
            /// Gets the stored definition cast as a row definition
            /// </summary>
            /// <value>Null if not a RowDefinition</value>
            public RowDefinition AsRowDefinition { get; private set; }

            /// <summary>
            /// Gets the stored definition cast as a column definition
            /// </summary>
            /// <value>Null if not a ColumnDefinition</value>
            public ColumnDefinition AsColumnDefinition { get; private set; }

            /// <summary>
            /// Gets the MaxHeight/MaxWidth for the row/column
            /// </summary>
            public double MaxSize
            {
                get
                {
                    if (this.AsRowDefinition != null)
                    {
                        return this.AsRowDefinition.MaxHeight;
                    }
                    return this.AsColumnDefinition.MaxWidth;
                }
            }

            /// <summary>
            /// Gets the MinHeight/MinWidth for the row/column
            /// </summary>
            public double MinSize
            {
                get
                {
                    if (this.AsRowDefinition != null)
                    {
                        return this.AsRowDefinition.MinHeight;
                    }
                    return this.AsColumnDefinition.MinWidth;
                }
            }

            /// <summary>
            /// Gets the Height/Width for the row/column
            /// </summary>
            public GridLength Size
            {
                get
                {
                    if (this.AsRowDefinition != null)
                    {
                        return this.AsRowDefinition.Height;
                    }
                    return this.AsColumnDefinition.Width;
                }
            }
        }

        #endregion

        #region SplitBehavior Nested Enum

        internal enum SplitBehavior
        {
            Split,
            ResizeDefinition1,
            ResizeDefinition2
        }

        #endregion

        #region GridResizeBehavior Nested Enum

        internal enum GridResizeBehavior
        {
            BasedOnAlignment,
            CurrentAndNext,
            PreviousAndCurrent,
            PreviousAndNext
        }

        #endregion

        #region GridResizeDirection Nested Enum

        internal enum GridResizeDirection
        {
            Auto,
            Columns,
            Rows
        }

        #endregion
    }
}
