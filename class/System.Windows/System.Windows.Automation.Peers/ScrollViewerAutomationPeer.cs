// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Copyright (c) 2009 Novell, Inc. (http://www.novell.com)
//
// Contact:
//   Moonlight Team (moonlight-list@lists.ximian.com)
//

using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Automation.Provider;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

namespace System.Windows.Automation.Peers {

	public class ScrollViewerAutomationPeer : FrameworkElementAutomationPeer, IScrollProvider {
	
		public ScrollViewerAutomationPeer (ScrollViewer owner) : base (owner)
		{
			scrollViewer = owner;
			CacheScrollbarPeers ();
		}

		public override object GetPattern (PatternInterface patternInterface)
		{
			if (patternInterface == PatternInterface.Scroll)
				return this;
			else
				return base.GetPattern (patternInterface);
		}

		protected override AutomationControlType GetAutomationControlTypeCore ()
		{
			return AutomationControlType.Pane;
		}

		protected override string GetClassNameCore ()
		{
			return "ScrollViewer";
		}

		protected override bool IsControlElementCore ()
		{
			return true;
		}

		#region IScrollProvide realization

		void IScrollProvider.Scroll (ScrollAmount horizontalAmount, ScrollAmount verticalAmount)
		{
			if (horizontalAmount != ScrollAmount.NoAmount 
			    && GetHorizontallyScrollable () && hScrollbarPeer != null) 
				hScrollbarPeer.InvokeByAmount (horizontalAmount);
			
			if (verticalAmount != ScrollAmount.NoAmount
			    && GetVerticallyScrollable () && vScrollbarPeer != null)
				vScrollbarPeer.InvokeByAmount (verticalAmount);
		}

		void IScrollProvider.SetScrollPercent (double horizontalPercent, double verticalPercent)
		{
			if (horizontalPercent != ScrollPatternIdentifiers.NoScroll
			    && GetHorizontallyScrollable ()) {
				if (horizontalPercent < 0 || horizontalPercent > 100)
					throw new ArgumentOutOfRangeException ();
				else 
	    				scrollViewer.ScrollToHorizontalOffset (horizontalPercent);
			}
			
			if (verticalPercent != ScrollPatternIdentifiers.NoScroll
			    && GetVerticallyScrollable ()) {
			    if (verticalPercent < 0 || verticalPercent > 100)
					throw new ArgumentOutOfRangeException ();
				else 
	    				scrollViewer.ScrollToVerticalOffset (verticalPercent);
			}
		}

		bool IScrollProvider.HorizontallyScrollable { 
			get { return GetHorizontallyScrollable (); }
		}

		double IScrollProvider.HorizontalScrollPercent { 
			get { 
				if (!GetHorizontallyScrollable ())
					return ScrollPatternIdentifiers.NoScroll;
				else { return scrollViewer.HorizontalOffset; }
			}
		}

		double IScrollProvider.HorizontalViewSize { 
			get { 
				if (!GetHorizontallyScrollable ())
					return 100d;
				return (scrollViewer.ViewportWidth * 100) / scrollViewer.ExtentWidth; 
			}
		}

		bool IScrollProvider.VerticallyScrollable { 
			get { return GetVerticallyScrollable (); }
		}

		double IScrollProvider.VerticalScrollPercent { 
			get { 
				if (!GetVerticallyScrollable ())
					return ScrollPatternIdentifiers.NoScroll;
				else { return scrollViewer.VerticalOffset; }
			}
		}

		double IScrollProvider.VerticalViewSize { 
			get { 
				if (!GetVerticallyScrollable ())
					return 100d;
				return (scrollViewer.ViewportHeight * 100) / scrollViewer.ExtentHeight; 
			}
		}

		#endregion

		internal override List<AutomationPeer> ChildrenCore {
			get { 
				List<AutomationPeer> children = base.ChildrenCore;
				if (children == null)
					return null;
					
				if (hScrollbarPeer != null 
					&& scrollViewer.ComputedHorizontalScrollBarVisibility == Visibility.Collapsed)
					children.Remove (hScrollbarPeer);
				if (vScrollbarPeer != null 
					&& scrollViewer.ComputedVerticalScrollBarVisibility == Visibility.Collapsed)
					children.Remove (vScrollbarPeer);

				if (children.Count == 0)
					return null;

				return children;
			}
		}

		private void CacheScrollbarPeers ()
		{
			IScrollProvider provider = (IScrollProvider) this;
			
			ScrollBar hscrollbar = scrollViewer.ElementHorizontalScrollBar;
			hCachedProperty = new CachedProperty ();
			SetAutomationEvents (provider,
			                     hscrollbar,
			                     GetHorizontallyScrollable,
			                     delegate { return scrollViewer.ComputedHorizontalScrollBarVisibility; },
			                     delegate { return provider.HorizontalScrollPercent; },
			                     delegate { return provider.HorizontalViewSize; },
					     ScrollPatternIdentifiers.HorizontalScrollPercentProperty,
					     ScrollPatternIdentifiers.HorizontallyScrollableProperty,
					     ScrollPatternIdentifiers.HorizontalViewSizeProperty,
					     AutomationOrientation.Horizontal);
			
			ScrollBar vscrollbar = scrollViewer.ElementVerticalScrollBar;
			vCachedProperty = new CachedProperty ();
			SetAutomationEvents (
			                     provider,
			                     vscrollbar,
			                     GetVerticallyScrollable,
			                     delegate { return scrollViewer.ComputedVerticalScrollBarVisibility; },
			                     delegate { return provider.VerticalScrollPercent; },
			                     delegate { return provider.VerticalViewSize; },
					     ScrollPatternIdentifiers.VerticalScrollPercentProperty,
					     ScrollPatternIdentifiers.VerticallyScrollableProperty,
					     ScrollPatternIdentifiers.VerticalViewSizeProperty,
					     AutomationOrientation.Vertical);
		}

