// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Windows.Automation.Peers;
using System.Windows.Input; 
using System.Windows.Markup; 
using System.Windows.Media.Animation;
using System.Windows.Controls.Primitives; 
using System.Windows.Media;
using System.Windows.Controls;

namespace System.Windows.Controls
{
    /// <summary>
    /// Slider control lets the user select from a range of values by moving a slider. 
    /// Slider is used to enable to user to gradually modify a value (range selection). 
    /// Slider is an easy and natural interface for users, because it provides good visual feedback.
    /// </summary> 
    [TemplatePart (Name = "HorizontalTemplate", Type = typeof(FrameworkElement))]
    [TemplatePart (Name = "HorizontalTrackLargeChangeIncreaseRepeatButton", Type = typeof(RepeatButton))] 
    [TemplatePart (Name = "HorizontalTrackLargeChangeDecreaseRepeatButton", Type = typeof(RepeatButton))]
    [TemplatePart (Name = "HorizontalThumb", Type = typeof(Thumb))]
    [TemplatePart (Name = "VerticalTemplate", Type = typeof(FrameworkElement))] 
    [TemplatePart (Name = "VerticalTrackLargeChangeIncreaseRepeatButton", Type = typeof(RepeatButton))] 
    [TemplatePart (Name = "VerticalTrackLargeChangeDecreaseRepeatButton", Type = typeof(RepeatButton))]
    [TemplatePart (Name = "ElementVerticalThumb", Type = typeof(Thumb))] 
    [TemplateVisualState (Name = "Normal", GroupName = "CommonStates")]
    [TemplateVisualState (Name = "Disabled", GroupName = "CommonStates")]
    [TemplateVisualState (Name = "MouseOver", GroupName = "CommonStates")]
    [TemplateVisualState (Name = "Focused", GroupName = "FocusStates")]
    [TemplateVisualState (Name = "Unfocused", GroupName = "FocusStates")]
    public class Slider : RangeBase
    { 
        #region Constructor 
        /// <summary>
        /// Constructor to setup the Slider class 
        /// </summary>
        public Slider()
        { 
            Orientation = Orientation.Horizontal;
#if false
            IsTabStop = true;
            IsEnabled = true; 
            GotFocus += delegate(object sender, RoutedEventArgs e) { IsFocused = true; OnGotFocus (e); };
            LostFocus += delegate(object sender, RoutedEventArgs e) { IsFocused = false; OnLostFocus (e); }; 
            KeyDown += delegate(object sender, KeyEventArgs e){ OnKeyDown(e); };
            MouseEnter += delegate(object sender, MouseEventArgs e) { OnMouseEnter(e); };
            MouseLeave += delegate(object sender, MouseEventArgs e) { OnMouseLeave(e); }; 
            MouseLeftButtonDown += delegate(object sender, MouseButtonEventArgs e) { OnMouseLeftButtonDown(e); }; 
            MouseLeftButtonUp += delegate(object sender, MouseButtonEventArgs e) { OnMouseLeftButtonUp(e); };
#endif
            SizeChanged += delegate { UpdateTrackLayout(); };
        }

