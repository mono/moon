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
            new PropertyMetadata(ScrollBarVisibility.Disabled, new PropertyChangedCallback(OnScrollBarVisibilityChanged)));
 
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
            new PropertyMetadata(ScrollBarVisibility.Disabled, new PropertyChangedCallback(OnScrollBarVisibilityChanged)));

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

        internal IScrollInfo ScrollInfo {
            get; set;
        } 

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
            UpdateScrollbarVisibility ();
        }

        [MonoTODO ("what does this do differently?")]
        protected override Size MeasureOverride (Size availableSize)
        {
            return base.MeasureOverride (availableSize);
        }

        void UpdateScrollBar (Orientation orientation, double value)
        {
            //bool previousReadOnlyDependencyPropertyChangesAllowed = _readOnlyDependencyPropertyChangesAllowed;
            try {
                //_readOnlyDependencyPropertyChangesAllowed = true;
                // Update relevant ScrollBar
                if (orientation == Orientation.Horizontal) {
                    SetValueImpl (HorizontalOffsetProperty, value);
                    // UIA Event
                    RaiseOffsetChanged (ScrollInfo.HorizontalOffset, AutomationOrientation.Horizontal);
                    if (ElementHorizontalScrollBar != null) {
                        // WPF's ScrollBar doesn't respond to TemplateBinding bound Value changes during the Scroll event
                        ElementHorizontalScrollBar.Value = value;
                    }
                } else {
                    SetValueImpl (VerticalOffsetProperty, value);
                    // UIA Event
                    RaiseOffsetChanged (ScrollInfo.VerticalOffset, AutomationOrientation.Vertical);
                    if (ElementVerticalScrollBar != null) {
                        // WPF's ScrollBar doesn't respond to TemplateBinding bound Value changes during the Scroll event
                        ElementVerticalScrollBar.Value = value;
                    }
                }
            } finally {
                //_readOnlyDependencyPropertyChangesAllowed = previousReadOnlyDependencyPropertyChangesAllowed; 
            }
        }

        void SetScrollOffset (Orientation orientation, double value)
        {
            if (ScrollInfo != null) {
                double scrollable = (orientation == Orientation.Horizontal) ? ScrollableWidth : ScrollableHeight;
                double clamped = Math.Max (value, 0);

                clamped = Math.Min (scrollable, clamped);

                // Update ScrollContentPresenter 
                if (orientation == Orientation.Horizontal)
                    ScrollInfo.SetHorizontalOffset (clamped);
                else
                    ScrollInfo.SetVerticalOffset (clamped);

                UpdateScrollBar (orientation, clamped);
            }
        }

        /// <summary> 
        /// Handles the ScrollBar.Scroll event and updates the UI.
        /// </summary>
        /// <param name="orientation">Orientation of the ScrollBar.</param> 
        /// <param name="e">A ScrollEventArgs that contains the event data.</param> 
        private void HandleScroll(Orientation orientation, ScrollEventArgs e)
        { 
            if (ScrollInfo != null) {
                bool horizontal = orientation == Orientation.Horizontal;
                
                // Calculate new offset 
                switch (e.ScrollEventType) {
                case ScrollEventType.ThumbPosition:
                case ScrollEventType.ThumbTrack:
                    SetScrollOffset (orientation, e.NewValue);
                    break;
                case ScrollEventType.LargeDecrement:
                    if (horizontal)
                        ScrollInfo.PageLeft ();
                    else
                        ScrollInfo.PageUp ();
                    break;
                case ScrollEventType.LargeIncrement:
                    if (horizontal)
                        ScrollInfo.PageRight ();
                    else
                        ScrollInfo.PageDown ();
                    break;
                case ScrollEventType.SmallDecrement:
                    if (horizontal)
                        ScrollInfo.LineLeft ();
                    else
                        ScrollInfo.LineUp ();
                    break;
                case ScrollEventType.SmallIncrement:
                    if (horizontal)
                        ScrollInfo.LineRight ();
                    else
                        ScrollInfo.LineDown ();
                    break;
                case ScrollEventType.First:
                    SetScrollOffset (orientation, double.MinValue);
                    break;
                case ScrollEventType.Last:
                    SetScrollOffset (orientation, double.MaxValue);
                    break;
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
                    bool handled = true;

                    switch (e.Key)
                    {
                        case Key.Up:
                            ScrollInfo.LineUp ();
                            break;
                        case Key.Down:
                            ScrollInfo.LineDown ();
                            break;
                        case Key.Left:
                            ScrollInfo.LineLeft ();
                            break; 
                        case Key.Right:
                            ScrollInfo.LineRight ();
                            break; 
                        case Key.PageUp:
                            ScrollInfo.PageUp ();
                            break;
                        case Key.PageDown:
                            ScrollInfo.PageDown ();
                            break;
                        case Key.Home:
                            if (!control)
                                SetScrollOffset (Orientation.Horizontal, double.MinValue);
                            else
                                SetScrollOffset (Orientation.Vertical, double.MinValue);
                            break;
                        case Key.End:
                            if (!control)
                                SetScrollOffset (Orientation.Horizontal, double.MaxValue);
                            else
                                SetScrollOffset (Orientation.Vertical, double.MaxValue);
                            break;
                        default:
                            handled = false;
                            break;
                    }

                    if (handled)
                        e.Handled = true;
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
 
        protected override void OnMouseWheel (MouseWheelEventArgs e)
        {
            base.OnMouseWheel (e);
        }

        /// <summary> 
        /// Scrolls the view in the specified direction.
        /// </summary> 
        /// <param name="key">Key corresponding to the direction.</param>
        /// <remarks>Similar to WPF's corresponding ScrollViewer method.</remarks>
        internal void ScrollInDirection(Key key) 
        {
            if (ScrollInfo != null) {
                switch (key) {
                case Key.Up:
                    ScrollInfo.LineUp ();
                    break;
                case Key.Down:
                    ScrollInfo.LineDown ();
                    break;
                case Key.Left:
                    ScrollInfo.LineLeft ();
                    break;
                case Key.Right:
                    ScrollInfo.LineRight ();
                    break;
                }
            }
        }
 
        /// <summary>
        /// Scrolls the content within the ScrollViewer to the specified horizontal offset position.
        /// </summary> 
        /// <param name="offset">The position that the content scrolls to.</param> 
        public void ScrollToHorizontalOffset(double offset)
        {
            SetScrollOffset (Orientation.Horizontal, offset);
        }
 
        /// <summary>
        /// Scrolls the content within the ScrollViewer to the specified vertical offset position.
        /// </summary> 
        /// <param name="offset">The position that the content scrolls to.</param> 
        public void ScrollToVerticalOffset(double offset)
        {
            SetScrollOffset (Orientation.Vertical, offset);
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
                if (scrollViewer.ScrollInfo != null) {
                    scrollViewer.ScrollInfo.CanHorizontallyScroll = scrollViewer.HorizontalScrollBarVisibility != ScrollBarVisibility.Disabled;
                    scrollViewer.ScrollInfo.CanVerticallyScroll = scrollViewer.VerticalScrollBarVisibility != ScrollBarVisibility.Disabled;
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
            if (ScrollInfo != null) {
                ExtentHeight = ScrollInfo.ExtentHeight;
                ExtentWidth = ScrollInfo.ExtentWidth;
                ViewportHeight = ScrollInfo.ViewportHeight;
                ViewportWidth = ScrollInfo.ViewportWidth;
                UpdateScrollBar (Orientation.Horizontal, ScrollInfo.HorizontalOffset);
                UpdateScrollBar (Orientation.Vertical, ScrollInfo.VerticalOffset);
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
                    horizontalVisibility = ScrollInfo == null || ScrollInfo.ExtentWidth <= ScrollInfo.ViewportWidth ? Visibility.Collapsed : Visibility.Visible;
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
                    verticalVisibility = ScrollInfo == null || ScrollInfo.ExtentHeight <= ScrollInfo.ViewportHeight ? Visibility.Collapsed : Visibility.Visible; 
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

	internal new event EventHandler<VisibilityEventArgs> UIAVisibilityChanged;
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
