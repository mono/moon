//
// Web Flash Policy Unit Tests
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
using System.Text;
using System.Windows;
using System.Windows.Browser.Net;
using System.Xml;

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.Security {

	[TestClass]
	public class WebFlashPolicyTest : SilverlightTest {

		static FlashCrossDomainPolicy GetPolicy (string policy)
		{
			using (MemoryStream ms = new MemoryStream (Encoding.UTF8.GetBytes (policy))) {
				return (FlashCrossDomainPolicy) FlashCrossDomainPolicy.FromStream (ms);
			}
		}

		Uri http = new Uri ("http://www.host.com/app.xap");
		Uri https = new Uri ("https://secure.host.net/app.xap");
		Uri file = new Uri ("file:///app.xap");

		Uri http_non_standard_port = new Uri ("http://www.host.com:8080/app.xap");

		[TestMethod]
		public void AllDomains ()
		{
			string policy = @"<?xml version=""1.0""?>
<!DOCTYPE cross-domain-policy SYSTEM ""http://www.macromedia.com/xml/dtds/cross-domain-policy.dtd"">
<cross-domain-policy>
	<allow-access-from domain=""*"" />
</cross-domain-policy>";
			FlashCrossDomainPolicy cdp = GetPolicy (policy);

			FlashCrossDomainPolicy.ApplicationUri = http;
			Assert.IsTrue (cdp.IsAllowed (new Uri ("http://host/data/file.txt"), null), "");
		}

		[TestMethod]
		public void AllDomain_Secure ()
		{
			string policy = @"<?xml version=""1.0""?>
<!DOCTYPE cross-domain-policy SYSTEM ""http://www.macromedia.com/xml/dtds/cross-domain-policy.dtd"">
<cross-domain-policy>
	<allow-access-from domain=""*"" secure=""true""/>
</cross-domain-policy>";
			FlashCrossDomainPolicy cdp = GetPolicy (policy);

			FlashCrossDomainPolicy.ApplicationUri = http;
			Assert.IsTrue (cdp.IsAllowed (new Uri ("http://www.host.com"), null), "");
		}

		[TestMethod]
		public void AllDomains_NoDTD ()
		{
			string policy = @"<?xml version='1.0'?><cross-domain-policy><allow-access-from domain='*'/></cross-domain-policy>";
			FlashCrossDomainPolicy cdp = GetPolicy (policy);

			FlashCrossDomainPolicy.ApplicationUri = http;
			Assert.IsTrue (cdp.IsAllowed (new Uri ("http://www.host.com"), null), "");
		}

		[TestMethod]
		public void AllDomains_PermittedCrossDomainPolicies_All ()
		{
			// 'all' is the default value
			// http://www.adobe.com/devnet/articles/crossdomain_policy_file_spec.html#site-control-permitted-cross-domain-policies
			string policy = @"<?xml version='1.0'?>
<!DOCTYPE cross-domain-policy SYSTEM 'http://www.macromedia.com/xml/dtds/cross-domain-policy.dtd'>
<cross-domain-policy>
	<site-control permitted-cross-domain-policies='all' />
	<allow-access-from domain='*' />
</cross-domain-policy>";
			FlashCrossDomainPolicy cdp = GetPolicy (policy);

			FlashCrossDomainPolicy.ApplicationUri = http;
			Assert.IsTrue (cdp.IsAllowed (new Uri ("http://www.host.com"), null), "http / http");
		}

		[TestMethod]
		public void AllDomains_PermittedCrossDomainPolicies_MasterOnly ()
		{
			string policy = @"<?xml version='1.0'?>
<!DOCTYPE cross-domain-policy SYSTEM 'http://www.macromedia.com/xml/dtds/cross-domain-policy.dtd'>
<cross-domain-policy>
	<site-control permitted-cross-domain-policies='master-only' />
	<allow-access-from domain='*' />
</cross-domain-policy>";
			FlashCrossDomainPolicy cdp = GetPolicy (policy);

			FlashCrossDomainPolicy.ApplicationUri = http;
			Assert.IsTrue (cdp.IsAllowed (new Uri ("http://www.host.com"), null), "http / http");
		}

		[TestMethod]
		public void AllDomains_PermittedCrossDomainPolicies_None ()
		{
			string policy = @"<?xml version='1.0'?>
<!DOCTYPE cross-domain-policy SYSTEM 'http://www.macromedia.com/xml/dtds/cross-domain-policy.dtd'>
<cross-domain-policy>
	<site-control permitted-cross-domain-policies='none' />
	<allow-access-from domain='*' />
</cross-domain-policy>";
			FlashCrossDomainPolicy cdp = GetPolicy (policy);

			FlashCrossDomainPolicy.ApplicationUri = http;
			Assert.IsFalse (cdp.IsAllowed (new Uri ("http://www.host.com"), null), "http / http");
		}

		[TestMethod]
		public void AllDomains_PermittedCrossDomainPolicies_ByContentType ()
		{
			string policy = @"<?xml version='1.0'?>
<!DOCTYPE cross-domain-policy SYSTEM 'http://www.macromedia.com/xml/dtds/cross-domain-policy.dtd'>
<cross-domain-policy>
	<site-control permitted-cross-domain-policies='by-content-type' />
	<allow-access-from domain='*' />
</cross-domain-policy>";
			FlashCrossDomainPolicy cdp = GetPolicy (policy);

			FlashCrossDomainPolicy.ApplicationUri = http;
			Assert.IsFalse (cdp.IsAllowed (new Uri ("http://www.host.com"), null), "http / http");
		}

		[TestMethod]
		public void AllDomains_PermittedCrossDomainPolicies_ByFtpFilename ()
		{
			string policy = @"<?xml version='1.0'?>
<!DOCTYPE cross-domain-policy SYSTEM 'http://www.macromedia.com/xml/dtds/cross-domain-policy.dtd'>
<cross-domain-policy>
	<site-control permitted-cross-domain-policies='by-ftp-filename' />
	<allow-access-from domain='*' />
</cross-domain-policy>";
			FlashCrossDomainPolicy cdp = GetPolicy (policy);

			FlashCrossDomainPolicy.ApplicationUri = http;
			Assert.IsTrue (cdp.IsAllowed (new Uri ("http://www.host.com"), null), "http / http");
		}

		private void FlashCrossDomainSecure (string url, bool pass)
		{
			switch (Application.Current.Host.Source.Scheme) {
			case "file":
			case "http":
				break;
			default:
				// don't execute this test for https (or any other except file and http)
				return;
			}

			DownloadStringCompletedEventArgs resultArgs = null;
			WebClient downloadClient = new WebClient ();
			bool downloadComplete = false;

			downloadClient.DownloadStringCompleted += delegate (object sender, DownloadStringCompletedEventArgs e) {
				resultArgs = e;
				downloadComplete = true;

			};

			Enqueue (() => downloadClient.DownloadStringAsync (new Uri (url)));
			EnqueueConditional (() => downloadComplete);

			Enqueue (() => {
				if (pass) {
					string errorMessage = (resultArgs.Error != null) ? resultArgs.Error.Message : String.Empty;
					Assert.IsNull (resultArgs.Error, "There was an error while attempting to download the string: " + errorMessage);
				} else {
					Assert.IsNotNull (resultArgs.Error, "Anticipated error did not occur");
				}
			});

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[Ignore ("Depending on outside site(s) is bound to fail too often")]
		public void FileToHTTPFlashCrossDomainTest()
		{
			// a FILE:// application can access an HTTP:// URL when policy specify Secure=true (default)
			// Note: this works because we can retrieve http://api.flickr.com/crossdomain.xml and 
			// because http://api.flickr.com/clientaccesspolicy.xml does not exists
			FlashCrossDomainSecure ("http://api.flickr.com/services/rest", true);
		}

		[TestMethod]
		[Asynchronous]
		[Ignore ("Depending on outside site(s) is bound to fail too often")]
		public void FileToHTTPSFlashCrossDomainTest ()
		{
			// a FILE:// application CANNOT access an HTTPS:// URL when policy specify Secure=true (default)
			// Note: this works because we can retrieve https://www.twitter.com/crossdomain.xml and 
			// because https://www.twitter.com/clientaccesspolicy.xml does not exists
			FlashCrossDomainSecure ("https://www.twitter.com", false);
		}
	}
}

