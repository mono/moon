//
// Unit tests for RangeBaseAutomationPeer
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
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class RangeBaseAutomationPeerTest {

		[TestMethod]
		public void Null ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				new RangeBaseAutomationPeer (null);
			});
		}

		[TestMethod]
		public void Public ()
		{
			RangeBaseAutomationPeer rbap = new RangeBaseAutomationPeer (new Slider ());

			Assert.IsNull (rbap.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (rbap.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (rbap.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Invoke), "Invoke");
			Assert.IsNull (rbap.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsTrue (Object.ReferenceEquals (rbap, rbap.GetPattern (PatternInterface.RangeValue)), "RangeValue");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Scroll), "Scroll");
			Assert.IsNull (rbap.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Selection), "Selection");
			Assert.IsNull (rbap.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (rbap.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Value), "Value");
			Assert.IsNull (rbap.GetPattern (PatternInterface.Window), "Window");
		}

		[TestMethod]
		public void RangeValueProvider ()
		{
			Slider s = new Slider ();
			IRangeValueProvider rbap = new RangeBaseAutomationPeer (s);

			Assert.AreEqual (s.LargeChange, rbap.LargeChange, "LargeChange");
			Assert.AreEqual (s.Maximum, rbap.Maximum, "Maximum");
			Assert.AreEqual (s.Minimum, rbap.Minimum, "Minimum");
			Assert.AreEqual (s.SmallChange, rbap.SmallChange, "SmallChange");
			Assert.AreEqual (s.Value, rbap.Value, "Value");

			rbap.SetValue (0.5);
			Assert.AreEqual (0.5, s.Value, "Slider.Value");
			Assert.AreEqual (0.5, rbap.Value, "IRangeValueProvider.Value");

			s.LargeChange = 0.9;
			Assert.AreEqual (0.9, rbap.LargeChange, "LargeChange-2");
			s.Maximum = 0.9;
			Assert.AreEqual (0.9, rbap.Maximum, "Maximum-2");
			s.Minimum = 0.1;
			Assert.AreEqual (0.1, rbap.Minimum, "Minimum-2");
			s.SmallChange = 0.1;
			Assert.AreEqual (0.1, rbap.SmallChange, "SmallChange-2");
		}
	}
}
