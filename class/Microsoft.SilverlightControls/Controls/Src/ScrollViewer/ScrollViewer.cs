// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.Diagnostics; 
using System.Windows; 
using System.Windows.Controls;
using System.Windows.Controls.Primitives; 
using System.Windows.Input;
#if WPF
using PropertyChangedCallback = System.Windows.FrameworkPropertyMetadata; 
#endif

#if WPF 
namespace WPF 
#else
namespace System.Windows.Controls
#endif
{
    /// <summary> 
    /// Represents a scrollable area that can contain other visible elements.
    /// </summary>
    [TemplatePart(Name = ScrollViewer.ElementScrollContentPresenterName, Type = typeof(ScrollContentPresenter))] 
    [TemplatePart(Name = ScrollViewer.ElementHorizontalScrollBarName, Type = typeof(ScrollBar))] 
    [TemplatePart(Name = ScrollViewer.ElementVerticalScrollBarName, Type = typeof(ScrollBar))]
    public sealed class ScrollViewer : ContentControl 
    {
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
        public static readonly DependencyProperty HorizontalScrollBarVisibilityProperty = DependencyProperty.RegisterAttached(
            "HorizontalScrollBarVisibility", typeof(ScrollBarVisibility), typeof(ScrollViewer),
            new PropertyMetadata(new PropertyChangedCallback(OnScrollBarVisibilityChanged))); 
 
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
        public static readonly DependencyProperty VerticalScrollBarVisibilityProperty = DependencyProperty.RegisterAttached(
            "VerticalScrollBarVisibility", typeof(ScrollBarVisibility), typeof(ScrollViewer),
            new PropertyMetadata(new PropertyChangedCallback(OnScrollBarVisibilityChanged))); 

        /// <summary>
        /// Gets the value of the horizontal offset of the content. 
        /// </summary> 
        public double HorizontalOffset
        { 
            get { return (double)GetValue(HorizontalOffsetProperty); }
        }
        /// <summary> 
        /// Identifies the HorizontalOffset dependency property.
        /// </summary>
        public static readonly DependencyProperty HorizontalOffsetProperty = DependencyProperty.RegisterReadOnly ( 
            "HorizontalOffset", typeof(double), typeof(ScrollViewer), 
            null);
//            new PropertyMetadata(new PropertyChangedCallback(OnReadOnlyDependencyPropertyChanged))); 
 
        /// <summary>
        /// Gets the value of the viewport width of the content.
        /// </summary> 
        public double ViewportWidth
        {
            get { return (double)GetValue(ViewportWidthProperty); } 
        } 
        /// <summary>
        /// Identifies the ViewportWidth dependency property. 
        /// </summary>
        public static readonly DependencyProperty ViewportWidthProperty = DependencyProperty.RegisterReadOnly (
            "ViewportWidth", typeof(double), typeof(ScrollViewer), 
            null);
//            new PropertyMetadata(new PropertyChangedCallback(OnReadOnlyDependencyPropertyChanged))); 

        /// <summary> 
        /// Gets the value of the scrollable width of the content. 
        /// </summary>
        public double ScrollableWidth 
        {
            get { return (double)GetValue(ScrollableWidthProperty); }
        } 
        /// <summary>
        /// Identifies the ScrollableWidth dependency property.
        /// </summary> 
        public static readonly DependencyProperty ScrollableWidthProperty = DependencyProperty.RegisterReadOnly ( 
            "ScrollableWidth", typeof(double), typeof(ScrollViewer),
            null);
//            new PropertyMetadata(new PropertyChangedCallback(OnReadOnlyDependencyPropertyChanged))); 

        /// <summary>
        /// Gets the value of the extent width of the content.
        /// </summary> 
        public double ExtentWidth
        {
            get { return (double)GetValue(ExtentWidthProperty); } 
        } 
        /// <summary>
        /// Identifies the ExtentWidth dependency property. 
        /// </summary>
        public static readonly DependencyProperty ExtentWidthProperty = DependencyProperty.RegisterReadOnly (
            "ExtentWidth", typeof(double), typeof(ScrollViewer), 
            null);
//            new PropertyMetadata(new PropertyChangedCallback(OnReadOnlyDependencyPropertyChanged))); 

        /// <summary>
        /// Gets a value that indicates whether the horizontal ScrollBar is visible. 
        /// </summary>
        public Visibility ComputedHorizontalScrollBarVisibility
        { 
            get { return (Visibility)GetValue(ComputedHorizontalScrollBarVisibilityProperty); } 
        }
        /// <summary> 
        /// Identifies the ComputedHorizontalScrollBarVisibility dependency property.
        /// </summary>
        public static readonly DependencyProperty ComputedHorizontalScrollBarVisibilityProperty = DependencyProperty.RegisterReadOnly ( 
            "ComputedHorizontalScrollBarVisibility", typeof(Visibility), typeof(ScrollViewer),
            null);
//            new PropertyMetadata(new PropertyChangedCallback(OnReadOnlyDependencyPropertyChanged))); 
 
        /// <summary> 
        /// Gets the value of the vertical offset of the content.
        /// </summary> 
        public double VerticalOffset
        {
            get { return (double)GetValue(VerticalOffsetProperty); } 
        }
        /// <summary>
        /// Identifies the VerticalOffset dependency property. 
        /// </summary> 
        public static readonly DependencyProperty VerticalOffsetProperty = DependencyProperty.RegisterReadOnly (
            "VerticalOffset", typeof(double), typeof(ScrollViewer), 
            null);
//            new PropertyMetadata(new PropertyChangedCallback(OnReadOnlyDependencyPropertyChanged))); 

        /// <summary> 
        /// Gets the value of the viewport height of the content.
        /// </summary>
        public double ViewportHeight 
        { 
            get { return (double)GetValue(ViewportHeightProperty); }
        } 
        /// <summary>
        /// Identifies the ViewportHeight dependency property.
        /// </summary> 
        public static readonly DependencyProperty ViewportHeightProperty = DependencyProperty.RegisterReadOnly (
            "ViewportHeight", typeof(double), typeof(ScrollViewer),
            null);
//            new PropertyMetadata(new PropertyChangedCallback(OnReadOnlyDependencyPropertyChanged))); 
 
        /// <summary>
        /// Gets the value of the scrollable height of the content. 
        /// </summary>
        public double ScrollableHeight
        { 
            get { return (double)GetValue(ScrollableHeightProperty); }
        }
        /// <summary> 
        /// Identifies the ScrollableHeight dependency property. 
        /// </summary>
        public static readonly DependencyProperty ScrollableHeightProperty = DependencyProperty.RegisterReadOnly ( 
            "ScrollableHeight", typeof(double), typeof(ScrollViewer),
            null);
//            new PropertyMetadata(new PropertyChangedCallback(OnReadOnlyDependencyPropertyChanged))); 

        /// <summary> 
        /// Gets the value of the extent height of the content.
        /// </summary>
        public double ExtentHeight 
        { 
            get { return (double)GetValue(ExtentHeightProperty); }
        } 
        /// <summary>
        /// Identifies the ViewportHeight dependency property.
        /// </summary> 
        public static readonly DependencyProperty ExtentHeightProperty = DependencyProperty.RegisterReadOnly (
            "ExtentHeight", typeof(double), typeof(ScrollViewer),
            null);
//            new PropertyMetadata(new PropertyChangedCallback(OnReadOnlyDependencyPropertyChanged))); 
 
        /// <summary>
        /// Gets a value that indicates whether the vertical ScrollBar is visible.
        /// </summary> 
        public Visibility ComputedVerticalScrollBarVisibility 
        {
            get { return (Visibility)GetValue(ComputedVerticalScrollBarVisibilityProperty); } 
        }
        /// <summary>
        /// Identifies the ComputedVerticalScrollBarVisibility dependency property. 
        /// </summary>
        public static readonly DependencyProperty ComputedVerticalScrollBarVisibilityProperty = DependencyProperty.RegisterReadOnly (
            "ComputedVerticalScrollBarVisibility", typeof(Visibility), typeof(ScrollViewer), 
            null);
//            new PropertyMetadata(new PropertyChangedCallback(OnReadOnlyDependencyPropertyChanged))); 

        /// <summary> 
        /// Indicates whether the parent handles scrolling itself.
        /// </summary>
        internal bool TemplatedParentHandlesScrolling 
        {
            get
            { 
                return _templatedParentHandlesScrolling; 
            }
            set 
            {
                _templatedParentHandlesScrolling = value;
                // Convert from standalone mode to ListBox mode 
                IsTabStop = !_templatedParentHandlesScrolling;
            }
        } 
        private bool _templatedParentHandlesScrolling; 

        /// <summary> 
        /// Reference to the ScrollContentPresenter child.
        /// </summary>
        internal ScrollContentPresenter ElementScrollContentPresenter { get; set; } 
        private const string ElementScrollContentPresenterName = "ScrollContentPresenter";

        /// <summary> 
        /// Reference to the horizontal ScrollBar child. 
        /// </summary>
        private ScrollBar ElementHorizontalScrollBar { get; set; } 
        private const string ElementHorizontalScrollBarName = "HorizontalScrollBar";

        /// <summary> 
        /// Reference to the vertical ScrollBar child.
        /// </summary>
        private ScrollBar ElementVerticalScrollBar { get; set; } 
        private const string ElementVerticalScrollBarName = "VerticalScrollBar"; 

        /// <summary> 
        /// Tracks whether changes to read-only DependencyProperties are allowed
        /// </summary>
//        private bool _readOnlyDependencyPropertyChangesAllowed; 

        /// <summary>
        /// Gets the value of the HorizontalScrollBarVisibility dependency property from a given element. 
        /// </summary> 
        /// <param name="element">The element from which the property value is read.</param>
        /// <returns>The value of the HorizontalScrollBarVisibility dependency property.</returns> 
        public static ScrollBarVisibility GetHorizontalScrollBarVisibility(DependencyObject element)
        {
            if (null == element) 
            {
                throw new ArgumentNullException("element");
            } 
            return (ScrollBarVisibility)element.GetValue(HorizontalScrollBarVisibilityProperty); 
        }
 
        /// <summary>
        /// Sets the value of the HorizontalScrollBarVisibility dependency property to a given element.
        /// </summary> 
        /// <param name="element">The element on which to set the property value.</param>
        /// <param name="horizontalScrollBarVisibility">The property value to set.</param>
        public static void SetHorizontalScrollBarVisibility(DependencyObject element, ScrollBarVisibility horizontalScrollBarVisibility) 
        { 
            if (null == element)
            { 
                throw new ArgumentNullException("element");
            }
            element.SetValue(HorizontalScrollBarVisibilityProperty, horizontalScrollBarVisibility); 
        }

        /// <summary> 
        /// Gets the value of the VerticalScrollBarVisibility dependency property from a given element. 
        /// </summary>
        /// <param name="element">The element from which the property value is read.</param> 
        /// <returns>The value of the VerticalScrollBarVisibility  dependency property.</returns>
        public static ScrollBarVisibility GetVerticalScrollBarVisibility(DependencyObject element)
        { 
            if (null == element)
            {
                throw new ArgumentNullException("element"); 
            } 
            return (ScrollBarVisibility)element.GetValue(VerticalScrollBarVisibilityProperty);
        } 

        /// <summary>
        /// Sets the value of the VerticalScrollBarVisibility dependency property to a given element. 
        /// </summary>
        /// <param name="element">The element on which to set the property value.</param>
        /// <param name="verticalScrollBarVisibility">The property value to set.</param> 
        public static void SetVerticalScrollBarVisibility(DependencyObject element, ScrollBarVisibility verticalScrollBarVisibility) 
        {
            if (null == element) 
            {
                throw new ArgumentNullException("element");
            } 
            element.SetValue(VerticalScrollBarVisibilityProperty, verticalScrollBarVisibility);
        }
 
        /// <summary> 
        /// Initializes a new instance of the ScrollViewer class.
        /// </summary> 
        public ScrollViewer()
        {
		DefaultStyleKey = typeof (ScrollViewer);

#if WPF 
            KeyboardNavigation.SetDirectionalNavigation(this, KeyboardNavigationMode.Local);
            VerticalScrollBarVisibility = ScrollBarVisibility.Visible;
            IsTabStop = false; 
#else 
            // DirectionalNavigation not supported by Silverlight
            IsTabStop = true; 
#endif
            KeyDown += delegate(object sender, KeyEventArgs e)
            { 
                OnKeyDown(e);
            };
            MouseLeftButtonDown += delegate(object sender, MouseButtonEventArgs e) 
            { 
                OnMouseLeftButtonDown(e);
            }; 
        }

#if WPF 
        public override void OnApplyTemplate()
#else
        /// <summary> 
        /// Invoked whenever application code or internal processes call 
        /// ApplyTemplate.
        /// </summary> 
        public override void OnApplyTemplate()
#endif
        { 
            base.OnApplyTemplate();
            ElementScrollContentPresenter = GetTemplateChild(ElementScrollContentPresenterName) as ScrollContentPresenter;
            if (null != ElementScrollContentPresenter) 
            { 
                ElementScrollContentPresenter.ScrollOwner = this;
#if !WPF && false
                if (_templatedParentHandlesScrolling)
                {
                    // Convert from standalone mode to ListBox mode 
                    ElementScrollContentPresenter.TabNavigation = KeyboardNavigationMode.Once;
                }
#endif 
            } 
            ElementHorizontalScrollBar = GetTemplateChild(ElementHorizontalScrollBarName) as ScrollBar;
            if (null != ElementHorizontalScrollBar) 
            {
                ElementHorizontalScrollBar.Scroll += delegate(Object sender, System.Windows.Controls.Primitives.ScrollEventArgs e) { HandleScroll(Orientation.Horizontal, e); }; 
            } 
            ElementVerticalScrollBar = GetTemplateChild(ElementVerticalScrollBarName) as ScrollBar;
            if (null != ElementVerticalScrollBar)
            { 
                ElementVerticalScrollBar.Scroll += delegate (Object sender, System.Windows.Controls.Primitives.ScrollEventArgs e) { HandleScroll(Orientation.Vertical, e); }; 
            }
        }


        /// <summary> 
        /// Called to remeasure a control.
        /// </summary>
        /// <param name="availableSize">Measurement constraints, a control cannot return a size larger than the constraint.</param> 
        /// <returns>The size of the control.</returns> 
        protected override Size MeasureOverride(Size availableSize)
        { 
            // Call base implementation before making changes so ScrollContentPresenter will layout
            Size baseMeasureOverride = base.MeasureOverride(availableSize);
            if (null != ElementScrollContentPresenter) 
            {
                try
                { 
//                    _readOnlyDependencyPropertyChangesAllowed = true; 

                    // Update horizontal ScrollBar 
                    Visibility horizontalVisibility;
                    switch (HorizontalScrollBarVisibility)
                    { 
                        case ScrollBarVisibility.Visible:
                            horizontalVisibility = Visibility.Visible;
                            break; 
                        case ScrollBarVisibility.Disabled: 
                        case ScrollBarVisibility.Hidden:
                            horizontalVisibility = Visibility.Collapsed; 
                            break;
                        default:  // Avoids compiler warning about uninitialized variable
                        case ScrollBarVisibility.Auto: 
                            horizontalVisibility = ElementScrollContentPresenter.ExtentWidth <= ElementScrollContentPresenter.ViewportWidth ? Visibility.Collapsed : Visibility.Visible;
                            break;
                    } 
                    SetValueImpl (ViewportWidthProperty, ElementScrollContentPresenter.ViewportWidth); 
                    SetValueImpl (ScrollableWidthProperty, Math.Max(0, ElementScrollContentPresenter.ExtentWidth - ElementScrollContentPresenter.ViewportWidth));
                    SetValueImpl (ComputedHorizontalScrollBarVisibilityProperty, horizontalVisibility); 

                    // Update vertical ScrollBar
                    Visibility verticalVisibility; 
                    switch (VerticalScrollBarVisibility)
                    {
                        case ScrollBarVisibility.Visible: 
                            verticalVisibility = Visibility.Visible; 
                            break;
                        case ScrollBarVisibility.Disabled: 
                        case ScrollBarVisibility.Hidden:
                            verticalVisibility = Visibility.Collapsed;
                            break; 
                        default:  // Avoids compiler warning about uninitialized variable
                        case ScrollBarVisibility.Auto:
                            verticalVisibility = ElementScrollContentPresenter.ExtentHeight <= ElementScrollContentPresenter.ViewportHeight ? Visibility.Collapsed : Visibility.Visible; 
                            break; 
                    }
                    SetValueImpl (ViewportHeightProperty, ElementScrollContentPresenter.ViewportHeight); 
                    SetValueImpl (ScrollableHeightProperty, Math.Max(0, ElementScrollContentPresenter.ExtentHeight - ElementScrollContentPresenter.ViewportHeight));
                    SetValueImpl (ComputedVerticalScrollBarVisibilityProperty, verticalVisibility);
                } 
                finally
                {
//                    _readOnlyDependencyPropertyChangesAllowed = false; 
                } 
            }
            return baseMeasureOverride; 
        }

        /// <summary> 
        /// Handles the ScrollBar.Scroll event and updates the UI.
        /// </summary>
        /// <param name="orientation">Orientation of the ScrollBar.</param> 
        /// <param name="e">A ScrollEventArgs that contains the event data.</param> 
        private void HandleScroll(Orientation orientation, ScrollEventArgs e)
        { 
            if (null != ElementScrollContentPresenter)
            {
                // Calculate new offset 
                double newValue = (Orientation.Horizontal == orientation) ?
                    Math.Min(ElementScrollContentPresenter.HorizontalOffset, ScrollableWidth) :
                    Math.Min(ElementScrollContentPresenter.VerticalOffset, ScrollableHeight); 
                double viewportDimension = (Orientation.Horizontal == orientation) ? 
                    ElementScrollContentPresenter.ViewportWidth :
                    ElementScrollContentPresenter.ViewportHeight; 
                switch (e.ScrollEventType)
                {
                    case System.Windows.Controls.Primitives.ScrollEventType.ThumbPosition:
                    case System.Windows.Controls.Primitives.ScrollEventType.ThumbTrack:
                        newValue = e.NewValue;
                        break;
                    case System.Windows.Controls.Primitives.ScrollEventType.LargeDecrement: 
                        newValue -= viewportDimension;
                        break;
                    case System.Windows.Controls.Primitives.ScrollEventType.LargeIncrement:
                        newValue += viewportDimension;
                        break;
                    case System.Windows.Controls.Primitives.ScrollEventType.SmallDecrement:
                        newValue -= 16;  // Matches ScrollContentPresenter behavior
                        break;
                    case System.Windows.Controls.Primitives.ScrollEventType.SmallIncrement: 
                        newValue += 16;  // Matches ScrollContentPresenter behavior
                        break;
                    case System.Windows.Controls.Primitives.ScrollEventType.First:
                        newValue = double.MinValue;
                        break;
                    case System.Windows.Controls.Primitives.ScrollEventType.Last:
                        newValue = double.MaxValue;
                        break; 
                } 

		double max_new_value = Math.Max (newValue, 0);
                // Update ScrollContentPresenter 
                if (Orientation.Horizontal == orientation)
                {
//                    ElementScrollContentPresenter.HorizontalOffset = Math.Max(newValue, 0); 
                    ElementScrollContentPresenter.SetHorizontalOffset (max_new_value);
                }
                else
                { 
//                    ElementScrollContentPresenter.VerticalOffset = Math.Max(newValue, 0); 
                    ElementScrollContentPresenter.SetVerticalOffset (max_new_value);
                }
 
//                bool previousReadOnlyDependencyPropertyChangesAllowed = _readOnlyDependencyPropertyChangesAllowed;
                try
                { 
//                    _readOnlyDependencyPropertyChangesAllowed = true;
                    // Update relevant ScrollBar
                    if (Orientation.Horizontal == orientation) 
                    { 
                        SetValueImpl (HorizontalOffsetProperty, ElementScrollContentPresenter.HorizontalOffset);
                        if (null != ElementHorizontalScrollBar) 
                        {
                            // WPF's ScrollBar doesn't respond to TemplateBinding bound Value changes during the Scroll event
                            ElementHorizontalScrollBar.Value = ElementScrollContentPresenter.HorizontalOffset; 
                        }
                    }
                    else 
                    { 
                        SetValueImpl (VerticalOffsetProperty, ElementScrollContentPresenter.VerticalOffset);
                        if (null != ElementVerticalScrollBar) 
                        {
                            // WPF's ScrollBar doesn't respond to TemplateBinding bound Value changes during the Scroll event
                            ElementVerticalScrollBar.Value = ElementScrollContentPresenter.VerticalOffset; 
                        }
                    }
                } 
                finally 
                {
//                    _readOnlyDependencyPropertyChangesAllowed = previousReadOnlyDependencyPropertyChangesAllowed; 
                }
            }
        } 

        /// <summary>
        /// Responds to the KeyDown event. 
        /// </summary> 
        /// <param name="e">Provides data for KeyEventArgs.</param>
        private void OnKeyDown(KeyEventArgs e) 
        {
            if (!e.Handled)
            { 
                if (!TemplatedParentHandlesScrolling)
                {
                    // Parent is not going to handle scrolling; do so here 
                    bool control = (ModifierKeys.Control == (Keyboard.Modifiers & ModifierKeys.Control)); 
                    Orientation orientation = Orientation.Vertical;
                    System.Windows.Controls.Primitives.ScrollEventType scrollEventType = System.Windows.Controls.Primitives.ScrollEventType.ThumbTrack; 
                    switch (e.Key)
                    {
                        case Key.Up:
                            scrollEventType = System.Windows.Controls.Primitives.ScrollEventType.SmallDecrement;
                            break;
                        case Key.Down:
                            scrollEventType = System.Windows.Controls.Primitives.ScrollEventType.SmallIncrement; 
                            break;
                        case Key.Left: 
                            orientation = Orientation.Horizontal;
                            scrollEventType = System.Windows.Controls.Primitives.ScrollEventType.SmallDecrement;
                            break; 
                        case Key.Right:
                            orientation = Orientation.Horizontal;
                            scrollEventType = System.Windows.Controls.Primitives.ScrollEventType.SmallIncrement; 
                            break; 
                        case Key.PageUp:
                            scrollEventType = System.Windows.Controls.Primitives.ScrollEventType.LargeDecrement; 
                            break;
                        case Key.PageDown:
                            scrollEventType = System.Windows.Controls.Primitives.ScrollEventType.LargeIncrement; 
                            break;
                        case Key.Home:
                            if (!control) 
                            { 
                                orientation = Orientation.Horizontal;
                            }
                            scrollEventType = System.Windows.Controls.Primitives.ScrollEventType.First;
                            break;
                        case Key.End: 
                            if (!control)
                            {
                                orientation = Orientation.Horizontal; 
                            }
                            scrollEventType = System.Windows.Controls.Primitives.ScrollEventType.Last;
                            break; 
                    }
                    // If the key was handled above, perform the scroll action
                    if (System.Windows.Controls.Primitives.ScrollEventType.ThumbTrack != scrollEventType) 
                    {
                        HandleScroll(orientation, new System.Windows.Controls.Primitives.ScrollEventArgs(scrollEventType, 0));
                        e.Handled = true; 
                    } 
                }
            } 
        }

        /// <summary> 
        /// Called when the user presses the left mouse button over the ListBoxItem.
        /// </summary>
        /// <param name="e">The event data.</param> 
        private void OnMouseLeftButtonDown(MouseButtonEventArgs e) 
        {
            // Set focus to the ScrollViewer to capture key input for scrolling 
            if (!e.Handled && Focus())
            {
                e.Handled = true; 
            }
        }
 
        /// <summary> 
        /// Scrolls the view in the specified direction.
        /// </summary> 
        /// <param name="key">Key corresponding to the direction.</param>
        /// <remarks>Similar to WPF's corresponding ScrollViewer method.</remarks>
        internal void ScrollInDirection(Key key) 
        {
            switch (key)
            { 
                case Key.Up:
                    HandleScroll(Orientation.Vertical, new System.Windows.Controls.Primitives.ScrollEventArgs(System.Windows.Controls.Primitives.ScrollEventType.SmallDecrement, 0));
                    break; 
                case Key.Down:
                    HandleScroll(Orientation.Vertical, new System.Windows.Controls.Primitives.ScrollEventArgs(System.Windows.Controls.Primitives.ScrollEventType.SmallIncrement, 0));
                    break; 
                case Key.Left:
                    HandleScroll(Orientation.Horizontal, new System.Windows.Controls.Primitives.ScrollEventArgs(System.Windows.Controls.Primitives.ScrollEventType.SmallDecrement, 0));
                    break; 
                case Key.Right:
                    HandleScroll(Orientation.Horizontal, new System.Windows.Controls.Primitives.ScrollEventArgs(System.Windows.Controls.Primitives.ScrollEventType.SmallIncrement, 0));
                    break; 
            }
        }
 
        /// <summary>
        /// Scrolls the content within the ScrollViewer to the specified horizontal offset position.
        /// </summary> 
        /// <param name="offset">The position that the content scrolls to.</param> 
        public void ScrollToHorizontalOffset(double offset)
        {
            HandleScroll(Orientation.Horizontal, new System.Windows.Controls.Primitives.ScrollEventArgs(System.Windows.Controls.Primitives.ScrollEventType.ThumbPosition, offset));
        }
 
        /// <summary>
        /// Scrolls the content within the ScrollViewer to the specified vertical offset position.
        /// </summary> 
        /// <param name="offset">The position that the content scrolls to.</param> 
        public void ScrollToVerticalOffset(double offset)
        {
            HandleScroll(Orientation.Vertical, new System.Windows.Controls.Primitives.ScrollEventArgs(System.Windows.Controls.Primitives.ScrollEventType.ThumbPosition, offset));
        }
 
        /// <summary>
        /// Called when the HorizontalScrollBarVisibility/VerticalScrollBarVisibility property has changed.
        /// </summary> 
        /// <param name="d">The DependencyObject for which the property changed.</param> 
        /// <param name="e">Provides data for DependencyPropertyChangedEventArgs.</param>
        private static void OnScrollBarVisibilityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            ScrollViewer scrollViewer = d as ScrollViewer;
            if (null != scrollViewer) 
            {
                Debug.Assert(typeof(ScrollBarVisibility).IsInstanceOfType(e.OldValue));
                Debug.Assert(typeof(ScrollBarVisibility).IsInstanceOfType(e.NewValue)); 
                scrollViewer.InvalidateMeasure(); 
            }
            else 
            {
                ListBox listBox = d as ListBox;
                if ((null != listBox) && (null != listBox.ElementScrollViewer)) 
                {
                    // Push the attached property values from ListBox to ScrollViewer because
                    // it's not possible to set up corresponding Bindings in OnApplyTemplate 
                    listBox.ElementScrollViewer.SetValue(e.Property, e.NewValue); 
                }
            } 
        }
#if false
        /// <summary> 
        /// Implements a generic PropertyChangedCallback for read-only properties.
        /// </summary>
        /// <param name="d">The DependencyObject for which the property changed.</param> 
        /// <param name="e">Provides data for DependencyPropertyChangedEventArgs.</param> 
        private static void OnReadOnlyDependencyPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            ScrollViewer scrollViewer = d as ScrollViewer;
            Debug.Assert(null != scrollViewer);
            if (!scrollViewer._readOnlyDependencyPropertyChangesAllowed) 
            {
                throw new InvalidOperationException(Resource.ScrollViewer_OnReadOnlyDependencyPropertyChanged_ReadOnly);
            } 
        } 
#endif
    }
} 
