//
// Web Base Policy Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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
using System.Windows.Browser.Net;
using System.Xml;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.Security {

	// Test domain and headers parsing/matching that is shared between the
	// Silverlight and Flash policies code

	[TestClass]
	public class WebBasePolicyTest {

		class ConcreteDomainPolicy : BaseDomainPolicy {

			public ConcreteDomainPolicy ()
			{
				Headers = new Headers ();
			}

			public Headers Headers { get; private set; }

			public override bool IsAllowed (Uri uri, params string [] headerKeys)
			{
				throw new NotImplementedException ();
			}
		}

		ConcreteDomainPolicy.Headers Parse (string raw)
		{
			ConcreteDomainPolicy pol = new ConcreteDomainPolicy ();
			pol.Headers.SetHeaders (raw);
			return pol.Headers;
		}

		string [] HeadersContentType = new string [] { "Content-Type" };
		string [] HeadersContentTypeUp = new string [] { "CONTENT-TYPE" };
		string [] HeadersContentTypeLow = new string [] { "content-type" };

		string [] HeadersSOAPAction1 = new string [] { "SOAPAction" };
		string [] HeadersSOAPAction2 = new string [] { "Content-Type", "SOAPAction" };
		string [] HeadersSOAPAction3 = new string [] { "SOAPActionX" };

		string [] XHeaders1 = new string [] { "x-flash-version" };
		string [] XHeaders2 = new string [] { "X-My-Header" };
		string [] XHeaders3 = new string [] { "x-flash-version", "X-My-Header" };

		string [] FlashHeaders1 = new string [] { "x-flash-version" };
		string [] FlashHeaders2 = new string [] { "X-Permitted-Cross-Domain-Policies" };

		[TestMethod]
		public void Headers_AllowAll ()
		{
			ConcreteDomainPolicy.Headers h = Parse ("*");

			Assert.IsTrue (h.AllowAllHeaders, "AllowAllHeaders");

			Assert.IsTrue (h.IsAllowed (HeadersContentType), "Content-Type");
			Assert.IsTrue (h.IsAllowed (HeadersContentTypeUp), "CONTENT-TYPE");
			Assert.IsTrue (h.IsAllowed (HeadersContentTypeLow), "content-type");

			Assert.IsTrue (h.IsAllowed (HeadersSOAPAction1), "SOAPAction");
			Assert.IsTrue (h.IsAllowed (HeadersSOAPAction2), "Content-Type, SOAPAction");
			Assert.IsTrue (h.IsAllowed (HeadersSOAPAction3), "SOAPActionX");

			Assert.IsTrue (h.IsAllowed (FlashHeaders1), "x-flash-version");
			Assert.IsTrue (h.IsAllowed (FlashHeaders2), "X-Permitted-Cross-Domain-Policies");
		}

		private void NullOrEmpty (ConcreteDomainPolicy.Headers h)
		{
			Assert.IsFalse (h.AllowAllHeaders, "AllowAllHeaders");

			Assert.IsTrue (h.IsAllowed (HeadersContentType), "Content-Type");
			Assert.IsTrue (h.IsAllowed (HeadersContentTypeUp), "CONTENT-TYPE");
			Assert.IsTrue (h.IsAllowed (HeadersContentTypeLow), "content-type");

			Assert.IsFalse (h.IsAllowed (HeadersSOAPAction1), "SOAPAction");
			Assert.IsFalse (h.IsAllowed (HeadersSOAPAction2), "Content-Type, SOAPAction");
			Assert.IsFalse (h.IsAllowed (HeadersSOAPAction3), "SOAPActionX");

			Assert.IsFalse (h.IsAllowed (FlashHeaders1), "x-flash-version");
			Assert.IsFalse (h.IsAllowed (FlashHeaders2), "X-Permitted-Cross-Domain-Policies");
		}

		[TestMethod]
		public void Headers_Null ()
		{
			NullOrEmpty (Parse (null));
		}

		[TestMethod]
		public void Headers_Empty ()
		{
			NullOrEmpty (Parse (String.Empty));
		}

		[TestMethod]
		public void Headers_SOAPAction ()
		{
			ConcreteDomainPolicy.Headers h = Parse ("SOAPAction");

			Assert.IsFalse (h.AllowAllHeaders, "AllowAllHeaders");

			Assert.IsTrue (h.IsAllowed (HeadersContentType), "Content-Type");
			Assert.IsTrue (h.IsAllowed (HeadersContentTypeUp), "CONTENT-TYPE");
			Assert.IsTrue (h.IsAllowed (HeadersContentTypeLow), "content-type");

			Assert.IsTrue (h.IsAllowed (HeadersSOAPAction1), "SOAPAction");
			Assert.IsTrue (h.IsAllowed (HeadersSOAPAction2), "Content-Type, SOAPAction");
			Assert.IsFalse (h.IsAllowed (HeadersSOAPAction3), "SOAPActionX");

			Assert.IsFalse (h.IsAllowed (FlashHeaders1), "x-flash-version");
			Assert.IsFalse (h.IsAllowed (FlashHeaders2), "X-Permitted-Cross-Domain-Policies");
		}

		[TestMethod]
		public void Headers_SOAPActionWild ()
		{
			ConcreteDomainPolicy.Headers h = Parse ("SOAPAction*");

			Assert.IsFalse (h.AllowAllHeaders, "AllowAllHeaders");

			Assert.IsTrue (h.IsAllowed (HeadersContentType), "Content-Type");
			Assert.IsTrue (h.IsAllowed (HeadersContentTypeUp), "CONTENT-TYPE");
			Assert.IsTrue (h.IsAllowed (HeadersContentTypeLow), "content-type");

			Assert.IsTrue (h.IsAllowed (HeadersSOAPAction1), "SOAPAction");
			Assert.IsTrue (h.IsAllowed (HeadersSOAPAction2), "Content-Type, SOAPAction");
			Assert.IsTrue (h.IsAllowed (HeadersSOAPAction3), "SOAPActionX");

			Assert.IsFalse (h.IsAllowed (FlashHeaders1), "x-flash-version");
			Assert.IsFalse (h.IsAllowed (FlashHeaders2), "X-Permitted-Cross-Domain-Policies");
		}
	}
}

