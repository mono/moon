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
			// Fails in Silverlight 3
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
			Assert.AreEqual ("unknown", whc ["zzMoonlightZz"], "this[string]-2a");
			Assert.AreEqual ("unknown", whc ["ZZmoonlightzZ"], "this[string]-2b");
			Assert.AreEqual ("unknown", whc ["zzmoonlightzz"], "this[string]-2c");
			Assert.AreEqual ("unknown", whc ["ZZMOONLIGHTZZ"], "this[string]-2d");

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
			// Fails in Silverlight 3
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

		static public string HttpRequestHeaderToString (HttpRequestHeader header)
		{
			switch (header) {
			case HttpRequestHeader.CacheControl:
				return "cache-control";
			case HttpRequestHeader.Connection:
				return "connection";
			case HttpRequestHeader.Date:
				return "date";
			case HttpRequestHeader.KeepAlive:
				return "keep-alive";
			case HttpRequestHeader.Pragma:
				return "pragma";
			case HttpRequestHeader.Trailer:
				return "trailer";
			case HttpRequestHeader.TransferEncoding:
				return "transfer-encoding";
			case HttpRequestHeader.Upgrade:
				return "upgrade";
			case HttpRequestHeader.Via:
				return "via";
			case HttpRequestHeader.Warning:
				return "warning";
			case HttpRequestHeader.Allow:
				return "allow";
			case HttpRequestHeader.ContentLength:
				return "content-length";
			case HttpRequestHeader.ContentType:
				return "content-type";
			case HttpRequestHeader.ContentEncoding:
				return "content-encoding";
			case HttpRequestHeader.ContentLanguage:
				return "content-language";
			case HttpRequestHeader.ContentLocation:
				return "content-location";
			case HttpRequestHeader.ContentMd5:
				return "content-md5";
			case HttpRequestHeader.ContentRange:
				return "content-range";
			case HttpRequestHeader.Expires:
				return "expires";
			case HttpRequestHeader.LastModified:
				return "last-modified";
			case HttpRequestHeader.Accept:
				return "accept";
			case HttpRequestHeader.AcceptCharset:
				return "accept-charset";
			case HttpRequestHeader.AcceptEncoding:
				return "accept-encoding";
			case HttpRequestHeader.AcceptLanguage:
				return "accept-language";
			case HttpRequestHeader.Authorization:
				return "authorization";
			case HttpRequestHeader.Cookie:
				return "cookie";
			case HttpRequestHeader.Expect:
				return "expect";
			case HttpRequestHeader.From:
				return "from";
			case HttpRequestHeader.Host:
				return "host";
			case HttpRequestHeader.IfMatch:
				return "if-match";
			case HttpRequestHeader.IfModifiedSince:
				return "if-modified-since";
			case HttpRequestHeader.IfNoneMatch:
				return "if-none-match";
			case HttpRequestHeader.IfRange:
				return "if-range";
			case HttpRequestHeader.IfUnmodifiedSince:
				return "if-unmodified-since";
			case HttpRequestHeader.MaxForwards:
				return "max-forwards";
			case HttpRequestHeader.ProxyAuthorization:
				return "proxy-authorization";
			case HttpRequestHeader.Referer:
				return "referer";
			case HttpRequestHeader.Range:
				return "range";
			case HttpRequestHeader.Te:
				return "te";
			case HttpRequestHeader.Translate:
				return "translate";
			case HttpRequestHeader.UserAgent:
				return "user-agent";
			default:
				throw new IndexOutOfRangeException ();
			}
		}

		[TestMethod]
		public void SetHeaders_HttpRequestHeader ()
		{
			// every value can be added to a collection that is not associated with a WebRequest
			WebHeaderCollection whc = new WebHeaderCollection ();
			// Enum.GetValues is not available on SL :(
			for (int i = (int) HttpRequestHeader.CacheControl; i <= (int) HttpRequestHeader.UserAgent; i++) {
				HttpRequestHeader hrh = (HttpRequestHeader) i;
				string header = HttpRequestHeaderToString (hrh);
				string s = i.ToString ();

				whc [hrh] = s;
				Assert.AreEqual (s, whc [hrh], "HttpRequestHeader." + header);
				whc [header] = s;
				Assert.AreEqual (s, whc [header], header);
			}
		}

		[TestMethod]
		public void Names_FromHttpRequestHeader ()
		{
			WebHeaderCollection whc = new WebHeaderCollection ();
			// Enum.GetValues is not available on SL :(
			for (int i = (int) HttpRequestHeader.CacheControl; i <= (int) HttpRequestHeader.UserAgent; i++) {
				HttpRequestHeader hrh = (HttpRequestHeader) i;
				whc [hrh] = hrh.ToString ();
			}
			Assert.AreEqual (41, whc.Count, "Count");
			string [] keys = whc.AllKeys;
			foreach (string key in keys) {
				// some names are equals to Enum.ToString
				if (whc [key] == key)
					continue;
				// while others are not (> 1 word)
				switch (key) {
				case "Cache-Control":
				case "Keep-Alive":
				case "Transfer-Encoding":
				case "Content-Length":
				case "Content-Type":
				case "Content-Encoding":
				case "Content-Language":
				case "Content-Location":
				case "Content-Range":
				case "Last-Modified":
				case "Accept-Charset":
				case "Accept-Encoding":
				case "Accept-Language":
				case "If-Match":
				case "If-Modified-Since":
				case "If-None-Match":
				case "If-Range":
				case "If-Unmodified-Since":
				case "Max-Forwards":
				case "Proxy-Authorization":
				case "User-Agent":
					Assert.AreEqual (key.Replace ("-", String.Empty), whc [key], key);
					break;
				case "Content-MD5":
					Assert.AreEqual ("ContentMd5", whc [key], key);
					break;
				case "TE":
					Assert.AreEqual ("Te", whc [key], key);
					break;
				default:
					Assert.Fail (key);
					break;
				}
			}
		}

		[TestMethod]
		public void Names_String ()
		{
			WebHeaderCollection whc = new WebHeaderCollection ();
			// Enum.GetValues is not available on SL :(
			for (int i = (int) HttpRequestHeader.CacheControl; i <= (int) HttpRequestHeader.UserAgent; i++) {
				HttpRequestHeader hrh = (HttpRequestHeader) i;
				// string is kept as supplied
				if ((i % 2) == 1)
					whc [HttpRequestHeaderToString (hrh).ToLower ()] = hrh.ToString ().ToLower ();
				else
					whc [HttpRequestHeaderToString (hrh).ToUpper ()] = hrh.ToString ().ToUpper ();
			}
			Assert.AreEqual (41, whc.Count, "Count");
			string [] keys = whc.AllKeys;
			foreach (string key in keys) {
				// some names are equals to Enum.ToString
				if (whc [key] == key)
					continue;
				// while others are not (> 1 word)
				switch (key) {
				case "CACHE-CONTROL":
				case "keep-alive":
				case "TRANSFER-ENCODING":
				case "content-length":
				case "CONTENT-TYPE":
				case "content-encoding":
				case "CONTENT-LANGUAGE":
				case "content-location":
				case "content-range":
				case "last-modified":
				case "accept-charset":
				case "ACCEPT-ENCODING":
				case "accept-language":
				case "if-match":
				case "IF-MODIFIED-SINCE":
				case "if-none-match":
				case "IF-RANGE":
				case "if-unmodified-since":
				case "MAX-FORWARDS":
				case "proxy-authorization":
				case "USER-AGENT":
					Assert.AreEqual (key.Replace ("-", String.Empty), whc [key], key);
					break;
				case "CONTENT-MD5":
					Assert.AreEqual ("CONTENTMD5", whc [key], key);
					break;
				case "TE":
					Assert.AreEqual ("Te", whc [key], key);
					break;
				default:
					Assert.Fail (key);
					break;
				}
			}
		}
	}
}

