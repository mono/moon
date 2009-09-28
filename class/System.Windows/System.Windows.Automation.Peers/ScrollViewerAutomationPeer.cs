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

	[MonoTODO("Write tests")]
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
			    && GetHorizontallyScrollable ()) 
				hScrollbarPeer.InvokeByAmount (horizontalAmount);
			
			if (verticalAmount != ScrollAmount.NoAmount
			    && GetVerticallyScrollable ())
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
					
				CacheScrollbarPeers ();

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
			ScrollBar hscrollbar = scrollViewer.ElementHorizontalScrollBar;
			IScrollProvider provider = (IScrollProvider) this;

			if (hscrollbar != null) {
				hScrollbarPeer 
					= FrameworkElementAutomationPeer.CreatePeerForElement (hscrollbar) as ScrollBarAutomationPeer;
				SetAutomationEvents (hscrollbar,
				                     GetHorizontallyScrollable,
				                     delegate { return scrollViewer.ComputedHorizontalScrollBarVisibility; },
				                     delegate { return provider.HorizontalScrollPercent; },
				                     delegate { return provider.HorizontalViewSize; },
						     ScrollPatternIdentifiers.HorizontalScrollPercentProperty,
						     ScrollPatternIdentifiers.HorizontallyScrollableProperty,
						     ScrollPatternIdentifiers.HorizontalViewSizeProperty);
			}
			
			ScrollBar vscrollbar = scrollViewer.ElementVerticalScrollBar;
			if (vscrollbar != null) {
				vScrollbarPeer 
					= FrameworkElementAutomationPeer.CreatePeerForElement (vscrollbar) as ScrollBarAutomationPeer;
				SetAutomationEvents (vscrollbar,
				                     GetVerticallyScrollable,
				                     delegate { return scrollViewer.ComputedVerticalScrollBarVisibility; },
				                     delegate { return provider.VerticalScrollPercent; },
				                     delegate { return provider.VerticalViewSize; },
						     ScrollPatternIdentifiers.VerticalScrollPercentProperty,
						     ScrollPatternIdentifiers.VerticallyScrollableProperty,
						     ScrollPatternIdentifiers.VerticalViewSizeProperty);
			}
		}

		private void SetAutomationEvents (ScrollBar scrollbar,
		                                  Func<bool> scrollableDelegate, 
		                                  Func<Visibility> computedVisibilityDelegate,
						  Func<double> scrollPercentDelegate,
						  Func<double> viewSizeDelegate,
						  AutomationProperty scrollPercentProperty,
						  AutomationProperty scrollableProperty,
						  AutomationProperty viewSizeProperty)
		{
			CachedProperty cachedProperty = new CachedProperty () {
				Scrollable            = scrollableDelegate (),
				Visible               = computedVisibilityDelegate () == Visibility.Visible,
				ScrollPercent         = scrollPercentDelegate (),
				ViewSize              = viewSizeDelegate (),
				ScrollPercentProperty = scrollPercentProperty,
				ScrollableProperty    = scrollableProperty,
				ViewSizeProperty      = viewSizeProperty
			};

			// StructureChanged event
			scrollbar.LayoutUpdated += (o, e) => {
				bool visible = computedVisibilityDelegate () == Visibility.Visible;
				if (visible != cachedProperty.Visible) {
					cachedProperty.Visible = visible;
					RaiseAutomationEvent (AutomationEvents.StructureChanged); 
				}
			};
			// Scroll.XXXScrollableProperty
			scrollbar.IsEnabledChanged += (o, e) => {
				bool scrollable = scrollableDelegate () && (bool) e.NewValue;
				if (cachedProperty.Scrollable != scrollable) {
					RaisePropertyChangedEvent (cachedProperty.ScrollableProperty, 
								   cachedProperty.Scrollable,
					                           scrollable); 
					cachedProperty.Scrollable = scrollable;
				}
			};
			// Scroll.XXXScrollPercent
			scrollbar.ValueChanged += (o, e) => {
				double percent = scrollPercentDelegate ();
				if (cachedProperty.ScrollPercent != percent) {
					RaisePropertyChangedEvent (cachedProperty.ScrollPercentProperty, 
								   cachedProperty.ScrollPercent,
				                	           percent); 
					cachedProperty.ScrollPercent = percent;
				 }
			};
			// Scroll.XXXViewSize
			scrollViewer.SizeChanged += (o, e) => {
				double viewSize = viewSizeDelegate ();
				if (cachedProperty.ViewSize != viewSize) {
					RaisePropertyChangedEvent (cachedProperty.ViewSizeProperty, 
								   cachedProperty.ViewSize,
				        	                   viewSize);
					cachedProperty.ViewSize = viewSize;
				}
			};
		}

		#region Wrapper methods

		private bool GetHorizontallyScrollable ()
		{
			return scrollViewer.ComputedHorizontalScrollBarVisibility == Visibility.Visible
				&& hScrollbarPeer != null;
		}

		private bool GetVerticallyScrollable ()
		{
			return scrollViewer.ComputedVerticalScrollBarVisibility == Visibility.Visible
				&& vScrollbarPeer != null;
		}

		#endregion

		internal class CachedProperty {
			public bool Scrollable { get; set; }
			public bool Visible { get; set; }
			public double ScrollPercent { get; set; }
			public double ViewSize { get; set; }
			public AutomationProperty ScrollPercentProperty { get; set; }
			public AutomationProperty ScrollableProperty { get; set; }
			public AutomationProperty ViewSizeProperty { get; set; }
		}

		private bool cached;
		private ScrollViewer scrollViewer;
		private ScrollBarAutomationPeer hScrollbarPeer;
		private ScrollBarAutomationPeer vScrollbarPeer;
	}
}

