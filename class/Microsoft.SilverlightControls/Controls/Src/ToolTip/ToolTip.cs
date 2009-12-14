// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Windows.Markup; 
using System.Windows.Media; 
using System.Windows.Media.Animation;
using System.Windows.Controls.Primitives;
using System.Windows.Controls;

namespace System.Windows.Controls
{ 
    /// <summary>
    /// A control to display information when the user hovers over an owner control
    /// </summary> 
    [TemplatePart(Name = System.Windows.Controls.ToolTip.NormalStateName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = System.Windows.Controls.ToolTip.RootElementName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = System.Windows.Controls.ToolTip.VisibleStateName, Type = typeof(Storyboard))] 
    public partial class ToolTip : ContentControl
    {
        #region Constants 

        private const double TOOLTIP_tolerance = 2.0;
 
        #endregion Constants 

        #region HorizontalOffset Property 

        /// <summary>
        /// Determines a horizontal offset in pixels from the left side of 
        /// the mouse bounding rectangle to the left side of the ToolTip.
        /// </summary>
        public double HorizontalOffset 
        { 
            get { return (double)GetValue(HorizontalOffsetProperty);}
            set { SetValue(HorizontalOffsetProperty, value);} 
        }

        /// <summary> 
        /// Identifies the HorizontalOffset dependency property.
        /// </summary>
        public static readonly DependencyProperty HorizontalOffsetProperty = 
            DependencyProperty.RegisterCore( 
                "HorizontalOffset",
                typeof(double), 
                typeof(ToolTip),
                new PropertyMetadata(new PropertyChangedCallback(OnHorizontalOffsetPropertyChanged)));
 
        private static void OnHorizontalOffsetPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            // HorizontalOffset dependency property should be defined on a ToolTip 
            ToolTip toolTip = (ToolTip)d; 

            double newOffset = (double)e.NewValue; 
            // Working around temporary limitations in Silverlight:
            // perform inequality test
            // 

            if (newOffset != (double)e.OldValue)
            { 
                toolTip.OnOffsetChanged(newOffset, 0); 
            }
        } 

        #endregion HorizontalOffset Property

	#region PlacementTarget Property
        /// <summary>
        /// Determines a horizontal offset in pixels from the left side of 
        /// the mouse bounding rectangle to the left side of the ToolTip.
        /// </summary>
        public UIElement PlacementTarget
        { 
            get { return (UIElement)GetValue(PlacementTargetProperty);}
            set { SetValue(PlacementTargetProperty, value);} 
        }

        /// <summary> 
        /// Identifies the HorizontalOffset dependency property.
        /// </summary>
        public static readonly DependencyProperty PlacementTargetProperty = 
            DependencyProperty.RegisterCore( 
                "PlacementTarget",
                typeof(UIElement), 
                typeof(ToolTip),
                new PropertyMetadata(new PropertyChangedCallback(OnPlacementTargetPropertyChanged)));
 
