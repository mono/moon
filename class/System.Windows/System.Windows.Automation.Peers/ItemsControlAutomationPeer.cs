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
// Copyright (c) 2008 Novell, Inc. (http://www.novell.com)
//
// Contact:
//   Moonlight Team (moonlight-list@lists.ximian.com)
//

using System.Windows;
using System.Windows.Controls;
using System.Collections.Generic;
using System.Linq;

namespace System.Windows.Automation.Peers {

	public abstract class ItemsControlAutomationPeer : FrameworkElementAutomationPeer {
		protected ItemsControlAutomationPeer (ItemsControl items) : base (items)
		{
			items.Items.ItemsChanged += (o, e) => {
				RaiseAutomationEvent (AutomationEvents.StructureChanged);
			};
		}

		public override object GetPattern (PatternInterface pattern)
		{
			if (pattern == PatternInterface.Scroll) {
				ScrollViewer scrollViewer = ScrollViewer;
				if (scrollViewer != null && scrollViewer.AutomationPeer != null)
					return scrollViewer.AutomationPeer.GetPattern (pattern);
			}

			return base.GetPattern (pattern);
		}

		protected override List<AutomationPeer> GetChildrenCore ()
		{
			List<AutomationPeer> children = base.GetChildrenCore ();

			ScrollViewer scrollViewer = ScrollViewerFromChildren (children);

			if (scrollViewer != null) {
				AutomationPeer scrollViewerPeer
					= FrameworkElementAutomationPeer.CreatePeerForElement (scrollViewer);
				if (scrollViewerPeer != null) {
					if (IsScrollViewerVisible (scrollViewer))
						return children;
					else {
						// Now dependending on the scrollbars visibility we
						// need to remove them
						children = scrollViewerPeer.GetChildren ();
						if (children == null)	
							return null;

						// We need a temporal copy because we are going
						// to modify "children" collection
						ScrollBarAutomationPeer[] scrollbarPeers 
							= children.OfType<ScrollBarAutomationPeer> ().ToArray ();
						foreach (ScrollBarAutomationPeer scrollbarPeer in scrollbarPeers) {
							if ((scrollbarPeer.GetOrientation () == AutomationOrientation.Horizontal
							     && scrollViewer.ComputedHorizontalScrollBarVisibility == Visibility.Collapsed)
							    || (scrollbarPeer.GetOrientation () == AutomationOrientation.Vertical
							        && scrollViewer.ComputedVerticalScrollBarVisibility == Visibility.Collapsed))
							    children.Remove (scrollbarPeer);
						}

						if (children.Count == 0)
							return null;
					}
				} else
					return children;
			}

			return children;
		}

		private ScrollViewer ScrollViewer {
			get { return ScrollViewerFromChildren (base.GetChildrenCore ()); }
		}

		private ScrollViewer ScrollViewerFromChildren (List<AutomationPeer> children) 
		{
			if (children == null)
				return null;

			ScrollViewerAutomationPeer[] scrollPeer
				= children.OfType<ScrollViewerAutomationPeer> ().ToArray ();
			if (scrollPeer != null)
				return (ScrollViewer) scrollPeer [0].Owner;

			return null;
		}

		private bool IsScrollViewerVisible (ScrollViewer scrollViewer)
		{
			return scrollViewer.ComputedHorizontalScrollBarVisibility != Visibility.Collapsed
			       || scrollViewer.ComputedVerticalScrollBarVisibility != Visibility.Collapsed;
		}

		internal UIElement GetChildAtIndex (int index)
		{
			if (index < 0)
				return null;

			List<AutomationPeer> children = base.GetChildrenCore ();

			ScrollViewer scrollViewer = ScrollViewerFromChildren (children);
			if (scrollViewer != null) {
				AutomationPeer scrollViewerPeer
					= FrameworkElementAutomationPeer.CreatePeerForElement (scrollViewer);
				children = scrollViewerPeer.GetChildren ();
			}

			if (index >= children.Count)
				return null;

			return ((FrameworkElementAutomationPeer) children [index]).Owner;
		}

	}

}
