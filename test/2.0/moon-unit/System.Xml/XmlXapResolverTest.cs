//
// Unit tests for System.Xml.XmlXapResolver
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
using System.IO;
using System.Windows;
using System.Xml;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Xml {

	[TestClass]
	public class XmlXapResolverTest {

		[TestMethod]
		public void GetEntity ()
		{
			XmlXapResolver xr = new XmlXapResolver ();

			Assert.Throws<ArgumentNullException> (delegate {
				xr.GetEntity (null, "role", typeof (Stream));
			}, "null,string,type");

			Uri absolute = new Uri ("http://www.mono-project.com/", UriKind.Absolute);

			Assert.Throws<XmlException> (delegate {
				xr.GetEntity (absolute, "role", typeof (Stream));
			}, "absolute,string,type");

			Uri relative = new Uri (String.Empty, UriKind.Relative);
			Assert.Throws<IndexOutOfRangeException> (delegate {
				xr.GetEntity (relative, "role", typeof (Stream));
			}, "empty,string,type");

			relative = new Uri ("AppManifest.xaml", UriKind.Relative);

			Stream s = (Stream) xr.GetEntity (relative, null, typeof (Stream));
			Assert.IsNotNull (s, "relative,null,type");
			Assert.IsTrue (s is Stream, "Stream");

			s = (Stream) xr.GetEntity (relative, String.Empty, null);
			Assert.IsNotNull (s, "relative,empty,null");

			Assert.Throws<XmlException> (delegate {
				xr.GetEntity (relative, null, s.GetType ());
			}, "relative,string,type/bad");

			absolute = new Uri (Application.Current.Host.Source, relative);
			Assert.Throws<XmlException> (delegate {
				xr.GetEntity (absolute, String.Empty, null);
			}, "absolute(app),empty,null");

			absolute = new Uri (Application.Current.Host.Source + "#AppManifest.xaml");
			Assert.Throws<XmlException> (delegate {
				xr.GetEntity (absolute, String.Empty, null);
			}, "absolute(fragment),empty,null");

			relative = new Uri ("does-not-exists.file", UriKind.Relative);
			Assert.Throws<XmlException> (delegate {
				xr.GetEntity (relative, String.Empty, null);
			}, "relative(doesnotexists),empty,null");
		}
	}
}

