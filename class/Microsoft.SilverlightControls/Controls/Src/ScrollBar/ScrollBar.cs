// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Windows.Input; 
using System.Windows.Markup; 
using System.Windows.Media.Animation;
using System.Windows.Media;
using System.Windows.Controls;
//using System.Windows.Controls.Primitives;

namespace System.Windows.Controls.Primitives
{ 
    /// <summary>
    ///    ScrollBars are the UI widgets that both let a user drive scrolling from the UI
    ///    and indicate status of scrolled content. 
    ///    These are used inside the ScrollViewer. 
    ///    Their visibility is determined by the scroller visibility properties on ScrollViewer.
    /// </summary> 
    [TemplatePart(Name = ScrollBar.ElementRootName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = ScrollBar.ElementHorizontalTemplateName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = ScrollBar.ElementHorizontalLargeIncreaseName, Type = typeof(RepeatButton))] 
    [TemplatePart(Name = ScrollBar.ElementHorizontalLargeDecreaseName, Type = typeof(RepeatButton))]
    [TemplatePart(Name = ScrollBar.ElementHorizontalThumbName, Type = typeof(Thumb))]
    [TemplatePart(Name = ScrollBar.ElementVerticalTemplateName, Type = typeof(FrameworkElement))] 
    [TemplatePart(Name = ScrollBar.ElementVerticalLargeIncreaseName, Type = typeof(RepeatButton))] 
    [TemplatePart(Name = ScrollBar.ElementVerticalLargeDecreaseName, Type = typeof(RepeatButton))]
    [TemplatePart(Name = ScrollBar.ElementVerticalThumbName, Type = typeof(Thumb))] 
    [TemplatePart(Name = ScrollBar.StateNormalName, Type = typeof(Storyboard))]
    [TemplatePart(Name = ScrollBar.StateMouseOverName, Type = typeof(Storyboard))]
    [TemplatePart(Name = ScrollBar.StateDisabledName, Type = typeof(Storyboard))] 
    public sealed class ScrollBar : RangeBase
    {
        #region Constructor 
        /// <summary> 
        /// Constructor to setup the ScrollBar class
        /// </summary> 
        public ScrollBar()
        {
            Minimum = 0; 
            Value = 0;
            Maximum = 10;
            ViewportSize = 0; 
 
            IsEnabled = true;
            Orientation = Orientation.Horizontal; 
            MouseEnter += delegate(object sender, MouseEventArgs e) { OnMouseEnter(e); };
            MouseLeave += delegate(object sender, MouseEventArgs e) { OnMouseLeave(e); };
            MouseLeftButtonDown += delegate(object sender, MouseButtonEventArgs e) { OnMouseLeftButtonDown(e); }; 
            MouseLeftButtonUp += delegate(object sender, MouseButtonEventArgs e) { OnMouseLeftButtonUp(e); };
            SizeChanged += delegate { UpdateTrackLayout(GetTrackLength()); };
        } 
 
