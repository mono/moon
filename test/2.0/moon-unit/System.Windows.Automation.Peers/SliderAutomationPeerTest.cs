//
// Unit tests for SliderAutomationPeer
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
	public class SliderAutomationPeerTest {

		[TestMethod]
		public void Null ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				new SliderAutomationPeer (null);
			});
		}

		[TestMethod]
		public void Public ()
		{
			SliderAutomationPeer sap = new SliderAutomationPeer (new Slider ());
			Assert.AreEqual (AutomationControlType.Slider, sap.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual ("Slider", sap.GetClassName (), "GetClassName");
			Point p = sap.GetClickablePoint ();
			Assert.IsTrue (Double.IsNaN (p.X), "X");
			Assert.IsTrue (Double.IsNaN (p.Y), "Y");
		}

		public class SliderAutomationPeerPoker : SliderAutomationPeer {

			public SliderAutomationPeerPoker (Slider owner)
				: base (owner)
			{
			}

			public AutomationControlType GetAutomationControlTypeCore_ ()
			{
				return base.GetAutomationControlTypeCore ();
			}

			public string GetClassNameCore_ ()
			{
				return base.GetClassNameCore ();
			}

			public Point GetClickablePointCore_ ()
			{
				return base.GetClickablePointCore ();
			}
		}

		[TestMethod]
		public void Protected ()
		{
			SliderAutomationPeerPoker sap = new SliderAutomationPeerPoker (new Slider ());
			Assert.AreEqual (AutomationControlType.Slider, sap.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
			Assert.AreEqual ("Slider", sap.GetClassNameCore_ (), "GetClassNameCore");
			Point p = sap.GetClickablePointCore_ ();
			Assert.IsTrue (Double.IsNaN (p.X), "X");
			Assert.IsTrue (Double.IsNaN (p.Y), "Y");
		}
	}
}
