//
// Unit tests for ThumbAutomationPeer
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
using System.Windows.Controls.Primitives;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class ThumbAutomationPeerTest {

		[TestMethod]
		public void Null ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				new ThumbAutomationPeer (null);
			});
		}

		[TestMethod]
		public void Public ()
		{
			ThumbAutomationPeer tap = new ThumbAutomationPeer (new Thumb ());
			Assert.AreEqual (AutomationControlType.Thumb, tap.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual ("Thumb", tap.GetClassName (), "GetClassName");
		}

		public class ThumbAutomationPeerPoker : ThumbAutomationPeer {

			public ThumbAutomationPeerPoker (Thumb owner)
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
		}

		[TestMethod]
		public void Protected ()
		{
			ThumbAutomationPeerPoker tap = new ThumbAutomationPeerPoker (new Thumb ());
			Assert.AreEqual (AutomationControlType.Thumb, tap.GetAutomationControlTypeCore_ (), "GetAutomationControlType");
			Assert.AreEqual ("Thumb", tap.GetClassNameCore_ (), "GetClassName");
		}
	}
}