        /// <summary>
        /// Apply a template to the ScrollBar. 
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
        public override void OnApplyTemplate() 
        {
            base.OnApplyTemplate();
 
            // Get the parts 
            ElementRoot = GetTemplateChild(ElementRootName) as FrameworkElement;
            ElementHorizontalLargeIncrease = GetTemplateChild(ElementHorizontalLargeIncreaseName) as RepeatButton; 
            ElementHorizontalLargeDecrease = GetTemplateChild(ElementHorizontalLargeDecreaseName) as RepeatButton;
            ElementHorizontalSmallIncrease = GetTemplateChild(ElementHorizontalSmallIncreaseName) as RepeatButton;
            ElementHorizontalSmallDecrease = GetTemplateChild(ElementHorizontalSmallDecreaseName) as RepeatButton; 
            ElementHorizontalThumb = GetTemplateChild(ElementHorizontalThumbName) as Thumb;
            ElementVerticalTemplate = GetTemplateChild(ElementVerticalTemplateName) as FrameworkElement;
            ElementVerticalLargeIncrease = GetTemplateChild(ElementVerticalLargeIncreaseName) as RepeatButton; 
            ElementVerticalLargeDecrease = GetTemplateChild(ElementVerticalLargeDecreaseName) as RepeatButton; 
            ElementVerticalSmallIncrease = GetTemplateChild(ElementVerticalSmallIncreaseName) as RepeatButton;
            ElementVerticalSmallDecrease = GetTemplateChild(ElementVerticalSmallDecreaseName) as RepeatButton; 
            ElementVerticalThumb = GetTemplateChild(ElementVerticalThumbName) as Thumb;

            // Get the states 
            if (ElementRoot != null)
            {
                StateNormal = ElementRoot.Resources[StateNormalName] as Storyboard; 
                StateMouseOver = ElementRoot.Resources[StateMouseOverName] as Storyboard; 
                StateDisabled = ElementRoot.Resources[StateDisabledName] as Storyboard;
            } 

            if (ElementHorizontalThumb != null)
            {
                ElementHorizontalThumb.DragStarted += new DragStartedEventHandler(OnThumbDragStarted);
                ElementHorizontalThumb.DragDelta += new DragDeltaEventHandler(OnThumbDragDelta);
                ElementHorizontalThumb.DragCompleted += new DragCompletedEventHandler(OnThumbDragCompleted);
            } 
            if (ElementHorizontalLargeDecrease != null)
            { 
                ElementHorizontalLargeDecrease.Click += delegate
                {
                    Value -= LargeChange; 
                    RaiseScrollEvent(ScrollEventType.LargeDecrement);
                };
            } 
            if (ElementHorizontalLargeIncrease != null) 
            {
                ElementHorizontalLargeIncrease.Click += delegate 
                {
                    Value += LargeChange;
                    RaiseScrollEvent(ScrollEventType.LargeIncrement); 
                };
            }
            if (ElementHorizontalSmallDecrease != null) 
            { 
                ElementHorizontalSmallDecrease.Click += delegate
                { 
                    Value -= SmallChange;
                    RaiseScrollEvent(ScrollEventType.SmallDecrement);
                }; 
            }
            if (ElementHorizontalSmallIncrease != null)
            { 
                ElementHorizontalSmallIncrease.Click += delegate 
                {
                    Value += SmallChange; 
                    RaiseScrollEvent(ScrollEventType.SmallIncrement);
                };
            } 
            if (ElementVerticalThumb != null)
            {
                ElementVerticalThumb.DragStarted += new DragStartedEventHandler(OnThumbDragStarted);
                ElementVerticalThumb.DragDelta += new DragDeltaEventHandler(OnThumbDragDelta);
                ElementVerticalThumb.DragCompleted += new DragCompletedEventHandler(OnThumbDragCompleted);
            } 
            if (ElementVerticalLargeDecrease != null)
            {
                ElementVerticalLargeDecrease.Click += delegate 
                {
                    Value -= LargeChange;
                    RaiseScrollEvent(ScrollEventType.LargeDecrement); 
                }; 
            }
            if (ElementVerticalLargeIncrease != null) 
            {
                ElementVerticalLargeIncrease.Click += delegate
                { 
                    Value += LargeChange;
                    RaiseScrollEvent(ScrollEventType.LargeIncrement);
                }; 
            } 
            if (ElementVerticalSmallDecrease != null)
            { 
                ElementVerticalSmallDecrease.Click += delegate
                {
                    Value -= SmallChange; 
                    RaiseScrollEvent(ScrollEventType.SmallDecrement);
                };
            } 
            if (ElementVerticalSmallIncrease != null) 
            {
                ElementVerticalSmallIncrease.Click += delegate 
                {
                    Value += SmallChange;
                    RaiseScrollEvent(ScrollEventType.SmallIncrement); 
                };
            }
            // Updating states for parts where properties might have been updated through 
            // XAML before the template was loaded. 
            OnIsEnabledChanged(IsEnabled);
            OnOrientationChanged(); 
            UpdateVisualState();
        }
        #endregion Constructor 

        #region Orientation
        /// <summary> 
        /// Gets whether the ScrollBar has an orientation of vertical or horizontal. 
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
                typeof(ScrollBar),
                new PropertyMetadata(OnOrientationPropertyChanged));
 