		private void SetAutomationEvents (IScrollProvider provider,
		                                  ScrollBar scrollbar,
		                                  Func<bool> scrollableDelegate, 
		                                  Func<Visibility> computedVisibilityDelegate,
						  Func<double> scrollPercentDelegate,
						  Func<double> viewSizeDelegate,
						  AutomationProperty scrollPercentProperty,
						  AutomationProperty scrollableProperty,
						  AutomationProperty viewSizeProperty,
						  AutomationOrientation orientation)
		{
			CachedProperty cachedProperty;
			if (orientation == AutomationOrientation.Horizontal)
				cachedProperty = hCachedProperty;
			else
				cachedProperty = vCachedProperty;

			cachedProperty.Scrollable = scrollableDelegate ();
			cachedProperty.Visible = computedVisibilityDelegate () == Visibility.Visible;
			cachedProperty.ScrollPercent = scrollPercentDelegate ();
			cachedProperty.ViewSize = viewSizeDelegate ();
			cachedProperty.ScrollPercentProperty = scrollPercentProperty;
			cachedProperty.ScrollableProperty = scrollableProperty;
			cachedProperty.ViewSizeProperty = viewSizeProperty;
			cachedProperty.Orientation = orientation;
			cachedProperty.ScrollBar = scrollbar;
			cachedProperty.ScrollPercentDelegate = scrollPercentDelegate;
			cachedProperty.ScrollableDelegate = scrollableDelegate;

			scrollViewer.UIAVisibilityChanged += (o, e) => {
				if (cachedProperty.Orientation != e.Orientation) {
					RaiseStructureChanged (cachedProperty, computedVisibilityDelegate);
					RaiseScrollableProperty (cachedProperty, scrollableDelegate);
					RaiseViewSizeProperty (cachedProperty, viewSizeDelegate);
					RaiseScrollPercentProperty (cachedProperty, scrollPercentDelegate);
				}
			};
			scrollViewer.UIAViewportChanged += (o, e) => {
				if (cachedProperty.Orientation == AutomationOrientation.Horizontal) 
					RaiseViewSizeProperty (cachedProperty, 
					                       viewSizeDelegate,
					                       provider.HorizontalViewSize);
				else
					RaiseViewSizeProperty (cachedProperty, 
					                       viewSizeDelegate,
					                       provider.VerticalViewSize);
			};
			scrollViewer.SizeChanged += (o, e) => {
				RaiseViewSizeProperty (cachedProperty, viewSizeDelegate);
			};
			scrollViewer.UIAOffsetChanged += (o, e) => {
				if (e.Orientation == cachedProperty.Orientation)
					RaiseScrollPercentProperty (cachedProperty, scrollPercentDelegate);
			};
			
			scrollViewer.UIAScrollBarSet += (o, e) => {
				if (e.Orientation == cachedProperty.Orientation) {
					if (cachedProperty.ScrollBar != null) {
						cachedProperty.ScrollBar.IsEnabledChanged -= ScrollBar_IsEnabledChanged;
						cachedProperty.ScrollBar.ValueChanged -= ScrollBar_ValueChanged;

					}
					cachedProperty.ScrollBar = e.NewValue;
					if (cachedProperty.ScrollBar != null) {
						cachedProperty.ScrollBar.IsEnabledChanged += ScrollBar_IsEnabledChanged;
						cachedProperty.ScrollBar.ValueChanged += ScrollBar_ValueChanged;
					}
					SetScrollBarAutomationPeer (cachedProperty.ScrollBar, cachedProperty.Orientation);
				}
			};
			if (cachedProperty.ScrollBar != null) {
				cachedProperty.ScrollBar.IsEnabledChanged += ScrollBar_IsEnabledChanged;
				cachedProperty.ScrollBar.ValueChanged += ScrollBar_ValueChanged;
			}
			SetScrollBarAutomationPeer (cachedProperty.ScrollBar, cachedProperty.Orientation);

		}

		#region Wrapper methods

		private bool GetHorizontallyScrollable ()
		{
			if (scrollViewer.HorizontalScrollBarVisibility != ScrollBarVisibility.Disabled) {
				if (hCachedProperty.ScrollBar != null 
				    && scrollViewer.HorizontalScrollBarVisibility != ScrollBarVisibility.Hidden)
					return scrollViewer.ComputedHorizontalScrollBarVisibility == Visibility.Visible
					   && scrollViewer.ExtentWidth != 0;
				return true;
			} else
				return false;
		}

