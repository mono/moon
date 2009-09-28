//
// Unit tests for System.Uri
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

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System {

	[TestClass]
	public class UriTest {

		[TestMethod]
		public void File ()
		{
			string s = "file:///dir1%2f..%2fdir%2fapp.xap#header";
			Uri uri = new Uri (s);
			Assert.AreEqual ("/dir/app.xap", uri.AbsolutePath, "AbsolutePath");
			// default port is removed
			Assert.AreEqual ("file:///dir/app.xap#header", uri.AbsoluteUri, "AbsoluteUri");
			Assert.AreEqual (String.Empty, uri.DnsSafeHost, "DnsSafeHost");
			Assert.AreEqual ("#header", uri.Fragment, "Fragment");
			Assert.AreEqual (String.Empty, uri.Host, "Host");
			Assert.IsTrue (uri.IsAbsoluteUri, "IsAbsoluteUri");
			Assert.IsFalse (uri.IsUnc, "IsUnc");
			Assert.AreEqual ("/dir/app.xap", uri.LocalPath, "LocalPath");
			Assert.AreEqual (s, uri.OriginalString, "OriginalString");
			Assert.AreEqual (-1, uri.Port, "Port");
			Assert.AreEqual (String.Empty, uri.Query, "Query");
			Assert.AreEqual ("file", uri.Scheme, "Scheme");
			Assert.IsFalse (uri.UserEscaped, "UserEscaped");
			Assert.AreEqual (String.Empty, uri.UserInfo, "UserInfo");
			Assert.AreEqual (uri.AbsoluteUri, uri.ToString (), "ToString");
		}

		[TestMethod]
		public void HttpWithDefaultPort ()
		{
			string s = "HTTP://host.domain.com:80/app.xap";
			Uri uri = new Uri (s);
			Assert.AreEqual ("/app.xap", uri.AbsolutePath, "AbsolutePath");
			// default port is removed
			Assert.AreEqual ("http://host.domain.com/app.xap", uri.AbsoluteUri, "AbsoluteUri");
			Assert.AreEqual ("host.domain.com", uri.DnsSafeHost, "DnsSafeHost");
			Assert.AreEqual (String.Empty, uri.Fragment, "Fragment");
			Assert.AreEqual ("host.domain.com", uri.Host, "Host");
			Assert.IsTrue (uri.IsAbsoluteUri, "IsAbsoluteUri");
			Assert.IsFalse (uri.IsUnc, "IsUnc");
			Assert.AreEqual ("/app.xap", uri.LocalPath, "LocalPath");
			Assert.AreEqual (s, uri.OriginalString, "OriginalString");
			Assert.AreEqual (80, uri.Port, "Port");
			Assert.AreEqual (String.Empty, uri.Query, "Query");
			Assert.AreEqual ("http", uri.Scheme, "Scheme");
			Assert.IsFalse (uri.UserEscaped, "UserEscaped");
			Assert.AreEqual (String.Empty, uri.UserInfo, "UserInfo");
			Assert.AreEqual (uri.AbsoluteUri, uri.ToString (), "ToString");
		}

		[TestMethod]
		public void HttpWithoutPort ()
		{
			string s = "Http://host.DOMAIN.com/dir/app.xap#options";
			Uri uri = new Uri (s);
			Assert.AreEqual ("/dir/app.xap", uri.AbsolutePath, "AbsolutePath");
			Assert.AreEqual ("http://host.domain.com/dir/app.xap#options", uri.AbsoluteUri, "AbsoluteUri");
			Assert.AreEqual ("host.domain.com", uri.DnsSafeHost, "DnsSafeHost");
			Assert.AreEqual ("#options", uri.Fragment, "Fragment");
			Assert.AreEqual ("host.domain.com", uri.Host, "Host");
			Assert.IsTrue (uri.IsAbsoluteUri, "IsAbsoluteUri");
			Assert.IsFalse (uri.IsUnc, "IsUnc");
			Assert.AreEqual ("/dir/app.xap", uri.LocalPath, "LocalPath");
			Assert.AreEqual (s, uri.OriginalString, "OriginalString");
			Assert.AreEqual (80, uri.Port, "Port");
			Assert.AreEqual (String.Empty, uri.Query, "Query");
			Assert.AreEqual ("http", uri.Scheme, "Scheme");
			Assert.IsFalse (uri.UserEscaped, "UserEscaped");
			Assert.AreEqual (String.Empty, uri.UserInfo, "UserInfo");
			Assert.AreEqual (uri.AbsoluteUri, uri.ToString (), "ToString");
		}

		[TestMethod]
		public void HttpWithNonStandardPort ()
		{
			string s = "http://monkey:s3kr3t@HOST.domain.Com:8080/dir/../app.xap?option=1";
			Uri uri = new Uri (s);
			Assert.AreEqual ("/app.xap", uri.AbsolutePath, "AbsolutePath");
			// non-standard port is present
			Assert.AreEqual ("http://monkey:s3kr3t@host.domain.com:8080/app.xap?option=1", uri.AbsoluteUri, "AbsoluteUri");
			Assert.AreEqual ("host.domain.com", uri.DnsSafeHost, "DnsSafeHost");
			Assert.AreEqual (String.Empty, uri.Fragment, "Fragment");
			Assert.AreEqual ("host.domain.com", uri.Host, "Host");
			Assert.IsTrue (uri.IsAbsoluteUri, "IsAbsoluteUri");
			Assert.IsFalse (uri.IsUnc, "IsUnc");
			Assert.AreEqual ("/app.xap", uri.LocalPath, "LocalPath");
			Assert.AreEqual (s, uri.OriginalString, "OriginalString");
			Assert.AreEqual (8080, uri.Port, "Port");
			Assert.AreEqual ("?option=1", uri.Query, "Query");
			Assert.AreEqual ("http", uri.Scheme, "Scheme");
			Assert.IsFalse (uri.UserEscaped, "UserEscaped");
			Assert.AreEqual ("monkey:s3kr3t", uri.UserInfo, "UserInfo");
			Assert.AreEqual (uri.AbsoluteUri, uri.ToString (), "ToString");
		}

		[TestMethod]
		public void HttpsWithDefaultPort ()
		{
			string s = "httpS://host.domain.com:443/";
			Uri uri = new Uri (s);
			Assert.AreEqual ("/", uri.AbsolutePath, "AbsolutePath");
			// default port is removed
			Assert.AreEqual ("https://host.domain.com/", uri.AbsoluteUri, "AbsoluteUri");
			Assert.AreEqual ("host.domain.com", uri.DnsSafeHost, "DnsSafeHost");
			Assert.AreEqual (String.Empty, uri.Fragment, "Fragment");
			Assert.AreEqual ("host.domain.com", uri.Host, "Host");
			Assert.IsTrue (uri.IsAbsoluteUri, "IsAbsoluteUri");
			Assert.IsFalse (uri.IsUnc, "IsUnc");
			Assert.AreEqual ("/", uri.LocalPath, "LocalPath");
			Assert.AreEqual (s, uri.OriginalString, "OriginalString");
			Assert.AreEqual (443, uri.Port, "Port");
			Assert.AreEqual (String.Empty, uri.Query, "Query");
			Assert.AreEqual ("https", uri.Scheme, "Scheme");
			Assert.IsFalse (uri.UserEscaped, "UserEscaped");
			Assert.AreEqual (String.Empty, uri.UserInfo, "UserInfo");
			Assert.AreEqual (uri.AbsoluteUri, uri.ToString (), "ToString");
		}

		[TestMethod]
		public void HttpsWithoutPort ()
		{
			string s = "Https://host.DOMAIN.com/dir%2fapp.xap#";
			Uri uri = new Uri (s);
			Assert.AreEqual ("/dir/app.xap", uri.AbsolutePath, "AbsolutePath");
			Assert.AreEqual ("https://host.domain.com/dir/app.xap#", uri.AbsoluteUri, "AbsoluteUri");
			Assert.AreEqual ("host.domain.com", uri.DnsSafeHost, "DnsSafeHost");
			Assert.AreEqual ("#", uri.Fragment, "Fragment");
			Assert.AreEqual ("host.domain.com", uri.Host, "Host");
			Assert.IsTrue (uri.IsAbsoluteUri, "IsAbsoluteUri");
			Assert.IsFalse (uri.IsUnc, "IsUnc");
			Assert.AreEqual ("/dir/app.xap", uri.LocalPath, "LocalPath");
			Assert.AreEqual (s, uri.OriginalString, "OriginalString");
			Assert.AreEqual (443, uri.Port, "Port");
			Assert.AreEqual (String.Empty, uri.Query, "Query");
			Assert.AreEqual ("https", uri.Scheme, "Scheme");
			Assert.IsFalse (uri.UserEscaped, "UserEscaped");
			Assert.AreEqual (String.Empty, uri.UserInfo, "UserInfo");
			Assert.AreEqual (uri.AbsoluteUri, uri.ToString (), "ToString");
		}

		[TestMethod]
		public void HttpsWithNonStandardPort ()
		{
			string s = "https://monkey:s3kr3t@HOST.domain.Com:4430/dir/..%5Capp.xap?";
			Uri uri = new Uri (s);
			Assert.AreEqual ("/app.xap", uri.AbsolutePath, "AbsolutePath");
			// non-standard port is present
			Assert.AreEqual ("https://monkey:s3kr3t@host.domain.com:4430/app.xap?", uri.AbsoluteUri, "AbsoluteUri");
			Assert.AreEqual ("host.domain.com", uri.DnsSafeHost, "DnsSafeHost");
			Assert.AreEqual (String.Empty, uri.Fragment, "Fragment");
			Assert.AreEqual ("host.domain.com", uri.Host, "Host");
			Assert.IsTrue (uri.IsAbsoluteUri, "IsAbsoluteUri");
			Assert.IsFalse (uri.IsUnc, "IsUnc");
			Assert.AreEqual ("/app.xap", uri.LocalPath, "LocalPath");
			Assert.AreEqual (s, uri.OriginalString, "OriginalString");
			Assert.AreEqual (4430, uri.Port, "Port");
			Assert.AreEqual ("?", uri.Query, "Query");
			Assert.AreEqual ("https", uri.Scheme, "Scheme");
			Assert.IsFalse (uri.UserEscaped, "UserEscaped");
			Assert.AreEqual ("monkey:s3kr3t", uri.UserInfo, "UserInfo");
			Assert.AreEqual (uri.AbsoluteUri, uri.ToString (), "ToString");
		}

		[TestMethod]
		public void Relative ()
		{
			Uri relative = new Uri ("/Moonlight", UriKind.Relative);

			Assert.Throws<UriFormatException> (delegate {
				new Uri ("/Moonlight");
			}, "string");
			Assert.Throws<UriFormatException> (delegate {
				new Uri ("/Moonlight", UriKind.Absolute);
			}, "string,Absolute");

			Assert.Throws<ArgumentNullException> (delegate {
				new Uri (null, "/Moonlight");
			}, "null,string");
			Assert.Throws<ArgumentNullException> (delegate {
				new Uri (null, relative);
			}, "null,Uri");

			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				new Uri (relative, "/Moonlight");
			}, "Uri,string");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				new Uri (relative, relative);
			}, "Uri,Uri");

			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				new Uri (relative, (string)null);
			}, "Uri,string-null");
			Assert.Throws<ArgumentOutOfRangeException> (delegate {
				new Uri (relative, (Uri)null);
			}, "Uri,Uri-null");
		}

		private void CheckRelativeUri (Uri uri)
		{
			Assert.Throws<InvalidOperationException> (delegate {
				Assert.IsNotNull (uri.AbsolutePath);
			}, "AbsolutePath");
			Assert.Throws<InvalidOperationException> (delegate {
				Assert.IsNotNull (uri.AbsoluteUri);
			}, "AbsoluteUri");
			Assert.Throws<InvalidOperationException> (delegate {
				Assert.IsNotNull (uri.DnsSafeHost);
			}, "DnsSafeHost");
			Assert.Throws<InvalidOperationException> (delegate {
				Assert.IsNotNull (uri.Fragment);
			}, "Fragment");
			Assert.Throws<InvalidOperationException> (delegate {
				Assert.IsNotNull (uri.Host);
			}, "Host");

			Assert.IsFalse (uri.IsAbsoluteUri, "IsAbsoluteUri");

			Assert.Throws<InvalidOperationException> (delegate {
				Assert.IsNotNull (uri.IsUnc);
			}, "IsUnc");
			Assert.Throws<InvalidOperationException> (delegate {
				Assert.IsNotNull (uri.LocalPath);
			}, "LocalPath");

			Assert.AreEqual ("/Moonlight", uri.OriginalString, "OriginalString");

			Assert.Throws<InvalidOperationException> (delegate {
				Assert.IsNotNull (uri.Port);
			}, "Port");
			Assert.Throws<InvalidOperationException> (delegate {
				Assert.IsNotNull (uri.Query);
			}, "Query");
			Assert.Throws<InvalidOperationException> (delegate {
				Assert.IsNotNull (uri.Scheme);
			}, "Scheme");

			Assert.IsFalse (uri.UserEscaped, "UserEscaped");

			Assert.Throws<InvalidOperationException> (delegate {
				Assert.IsNotNull (uri.UserInfo);
			}, "UserInfo");

			Assert.AreEqual ("/Moonlight", uri.ToString (), "ToString");
		}

		[TestMethod]
		public void Relative_AsRelative ()
		{
			Uri uri = new Uri ("/Moonlight", UriKind.Relative);
			CheckRelativeUri (uri);
		}

		[TestMethod]
		public void Relative_AsRelativeOrAbsolute ()
		{
			Uri uri = new Uri ("/Moonlight", UriKind.RelativeOrAbsolute);
			CheckRelativeUri (uri);
		}

		[TestMethod]
		public void Host ()
		{
			Assert.Throws<UriFormatException> (delegate {
				new Uri ("http://$myhost/test.text");
			}, "one invalid character in host name");
			Assert.Throws<UriFormatException> (delegate {
				new Uri ("https://!@#$%^&*()/index.html");
			}, "many invalid characters in host name");
		}
	}
}