        /// <summary> 
        /// OrientationProperty property changed handler.
        /// </summary> 
        /// <param name="d">ScrollBar that changed Orientation.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnOrientationPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            ScrollBar s = d as ScrollBar;
            Debug.Assert(s != null); 
 
            s.OnOrientationChanged();
        } 

        #endregion Orientation
 
        #region ViewportSize
        /// <summary>
        /// ViewportSize is the amount of the scrolled extent currently visible. 
        /// For most scrolled content, this value will be bound to one of ScrollViewer's 
        /// ViewportSize properties. This property is in logical scrolling units.
        /// </summary> 
        public double ViewportSize
        {
            get { return (double)GetValue(ViewportSizeProperty); } 
            set { SetValue(ViewportSizeProperty, value); }
        }
 
        /// <summary> 
        /// Identifies the ViewportSize dependency property.
        /// </summary> 
        public static readonly DependencyProperty ViewportSizeProperty =
            DependencyProperty.Register(
                "ViewportSize", 
                typeof(double),
                typeof(ScrollBar),
                new PropertyMetadata(OnViewportSizeChanged)); 
 
        private static void OnViewportSizeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            ScrollBar s = d as ScrollBar;
            Debug.Assert(s != null);
 
            s.UpdateTrackLayout(s.GetTrackLength());
        }
 
        #endregion ViewportSize 

        #region IsEnabled 
        /// <summary>
        /// Gets or sets a value that indicates whether this element is enabled
        /// in the user interface (UI). 
        /// </summary>
        public bool IsEnabled
        { 
            get { return (bool)GetValue(IsEnabledProperty); } 
            set { SetValue(IsEnabledProperty, value); }
        } 

        /// <summary>
        /// Identifies the IsEnabled dependency property. 
        /// </summary>
        public static readonly DependencyProperty IsEnabledProperty =
            DependencyProperty.Register( 
                "IsEnabled", 
                typeof(bool),
                typeof(ScrollBar), 
                new PropertyMetadata(OnIsEnabledPropertyChanged));

        /// <summary> 
        /// IsEnabledProperty property changed handler.
        /// </summary>
        /// <param name="d">ScrollBar that changed IsEnabled.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnIsEnabledPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            ScrollBar s = d as ScrollBar;
            Debug.Assert(s != null);
 
            bool value = (bool)e.NewValue;
            s.OnIsEnabledChanged(value);
        } 
 
        /// <summary>
        /// Called when the IsEnabled property changes. 
        /// </summary>
        /// <param name="isEnabled">New value of the IsEnabled property.</param>
        private void OnIsEnabledChanged(bool isEnabled) 
        {
            if (ElementHorizontalSmallDecrease != null)
            { 
                ElementHorizontalSmallDecrease.IsEnabled = isEnabled; 
            }
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
            if (ElementHorizontalSmallIncrease != null) 
            {
                ElementHorizontalSmallIncrease.IsEnabled = isEnabled;
            } 
            if (ElementVerticalSmallDecrease != null) 
            {
                ElementVerticalSmallDecrease.IsEnabled = isEnabled; 
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
            if (ElementVerticalSmallIncrease != null)
            { 
                ElementVerticalSmallIncrease.IsEnabled = isEnabled; 
            }
 
            UpdateVisualState();
        }
 
        #endregion IsEnabled

        #region OnValueChanged 
 
        /// <summary>
        /// Called when the Value property changes. 
        /// </summary>
        /// <param name="oldValue">Old value of the Value property.</param>
        /// <param name="newValue">New value of the Value property.</param> 
        protected override void OnValueChanged(double oldValue, double newValue)
        {
            double trackLength = GetTrackLength(); 
 
            base.OnValueChanged(oldValue, newValue);
 
            UpdateTrackLayout(trackLength);
        }
 
        /// <summary>
        /// Called when the Maximum property changes
        /// </summary> 
        /// <param name="oldMaximum">Old value of the Maximum property.</param> 
        /// <param name="newMaximum">New value of the Maximum property.</param>
        protected override void OnMaximumChanged(double oldMaximum, double newMaximum) 
        {
            double trackLength = GetTrackLength();
 
            base.OnMaximumChanged(oldMaximum, newMaximum);
            UpdateTrackLayout(trackLength);
        } 
 
        /// <summary>
        /// Called when the Minimum property changes 
        /// </summary>
        /// <param name="oldMinimum">Old value of the Minimum property.</param>
        /// <param name="newMinimum">New value of the Minimum property.</param> 
        protected override void OnMinimumChanged(double oldMinimum, double newMinimum)
        {
            double trackLength = GetTrackLength(); 
 
            base.OnMinimumChanged(oldMinimum, newMinimum);
            UpdateTrackLayout(trackLength); 
        }

        #endregion 

        #region MouseEvents
        /// <summary> 
        /// Responds to the MouseEnter event. 
        /// </summary>
        /// <param name="e">The event data for the MouseEnter event.</param> 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")]
        private void OnMouseEnter(MouseEventArgs e)
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
        private void OnMouseLeave(MouseEventArgs e) 
        {
//            e.Handled = true;
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
        private void OnMouseLeftButtonDown(MouseButtonEventArgs e) 
        {
            e.Handled = true; 
            CaptureMouse();
        }
 
        /// <summary>
        /// Responds to the MouseLeftButtonUp event.
        /// </summary> 
        /// <param name="e">The event data for the MouseLeftButtonUp event.</param> 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")]
        private void OnMouseLeftButtonUp(MouseButtonEventArgs e) 
        {
            e.Handled = true;
            ReleaseMouseCapture(); 
            UpdateVisualState();
        }
        #endregion MouseEvents 
 
        #region ScrollEvent
        /// <summary> 
        /// Event that gets fired when the ScrollBar's value is changed
        /// through the Thumb, RepeatButtons, or Keyboard interaction
        /// </summary> 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1009:DeclareEventHandlersCorrectly", Justification = "Derives from RoutedEventArgs instead of EventArgs")]
        internal event EventHandler<ScrollEventArgs> Scroll;
 
        /// <summary> 
        /// This raises the Scroll event, passing in the scrollEventType
        /// as a parameter to let the handler know what triggered this event. 
        /// </summary>
        /// <param name="scrollEventType">ScrollEventType</param>
        internal void RaiseScrollEvent(ScrollEventType scrollEventType) 
        {
            ScrollEventArgs newEvent = new ScrollEventArgs(scrollEventType, Value);
            newEvent.OriginalSource = this; 
 
            if (Scroll != null)
            { 
                Scroll(this, newEvent);
            }
        } 

        #endregion ScrollEvent
 
        #region Change State 
        /// <summary>
        /// Update the current visual state of the ScrollBar. 
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
        }

        /// <summary> 
        /// Change the visual state of the ScrollBar.
        /// </summary>
        /// <param name="state">Next visual state of the ScrollBar.</param> 
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

        #region ThumbDrag

        private void OnThumbDragCompleted(Object sender, DragCompletedEventArgs e) 
        {
            RaiseScrollEvent(ScrollEventType.EndScroll); 
        }

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
                offset = e.HorizontalChange / (GetTrackLength() - ElementHorizontalThumb.ActualWidth) * (Maximum - Minimum);
            } 
            else if (Orientation == Orientation.Vertical && ElementVerticalThumb != null)
            {
                offset = e.VerticalChange / (GetTrackLength() - ElementVerticalThumb.ActualHeight) * (Maximum - Minimum); 
            } 

            if (!double.IsNaN(offset) && !double.IsInfinity(offset)) 
            {
                _dragValue += offset;
 
                double newValue = Math.Min(Maximum, Math.Max(Minimum, _dragValue));

                if (newValue != Value) 
                { 
                    Value = newValue;
                    RaiseScrollEvent(ScrollEventType.ThumbTrack); 
                }
            }
        } 

        #endregion ThumbDrag
 
        #region UpdateTrackLayout 

        /// <summary> 
        /// This code will run whenever Orientation changes, to change the template
        /// being used to display this control.
        /// </summary> 
        private void OnOrientationChanged()
        {
            double trackLength = GetTrackLength(); 
 
            if (ElementHorizontalTemplate != null)
            { 
                ElementHorizontalTemplate.Visibility = (Orientation == Orientation.Horizontal ? Visibility.Visible : Visibility.Collapsed);
            }
            if (ElementVerticalTemplate != null) 
            {
                ElementVerticalTemplate.Visibility = (Orientation == Orientation.Horizontal ? Visibility.Collapsed : Visibility.Visible);
            } 
            UpdateTrackLayout(trackLength); 
        }
 
        /// <summary>
        /// This method will take the current min, max, and value to
        /// calculate and layout the current control measurements. 
        /// </summary>
        private void UpdateTrackLayout(double trackLength)
        { 
            double maximum = Maximum; 
            double minimum = Minimum;
            double value = Value; 
            double multiplier = (value - minimum) / (maximum - minimum);

            double thumbSize = UpdateThumbSize(trackLength); 

            if (Orientation == Orientation.Horizontal && ElementHorizontalLargeDecrease != null && ElementHorizontalThumb != null)
            { 
                ElementHorizontalLargeDecrease.Width = multiplier * (trackLength - thumbSize); 
            }
            else if (Orientation == Orientation.Vertical && ElementVerticalLargeDecrease != null && ElementVerticalThumb != null) 
            {
                ElementVerticalLargeDecrease.Height = multiplier * (trackLength - thumbSize);
            } 
        }

        /// <summary> 
        /// Based on the size of the Large Increase/Decrease RepeatButtons 
        /// and on the Thumb, we will calculate the size of the track area
        /// of the ScrollBar 
        /// </summary>
        /// <returns>The length of the track</returns>
        internal double GetTrackLength() 
        {
            double length = Double.NaN;
 
            if (Orientation == Orientation.Horizontal) 
            {
                length = this.ActualWidth; 

                if (ElementHorizontalSmallDecrease != null)
                { 
                    length -= ElementHorizontalSmallDecrease.ActualWidth;
                }
                if (ElementHorizontalSmallIncrease != null) 
                { 
                    length -= ElementHorizontalSmallIncrease.ActualWidth;
                } 
            }
            else
            { 
                length = this.ActualHeight;

                if (ElementVerticalSmallDecrease != null) 
                { 
                    length -= ElementVerticalSmallDecrease.ActualHeight;
                } 
                if (ElementVerticalSmallIncrease != null)
                {
                    length -= ElementVerticalSmallIncrease.ActualHeight; 
                }
            }
 
            return length; 
        }
 
        /// <summary>
        /// Based on the ViewportSize, the Track's length, and the
        /// Minimum and Maximum values, we will calculate the length 
        /// of the thumb.
        /// </summary>
        /// <returns>Double value representing display unit length</returns> 
        private double ConvertViewportSizeToDisplayUnits(double trackLength) 
        {
            double viewRangeValue = Maximum - Minimum; 

            return trackLength * ViewportSize / (ViewportSize + viewRangeValue);
        } 

        /// <summary>
        /// This will resize the Thumb, based on calculations with the ViewportSize, 
        /// the Track's length, and the Minimum and Maximum values. 
        /// </summary>
        internal double UpdateThumbSize(double trackLength) 
        {
            double result = Double.NaN;
 
            if((Orientation == Orientation.Horizontal && ElementHorizontalThumb != null) ||
                (Orientation == Orientation.Vertical && ElementVerticalThumb != null))
            { 
                if (trackLength > 0) 
                {
                    // hide the thumb if too big 
                    if (Maximum - Minimum == 0)
                    {
                        ElementHorizontalThumb.Visibility = Visibility.Collapsed; 
                        ElementVerticalThumb.Visibility = Visibility.Collapsed;
                    }
                    else 
                    { 
                        result = ConvertViewportSizeToDisplayUnits(trackLength);
 
                        ElementHorizontalThumb.Visibility = Visibility.Visible;
                        ElementVerticalThumb.Visibility = Visibility.Visible;
 
                        // calculate size
                        if (Orientation == Orientation.Horizontal)
                        { 
                            ElementHorizontalThumb.Width = result; 
                            result = ElementHorizontalThumb.ActualWidth;
                        } 
                        else
                        {
                            ElementVerticalThumb.Height = result; 
                            result = ElementVerticalThumb.ActualHeight;
                        }
                    } 
                } 
            }
 
            return result;
        }
 
        #endregion UpdateTrackLayout

        #region Template Parts 
 
        /// <summary>
        /// Root of the ScrollBar template. 
        /// </summary>
        internal FrameworkElement ElementRoot { get; set; }
        internal const string ElementRootName = "RootElement"; 

        /// <summary>
        /// Horizontal template root 
        /// </summary> 
        internal FrameworkElement ElementHorizontalTemplate { get; set; }
        internal const string ElementHorizontalTemplateName = "HorizontalRootElement"; 

        /// <summary>
        /// Large increase repeat button 
        /// </summary>
        internal RepeatButton ElementHorizontalLargeIncrease { get; set; }
        internal const string ElementHorizontalLargeIncreaseName = "HorizontalLargeIncreaseElement"; 
 
        /// <summary>
        /// Large decrease repeat button 
        /// </summary>
        internal RepeatButton ElementHorizontalLargeDecrease { get; set; }
        internal const string ElementHorizontalLargeDecreaseName = "HorizontalLargeDecreaseElement"; 

        /// <summary>
        /// Small increase repeat button 
        /// </summary> 
        internal RepeatButton ElementHorizontalSmallIncrease { get; set; }
        internal const string ElementHorizontalSmallIncreaseName = "HorizontalSmallIncreaseElement"; 

        /// <summary>
        /// Small decrease repeat button 
        /// </summary>
        internal RepeatButton ElementHorizontalSmallDecrease { get; set; }
        internal const string ElementHorizontalSmallDecreaseName = "HorizontalSmallDecreaseElement"; 
 
        /// <summary>
        /// Thumb for dragging track 
        /// </summary>
        internal Thumb ElementHorizontalThumb { get; set; }
        internal const string ElementHorizontalThumbName = "HorizontalThumbElement"; 

        /// <summary>
        /// Vertical template root 
        /// </summary> 
        internal FrameworkElement ElementVerticalTemplate { get; set; }
        internal const string ElementVerticalTemplateName = "VerticalRootElement"; 

        /// <summary>
        /// Large increase repeat button 
        /// </summary>
        internal RepeatButton ElementVerticalLargeIncrease { get; set; }
        internal const string ElementVerticalLargeIncreaseName = "VerticalLargeIncreaseElement"; 
 
        /// <summary>
        /// Large decrease repeat button 
        /// </summary>
        internal RepeatButton ElementVerticalLargeDecrease { get; set; }
        internal const string ElementVerticalLargeDecreaseName = "VerticalLargeDecreaseElement"; 

        /// <summary>
        /// Small increase repeat button 
        /// </summary> 
        internal RepeatButton ElementVerticalSmallIncrease { get; set; }
        internal const string ElementVerticalSmallIncreaseName = "VerticalSmallIncreaseElement"; 

        /// <summary>
        /// Small decrease repeat button 
        /// </summary>
        internal RepeatButton ElementVerticalSmallDecrease { get; set; }
        internal const string ElementVerticalSmallDecreaseName = "VerticalSmallDecreaseElement"; 
 
        /// <summary>
        /// Thumb for dragging track 
        /// </summary>
        internal Thumb ElementVerticalThumb { get; set; }
        internal const string ElementVerticalThumbName = "VerticalThumbElement"; 

        /// <summary>
        /// Transition into the Normal state in the ScrollBar template. 
        /// </summary> 
        internal Storyboard StateNormal { get; set; }
        internal const string StateNormalName = "Normal State"; 

        /// <summary>
        /// Transition into the MouseOver state in the ScrollBar template. 
        /// </summary>
        internal Storyboard StateMouseOver { get; set; }
        internal const string StateMouseOverName = "MouseOver State"; 
 
        /// <summary>
        /// Transition into the Disabled state in the ScrollBar template. 
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
