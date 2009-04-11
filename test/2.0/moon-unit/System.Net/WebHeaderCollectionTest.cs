//
// Unit tests for System.Net.WebHeaderCollection
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
using System.Net;
using System.Net.Sockets;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Net {

	[TestClass]
	public class WebHeaderCollectionTest {

		[TestMethod]
		public void DefaultValues ()
		{
			WebHeaderCollection whc = new WebHeaderCollection ();
			Assert.AreEqual (0, whc.AllKeys.Length, "AllKeys");
			Assert.AreEqual (0, whc.Count, "Count");
		}

		[TestMethod]
		public void Items_Get_String ()
		{
			WebHeaderCollection whc = new WebHeaderCollection ();
			Assert.Throws<ArgumentNullException> (delegate {
				Assert.IsNull (whc [null]);
			}, "Items[null]");

			Assert.IsNull (whc [String.Empty], "String.Empty");
			Assert.IsNull (whc ["cache-control"], "cache-control");
			Assert.IsNull (whc ["zzMoonlightZz"], "zzMoonlightZz");
		}

		[TestMethod]
		public void Items_Set_String ()
		{
			WebHeaderCollection whc = new WebHeaderCollection ();
			Assert.Throws<ArgumentNullException> (delegate {
				whc [null] = "null";
			}, "null");
			Assert.Throws<ArgumentException> (delegate {
				whc [String.Empty] = "empty";
			}, "Empty");

			whc ["cache-control"] = "none";
			Assert.AreEqual (1, whc.AllKeys.Length, "AllKeys-1");
			Assert.AreEqual (1, whc.Count, "Count-1");
			Assert.AreEqual ("none", whc ["cache-control"], "this[string]");
			Assert.AreEqual ("none", whc [HttpRequestHeader.CacheControl], "this[HttpRequestHeader]");

			whc ["zzMoonlightZz"] = "unknown";
			Assert.AreEqual (2, whc.AllKeys.Length, "AllKeys-2");
			Assert.AreEqual (2, whc.Count, "Count-2");
			Assert.AreEqual ("unknown", whc ["zzMoonlightZz"], "this[string]-2");

			// setting to null does not remove entries from the collection
			whc ["cache-control"] = null;
			Assert.AreEqual (2, whc.AllKeys.Length, "AllKeys-3");
			Assert.AreEqual (2, whc.Count, "Count-3");
		}

		[TestMethod]
		public void Items_Get_HttpRequestHeader ()
		{
			WebHeaderCollection whc = new WebHeaderCollection ();
			Assert.IsNull (whc [HttpRequestHeader.CacheControl], "CacheControl");
			Assert.IsNull (whc [HttpRequestHeader.Connection], "Connection");
			Assert.IsNull (whc [HttpRequestHeader.Date], "Date");
			Assert.IsNull (whc [HttpRequestHeader.KeepAlive], "KeepAlive");
			Assert.IsNull (whc [HttpRequestHeader.Pragma], "Pragma");
			Assert.IsNull (whc [HttpRequestHeader.Trailer], "Trailer");
			Assert.IsNull (whc [HttpRequestHeader.TransferEncoding], "TransferEncoding");
			Assert.IsNull (whc [HttpRequestHeader.Upgrade], "Upgrade");
			Assert.IsNull (whc [HttpRequestHeader.Via], "Via");
			Assert.IsNull (whc [HttpRequestHeader.Warning], "Warning");
			Assert.IsNull (whc [HttpRequestHeader.Allow], "Allow");
			Assert.IsNull (whc [HttpRequestHeader.ContentLength], "ContentLength");
			Assert.IsNull (whc [HttpRequestHeader.ContentType], "ContentType");
			Assert.IsNull (whc [HttpRequestHeader.ContentEncoding], "ContentEncoding");
			Assert.IsNull (whc [HttpRequestHeader.ContentLanguage], "ContentLanguage");
			Assert.IsNull (whc [HttpRequestHeader.ContentLocation], "ContentLocation");
			Assert.IsNull (whc [HttpRequestHeader.ContentMd5], "ContentMd5");
			Assert.IsNull (whc [HttpRequestHeader.ContentRange], "ContentRange");
			Assert.IsNull (whc [HttpRequestHeader.Expires], "Expires");
			Assert.IsNull (whc [HttpRequestHeader.LastModified], "LastModified");
			Assert.IsNull (whc [HttpRequestHeader.Accept], "Accept");
			Assert.IsNull (whc [HttpRequestHeader.AcceptCharset], "AcceptCharset");
			Assert.IsNull (whc [HttpRequestHeader.AcceptEncoding], "AcceptEncoding");
			Assert.IsNull (whc [HttpRequestHeader.AcceptLanguage], "AcceptLanguage");
			Assert.IsNull (whc [HttpRequestHeader.Authorization], "Authorization");
			Assert.IsNull (whc [HttpRequestHeader.Cookie], "Cookie");
			Assert.IsNull (whc [HttpRequestHeader.Expect], "Expect");
			Assert.IsNull (whc [HttpRequestHeader.From], "From");
			Assert.IsNull (whc [HttpRequestHeader.Host], "Host");
			Assert.IsNull (whc [HttpRequestHeader.IfMatch], "IfMatch");
			Assert.IsNull (whc [HttpRequestHeader.IfModifiedSince], "IfModifiedSince");
			Assert.IsNull (whc [HttpRequestHeader.IfNoneMatch], "IfNoneMatch");
			Assert.IsNull (whc [HttpRequestHeader.IfRange], "IfRange");
			Assert.IsNull (whc [HttpRequestHeader.IfUnmodifiedSince], "IfUnmodifiedSince");
			Assert.IsNull (whc [HttpRequestHeader.MaxForwards], "MaxForwards");
			Assert.IsNull (whc [HttpRequestHeader.ProxyAuthorization], "ProxyAuthorization");
			Assert.IsNull (whc [HttpRequestHeader.Referer], "Referer");
			Assert.IsNull (whc [HttpRequestHeader.Range], "Range");
			Assert.IsNull (whc [HttpRequestHeader.Te], "Te");
			Assert.IsNull (whc [HttpRequestHeader.Translate], "Translate");
			Assert.IsNull (whc [HttpRequestHeader.UserAgent], "UserAgent");

			Assert.Throws<IndexOutOfRangeException> (delegate {
				Assert.IsNull (whc [(HttpRequestHeader) Int32.MinValue]);
			}, "MinValue");
		}

		[TestMethod]
		public void Items_Set_HttpRequestHeader ()
		{
			WebHeaderCollection whc = new WebHeaderCollection ();
			Assert.Throws<IndexOutOfRangeException> (delegate {
				whc [(HttpRequestHeader) Int32.MinValue] = "invalid";
			}, "invalid");

			whc [HttpRequestHeader.CacheControl] = "none";
			Assert.AreEqual (1, whc.AllKeys.Length, "AllKeys-1");
			Assert.AreEqual (1, whc.Count, "Count-1");
			Assert.AreEqual ("none", whc ["cache-control"], "this[string]");
			Assert.AreEqual ("none", whc [HttpRequestHeader.CacheControl], "this[HttpRequestHeader]");

			whc [HttpRequestHeader.CacheControl] = null;
			Assert.AreEqual (1, whc.AllKeys.Length, "AllKeys-2");
			Assert.AreEqual (1, whc.Count, "Count-2");
		}
	}
}