		private bool GetVerticallyScrollable ()
		{
			if (scrollViewer.VerticalScrollBarVisibility != ScrollBarVisibility.Disabled) {
				if (vCachedProperty.ScrollBar != null
				    && scrollViewer.VerticalScrollBarVisibility != ScrollBarVisibility.Hidden)
					return scrollViewer.ComputedVerticalScrollBarVisibility == Visibility.Visible
					     && scrollViewer.ExtentHeight != 0;
				return true;
			} else
				return false;
		}

		#endregion

		#region Private Methods used to Raise UIA Events

		private void RaiseScrollableProperty (CachedProperty cachedProperty,
		                                      Func<bool> scrollableDelegate)
		{
			bool scrollable = scrollableDelegate ();
			if (cachedProperty.Scrollable != scrollable) {
				RaisePropertyChangedEvent (cachedProperty.ScrollableProperty, 
							   cachedProperty.Scrollable,
				                           scrollable); 
				cachedProperty.Scrollable = scrollable;
			}
		}

		private void RaiseViewSizeProperty (CachedProperty cachedProperty, 
		                                    Func<double> viewSizeDelegate)
		{
			double viewSize = viewSizeDelegate ();
			RaiseViewSizeProperty (cachedProperty, viewSizeDelegate, viewSize);
		}

		private void RaiseViewSizeProperty (CachedProperty cachedProperty, 
		                                    Func<double> viewSizeDelegate, 
						    double viewSize)
		{
			if (cachedProperty.ViewSize != viewSize) {
				RaisePropertyChangedEvent (cachedProperty.ViewSizeProperty, 
							   cachedProperty.ViewSize,
			        	                   viewSize);
				cachedProperty.ViewSize = viewSize;
			}
		}

		private void RaiseScrollPercentProperty (CachedProperty cachedProperty,
		                                         Func<double> scrollPercentDelegate)
		{
			double percent = scrollPercentDelegate ();
			if (cachedProperty.ScrollPercent != percent) {
				RaisePropertyChangedEvent (cachedProperty.ScrollPercentProperty, 
							   cachedProperty.ScrollPercent,
			                	           percent); 
				cachedProperty.ScrollPercent = percent;
			}
		}

		private void RaiseStructureChanged (CachedProperty cachedProperty,
		                                    Func<Visibility> computedVisibilityDelegate)
		{
			bool visible = computedVisibilityDelegate () == Visibility.Visible;
			if (visible != cachedProperty.Visible) {
				cachedProperty.Visible = visible;
				RaiseAutomationEvent (AutomationEvents.StructureChanged); 
			}
		}

		#endregion

		private void ScrollBar_IsEnabledChanged (object sender, DependencyPropertyChangedEventArgs e)
		{
			CachedProperty cachedProperty = GetCachedProperty ((ScrollBar) sender);
			RaiseScrollableProperty (cachedProperty, cachedProperty.ScrollableDelegate);
		}

		private void ScrollBar_ValueChanged (object sender, RoutedPropertyChangedEventArgs<double> e)
		{
			CachedProperty cachedProperty = GetCachedProperty ((ScrollBar) sender);
			RaiseScrollPercentProperty (cachedProperty, cachedProperty.ScrollPercentDelegate);
		}

		CachedProperty GetCachedProperty (ScrollBar scrollbar)
		{
			if (scrollbar.Orientation == Orientation.Horizontal)
				return hCachedProperty;
			else
				return vCachedProperty;
		}

		private void SetScrollBarAutomationPeer (ScrollBar scrollbar, AutomationOrientation orientation)
		{
			if (scrollbar == null) {
				if (orientation == AutomationOrientation.Horizontal)
					hScrollbarPeer = null;
				else
					vScrollbarPeer = null;
			} else {
				if (orientation == AutomationOrientation.Horizontal)
					hScrollbarPeer
						= FrameworkElementAutomationPeer.CreatePeerForElement (scrollbar) as ScrollBarAutomationPeer;
				else
					vScrollbarPeer 
						= FrameworkElementAutomationPeer.CreatePeerForElement (scrollbar) as ScrollBarAutomationPeer;
			}
		}

		internal class CachedProperty {
			public bool Scrollable { get; set; }
			public bool Visible { get; set; }
			public double ScrollPercent { get; set; }
			public double ViewSize { get; set; }
			public AutomationProperty ScrollPercentProperty { get; set; }
			public AutomationProperty ScrollableProperty { get; set; }
			public AutomationProperty ViewSizeProperty { get; set; }
			public AutomationOrientation Orientation { get; set; }
			public ScrollBar ScrollBar { get; set; }
			public Func<double> ScrollPercentDelegate { get; set; }
			public Func<bool> ScrollableDelegate { get; set; }
		}

		private bool cached;
		private ScrollViewer scrollViewer;
		private ScrollBarAutomationPeer hScrollbarPeer;
		private ScrollBarAutomationPeer vScrollbarPeer;
		private CachedProperty hCachedProperty;
		private CachedProperty vCachedProperty;
	}
}

