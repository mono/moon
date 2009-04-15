//
// Unit tests for TextBlockAutomationPeer
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
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class TextBlockAutomationPeerTest {

		public class TextBlockAutomationPeerPoker : TextBlockAutomationPeer {

			public TextBlockAutomationPeerPoker (TextBlock owner)
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

			public string GetNameCore_ ()
			{
				return base.GetNameCore ();
			}

			public bool IsContentElementCore_ ()
			{
				return base.IsContentElementCore ();
			}
		}

		[TestMethod]
		public void Null ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				new TextBlockAutomationPeerPoker (null);
			});
		}

		[TestMethod]
		public void GetAutomationControlType ()
		{
			TextBlockAutomationPeerPoker tbap = new TextBlockAutomationPeerPoker (new TextBlock ());
			Assert.AreEqual (AutomationControlType.Text, tbap.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual (AutomationControlType.Text, tbap.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
		}

		[TestMethod]
		public void GetClassName ()
		{
			TextBlockAutomationPeerPoker tbap = new TextBlockAutomationPeerPoker (new TextBlock ());
			Assert.AreEqual ("TextBlock", tbap.GetClassName (), "GetClassName");
			Assert.AreEqual ("TextBlock", tbap.GetClassNameCore_ (), "GetClassNameCore");
		}

		[TestMethod]
		public void GetName ()
		{
			TextBlock  textblock = new TextBlock ();
			TextBlockAutomationPeerPoker tbap = new TextBlockAutomationPeerPoker (textblock);
			Assert.AreEqual (String.Empty, tbap.GetName (), "GetName");
			Assert.AreEqual (String.Empty, tbap.GetNameCore_ (), "GetNameCore");

			string textBlockname = "Textblock name!";
			textblock.Text = textBlockname;
			Assert.AreEqual (textBlockname, tbap.GetName (), "GetName #1");
			Assert.AreEqual (textBlockname, tbap.GetNameCore_ (), "GetNameCore #1");
		}

		[TestMethod]
		public void IsContentElement ()
		{
			TextBlockAutomationPeerPoker tbap = new TextBlockAutomationPeerPoker (new TextBlock ());
			Assert.IsTrue (tbap.IsContentElement (), "IsContentElement");
			Assert.IsTrue (tbap.IsContentElementCore_ (), "IsContentElementCore");
		}
	}
}
