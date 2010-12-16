//
// Web Silverlight Policy Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009-2010 Novell, Inc.
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
using System.Net.Policy;
using System.Text;
using System.Xml;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.Security {

	[TestClass]
	public class WebSilverlightPolicyTest {

		static ClientAccessPolicy GetPolicy (string policy)
		{
			using (MemoryStream ms = new MemoryStream (Encoding.UTF8.GetBytes (policy))) {
				return (ClientAccessPolicy) ClientAccessPolicy.FromStream (ms);
			}
		}

		Uri http = new Uri ("http://www.host.com/app.xap");
		// Uri https = new Uri ("https://secure.host.net/app.xap");
		// Uri file = new Uri ("file:///app.xap");

		// Uri http_non_standard_port = new Uri ("http://www.host.com:8080/app.xap");

		[TestMethod]
		public void AllDomainsAllSubPaths ()
		{
			string policy = @"<?xml version='1.0'?>
<access-policy>
<cross-domain-access>
<policy>
<allow-from>
	<domain uri='*'/>
</allow-from>
<grant-to>
	<resource path='/' include-subpaths='true'/>
</grant-to>
</policy>
</cross-domain-access>
</access-policy>
";
			ClientAccessPolicy cap = GetPolicy (policy);

			ClientAccessPolicy.ApplicationUri = new Uri ("http://localhost/");

			Assert.IsTrue (cap.IsAllowed (new Uri ("http://jolt-web01-a03/Resources/test.txt"), "GET", null), "");
		}

		[TestMethod]
		public void AllDomainsNoSubPaths ()
		{
			string policy = @"<?xml version='1.0'?>
<access-policy>
<cross-domain-access>
<policy>
<allow-from>
	<domain uri='*'/>
</allow-from>
<grant-to>
	<resource path='/'/>
</grant-to>
</policy>
</cross-domain-access>
</access-policy>
";
			ClientAccessPolicy cap = GetPolicy (policy);

			ClientAccessPolicy.ApplicationUri = http;

			// this one needs validation
			Assert.IsFalse (cap.IsAllowed (new Uri ("http://local/file.txt"), "GET", null), "");
			Assert.IsFalse (cap.IsAllowed (new Uri ("http://local/dir/file.txt"), "GET", null), "");
		}

		[TestMethod]
		public void AllowSingleHost ()
		{
			string policy = @"<?xml version='1.0'?>
<access-policy>
<cross-domain-access>
<policy>
<allow-from>
	<domain uri='http://localhost/'/>
</allow-from>
<grant-to>
	<resource path='/' include-subpaths='true'/>
</grant-to>
</policy>
</cross-domain-access>
</access-policy>
";
			// the policy allows callers from http://localhost only
			ClientAccessPolicy cap = GetPolicy (policy);

			// and this is not localhost, the url we want to access is not helpful
			ClientAccessPolicy.ApplicationUri = http;
			Assert.IsFalse (cap.IsAllowed (new Uri ("http://localhost/some/data/file.txt"), "GET", null), "");

			Uri localhost = new Uri ("http://localhost/local.xap");
			ClientAccessPolicy.ApplicationUri = localhost;
			Assert.IsTrue (cap.IsAllowed (new Uri ("http://localhost/some/data/file.txt"), "GET", null), "");
		}

		[TestMethod]
		public void GrantSpecificPathAndSubpaths ()
		{
			string policy = @"<?xml version='1.0'?>
<access-policy>
<cross-domain-access>
<policy>
<allow-from>
	<domain uri='*'/>
</allow-from>
<grant-to>
	<resource path='/data' include-subpaths='true'/>
</grant-to>
</policy>
</cross-domain-access>
</access-policy>
";
			ClientAccessPolicy cap = GetPolicy (policy);

			ClientAccessPolicy.ApplicationUri = http;
			Assert.IsTrue (cap.IsAllowed (new Uri ("http://localhost/data/file.txt"), "GET", null), "");
			Assert.IsFalse (cap.IsAllowed (new Uri ("http://localhost/data/./file.txt"), "GET", null), "");
		}

		[TestMethod]
		public void NoRequestHeadersAllowed ()
		{
			string policy = @"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from>
				<domain uri=""*"" />
			</allow-from>
			<grant-to>
				<resource path=""/"" include-subpaths=""true"" />
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>";
			// without a http-request-headers no headers (expect for Content-Type) are allowed
			ClientAccessPolicy cap = GetPolicy (policy);
			ClientAccessPolicy.ApplicationUri = http;

			Assert.IsTrue (cap.IsAllowed (http, "GET", null), "null");
			Assert.IsTrue (cap.IsAllowed (http, "GET", new string [0]), "none");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "Content-Type"), "Content-Type");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "content-type"), "content-type");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "CONTENT-TYPE"), "CONTENT-TYPE");
			Assert.IsFalse (cap.IsAllowed (http, "GET", "Content-*"), "Content-*");
		}

		[TestMethod]
		public void AllowAllRequestHeaders ()
		{
			string policy = @"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from http-request-headers=""*"">
				<domain uri=""*"" />
			</allow-from>
			<grant-to>
				<resource path=""/"" include-subpaths=""true"" />
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);
			ClientAccessPolicy.ApplicationUri = http;

			Assert.IsTrue (cap.IsAllowed (http, "GET", null), "null");
			Assert.IsTrue (cap.IsAllowed (http, "GET", new string [0]), "none");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "Content-Type"), "Content-Type");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "content-type"), "content-type");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "CONTENT-TYPE"), "CONTENT-TYPE");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "Content-*"), "Content-*");
		}

		[TestMethod]
		public void AllowRequestHeadersWithWildcardSuffix ()
		{
			string policy = @"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from http-request-headers=""Prefix*"">
				<domain uri=""*"" />
			</allow-from>
			<grant-to>
				<resource path=""/"" include-subpaths=""true"" />
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);
			ClientAccessPolicy.ApplicationUri = http;

			Assert.IsTrue (cap.IsAllowed (http, "GET", null), "null");
			Assert.IsTrue (cap.IsAllowed (http, "GET", new string [0]), "none");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "Content-Type"), "Content-Type");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "content-type"), "content-type");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "CONTENT-TYPE"), "CONTENT-TYPE");
			Assert.IsFalse (cap.IsAllowed (http, "GET", "Content-*"), "Content-*");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "PrefixLength"), "PrefixLength");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "prefix"), "prefix");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "PREFIX"), "PREFIX");
		}

		[TestMethod]
		public void AllowSuffixRequestHeaders ()
		{
			string policy = @"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from http-request-headers=""*Suffix"">
				<domain uri=""*"" />
			</allow-from>
			<grant-to>
				<resource path=""/"" include-subpaths=""true"" />
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);
			ClientAccessPolicy.ApplicationUri = http;

			Assert.IsTrue (cap.IsAllowed (http, "GET", null), "null");
			Assert.IsTrue (cap.IsAllowed (http, "GET", new string [0]), "none");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "Content-Type"), "Content-Type");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "content-type"), "content-type");
			Assert.IsTrue (cap.IsAllowed (http, "GET", "CONTENT-TYPE"), "CONTENT-TYPE");
			
			Assert.IsFalse (cap.IsAllowed (http, "GET", "Content-*"), "Content-*");
			Assert.IsFalse (cap.IsAllowed (http, "GET", "LengthSuffix"), "LengthSuffix");
			Assert.IsFalse (cap.IsAllowed (http, "GET", "suffix"), "suffix");
			Assert.IsFalse (cap.IsAllowed (http, "GET", "SUFFIX"), "SUFFIX");
		}

		[TestMethod]
		public void AllowSpecificResourceDirectory()
		{
			string policy = @"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from>
				<domain uri=""*"" />
			</allow-from>
			<grant-to>
				<resource path=""/data/"" include-subpaths=""true"" />
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);
			ClientAccessPolicy.ApplicationUri = http;

			Assert.IsTrue (cap.IsAllowed (new Uri ("http://server/data/file.txt"), "GET", null), "granted");
			Assert.IsFalse (cap.IsAllowed (new Uri ("http://server/file.txt"), "GET", null), "not-granted-parent");
		}

		[TestMethod]
		public void MultiScheme ()
		{
			string policy = @"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from>
				<domain uri=""http://*"" />
				<domain uri=""https://*"" />
			</allow-from>
			<grant-to>
				<resource path=""/data/"" include-subpaths=""true"" />
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);
			ClientAccessPolicy.ApplicationUri = http;

			Assert.IsTrue (cap.IsAllowed (new Uri ("http://server/data/file.txt"), "GET", null), "http/granted");
			Assert.IsFalse (cap.IsAllowed (new Uri ("http://server/file.txt"), "GET", null), "http/not-granted-parent");

			Assert.IsTrue (cap.IsAllowed (new Uri ("https://server/data/file.txt"), "GET", null), "https/granted");
			Assert.IsFalse (cap.IsAllowed (new Uri ("https://server/file.txt"), "GET", null), "https/not-granted-parent");
		}
	}
}

