//
// Unit tests for FrameworkElementAutomationPeer
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
using System.Windows;
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class FrameworkElementAutomationPeerTest {

		[TestMethod]
		public void CreatePeerForElement ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				FrameworkElementAutomationPeer.CreatePeerForElement (null);
			}, "null");
		}

		public class ConcreteFrameworkElement : FrameworkElement {
		}

		[TestMethod]
		[MoonlightBug]
		public void Null ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				new FrameworkElementAutomationPeer (null);
			});
		}

		[TestMethod]
		public void GetPattern ()
		{
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement ();
			FrameworkElementAutomationPeer feap = new FrameworkElementAutomationPeer (fe);

			Assert.IsNull (feap.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (feap.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (feap.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (feap.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Invoke), "Invoke");
			Assert.IsNull (feap.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (feap.GetPattern (PatternInterface.RangeValue), "RangeValue");
			Assert.IsNull (feap.GetPattern (PatternInterface.Scroll), "Scroll");
			Assert.IsNull (feap.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Selection), "Selection");
			Assert.IsNull (feap.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (feap.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (feap.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (feap.GetPattern (PatternInterface.Value), "Value");
			Assert.IsNull (feap.GetPattern (PatternInterface.Window), "Window");
		}

		public class FrameworkElementAutomationPeerPoker : FrameworkElementAutomationPeer {

			public FrameworkElementAutomationPeerPoker (FrameworkElement fe)
				: base (fe)
			{
			}

			public AutomationPeer GetLabeledByCore_ ()
			{
				return base.GetLabeledByCore ();
			}

			public string GetNameCore_ ()
			{
				return base.GetNameCore ();
			}

			public bool IsContentElementCore_ ()
			{
				return base.IsContentElementCore ();
			}

			public bool IsControlElementCore_ ()
			{
				return base.IsControlElementCore ();
			}
		}

		[TestMethod]
		public void GetName ()
		{
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement ();
			FrameworkElementAutomationPeerPoker feap = new FrameworkElementAutomationPeerPoker (fe);
			Assert.AreEqual (String.Empty, feap.GetName (), "GetName");
			Assert.AreEqual (String.Empty, feap.GetNameCore_ (), "GetNameCore");
		}

		[TestMethod]
		public void GetLabeledBy ()
		{
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement ();
			FrameworkElementAutomationPeerPoker feap = new FrameworkElementAutomationPeerPoker (fe);
			Assert.IsNull (feap.GetLabeledBy (), "GetLabeledBy");
			Assert.IsNull (feap.GetLabeledByCore_ (), "GetLabeledByCore");
		}

		[TestMethod]
		public void IsContentElement ()
		{
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement ();
			FrameworkElementAutomationPeerPoker feap = new FrameworkElementAutomationPeerPoker (fe);
			Assert.IsTrue (feap.IsContentElement (), "IsContentElement");
			Assert.IsTrue (feap.IsContentElementCore_ (), "IsContentElementCore");
		}

		[TestMethod]
		public void IsControlElement ()
		{
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement ();
			FrameworkElementAutomationPeerPoker feap = new FrameworkElementAutomationPeerPoker (fe);
			Assert.IsTrue (feap.IsControlElement (), "IsControlElement");
			Assert.IsTrue (feap.IsControlElementCore_ (), "IsControlElementCore");
		}
	}
}
