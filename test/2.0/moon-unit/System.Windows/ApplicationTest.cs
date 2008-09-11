//
// Unit tests for System.Windows.Application
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008 Novell, Inc (http://www.novell.com)
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
using System.IO;
using System.Windows;
using System.Windows.Media;
using System.Windows.Resources;
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.Windows {

	[TestClass]
	public class ApplicationTest {

		static private Uri uri = new Uri ("http://www.mono-project.com");

		void Check (Application app)
		{
			Assert.IsNotNull (app.Host, "Host");

			// not yet implemented in moonlight
//			Assert.IsNotNull (app.Resources, "Resources");

			Assert.IsNotNull (app.RootVisual, "RootVisual");
		}

		[TestMethod]
		[KnownFailure]
		public void New ()
		{
			Application app = new Application ();
			// AFAIK this is *nearly* identical to Application.Current
			Check (app);
			// funny thing is that creating a SilverlightHost defaults to true :|
			Assert.IsFalse (app.Host.IsLoaded, "Host.IsLoaded");
		}

		[TestMethod]
		[KnownFailure]
		public void Current ()
		{
			Application app = Application.Current;
			Check (app);
			Assert.IsTrue (app.Host.IsLoaded, "Host.IsLoaded");
		}

		[TestMethod]
		public void Static ()
		{
			Assert.Throws (delegate { Application.GetResourceStream (null); }, typeof (ArgumentNullException), "GetResourceStream(null)");

			Assert.Throws (delegate { Application.GetResourceStream (null, uri); }, typeof (ArgumentNullException), "GetResourceStream(null,uri)");

			StreamResourceInfo sri = new StreamResourceInfo (new MemoryStream (), String.Empty);
			Assert.Throws (delegate { Application.GetResourceStream (sri, null); }, typeof (ArgumentNullException), "GetResourceStream(sri,null)");

			Assert.Throws (delegate { Application.LoadComponent (null, uri); }, typeof (ArgumentException), "LoadComponent(null,uri)");
		}

		[TestMethod]
		[KnownFailure]
		public void LoadComponent_Application ()
		{
			// Exception message says the component is expected to be a Application or DependencyObject
			// however it does not accept Application (and MSDN does talk about anything other than a DO)
			Assert.Throws (delegate { Application.LoadComponent (Application.Current, null); }, typeof (ArgumentException), "LoadComponent(app,null)");
			Assert.Throws (delegate { Application.LoadComponent (Application.Current, uri); }, typeof (ArgumentException), "LoadComponent(app,uri)");
		}
	}
}
