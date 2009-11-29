//Permission is hereby granted, free of charge, to any person obtaining
//a copy of this software and associated documentation files (the
//"Software"), to deal in the Software without restriction, including
//without limitation the rights to use, copy, modify, merge, publish,
//distribute, sublicense, and/or sell copies of the Software, and to
//permit persons to whom the Software is furnished to do so, subject to
//the following conditions:
//
//The above copyright notice and this permission notice shall be
//included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Copyright (c) 2009 Novell, Inc.
//
// Authors:
//	Moonlight List (moonlight-list@lists.ximian.com)
//

using System;
using System.Windows;
using System.Windows.Browser;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Browser {

	[TestClass]	
	public class HtmlObjectTest {

		private void Handler (object obj, EventArgs e)
		{
		}

		private void GenericHandler (object obj, HtmlEventArgs e)
		{
		}

		[TestMethod]
		public void AttachEvent_Validations ()
		{
			HtmlObject ho = HtmlPage.Document.CreateElement ("div");

			EventHandler eh = new EventHandler (Handler);

			Assert.Throws<ArgumentNullException> (delegate {
				ho.AttachEvent (null, eh);
			}, "AttachEvent(null,EventHandler)");
			Assert.Throws<ArgumentException> (delegate {
				ho.AttachEvent (String.Empty, eh);
			}, "AttachEvent(Empty,EventHandler)");
			Assert.Throws<ArgumentNullException> (delegate {
				ho.AttachEvent ("a", (EventHandler) null);
			}, "AttachEvent(string,null");
			Assert.Throws<ArgumentException> (delegate {
				ho.AttachEvent ("a\0b", eh);
			}, "AttachEvent(string-with-null,EventHandler");

			EventHandler<HtmlEventArgs> geh = new EventHandler<HtmlEventArgs> (GenericHandler);

			Assert.Throws<ArgumentNullException> (delegate {
				ho.AttachEvent (null, geh);
			}, "AttachEvent(null,EventHandler<HtmlEventArgs>)");
			Assert.Throws<ArgumentException> (delegate {
				ho.AttachEvent (String.Empty, geh);
			}, "AttachEvent(Empty,EventHandler<HtmlEventArgs>)");
			Assert.Throws<ArgumentNullException> (delegate {
				ho.AttachEvent ("a", (EventHandler<HtmlEventArgs>) null);
			}, "AttachEvent(string,EventHandler<HtmlEventArgs>null)");
			Assert.Throws<ArgumentException> (delegate {
				ho.DetachEvent ("a\0b", geh);
			}, "AttachEvent(string-with-null,EventHandler<HtmlEventArgs>");
		}

		[TestMethod]
		public void AttachEvent ()
		{
			HtmlObject ho = HtmlPage.Document.CreateElement ("div");
			EventHandler eh = new EventHandler (Handler);
			Assert.IsTrue (ho.AttachEvent ("a", eh), "1");
			// twice (same handler)
			Assert.IsTrue (ho.AttachEvent ("a", eh), "2");
			// again (different handler)
			Assert.IsTrue (ho.AttachEvent ("a", new EventHandler<HtmlEventArgs> (GenericHandler)), "3");
		}

		[TestMethod]
		public void DetachEvent_Validations ()
		{
			HtmlObject ho = HtmlPage.Document.CreateElement ("div");

			EventHandler eh = new EventHandler (Handler);

			Assert.Throws<ArgumentNullException> (delegate {
				ho.DetachEvent (null, eh);
			}, "DetachEvent(null,EventHandler)");
			Assert.Throws<ArgumentException> (delegate {
				ho.DetachEvent (String.Empty, eh);
			}, "DetachEvent(Empty,EventHandler)");
			Assert.Throws<ArgumentNullException> (delegate {
				ho.DetachEvent ("a", (EventHandler) null);
			}, "DetachEvent(string,null");
			Assert.Throws<ArgumentException> (delegate {
				ho.DetachEvent ("a\0b", eh);
			}, "DetachEvent(string-with-null,EventHandler");

			EventHandler<HtmlEventArgs> geh = new EventHandler<HtmlEventArgs> (GenericHandler);

			Assert.Throws<ArgumentNullException> (delegate {
				ho.DetachEvent (null, geh);
			}, "DetachEvent(null,EventHandler<HtmlEventArgs>)");
			Assert.Throws<ArgumentException> (delegate {
				ho.DetachEvent (String.Empty, geh);
			}, "DetachEvent(Empty,EventHandler<HtmlEventArgs>)");
			Assert.Throws<ArgumentNullException> (delegate {
				ho.DetachEvent ("a", (EventHandler<HtmlEventArgs>) null);
			}, "DetachEvent(string,EventHandler<HtmlEventArgs>null)");
			Assert.Throws<ArgumentException> (delegate {
				ho.DetachEvent ("a\0b", geh);
			}, "DetachEvent(string-with-null,EventHandler<HtmlEventArgs>");
		}

		[TestMethod]
		public void DetachEvent ()
		{
			HtmlObject ho = HtmlPage.Document.CreateElement ("div");
			EventHandler eh = new EventHandler (Handler);
			// detach inexisting
			ho.DetachEvent ("a", eh);
			// detach inexisting
			ho.DetachEvent ("a", new EventHandler<HtmlEventArgs> (GenericHandler));
		}

		class HtmlObjectPoker : HtmlObject {

			public HtmlObjectPoker ()
			{
			}

			public object ConvertTo_ (Type targetType, bool allowSerialization)
			{
				return base.ConvertTo (targetType, allowSerialization);
			}
		}

		[TestMethod]
		public void ConvertTo ()
		{
			HtmlObjectPoker ho = new HtmlObjectPoker ();
			Assert.Throws<NullReferenceException> (delegate {
				ho.ConvertTo_ (null, true);
			}, "null,true");

			Assert.Throws<ArgumentException> (delegate {
				ho.ConvertTo_ (ho.GetType (), true);
			}, "Type,true");
			Assert.Throws<ArgumentException> (delegate {
				ho.ConvertTo_ (typeof(int), false);
			}, "Type,false");

			Assert.Throws<ArgumentException> (delegate {
				ho.ConvertTo<HtmlObjectPoker> ();
			}, "ConvertTo<HtmlObjectPoker>");
			Assert.Throws<ArgumentException> (delegate {
				ho.ConvertTo<int> ();
			}, "ConvertTo<int>");
		}
	}
}

