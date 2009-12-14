// (c) Copyright Microsoft Corporation. 
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved. 

using System;
using System.Diagnostics; 
using System.Windows; 
using System.Windows.Input;
using System.Windows.Markup; 
using System.Windows.Media.Animation;
using System.Windows.Controls.Primitives;
using System.Windows.Media; 
using System.Windows.Automation.Peers;
using System.Windows.Controls;
 
namespace System.Windows.Controls
{ 
    /// <summary>
    /// Slider control lets the user select from a range of values by moving a slider.
    /// Slider is used to enable to user to gradually modify a value (range selection). 
    /// Slider is an easy and natural interface for users, because it provides good visual feedback.
    /// </summary>
    [TemplatePart(Name = Slider.ElementHorizontalTemplateName, Type = typeof(FrameworkElement))] 
    [TemplatePart(Name = Slider.ElementHorizontalLargeIncreaseName, Type = typeof(RepeatButton))] 
    [TemplatePart(Name = Slider.ElementHorizontalLargeDecreaseName, Type = typeof(RepeatButton))]
    [TemplatePart(Name = Slider.ElementHorizontalThumbName, Type = typeof(Thumb))] 
    [TemplatePart(Name = Slider.ElementVerticalTemplateName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = Slider.ElementVerticalLargeIncreaseName, Type = typeof(RepeatButton))]
    [TemplatePart(Name = Slider.ElementVerticalLargeDecreaseName, Type = typeof(RepeatButton))] 
    [TemplatePart(Name = Slider.ElementVerticalThumbName, Type = typeof(Thumb))]
    [TemplateVisualState(Name = VisualStates.StateNormal, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateMouseOver, GroupName = VisualStates.GroupCommon)] 
    [TemplateVisualState(Name = VisualStates.StateDisabled, GroupName = VisualStates.GroupCommon)] 
    [TemplateVisualState(Name = VisualStates.StateUnfocused, GroupName = VisualStates.GroupFocus)]
    [TemplateVisualState(Name = VisualStates.StateFocused, GroupName = VisualStates.GroupFocus)] 
    public partial class Slider : RangeBase
    {
        #region Constructor 

        /// <summary>
        /// Constructor to setup the Slider class 
        /// </summary> 
        public Slider()
        { 
            SizeChanged += delegate { UpdateTrackLayout(); };

            DefaultStyleKey = typeof(Slider); 
            IsEnabledChanged += OnIsEnabledChanged;
        }
 
        /// <summary> 
        /// Apply a template to the slider.
        /// </summary> 
        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate(); 

            // Get the parts
            ElementHorizontalTemplate = GetTemplateChild(ElementHorizontalTemplateName) as FrameworkElement; 
            ElementHorizontalLargeIncrease = GetTemplateChild(ElementHorizontalLargeIncreaseName) as RepeatButton; 
            ElementHorizontalLargeDecrease = GetTemplateChild(ElementHorizontalLargeDecreaseName) as RepeatButton;
            ElementHorizontalThumb = GetTemplateChild(ElementHorizontalThumbName) as Thumb; 
            ElementVerticalTemplate = GetTemplateChild(ElementVerticalTemplateName) as FrameworkElement;
            ElementVerticalLargeIncrease = GetTemplateChild(ElementVerticalLargeIncreaseName) as RepeatButton;
            ElementVerticalLargeDecrease = GetTemplateChild(ElementVerticalLargeDecreaseName) as RepeatButton; 
            ElementVerticalThumb = GetTemplateChild(ElementVerticalThumbName) as Thumb;

            if (ElementHorizontalThumb != null) 
            { 
                ElementHorizontalThumb.DragStarted += delegate(object sender, DragStartedEventArgs e) { this.Focus(); OnThumbDragStarted(); };
                ElementHorizontalThumb.DragDelta += delegate(object sender, DragDeltaEventArgs e) { OnThumbDragDelta(e); }; 
            }
            if (ElementHorizontalLargeDecrease != null)
            { 
                ElementHorizontalLargeDecrease.Click += delegate { this.Focus(); Value -= LargeChange; };
            }
            if (ElementHorizontalLargeIncrease != null) 
            { 
                ElementHorizontalLargeIncrease.Click += delegate { this.Focus(); Value += LargeChange; };
            } 

            if (ElementVerticalThumb != null)
            { 
                ElementVerticalThumb.DragStarted += delegate(object sender, DragStartedEventArgs e) { this.Focus(); OnThumbDragStarted(); };
                ElementVerticalThumb.DragDelta += delegate(object sender, DragDeltaEventArgs e) { OnThumbDragDelta(e); };
            } 
            if (ElementVerticalLargeDecrease != null) 
            {
                ElementVerticalLargeDecrease.Click += delegate { this.Focus(); Value -= LargeChange; }; 
            }
            if (ElementVerticalLargeIncrease != null)
            { 
                ElementVerticalLargeIncrease.Click += delegate { this.Focus(); Value += LargeChange; };
            }
            // Updating states for parts where properties might have been updated through 
            // XAML before the template was loaded. 
            OnOrientationChanged();
            ChangeVisualState(false); 
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
            DependencyProperty.RegisterCore( 
                "Orientation",
                typeof(Orientation),
                typeof(Slider), 
                new PropertyMetadata(Orientation.Horizontal, OnOrientationPropertyChanged));

