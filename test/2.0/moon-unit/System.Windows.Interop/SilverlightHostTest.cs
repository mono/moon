//
// Unit tests for System.Windows.Interop.SilverlightHost
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
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Resources;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows {

	[TestClass]
	public class SilverlightHostTest {

		static private Uri uri = new Uri ("http://www.mono-project.com");

		void Check (SilverlightHost host)
		{
			Assert.AreEqual (Colors.White, host.Background, "Background");
			Assert.IsNotNull (host.Content, "Content");
			Assert.IsTrue (host.IsLoaded, "IsLoaded");
			Assert.IsNotNull (host.Settings, "Settings");
			Assert.IsTrue (host.Source.AbsoluteUri.EndsWith ("moon-unit.xap"), "Source");
		}

		[TestMethod]
		public void New ()
		{
			SilverlightHost host = new SilverlightHost ();
			Check (host);
		}

		[TestMethod]
		public void Current ()
		{
			SilverlightHost host = Application.Current.Host;
			Check (host);
		}

		[TestMethod]
		[SilverlightBug(PlatformID.MacOSX)]
		public void IsVersionSupported ()
		{
			SilverlightHost host = new SilverlightHost ();

			Assert.Throws (delegate { host.IsVersionSupported (null); }, typeof (NullReferenceException), "IsVersionSupported(null)");

			Assert.IsFalse (host.IsVersionSupported (String.Empty), "Empty");
			Assert.IsFalse (host.IsVersionSupported ("1"), "1");
			Assert.IsFalse (host.IsVersionSupported ("1."), "1.");
			Assert.IsTrue  (host.IsVersionSupported ("1.0"), "1.0");
			Assert.IsFalse (host.IsVersionSupported ("1.0."), "1.0.");
			Assert.IsTrue  (host.IsVersionSupported ("1.0.0"), "1.0.0");
			Assert.IsFalse (host.IsVersionSupported ("1.0.0."), "1.0.0.");
			Assert.IsTrue  (host.IsVersionSupported ("1.0.0.0"), "1.0.0.0");
			Assert.IsFalse (host.IsVersionSupported ("1.0.0.0."), "1.0.0.0.");
			Assert.IsFalse (host.IsVersionSupported ("1.0.0.0.0"), "1.0.0.0.0");
		}
	}
}
