//
// System.Windows.Automation.Peers.ScrollBarAutomationPeer
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
//
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

using System;
using System.Collections.Generic;
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

namespace System.Windows.Automation.Peers {

	public class ScrollBarAutomationPeer : RangeBaseAutomationPeer {

		public ScrollBarAutomationPeer (ScrollBar owner)
			: base (owner)
		{
			owner.LayoutUpdated += (o, e) => { 
				children = null; 
				// This usually happends when orientation changes.
				RaiseAutomationEvent (AutomationEvents.StructureChanged);
			};
		}

		protected override AutomationControlType GetAutomationControlTypeCore ()
		{
			return AutomationControlType.ScrollBar;
		}

		protected override string GetClassNameCore ()
		{
			return "ScrollBar";
		}

		protected override Point GetClickablePointCore ()
		{
			return new Point (double.NaN, double.NaN);
		}

		protected override AutomationOrientation GetOrientationCore ()
		{
			ScrollBar scrollbar = (ScrollBar) Owner;
			if (scrollbar.Orientation == Orientation.Vertical)
				return AutomationOrientation.Vertical;
			else
				return AutomationOrientation.Horizontal;
		}

		// Method used by ScrollViewerAutomationPeer to IScrollProvider.Scroll
		internal void InvokeByAmount (ScrollAmount amount)
		{
			CacheChildren ();

			if (children == null || amount == ScrollAmount.NoAmount)
				return;

			// 0 = First button  - Small decrement
			// 1 = Second button - Large decrement
			// 2 = Thumb 
			// 3 = Third button  - Large increment
			// 4 = Fourth button - Small increment
			int index = -1;
			switch (amount) {
			case ScrollAmount.LargeIncrement:
				index = 3;
				break;
			case ScrollAmount.SmallIncrement:
				index = 4;
				break;
			case ScrollAmount.LargeDecrement:
				index = 1;
				break;
			case ScrollAmount.SmallDecrement:
				index = 0;
				break;
			}

			IInvokeProvider invokeProvider 
				= (IInvokeProvider) children [index].GetPattern (PatternInterface.Invoke);
			if (invokeProvider != null)
				invokeProvider.Invoke ();
		}

		internal override List<AutomationPeer> ChildrenCore {
			get { 
				CacheChildren ();
				return children;
			}
		}

		private void CacheChildren ()
		{
			if (children == null || children.Count == 0) {
				// base.ChildrenCore returns 10 children:
				// - Child0-Child4 are used when orientation is Vertical
				// - Child5-Child9 are used when orientation is Horizontal
				children = new List<AutomationPeer> ();
				List<AutomationPeer> baseChildren = base.ChildrenCore;
				if (baseChildren == null)
					return;

				ScrollBar scrollbar = (ScrollBar) Owner;
				int begin = 0;
				if (scrollbar.Orientation == Orientation.Vertical)
					begin = 5;
				for (int index = begin; index < begin + 5; index++)
					children.Add (baseChildren [index]);
			}
		}

		private List<AutomationPeer> children;
	}
}