        /// <summary> 
        /// OrientationProperty property changed handler. 
        /// </summary>
        /// <param name="d">Slider that changed Orientation.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnOrientationPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            Slider s = d as Slider;
            Debug.Assert(s != null);
 
            s.OnOrientationChanged(); 
        }
 
        #endregion Orientation

        #region IsFocused 
        /// <summary>
        /// Gets a value that determines whether this element has logical focus.
        /// </summary> 
        public bool IsFocused 
        {
            get { return (bool)GetValue(IsFocusedProperty); } 
            internal set { SetValueImpl(IsFocusedProperty, value); }
        }
 
        /// <summary>
        /// Identifies the IsFocused dependency property.
        /// </summary> 
        public static readonly DependencyProperty IsFocusedProperty = 
            DependencyProperty.RegisterReadOnlyCore(
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

            s.OnIsFocusChanged(e); 
        } 

        /// <summary> 
        /// Called when the IsFocused property changes.
        /// </summary>
        /// <param name="e"> 
        /// The data for DependencyPropertyChangedEventArgs.
        /// </param>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")] 
        internal virtual void OnIsFocusChanged(DependencyPropertyChangedEventArgs e) 
        {
            UpdateVisualState(); 
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
            DependencyProperty.RegisterCore(
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
 
            s.UpdateTrackLayout();
 
        }

        #endregion IsDirectionReversed 

        #region IsEnabled
 
        /// <summary> 
        /// Called when the IsEnabled property changes.
        /// </summary> 
        /// <param name="e">Property changed args</param>
        private void OnIsEnabledChanged(object sender, DependencyPropertyChangedEventArgs e)
        { 
            if (!IsEnabled)
            {
                IsMouseOver = false; 
            } 

            UpdateVisualState(); 
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
            UpdateTrackLayout(); 
        } 

        /// <summary> 
        /// Called when the Minimum property changes.
        /// </summary>
        /// <param name="oldMinimum">Old value of the Minimum property.</param> 
        /// <param name="newMinimum">New value of the Minimum property.</param>
        protected override void OnMinimumChanged(double oldMinimum, double newMinimum)
        { 
            base.OnMinimumChanged(oldMinimum, newMinimum); 
            UpdateTrackLayout();
        } 

        /// <summary>
        /// Called when the Maximum property changes. 
        /// </summary>
        /// <param name="oldMaximum">Old value of the Maximum property.</param>
        /// <param name="newMaximum">New value of the Maximum property.</param> 
        protected override void OnMaximumChanged(double oldMaximum, double newMaximum) 
        {
            base.OnMaximumChanged(oldMaximum, newMaximum); 
            UpdateTrackLayout();
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
            base.OnMouseEnter(e);
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
            base.OnMouseLeave(e); 
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
            base.OnMouseLeftButtonDown(e);
            if (e.Handled)
            { 
                return;
            }
            e.Handled = true; 
            Focus(); 
            CaptureMouse();
        } 
        #endregion MouseEvents 

        #region KeyEvents
        /// <summary> 
        /// Responds to the KeyPressed event.
        /// </summary>
        /// <param name="e">The event data for the KeyPressed event.</param> 
        protected override void OnKeyDown(KeyEventArgs e) 
        {
            base.OnKeyDown(e); 
            if (e.Handled)
            {
                return; 
            }

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
 
        #region Focus Handlers
        protected override void OnGotFocus(RoutedEventArgs e) 
        {
            base.OnGotFocus(e);
            IsFocused = true; 
        }

        protected override void OnLostFocus(RoutedEventArgs e) 
        { 
            base.OnLostFocus(e);
            IsFocused = false; 
        }
        #endregion

        protected override void OnLostMouseCapture (MouseEventArgs e)
        {
            base.OnLostMouseCapture (e);
            
            UpdateVisualState ();
        }
	
        #region Change State
        /// <summary>
        /// Update the current visual state of the slider. 
        /// </summary> 
        internal void UpdateVisualState()
        { 
            ChangeVisualState(true);
        }
 
        /// <summary>
        /// Change to the correct visual state for the Slider.
        /// </summary> 
        /// <param name="useTransitions"> 
        /// true to use transitions when updating the visual state, false to
        /// snap directly to the new visual state. 
        /// </param>
        internal void ChangeVisualState(bool useTransitions)
        { 
            if (!IsEnabled)
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateDisabled, VisualStates.StateNormal); 
            } 
            else if (IsMouseOver)
            { 
                VisualStates.GoToState(this, useTransitions, VisualStates.StateMouseOver, VisualStates.StateNormal);
            }
            else 
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateNormal);
            } 
 
            if (IsFocused && IsEnabled)
            { 
                VisualStates.GoToState(this, useTransitions, VisualStates.StateFocused, VisualStates.StateUnfocused);
            }
            else 
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateUnfocused);
            } 
        } 
        #endregion Change State
 
        #region ThumbDragDelta

        /// <summary> 
        /// Called whenever the Thumb drag operation is started
        /// </summary>
        private void OnThumbDragStarted() 
        { 
            this._dragValue = this.Value;
        } 

        /// <summary>
        /// Whenever the thumb gets dragged, we handle the event through 
        /// this function to update the current value depending upon the
        /// thumb drag delta.
        /// </summary> 
        /// <param name="e">DragEventArgs</param> 
        private void OnThumbDragDelta(DragDeltaEventArgs e)
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
        internal virtual void OnOrientationChanged() 
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
        internal virtual void UpdateTrackLayout()
        { 
            double maximum = Maximum; 
            double minimum = Minimum;
            double value = Value; 
            double multiplier = 1 - (maximum - value) / (maximum - minimum);

            Grid templateGrid = (Orientation == Orientation.Horizontal) ? (ElementHorizontalTemplate as Grid) : (ElementVerticalTemplate as Grid); 
            if (templateGrid != null)
            {
                if (Orientation == Orientation.Horizontal) 
                { 
                    if(templateGrid.ColumnDefinitions != null && templateGrid.ColumnDefinitions.Count == 3)
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
 
                    if (ElementHorizontalLargeDecrease != null && ElementHorizontalThumb != null)
                    {
                        ElementHorizontalLargeDecrease.Width = Math.Max(0, multiplier * (ActualWidth - ElementHorizontalThumb.ActualWidth)); 
                    } 
                }
                else 
                {
                    if (templateGrid.RowDefinitions != null && templateGrid.RowDefinitions.Count == 3)
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
 
                    if (ElementVerticalLargeDecrease != null && ElementVerticalThumb != null) 
                    {
                        ElementVerticalLargeDecrease.Height = multiplier * (ActualHeight - ElementVerticalThumb.ActualHeight); 
                    }
                }
            } 
        }

        #endregion UpdateTrackLayout 
 
        #region Template Parts
        /// <summary> 
        /// Horizontal template root
        /// </summary>
        internal virtual FrameworkElement ElementHorizontalTemplate { get; set; } 
        internal const string ElementHorizontalTemplateName = "HorizontalTemplate";

        /// <summary> 
        /// Large increase repeat button 
        /// </summary>
        internal virtual RepeatButton ElementHorizontalLargeIncrease { get; set; } 
        internal const string ElementHorizontalLargeIncreaseName = "HorizontalTrackLargeChangeIncreaseRepeatButton";

        /// <summary> 
        /// Large decrease repeat button
        /// </summary>
        internal virtual RepeatButton ElementHorizontalLargeDecrease { get; set; } 
        internal const string ElementHorizontalLargeDecreaseName = "HorizontalTrackLargeChangeDecreaseRepeatButton"; 

        /// <summary> 
        /// Thumb for dragging track
        /// </summary>
        internal virtual Thumb ElementHorizontalThumb { get; set; } 
        internal const string ElementHorizontalThumbName = "HorizontalThumb";

        /// <summary> 
        /// Vertical template root 
        /// </summary>
        internal virtual FrameworkElement ElementVerticalTemplate { get; set; } 
        internal const string ElementVerticalTemplateName = "VerticalTemplate";

        /// <summary> 
        /// Large increase repeat button
        /// </summary>
        internal virtual RepeatButton ElementVerticalLargeIncrease { get; set; } 
        internal const string ElementVerticalLargeIncreaseName = "VerticalTrackLargeChangeIncreaseRepeatButton"; 

        /// <summary> 
        /// Large decrease repeat button
        /// </summary>
        internal virtual RepeatButton ElementVerticalLargeDecrease { get; set; } 
        internal const string ElementVerticalLargeDecreaseName = "VerticalTrackLargeChangeDecreaseRepeatButton";

        /// <summary> 
        /// Thumb for dragging track 
        /// </summary>
        internal virtual Thumb ElementVerticalThumb { get; set; } 
        internal const string ElementVerticalThumbName = "VerticalThumb";

 
        #endregion Template Parts

        #region Member Variables 
        /// <summary> 
        /// Whether the mouse is currently over the control
        /// </summary> 
        internal bool IsMouseOver { get; set; }

        /// <summary> 
        /// Accumulates drag offsets in case the mouse drags off the end of the track.
        /// </summary>
        private double _dragValue; 
        #endregion Member Variables 
    }
} 