        /// <summary> 
        /// Apply a template to the slider.
        /// </summary>
        public override void OnApplyTemplate() 
        { 
            base.OnApplyTemplate();
 
            // Get the parts
            ElementRoot = GetTemplateChild(ElementRootName) as FrameworkElement;
            ElementHorizontalTemplate = GetTemplateChild(ElementHorizontalTemplateName) as FrameworkElement; 
            ElementHorizontalLargeIncrease = GetTemplateChild(ElementHorizontalLargeIncreaseName) as RepeatButton;
            ElementHorizontalLargeDecrease = GetTemplateChild(ElementHorizontalLargeDecreaseName) as RepeatButton;
            ElementHorizontalThumb = GetTemplateChild(ElementHorizontalThumbName) as Thumb; 
            ElementVerticalTemplate = GetTemplateChild(ElementVerticalTemplateName) as FrameworkElement; 
            ElementVerticalLargeIncrease = GetTemplateChild(ElementVerticalLargeIncreaseName) as RepeatButton;
            ElementVerticalLargeDecrease = GetTemplateChild(ElementVerticalLargeDecreaseName) as RepeatButton; 
            ElementVerticalThumb = GetTemplateChild(ElementVerticalThumbName) as Thumb;
            ElementFocusVisual = GetTemplateChild(ElementFocusVisualName) as FrameworkElement;
 
            // Get the states
            if (ElementRoot != null)
            { 
                StateNormal = ElementRoot.Resources[StateNormalName] as Storyboard; 
                StateMouseOver = ElementRoot.Resources[StateMouseOverName] as Storyboard;
                StateDisabled =  ElementRoot.Resources[StateDisabledName] as Storyboard; 
            }
            if (ElementHorizontalThumb != null)
            { 
                ElementHorizontalThumb.DragStarted += new DragStartedEventHandler (OnThumbDragStarted);
                ElementHorizontalThumb.DragDelta += new DragDeltaEventHandler(OnThumbDragDelta);
            } 
            if (ElementHorizontalLargeDecrease != null) 
            {
                ElementHorizontalLargeDecrease.Click += delegate { Value -= LargeChange; }; 
            }
            if (ElementHorizontalLargeIncrease != null)
            { 
                ElementHorizontalLargeIncrease.Click += delegate { Value += LargeChange; };
            }
 
            if (ElementVerticalThumb != null) 
            {
                ElementVerticalThumb.DragStarted += new DragStartedEventHandler(OnThumbDragStarted);
                ElementVerticalThumb.DragDelta += new DragDeltaEventHandler(OnThumbDragDelta);
            }
            if (ElementVerticalLargeDecrease != null) 
            {
                ElementVerticalLargeDecrease.Click += delegate { Value -= LargeChange; };
            } 
            if (ElementVerticalLargeIncrease != null) 
            {
                ElementVerticalLargeIncrease.Click += delegate { Value += LargeChange; }; 
            }
            // Updating states for parts where properties might have been updated through
            // XAML before the template was loaded. 
            IsEnabledChanged += delegate { OnIsEnabledChanged(IsEnabled); };
            UpdateVisualState(); 
        } 
        #endregion Constructor
 
        #region Orientation
        /// <summary>
        /// Gets whether the Slider has an orientation of vertical or horizontal. 
        /// </summary>
        public Orientation Orientation
        { 
            get { return (Orientation)GetValue(OrientationProperty); } 
            set { SetValue(OrientationProperty, value); }
        } 

        /// <summary>
        /// Identifies the Orientation dependency property. 
        /// </summary>
        public static readonly DependencyProperty OrientationProperty =
            DependencyProperty.Register( 
                "Orientation", 
                typeof(Orientation),
                typeof(Slider), 
                new PropertyMetadata(OnOrientationPropertyChanged));

        /// <summary> 
        /// OrientationProperty property changed handler.
        /// </summary>
        /// <param name="d">Slider that changed Orientation.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnOrientationPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            Slider s = d as Slider;
            Debug.Assert(s != null);
 
            if (s.ElementRoot != null)
            {
                s.OnOrientationChanged(); 
            } 
        }
 
        #endregion Orientation

        #region IsFocused 
        /// <summary>
        /// Gets a value that determines whether this element has logical focus.
        /// </summary> 
        public bool IsFocused 
        {
            get { return (bool)GetValue(IsFocusedProperty); } 
            internal set { SetValue(IsFocusedProperty, value); }
        }
 
        /// <summary>
        /// Identifies the IsFocused dependency property.
        /// </summary> 
        public static readonly DependencyProperty IsFocusedProperty = 
            DependencyProperty.Register(
                "IsFocused", 
                typeof(bool),
                typeof(Slider),
                new PropertyMetadata(OnIsFocusedPropertyChanged)); 

        /// <summary>
        /// IsFocusedProperty property changed handler. 
        /// </summary> 
        /// <param name="d">Slider that changed IsFocused.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnIsFocusedPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            Slider s = d as Slider; 
            Debug.Assert(s != null);

