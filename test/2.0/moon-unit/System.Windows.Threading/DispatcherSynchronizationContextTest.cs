//
// Unit tests for DispatcherSynchronizationContext
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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
using System.ComponentModel;
using System.Windows;
using System.Windows.Threading;

using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Threading {

	[TestClass]
	public class DispatcherSynchronizationContextTest : SilverlightTest {

		[TestMethod]
		public void Default ()
		{
			DispatcherSynchronizationContext dsc = new DispatcherSynchronizationContext ();
			Assert.AreNotSame (dsc, dsc.CreateCopy (), "CreateCopy");
		}

		[TestMethod]
		public void UserSuppliedDispatcher ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				new DispatcherSynchronizationContext (null);
			}, ".ctor(null)");

			Dispatcher d = Application.Current.RootVisual.Dispatcher;
			DispatcherSynchronizationContext dsc = new DispatcherSynchronizationContext (d);
			Assert.AreNotSame (dsc, dsc.CreateCopy (), "CreateCopy");
		}

		[TestMethod]
		[Asynchronous]
		public void Send ()
		{
			DispatcherSynchronizationContext dsc = new DispatcherSynchronizationContext ();
			// SL throw an NRE while moonlight does not
#if false
			Assert.Throws<NullReferenceException> (delegate {
				dsc.Send (null, this);
			}, "Send(null,object)");
#endif
			bool complete = false;
			dsc.Send (delegate (object obj) {
				Assert.IsNull (obj, "Send");
				complete = true;
			}, null);

			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void Post ()
		{
			DispatcherSynchronizationContext dsc = new DispatcherSynchronizationContext ();
			// that would throw a NRE but we can't catch it
			//dsc.Post (null, this);

			bool complete = false;
			dsc.Post (delegate (object obj) {
				Assert.IsNull (obj, "Post");
				complete = true;
			}, null);

			EnqueueConditional (() => complete);
			EnqueueTestComplete ();
		}
	}
}