        private static void OnPlacementTargetPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        } 
	#endregion PlacementTarget Property

	#region Placement Property
        /// <summary>
        /// Determines a horizontal offset in pixels from the left side of 
        /// the mouse bounding rectangle to the left side of the ToolTip.
        /// </summary>
        public PlacementMode Placement
        { 
            get { return (PlacementMode)GetValue(PlacementProperty);}
            set { SetValue(PlacementProperty, value);} 
        }

        /// <summary> 
        /// Identifies the HorizontalOffset dependency property.
        /// </summary>
        public static readonly DependencyProperty PlacementProperty = 
            DependencyProperty.RegisterCore( 
                "Placement",
                typeof(PlacementMode),
                typeof(ToolTip),
                new PropertyMetadata(new PropertyChangedCallback(OnPlacementPropertyChanged)));
 
        private static void OnPlacementPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        } 
	#endregion PlacementTarget Property

        #region IsOpen Property

        /// <summary> 
        /// Gets a value that determines whether tooltip is displayed or not. 
        /// </summary>
        public bool IsOpen 
        {
            get { return (bool)GetValue(IsOpenProperty);}
            set { SetValue(IsOpenProperty, value);} 
        }

        /// <summary> 
        /// Identifies the IsOpen dependency property. 
        /// </summary>
        public static readonly DependencyProperty IsOpenProperty = 
            DependencyProperty.RegisterCore(
                "IsOpen",
                typeof(bool), 
                typeof(ToolTip),
                new PropertyMetadata(new PropertyChangedCallback(OnIsOpenPropertyChanged)));
 
        private static void OnIsOpenPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            // IsOpen dependency property should be defined on a ToolTip 
            ToolTip toolTip = (ToolTip)d;

            if (((bool)e.NewValue != (bool)e.OldValue)) 
            {
                toolTip.OnIsOpenChanged((bool)e.NewValue);
            } 
        } 

        #endregion IsOpen Property 

        #region VerticalOffset Property
 
        /// <summary>
        /// Determines a vertical offset in pixels from the bottom of the
        /// mouse bounding rectangle to the top of the ToolTip. 
        /// </summary> 
        public double VerticalOffset
        { 
            get { return (double)GetValue(VerticalOffsetProperty);}
            set { SetValue(VerticalOffsetProperty, value);}
        } 

        /// <summary>
        /// Identifies the VerticalOffset dependency property. 
        /// </summary> 
        public static readonly DependencyProperty VerticalOffsetProperty =
            DependencyProperty.RegisterCore( 
                "VerticalOffset",
                typeof(double),
                typeof(ToolTip), 
                new PropertyMetadata(new PropertyChangedCallback(OnVerticalOffsetPropertyChanged)));

        private static void OnVerticalOffsetPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        { 
            // VerticalOffset dependency property should be defined on a ToolTip
            ToolTip toolTip = (ToolTip)d; 

            double newOffset = (double)e.NewValue;
            if (newOffset != (double)e.OldValue) 
            {
                toolTip.OnOffsetChanged(0, newOffset);
            } 
        } 

        #endregion VerticalOffset Property 

        #region Template Parts
 
        /// <summary>
        /// This storyboard runs when the ToolTip closes.
        /// </summary> 
        private Storyboard NormalState; 
        private const string NormalStateName = "Normal State";
 
        /// <summary>
        /// Part for the ToolTip.
        /// </summary> 
        internal FrameworkElement RootElement;
        internal const string RootElementName = "RootElement";
 
        /// <summary> 
        /// This storyboard runs when the ToolTip opens.
        /// </summary> 
        private Storyboard VisibleState;
        private const string VisibleStateName = "Visible State";
 
        #endregion Template Parts

        #region Events 
 
        /// <summary>
        /// Occurs when a ToolTip is closed and is no longer visible. 
        /// </summary>
        public event RoutedEventHandler Closed;
 
        /// <summary>
        /// Occurs when a ToolTip becomes visible.
        /// </summary> 
        public event RoutedEventHandler Opened; 

        #endregion Events 

        #region Data
 
        private bool _beginClosing;
        private bool _closingCompleted = true;
        private Size _lastSize; 
        private bool _openingCompleted = true; 
        private bool _openPopup;
        private Popup _parentPopup; 

        private delegate void PerformOnNextTick();
 
        internal Popup ParentPopup
        {
            get { return this._parentPopup; } 
            private set { this._parentPopup = value; } 
        }
 
        #endregion Data

        /// <summary> 
        /// Creates a default ToolTip element
        /// </summary>
        public ToolTip() : base() 
        { 
	    DefaultStyleKey = typeof (ToolTip);
            this.SizeChanged += new SizeChangedEventHandler(OnToolTipSizeChanged);
        } 

        #region Protected Methods
 
        /// <summary>
        /// Apply a template to the ToolTip, invoked from ApplyTemplate
        /// </summary> 
        public override void OnApplyTemplate() 
        {
            base.OnApplyTemplate(); 

            // If the part is not present in the template,
            // don't display content, but don't throw either 

            // get the element
            RootElement = GetTemplateChild(System.Windows.Controls.ToolTip.RootElementName) as FrameworkElement; 
 
            if (RootElement != null)
            { 
                // get the states
                this.VisibleState = RootElement.Resources[VisibleStateName] as Storyboard;
                this.NormalState = RootElement.Resources[NormalStateName] as Storyboard; 

                if (this.VisibleState != null)
                { 
                    this.VisibleState.Completed += new EventHandler(OnOpeningCompleted); 

                    // first time through when the opened event is fired, the storyboard is not 
                    // loaded from the template yet, because ApplyTemplate wasn't called yet,
                    // so I start the storyboard manually
                    // 

                    OnPopupOpened(null, EventArgs.Empty);
                } 
 
                if (this.NormalState != null)
                { 
                    this.NormalState.Completed += new EventHandler(OnClosingCompleted);
                }
            } 
        }

        #endregion Protected Methods 
 
        #region Private Methods
 
        private void BeginClosing()
        {
            this._beginClosing = false; 

            // close the popup after the animation is completed
            if (this.NormalState != null) 
            { 
                this._closingCompleted = false;
                this.NormalState.Begin(); 
            }
        }
 
        private void HookupParentPopup()
        {
            Debug.Assert(this._parentPopup == null, "this._parentPopup should be null, we want to set visual tree once"); 
 
            this._parentPopup = new Popup();
 
            this.IsTabStop = false;

            this._parentPopup.Child = this; 

            // Working around temporary limitations in Silverlight:
            // set IsHitTestVisible on both the popup and the child 
            // 
            this._parentPopup.IsHitTestVisible = false;
            this.IsHitTestVisible = false; 

            //
 


 
 

        } 

        private void OnClosed(RoutedEventArgs e)
        { 
            RoutedEventHandler snapshot = this.Closed;
            if (snapshot != null)
            { 
                snapshot(this, e); 
            }
        } 

        // called when the closing state transition is completed
        private void OnClosingCompleted(object sender, EventArgs e) 
        {
            this._closingCompleted = true;
            this._parentPopup.IsOpen = false; 
 
            // Working around temporary limitations in Silverlight:
            // send the event manually 
            //

            this.Dispatcher.BeginInvoke(delegate() { OnPopupClosed (null, EventArgs.Empty); }); 

            // if the tooltip was forced to stop the closing animation, because it has to reopen,
            // proceed with open 
            if (this._openPopup) 
            {
		    this.Dispatcher.BeginInvoke(delegate() { OpenPopup(); }); 
            }
        }
 
        private void OnIsOpenChanged(bool isOpen)
        {
            if (isOpen) 
            { 
                if (this._parentPopup == null)
                { 
                    HookupParentPopup();
                    OpenPopup();
                    return; 
                }

                if (!this._closingCompleted) 
                { 
                    Debug.Assert(this.NormalState != null);
 
                    // Completed event for the closing storyboard will open the parent popup
                    // because _openPopup is set
                    this._openPopup = true; 

                    this.NormalState.SkipToFill();
                    return; 
                } 

                PerformPlacement(this.HorizontalOffset, this.VerticalOffset); 
                OpenPopup();
            }
            else 
            {
                Debug.Assert(this._parentPopup != null);
 
                if (!this._openingCompleted) 
                {
                    if (this.NormalState != null) 
                    {
                        this._beginClosing = true;
                    } 
                    this.VisibleState.SkipToFill();
                    // delay start of the closing storyboard until the opening one is completed
                    return; 
                } 

                if ((this.NormalState == null) || (this.NormalState.Children.Count != 0)) 
                {
                    // close immediatelly if no storyboard provided
                    this._parentPopup.IsOpen = false; 
                    this.Dispatcher.BeginInvoke(delegate () { OnPopupClosed (null, EventArgs.Empty); });
                }
                else 
                { 
                    // close the popup after the animation is completed
                    this._closingCompleted = false; 
                    this.NormalState.Begin();
                }
            } 
        }

        private void OpenPopup() 
        { 
            this._parentPopup.IsOpen = true;
 
            // Working around temporary limitations in Silverlight:
            // send the Opened event manually
            // 
            this.Dispatcher.BeginInvoke(delegate () { OnPopupOpened (null, EventArgs.Empty); });

            this._openPopup = false; 
        } 

        private void OnOffsetChanged(double horizontalOffset, double verticalOffset) 
        {
            if (this._parentPopup == null)
            { 
                return;
            }
 
            if (IsOpen) 
            {
                // update the current ToolTip position if needed 
                PerformPlacement(horizontalOffset, verticalOffset);
            }
        } 

        private void OnOpened(RoutedEventArgs e)
        { 
            RoutedEventHandler snapshot = this.Opened; 
            if (snapshot != null)
            { 
                snapshot(this, e);
            }
        } 

        // called when the Visible state transition is completed
        private void OnOpeningCompleted(object sender, EventArgs e) 
        { 
            this._openingCompleted = true;
 
            if (this._beginClosing)
            {
		    this.Dispatcher.BeginInvoke(delegate () { BeginClosing(); }); 
            }
        }
 
        private void OnPopupClosed(object source, EventArgs e) 
        {
            OnClosed(new RoutedEventArgs { OriginalSource = this }); 
        }

        private void OnPopupOpened(object source, EventArgs e) 
        {
            //
            if (this.VisibleState != null) 
            { 
                this._openingCompleted = false;
                this.VisibleState.Begin(); 
            }
            OnOpened(new RoutedEventArgs { OriginalSource = this });
        } 

        internal void OnRootVisualSizeChanged()
        { 
            if (this._parentPopup != null) 
            {
                PerformPlacement(this.HorizontalOffset, this.VerticalOffset); 
            }
        }
 
        private void OnToolTipSizeChanged(object sender, SizeChangedEventArgs e)
        {
            this._lastSize = e.NewSize; 
            if (this._parentPopup != null) 
            {
                PerformPlacement(this.HorizontalOffset, this.VerticalOffset); 
            }
        }
 
        private void PerformClipping(Size size)
        {
            Point mouse = ToolTipService.MousePosition; 
            RectangleGeometry rectangle = new RectangleGeometry(); 
            rectangle.Rect = new Rect(mouse.X, mouse.Y, size.Width, size.Height);
            ((UIElement)Content).Clip = rectangle; 
        }

        private void PerformPlacement(double horizontalOffset, double verticalOffset) 
        {
            Point mouse = ToolTipService.MousePosition;
 
            // align ToolTip with the bottom left corner of mouse bounding rectangle 
            //
            double top = mouse.Y + new TextBlock().FontSize; 
            double left = mouse.X;

            top += verticalOffset; 
            left += horizontalOffset;

            double maxY = ToolTipService.RootVisual.ActualHeight; 
            double maxX = ToolTipService.RootVisual.ActualWidth; 

            Rect toolTipRect = new Rect(left, top, this._lastSize.Width, this._lastSize.Height); 
            Rect intersectionRect = new Rect(0, 0, maxX, maxY);

            intersectionRect.Intersect(toolTipRect); 
            if ((Math.Abs(intersectionRect.Width - toolTipRect.Width) < TOOLTIP_tolerance) &&
                (Math.Abs(intersectionRect.Height - toolTipRect.Height) < TOOLTIP_tolerance))
            { 
                // ToolTip is almost completely inside the plug-in 
                this._parentPopup.VerticalOffset = top;
                this._parentPopup.HorizontalOffset = left; 
                return;
            }
            else if (intersectionRect.Equals(new Rect(0, 0, maxX, maxY))) 
            {
                //ToolTip is bigger than the plug-in
                PerformClipping(new Size(maxX, maxY)); 
                this._parentPopup.VerticalOffset = 0; 
                this._parentPopup.HorizontalOffset = 0;
 
                PerformClipping(new Size(maxX, maxY));
                return;
            } 

            double right = left + toolTipRect.Width;
            double bottom = top + toolTipRect.Height; 
 
            if (bottom > maxY)
            { 
                // If the lower edge of the plug-in obscures the ToolTip,
                // it repositions itself to align with the upper edge of the bounding box of the mouse.
                bottom = top; 
                top -= toolTipRect.Height;
            }
 
            if (top < 0) 
            {
                // If the upper edge of Plug-in obscures the ToolTip, 
                // the control repositions itself to align with the upper edge.
                // align with the top of the plug-in
                top = 0; 
            }
            else if (bottom > maxY)
            { 
                // align with the bottom edge 
                top = Math.Max(0, maxY - toolTipRect.Height);
            } 

            if (right > maxX)
            { 
                // If the right edge obscures the ToolTip,
                // it opens in the opposite direction from the obscuring edge.
                right = left; 
                left -= toolTipRect.Width; 
            }
 
            if (left < 0)
            {
                // If the left edge obscures the ToolTip, 
                // it then aligns with the obscuring screen edge
                left = 0;
            } 
            else if (right > maxX) 
            {
                // align with the right edge 
                left = Math.Max(0, maxX - toolTipRect.Width);
            }
 
            // position the parent Popup
            this._parentPopup.VerticalOffset = top;
            this._parentPopup.HorizontalOffset = left; 
 
            bottom = top + toolTipRect.Height;
            right = left + toolTipRect.Width; 

            // if right/bottom doesn't fit into the plug-in bounds, clip the ToolTip
            double dX = right - maxX; 
            double dY = bottom - maxY;
            if ((dX >= TOOLTIP_tolerance) || (dY >= TOOLTIP_tolerance))
            { 
                PerformClipping(new Size(toolTipRect.Width - dX, toolTipRect.Height - dY)); 
            }
        } 

        #endregion Private Methods
    } 
}
