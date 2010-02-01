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
using System.Windows.Automation.Peers;
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
        public static readonly DependencyProperty HorizontalScrollBarVisibilityProperty = DependencyProperty.RegisterAttachedCore(
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
        public static readonly DependencyProperty VerticalScrollBarVisibilityProperty = DependencyProperty.RegisterAttachedCore(
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
        public static readonly DependencyProperty HorizontalOffsetProperty = DependencyProperty.RegisterReadOnlyCore ( 
            "HorizontalOffset", typeof(double), typeof(ScrollViewer), 
            new PropertyMetadata(new PropertyChangedCallback(OnScrollInfoDependencyPropertyChanged)));
 
        /// <summary>
        /// Gets the value of the viewport width of the content.
        /// </summary> 
        public double ViewportWidth
        {
            get { return (double)GetValue(ViewportWidthProperty); } 
            private set { SetValueImpl (ViewportWidthProperty, value); }
        } 
        /// <summary>
        /// Identifies the ViewportWidth dependency property. 
        /// </summary>
        public static readonly DependencyProperty ViewportWidthProperty = DependencyProperty.RegisterReadOnlyCore (
            "ViewportWidth", typeof(double), typeof(ScrollViewer), 
            new PropertyMetadata(new PropertyChangedCallback(OnScrollInfoDependencyPropertyChanged)));

        /// <summary> 
        /// Gets the value of the scrollable width of the content. 
        /// </summary>
        public double ScrollableWidth 
        {
            get { return (double)GetValue(ScrollableWidthProperty); }
	    internal set { SetValueImpl (ScrollableWidthProperty, value); }
        } 
        /// <summary>
        /// Identifies the ScrollableWidth dependency property.
        /// </summary> 
        public static readonly DependencyProperty ScrollableWidthProperty = DependencyProperty.RegisterReadOnlyCore ( 
            "ScrollableWidth", typeof(double), typeof(ScrollViewer),
            null);
//            new PropertyMetadata(new PropertyChangedCallback(OnReadOnlyDependencyPropertyChanged))); 

        /// <summary>
        /// Gets the value of the extent width of the content.
        /// </summary> 
        public double ExtentWidth
        {
            get { return (double)GetValue(ExtentWidthProperty); }
            private set { SetValueImpl (ExtentWidthProperty, value); }
        } 
        /// <summary>
        /// Identifies the ExtentWidth dependency property. 
        /// </summary>
        public static readonly DependencyProperty ExtentWidthProperty = DependencyProperty.RegisterReadOnlyCore (
            "ExtentWidth", typeof(double), typeof(ScrollViewer), 
            new PropertyMetadata(new PropertyChangedCallback(OnScrollInfoDependencyPropertyChanged)));

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
        public static readonly DependencyProperty ComputedHorizontalScrollBarVisibilityProperty = DependencyProperty.RegisterReadOnlyCore ( 
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
        public static readonly DependencyProperty VerticalOffsetProperty = DependencyProperty.RegisterReadOnlyCore (
            "VerticalOffset", typeof(double), typeof(ScrollViewer), 
            new PropertyMetadata(new PropertyChangedCallback(OnScrollInfoDependencyPropertyChanged)));

        /// <summary> 
        /// Gets the value of the viewport height of the content.
        /// </summary>
        public double ViewportHeight 
        { 
            get { return (double)GetValue(ViewportHeightProperty); }
            private set { SetValueImpl (ViewportHeightProperty, value); }
        } 
        /// <summary>
        /// Identifies the ViewportHeight dependency property.
        /// </summary> 
        public static readonly DependencyProperty ViewportHeightProperty = DependencyProperty.RegisterReadOnlyCore (
            "ViewportHeight", typeof(double), typeof(ScrollViewer),
            new PropertyMetadata(new PropertyChangedCallback(OnScrollInfoDependencyPropertyChanged)));
 
        /// <summary>
        /// Gets the value of the scrollable height of the content. 
        /// </summary>
        public double ScrollableHeight
        { 
            get { return (double)GetValue(ScrollableHeightProperty); }
            internal set { SetValueImpl (ScrollableHeightProperty, value); }
        }
        /// <summary> 
        /// Identifies the ScrollableHeight dependency property. 
        /// </summary>
        public static readonly DependencyProperty ScrollableHeightProperty = DependencyProperty.RegisterReadOnlyCore ( 
            "ScrollableHeight", typeof(double), typeof(ScrollViewer),
            null);
//            new PropertyMetadata(new PropertyChangedCallback(OnReadOnlyDependencyPropertyChanged))); 

        /// <summary> 
        /// Gets the value of the extent height of the content.
        /// </summary>
        public double ExtentHeight 
        { 
            get { return (double)GetValue(ExtentHeightProperty); }
            private set { SetValueImpl (ExtentHeightProperty, value); }
        } 
        /// <summary>
        /// Identifies the ViewportHeight dependency property.
        /// </summary> 
        public static readonly DependencyProperty ExtentHeightProperty = DependencyProperty.RegisterReadOnlyCore (
            "ExtentHeight", typeof(double), typeof(ScrollViewer),
            new PropertyMetadata(new PropertyChangedCallback(OnScrollInfoDependencyPropertyChanged)));
 
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
        public static readonly DependencyProperty ComputedVerticalScrollBarVisibilityProperty = DependencyProperty.RegisterReadOnlyCore (
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
        internal ScrollBar ElementHorizontalScrollBar { 
		get { return elementHorizontalScrollBar; }
		private set {
			if (elementHorizontalScrollBar != value) {
				ScrollBar oldValue = elementHorizontalScrollBar;
				elementHorizontalScrollBar = value;
				// UIA Event
				RaiseUIAScrollBarSet (AutomationOrientation.Horizontal, 
				                      oldValue, 
						      elementHorizontalScrollBar);
			}
		}
	} 
	private ScrollBar elementHorizontalScrollBar;
        private const string ElementHorizontalScrollBarName = "HorizontalScrollBar";

        /// <summary> 
        /// Reference to the vertical ScrollBar child.
        /// </summary>
        internal ScrollBar ElementVerticalScrollBar { 
		get { return elementVerticalScrollBar; }
		private set {
			if (elementVerticalScrollBar != value) {
				ScrollBar oldValue = elementVerticalScrollBar;
				elementVerticalScrollBar = value;
				// UIA Event
				RaiseUIAScrollBarSet (AutomationOrientation.Vertical, 
				                      oldValue, 
						      elementVerticalScrollBar);
			}
		}
	}
	private ScrollBar elementVerticalScrollBar;
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
                ElementScrollContentPresenter.CanHorizontallyScroll = HorizontalScrollBarVisibility != ScrollBarVisibility.Disabled;
                ElementScrollContentPresenter.CanVerticallyScroll = VerticalScrollBarVisibility != ScrollBarVisibility.Disabled;
                InvalidateScrollInfo ();
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
        /// Handles the ScrollBar.Scroll event and updates the UI.
        /// </summary>
        /// <param name="orientation">Orientation of the ScrollBar.</param> 
        /// <param name="e">A ScrollEventArgs that contains the event data.</param> 
        private void HandleScroll(Orientation orientation, ScrollEventArgs e)
        { 
            if (null != ElementScrollContentPresenter)
            {
                double scrollable = (Orientation.Horizontal == orientation) ?
                    ScrollableWidth :
                    ScrollableHeight;
                
                double offset = (Orientation.Horizontal == orientation) ?
                    ElementScrollContentPresenter.HorizontalOffset : 
                    ElementScrollContentPresenter.VerticalOffset;
                
                double viewportDimension = (Orientation.Horizontal == orientation) ? 
                    ElementScrollContentPresenter.ViewportWidth :
                    ElementScrollContentPresenter.ViewportHeight; 
                
                // Calculate new offset 
                double newValue = Math.Min (scrollable, offset);
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
                max_new_value = Math.Min (scrollable, max_new_value);
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
                        SetValueImpl (HorizontalOffsetProperty, max_new_value);
			// UIA Event
			RaiseOffsetChanged (ElementScrollContentPresenter.HorizontalOffset, AutomationOrientation.Horizontal);
                        if (null != ElementHorizontalScrollBar) 
                        {
                            // WPF's ScrollBar doesn't respond to TemplateBinding bound Value changes during the Scroll event
                            ElementHorizontalScrollBar.Value = max_new_value; 
                        }
                    }
                    else 
                    { 
                        SetValueImpl (VerticalOffsetProperty, max_new_value);
			// UIA Event
			RaiseOffsetChanged (ElementScrollContentPresenter.VerticalOffset, AutomationOrientation.Vertical);
                        if (null != ElementVerticalScrollBar) 
                        {
                            // WPF's ScrollBar doesn't respond to TemplateBinding bound Value changes during the Scroll event
                            ElementVerticalScrollBar.Value = max_new_value; 
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
        protected override void OnKeyDown(KeyEventArgs e) 
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

            base.OnKeyDown (e);
        }

        /// <summary> 
        /// Called when the user presses the left mouse button over the ListBoxItem.
        /// </summary>
        /// <param name="e">The event data.</param> 
        protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e) 
        {
            // Set focus to the ScrollViewer to capture key input for scrolling 
            if (!e.Handled && Focus())
            {
                e.Handled = true; 
            }
	    
            base.OnMouseLeftButtonDown (e);
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
                if (scrollViewer.ElementScrollContentPresenter != null) {
                    scrollViewer.ElementScrollContentPresenter.CanHorizontallyScroll = scrollViewer.HorizontalScrollBarVisibility != ScrollBarVisibility.Disabled;
                    scrollViewer.ElementScrollContentPresenter.CanVerticallyScroll = scrollViewer.VerticalScrollBarVisibility != ScrollBarVisibility.Disabled;
                }
                scrollViewer.UpdateScrollbarVisibility ();
            }
            else 
            {
                ListBox listBox = d as ListBox;
                if ((null != listBox) && (null != listBox.TemplateScrollViewer)) 
                {
                    // Push the attached property values from ListBox to ScrollViewer because
                    // it's not possible to set up corresponding Bindings in OnApplyTemplate 
                    listBox.TemplateScrollViewer.SetValue(e.Property, e.NewValue); 
                }
            } 
        }

        private static void OnScrollInfoDependencyPropertyChanged (DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            // Unnecessary?
        }
        
        public void InvalidateScrollInfo ()
        {
            ScrollContentPresenter p = ElementScrollContentPresenter;
            if (p != null) {
                ExtentHeight = p.ExtentHeight;
                ExtentWidth = p.ExtentWidth;
                ViewportHeight = p.ViewportHeight;
                ViewportWidth = p.ViewportWidth;
                UpdateScrollbarVisibility ();
            }
            // UIA Event
            RaiseViewportChangedEvent (ViewportWidth, ViewportHeight);
            if (Math.Max(0, ExtentHeight - ViewportHeight) != ScrollableHeight) {
                SetValueImpl (ScrollableHeightProperty, Math.Max(0, ExtentHeight - ViewportHeight));
                InvalidateMeasure ();
            }
            if (Math.Max(0, ExtentWidth - ViewportWidth) != ScrollableWidth) {
                SetValueImpl (ScrollableWidthProperty, Math.Max(0, ExtentWidth - ViewportWidth));
                InvalidateMeasure ();
            }
        }
        
        void UpdateScrollbarVisibility ()
        {
            if (ElementScrollContentPresenter  == null)
                return;
            
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

            if (horizontalVisibility != ComputedHorizontalScrollBarVisibility) {
                SetValueImpl (ComputedHorizontalScrollBarVisibilityProperty, horizontalVisibility); 
                RaiseVisibilityChangedEvent (horizontalVisibility, AutomationOrientation.Horizontal);
                InvalidateMeasure ();
            }
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
    
            if (verticalVisibility != ComputedVerticalScrollBarVisibility) {
                SetValueImpl (ComputedVerticalScrollBarVisibilityProperty, verticalVisibility);
                RaiseVisibilityChangedEvent (verticalVisibility, AutomationOrientation.Vertical);
                InvalidateMeasure ();
            }
        }

	protected override AutomationPeer OnCreateAutomationPeer ()
	{
		return new ScrollViewerAutomationPeer (this);
	}

	#region UIA Internal Events

	internal void RaiseVisibilityChangedEvent (Visibility visibility, AutomationOrientation orientation) 
	{
		if (UIAVisibilityChanged != null)
			UIAVisibilityChanged (this, new VisibilityEventArgs () {
			                                Visibility  = visibility,
						        Orientation = orientation });
	}

	internal void RaiseOffsetChanged (double offset, AutomationOrientation orientation) 
	{
		if (UIAOffsetChanged != null)
			UIAOffsetChanged (this, new OffsetEventArgs () {
			                            Offset = offset,
						    Orientation = orientation });
	}

	internal void RaiseViewportChangedEvent (double viewportWidth, double viewportHeight) 
	{
		if (UIAViewportChanged != null)
			UIAViewportChanged (this, new ViewportEventArgs () {
			                              ViewportWidth = viewportWidth,
			                              ViewportHeight = viewportHeight });
	}

	internal void RaiseUIAScrollBarSet (AutomationOrientation orientation, ScrollBar oldValue, ScrollBar newValue) 
	{
		if (UIAScrollBarSet != null)
			UIAScrollBarSet (this, new ScrollBarSetEventArgs () {
			                           Orientation = orientation,
			                           OldValue    = oldValue,
			                           NewValue    = newValue
					 });
	}

	internal event EventHandler<VisibilityEventArgs> UIAVisibilityChanged;
	internal event EventHandler<OffsetEventArgs> UIAOffsetChanged;
	internal event EventHandler<ViewportEventArgs> UIAViewportChanged;
	internal event EventHandler<ScrollBarSetEventArgs> UIAScrollBarSet;

	internal class VisibilityEventArgs : EventArgs {
		public Visibility Visibility { get; set; }
		public AutomationOrientation Orientation { get; set; }
	}

	internal class OffsetEventArgs : EventArgs {
		public double Offset { get; set; }
		public AutomationOrientation Orientation { get; set; }
	}

	internal class ViewportEventArgs : EventArgs {
		public double ViewportHeight { get; set; }
		public double ViewportWidth { get; set; }
	}

	internal class ScrollBarSetEventArgs : EventArgs {
		public AutomationOrientation Orientation { get; set; }
		public ScrollBar OldValue { get; set; }
		public ScrollBar NewValue { get; set; }
	}

	#endregion
    }
} 