            if (s.ElementRoot != null) 
            { 
                s.UpdateVisualState(); 
            } 
        }

	protected override void OnGotFocus (RoutedEventArgs e)
	{
	}

	protected override void OnLostFocus (RoutedEventArgs e)
	{
	}

        #endregion IsFocused 
 
        #region IsDirectionReversed
        /// <summary> 
        /// Gets a value that determines whether the direction is reversed.
        /// </summary>
        public bool IsDirectionReversed 
        {
            get { return (bool)GetValue(IsDirectionReversedProperty); }
            set { SetValue(IsDirectionReversedProperty, value); } 
        } 

        /// <summary> 
        /// Identifies the IsDirectionReversed dependency property.
        /// </summary>
        public static readonly DependencyProperty IsDirectionReversedProperty = 
            DependencyProperty.Register(
                "IsDirectionReversed",
                typeof(bool), 
                typeof(Slider), 
                new PropertyMetadata(OnIsDirectionReversedChanged));
 
        /// <summary>
        /// IsDirectionReversedProperty property changed handler.
        /// </summary> 
        /// <param name="d">Slider that changed IsDirectionReversed.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnIsDirectionReversedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        { 
            Slider s = d as Slider;
            Debug.Assert(s != null); 

            if (s.ElementRoot != null)
            { 
                s.UpdateTrackLayout();
            }
        } 
 
        #endregion IsDirectionReversed
 
        #region IsEnabled
        /// <summary> 
        /// Called when the IsEnabled property changes.
        /// </summary>
        /// <param name="isEnabled">New value of the IsEnabled property.</param> 
        internal void OnIsEnabledChanged(bool isEnabled)
        {
            if (ElementHorizontalLargeDecrease != null) 
            { 
                ElementHorizontalLargeDecrease.IsEnabled = isEnabled;
            } 
            if (ElementHorizontalThumb != null)
            {
                ElementHorizontalThumb.IsEnabled = isEnabled; 
            }
            if (ElementHorizontalLargeIncrease != null)
            { 
                ElementHorizontalLargeIncrease.IsEnabled = isEnabled; 
            }
            if (ElementVerticalLargeDecrease != null) 
            {
                ElementVerticalLargeDecrease.IsEnabled = isEnabled;
            } 
            if (ElementVerticalThumb != null)
            {
                ElementVerticalThumb.IsEnabled = isEnabled; 
            } 
            if (ElementVerticalLargeIncrease != null)
            { 
                ElementVerticalLargeIncrease.IsEnabled = isEnabled;
            }
 
            UpdateVisualState();

            if (isEnabled) 
            { 
                IsTabStop = true;
            } 
            else
            {
                IsTabStop = false; 
            }
        }
 
        #endregion IsEnabled 

        #region OverridePropertyChanges 

        /// <summary>
        /// Called when the Value property changes. 
        /// </summary>
        /// <param name="oldValue">Old value of the Value property.</param>
        /// <param name="newValue">New value of the Value property.</param> 
        protected override void OnValueChanged(double oldValue, double newValue) 
        {
            base.OnValueChanged(oldValue, newValue); 
            if (ElementRoot != null)
            {
                UpdateTrackLayout(); 
            }
        }
 
        /// <summary> 
        /// Called when the Minimum property changes.
        /// </summary> 
        /// <param name="oldMinimum">Old value of the Minimum property.</param>
        /// <param name="newMinimum">New value of the Minimum property.</param>
        protected override void OnMinimumChanged(double oldMinimum, double newMinimum) 
        {
            base.OnMinimumChanged(oldMinimum, newMinimum);
            if (ElementRoot != null) 
            { 
                UpdateTrackLayout();
            } 
        }

        /// <summary> 
        /// Called when the Maximum property changes.
        /// </summary>
        /// <param name="oldMaximum">Old value of the Maximum property.</param> 
        /// <param name="newMaximum">New value of the Maximum property.</param> 
        protected override void OnMaximumChanged(double oldMaximum, double newMaximum)
        { 
            base.OnMaximumChanged(oldMaximum, newMaximum);
            if (ElementRoot != null)
            { 
                UpdateTrackLayout();
            }
        } 
 
        #endregion OverridePropertyChanges
 
        #region MouseEvents
        /// <summary>
        /// Responds to the MouseEnter event. 
        /// </summary>
        /// <param name="e">The event data for the MouseEnter event.</param>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")] 
        protected override void OnMouseEnter(MouseEventArgs e) 
        {
//            e.Handled = true; 
            IsMouseOver = true;
            if ((Orientation == Orientation.Horizontal && ElementHorizontalThumb != null && !ElementHorizontalThumb.IsDragging) ||
                (Orientation == Orientation.Vertical && ElementVerticalThumb != null && !ElementVerticalThumb.IsDragging)) 
            {
                UpdateVisualState();
            } 
        } 

        /// <summary> 
        /// Responds to the MouseLeave event.
        /// </summary>
        /// <param name="e">The event data for the MouseLeave event.</param> 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")]
        protected override void OnMouseLeave(MouseEventArgs e)
        { 
//           e.Handled = true; 
            IsMouseOver = false;
            if ((Orientation == Orientation.Horizontal && ElementHorizontalThumb != null && !ElementHorizontalThumb.IsDragging) || 
                (Orientation == Orientation.Vertical && ElementVerticalThumb != null && !ElementVerticalThumb.IsDragging))
            {
                UpdateVisualState(); 
            }
        }
 
        /// <summary> 
        /// Responds to the MouseLeftButtonDown event.
        /// </summary> 
        /// <param name="e">The event data for the MouseLeftButtonDown event.</param>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")]
        protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e) 
        {
            e.Handled = true;
            Focus(); 
            CaptureMouse(); 
        }
 
        /// <summary>
        /// Responds to the MouseLeftButtonUp event.
        /// </summary> 
        /// <param name="e">The event data for the MouseLeftButtonUp event.</param>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")]
        protected override void OnMouseLeftButtonUp(MouseButtonEventArgs e) 
        { 
            e.Handled = true;
            ReleaseMouseCapture(); 
            UpdateVisualState();
        }
        #endregion MouseEvents 

        #region KeyEvents
        /// <summary> 
        /// Responds to the KeyDown event. 
        /// </summary>
        /// <param name="e">The event data for the KeyDown event.</param> 
        protected override void OnKeyDown(KeyEventArgs e)
        {
            if (!IsEnabled) 
            {
                return;
            } 
 
            if (e.Key == Key.Left || e.Key == Key.Down)
            { 
                if (IsDirectionReversed)
                {
                    Value += SmallChange; 
                }
                else
                { 
                    Value -= SmallChange; 
                }
            } 
            else if (e.Key == Key.Right || e.Key == Key.Up)
            {
                if (IsDirectionReversed) 
                {
                    Value -= SmallChange;
                } 
                else 
                {
                    Value += SmallChange; 
                }
            }
            else if (e.Key == Key.Home) 
            {
                Value = Minimum;
            } 
            else if (e.Key == Key.End) 
            {
                Value = Maximum; 
            }
        }
        #endregion KeyEvents 

        #region Change State
        /// <summary> 
        /// Update the current visual state of the slider. 
        /// </summary>
        internal void UpdateVisualState() 
        {
            if (!IsEnabled)
            { 
                ChangeVisualState(StateDisabled ?? StateNormal);
            }
            else 
            { 
                if (IsMouseOver)
                { 
                    ChangeVisualState(StateMouseOver ?? StateNormal);
                }
                else 
                {
                    ChangeVisualState(StateNormal);
                } 
            } 

            if (ElementFocusVisual != null) 
            {
                ElementFocusVisual.Visibility = (IsFocused && IsEnabled) ?
                    Visibility.Visible : Visibility.Collapsed; 
            }
        }
 
        /// <summary> 
        /// Change the visual state of the Slider.
        /// </summary> 
        /// <param name="state">Next visual state of the Slider.</param>
        /// <remarks>
        /// This method should not be called by controls to force a change to 
        /// the current visual state.  UpdateVisualState is preferred because
        /// it properly handles suspension of state changes.
        /// </remarks> 
        internal void ChangeVisualState(Storyboard state) 
        {
            Storyboard previousState = _currentState; 
            if (state == previousState)
            {
                return; 
            }

            if (state != null) 
            { 
                if (previousState != null)
                { 
                    previousState.Stop();
                }
                _currentState = state; 
                state.Begin();
            }
        } 
        #endregion Change State 

        #region ThumbDragDelta 

        private void OnThumbDragStarted(Object sender, DragStartedEventArgs e)
        { 
            this._dragValue = this.Value;
        }
 
        /// <summary> 
        /// Whenever the thumb gets dragged, we handle the event through
        /// this function to update the current value depending upon the 
        /// thumb drag delta.
        /// </summary>
        /// <param name="e">DragEventArgs</param> 
        private void OnThumbDragDelta(Object sender, DragDeltaEventArgs e)
        {
            double offset = 0; 
 
            if (Orientation == Orientation.Horizontal && ElementHorizontalThumb != null)
            { 
                offset = e.HorizontalChange / (ActualWidth - ElementHorizontalThumb.ActualWidth) * (Maximum - Minimum);
            }
            else if (Orientation == Orientation.Vertical && ElementVerticalThumb != null) 
            {
                offset = -e.VerticalChange / (ActualHeight - ElementVerticalThumb.ActualHeight) * (Maximum - Minimum);
            } 
 
            if (!double.IsNaN(offset) && !double.IsInfinity(offset))
            { 
                _dragValue += IsDirectionReversed ? -offset : offset;

                double newValue = Math.Min(Maximum, Math.Max(Minimum, _dragValue)); 

                if (newValue != Value)
                { 
                    Value = newValue; 
                }
            } 
        }

        #endregion ThumbDragDelta 

        #region UpdateTrackLayout
 
        /// <summary> 
        /// This code will run whenever Orientation changes, to change the template
        /// being used to display this control. 
        /// </summary>
        private void OnOrientationChanged()
        { 
            if (ElementHorizontalTemplate != null)
            {
                ElementHorizontalTemplate.Visibility = (Orientation == Orientation.Horizontal ? Visibility.Visible : Visibility.Collapsed); 
            } 
            if (ElementVerticalTemplate != null)
            { 
                ElementVerticalTemplate.Visibility = (Orientation == Orientation.Horizontal ? Visibility.Collapsed : Visibility.Visible);
            }
            UpdateTrackLayout(); 
        }

        /// <summary> 
        /// This method will take the current min, max, and value to 
        /// calculate and layout the current control measurements.
        /// </summary> 
        private void UpdateTrackLayout()
        {
            double maximum = Maximum; 
            double minimum = Minimum;
            double value = Value;
            double multiplier = 1 - (maximum - value) / (maximum - minimum); 
 
            Grid templateGrid = (Orientation == Orientation.Horizontal) ? (ElementHorizontalTemplate as Grid) : (ElementVerticalTemplate as Grid);
            if (templateGrid != null) 
            {
                if (Orientation == Orientation.Horizontal && templateGrid.ColumnDefinitions != null &&
                    templateGrid.ColumnDefinitions.Count == 3) 
                {
                    templateGrid.ColumnDefinitions[0].Width = new GridLength(1, IsDirectionReversed ? GridUnitType.Star : GridUnitType.Auto);
                    templateGrid.ColumnDefinitions[2].Width = new GridLength(1, IsDirectionReversed ? GridUnitType.Auto : GridUnitType.Star); 
                    if (ElementHorizontalLargeDecrease != null) 
                    {
                        ElementHorizontalLargeDecrease.SetValue(Grid.ColumnProperty, IsDirectionReversed ? 2 : 0); 
                    }
                    if (ElementHorizontalLargeIncrease != null)
                    { 
                        ElementHorizontalLargeIncrease.SetValue(Grid.ColumnProperty, IsDirectionReversed ? 0 : 2);
                    }
                } 
                else if (Orientation == Orientation.Vertical && templateGrid.RowDefinitions != null && 
                    templateGrid.RowDefinitions.Count == 3)
                { 
                    templateGrid.RowDefinitions[0].Height = new GridLength(1, IsDirectionReversed ? GridUnitType.Auto : GridUnitType.Star);
                    templateGrid.RowDefinitions[2].Height = new GridLength(1, IsDirectionReversed ? GridUnitType.Star : GridUnitType.Auto);
                    if (ElementVerticalLargeDecrease != null) 
                    {
                        ElementVerticalLargeDecrease.SetValue(Grid.RowProperty, IsDirectionReversed ? 0 : 2);
                    } 
                    if (ElementVerticalLargeIncrease != null) 
                    {
                        ElementVerticalLargeIncrease.SetValue(Grid.RowProperty, IsDirectionReversed ? 2 : 0); 
                    }
                }
            } 

            if (Orientation == Orientation.Horizontal && ElementHorizontalLargeDecrease != null && ElementHorizontalThumb != null)
            { 
                ElementHorizontalLargeDecrease.Width = multiplier * (ActualWidth - ElementHorizontalThumb.ActualWidth); 
            }
            else if (Orientation == Orientation.Vertical && ElementVerticalLargeDecrease != null && ElementVerticalThumb != null) 
            {
                ElementVerticalLargeDecrease.Height = multiplier * (ActualHeight - ElementVerticalThumb.ActualHeight);
            } 
        }

        #endregion UpdateTrackLayout 
 
	protected override AutomationPeer OnCreateAutomationPeer ()
	{
		throw new NotImplementedException ();
	}

        #region Template Parts
        /// <summary> 
        /// Root of the thumb template.
        /// </summary>
        internal FrameworkElement ElementRoot { get; set; } 
        internal const string ElementRootName = "RootElement";

        /// <summary> 
        /// Horizontal template root 
        /// </summary>
        internal virtual FrameworkElement ElementHorizontalTemplate { get; set; } 
        internal const string ElementHorizontalTemplateName = "HorizontalTemplateElement";

        /// <summary> 
        /// Large increase repeat button
        /// </summary>
        internal virtual RepeatButton ElementHorizontalLargeIncrease { get; set; } 
        internal const string ElementHorizontalLargeIncreaseName = "HorizontalTrackLargeChangeIncreaseRepeatButtonElement"; 

        /// <summary> 
        /// Large decrease repeat button
        /// </summary>
        internal virtual RepeatButton ElementHorizontalLargeDecrease { get; set; } 
        internal const string ElementHorizontalLargeDecreaseName = "HorizontalTrackLargeChangeDecreaseRepeatButtonElement";

        /// <summary> 
        /// Thumb for dragging track 
        /// </summary>
        internal virtual Thumb ElementHorizontalThumb { get; set; } 
        internal const string ElementHorizontalThumbName = "HorizontalThumbElement";

        /// <summary> 
        /// Vertical template root
        /// </summary>
        internal virtual FrameworkElement ElementVerticalTemplate { get; set; } 
        internal const string ElementVerticalTemplateName = "VerticalTemplateElement"; 

        /// <summary> 
        /// Large increase repeat button
        /// </summary>
        internal virtual RepeatButton ElementVerticalLargeIncrease { get; set; } 
        internal const string ElementVerticalLargeIncreaseName = "VerticalTrackLargeChangeIncreaseRepeatButtonElement";

        /// <summary> 
        /// Large decrease repeat button 
        /// </summary>
        internal virtual RepeatButton ElementVerticalLargeDecrease { get; set; } 
        internal const string ElementVerticalLargeDecreaseName = "VerticalTrackLargeChangeDecreaseRepeatButtonElement";

        /// <summary> 
        /// Thumb for dragging track
        /// </summary>
        internal virtual Thumb ElementVerticalThumb { get; set; } 
        internal const string ElementVerticalThumbName = "VerticalThumbElement"; 

        /// <summary> 
        /// Focus indicator of the Slider template.
        /// </summary>
        internal FrameworkElement ElementFocusVisual { get; set; } 
        internal const string ElementFocusVisualName = "FocusVisualElement";

        /// <summary> 
        /// Transition into the Normal state in the Slider template. 
        /// </summary>
        internal Storyboard StateNormal { get; set; } 
        internal const string StateNormalName = "Normal State";

        /// <summary> 
        /// Transition into the MouseOver state in the Slider template.
        /// </summary>
        internal Storyboard StateMouseOver { get; set; } 
        internal const string StateMouseOverName = "MouseOver State"; 

        /// <summary> 
        /// Transition into the Disabled state in the Slider template.
        /// </summary>
        internal Storyboard StateDisabled { get; set; } 
        internal const string StateDisabledName = "Disabled State";

        #endregion Template Parts 
 
        #region Member Variables
        /// <summary> 
        /// Whether the mouse is currently over the control
        /// </summary>
        internal bool IsMouseOver { get; set; } 

        /// <summary>
        /// Current state of the control 
        /// </summary> 
        internal Storyboard _currentState;
 
        /// <summary>
        /// Accumulates drag offsets in case the mouse drags off the end of the track.
        /// </summary> 
        private double _dragValue;
        #endregion Member Variables
    } 
} 
