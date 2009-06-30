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

using System.Windows;
using System.Windows.Automation.Provider;
using System.Windows.Controls;

namespace System.Windows.Automation.Peers {

	[MonoTODO("Write tests")]
	public class ScrollViewerAutomationPeer : FrameworkElementAutomationPeer, IScrollProvider {
	
		public ScrollViewerAutomationPeer (ScrollViewer owner) : base (owner)
		{
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
		}

		void IScrollProvider.SetScrollPercent (double horizontalPercent, double verticalPercent)
		{
		}

		bool IScrollProvider.HorizontallyScrollable { 
			get { return false; }
		}

		double IScrollProvider.HorizontalScrollPercent { 
			get { return (double) 0; }
		}

		double IScrollProvider.HorizontalViewSize { 
			get { return (double) 0; }
		}

		bool IScrollProvider.VerticallyScrollable { 
			get { return false; }
		}

		double IScrollProvider.VerticalScrollPercent { 
			get { return (double) 0; }
		}

		double IScrollProvider.VerticalViewSize { 
			get { return (double) 0; }
		}

		#endregion

	}

}
