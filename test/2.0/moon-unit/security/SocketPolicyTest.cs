//
// Socket Policy Unit Tests
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
using System.Windows.Browser.Net;
using System.Xml;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.Security {

	[TestClass]
	public class SocketPolicyTest {

		static ClientAccessPolicy GetPolicy (string policy)
		{
			using (MemoryStream ms = new MemoryStream (Encoding.UTF8.GetBytes (policy))) {
				return (ClientAccessPolicy) ClientAccessPolicy.FromStream (ms);
			}
		}

		Uri http = new Uri ("http://www.host.com/app.xap");
		Uri https = new Uri ("https://secure.host.net/app.xap");
		Uri file = new Uri ("file:///app.xap");

		Uri http_non_standard_port = new Uri ("http://www.host.com:8080/app.xap");

		// invalid
		IPEndPoint p4501 = new IPEndPoint (IPAddress.Any, 4501);
		// maximum valid range is between 4502 and 4534, policy can be more restrictive
		IPEndPoint p4502 = new IPEndPoint (IPAddress.Any, 4502);
		IPEndPoint p4503 = new IPEndPoint (IPAddress.Any, 4503);
		IPEndPoint p4504 = new IPEndPoint (IPAddress.Any, 4504);
		IPEndPoint p4505 = new IPEndPoint (IPAddress.Any, 4505);
		IPEndPoint p4506 = new IPEndPoint (IPAddress.Any, 4506);
		IPEndPoint p4507 = new IPEndPoint (IPAddress.Any, 4507);
		IPEndPoint p4508 = new IPEndPoint (IPAddress.Any, 4508);
		IPEndPoint p4509 = new IPEndPoint (IPAddress.Any, 4509);
		IPEndPoint p4510 = new IPEndPoint (IPAddress.Any, 4510);
		IPEndPoint p4511 = new IPEndPoint (IPAddress.Any, 4511);
		IPEndPoint p4512 = new IPEndPoint (IPAddress.Any, 4512);
		IPEndPoint p4513 = new IPEndPoint (IPAddress.Any, 4513);
		IPEndPoint p4514 = new IPEndPoint (IPAddress.Any, 4514);
		IPEndPoint p4515 = new IPEndPoint (IPAddress.Any, 4515);
		IPEndPoint p4516 = new IPEndPoint (IPAddress.Any, 4516);
		IPEndPoint p4517 = new IPEndPoint (IPAddress.Any, 4517);
		IPEndPoint p4518 = new IPEndPoint (IPAddress.Any, 4518);
		IPEndPoint p4519 = new IPEndPoint (IPAddress.Any, 4519);
		IPEndPoint p4520 = new IPEndPoint (IPAddress.Any, 4520);
		IPEndPoint p4521 = new IPEndPoint (IPAddress.Any, 4521);
		IPEndPoint p4522 = new IPEndPoint (IPAddress.Any, 4522);
		IPEndPoint p4523 = new IPEndPoint (IPAddress.Any, 4523);
		IPEndPoint p4524 = new IPEndPoint (IPAddress.Any, 4524);
		IPEndPoint p4525 = new IPEndPoint (IPAddress.Any, 4525);
		IPEndPoint p4526 = new IPEndPoint (IPAddress.Any, 4526);
		IPEndPoint p4527 = new IPEndPoint (IPAddress.Any, 4527);
		IPEndPoint p4528 = new IPEndPoint (IPAddress.Any, 4528);
		IPEndPoint p4529 = new IPEndPoint (IPAddress.Any, 4529);
		IPEndPoint p4530 = new IPEndPoint (IPAddress.Any, 4530);
		IPEndPoint p4531 = new IPEndPoint (IPAddress.Any, 4531);
		IPEndPoint p4532 = new IPEndPoint (IPAddress.Any, 4532);
		IPEndPoint p4533 = new IPEndPoint (IPAddress.Any, 4533);
		IPEndPoint p4534 = new IPEndPoint (IPAddress.Any, 4534);
		// invalid
		IPEndPoint p4535 = new IPEndPoint (IPAddress.Any, 4535);

		[TestMethod]
		public void All ()
		{
			string policy = @"<?xml version=""1.0"" encoding=""utf-8""?>
<access-policy>
  <cross-domain-access>
    <policy>
      <allow-from>
        <domain uri=""*"" />
      </allow-from>
      <grant-to>
        <socket-resource port=""4502-4534"" protocol=""tcp"" />
      </grant-to>
    </policy>
  </cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);

			ClientAccessPolicy.ApplicationUri = http;
			Assert.IsFalse (cap.IsAllowed (p4501), "http / 4501");
			Assert.IsTrue (cap.IsAllowed (p4502), "http / 4502");
			Assert.IsTrue (cap.IsAllowed (p4503), "http / 4503");
			Assert.IsTrue (cap.IsAllowed (p4504), "http / 4504");
			Assert.IsTrue (cap.IsAllowed (p4505), "http / 4505");
			Assert.IsTrue (cap.IsAllowed (p4506), "http / 4506");
			Assert.IsTrue (cap.IsAllowed (p4507), "http / 4507");
			Assert.IsTrue (cap.IsAllowed (p4508), "http / 4508");
			Assert.IsTrue (cap.IsAllowed (p4509), "http / 4509");
			Assert.IsTrue (cap.IsAllowed (p4510), "http / 4510");
			Assert.IsTrue (cap.IsAllowed (p4511), "http / 4511");

			ClientAccessPolicy.ApplicationUri = https;
			Assert.IsTrue (cap.IsAllowed (p4512), "https / 4512");
			Assert.IsTrue (cap.IsAllowed (p4513), "https / 4513");
			Assert.IsTrue (cap.IsAllowed (p4514), "https / 4514");
			Assert.IsTrue (cap.IsAllowed (p4515), "https / 4515");
			Assert.IsTrue (cap.IsAllowed (p4516), "https / 4516");
			Assert.IsTrue (cap.IsAllowed (p4517), "https / 4517");
			Assert.IsTrue (cap.IsAllowed (p4518), "https / 4518");
			Assert.IsTrue (cap.IsAllowed (p4519), "https / 4519");
			Assert.IsTrue (cap.IsAllowed (p4520), "https / 4520");
			Assert.IsTrue (cap.IsAllowed (p4521), "https / 4521");
			Assert.IsTrue (cap.IsAllowed (p4522), "https / 4522");

			ClientAccessPolicy.ApplicationUri = file;
			Assert.IsTrue (cap.IsAllowed (p4523), "file / 4523");
			Assert.IsTrue (cap.IsAllowed (p4524), "file / 4524");
			Assert.IsTrue (cap.IsAllowed (p4525), "file / 4525");
			Assert.IsTrue (cap.IsAllowed (p4526), "file / 4526");
			Assert.IsTrue (cap.IsAllowed (p4527), "file / 4527");
			Assert.IsTrue (cap.IsAllowed (p4528), "file / 4528");
			Assert.IsTrue (cap.IsAllowed (p4529), "file / 4529");
			Assert.IsTrue (cap.IsAllowed (p4530), "file / 4530");
			Assert.IsTrue (cap.IsAllowed (p4531), "file / 4531");
			Assert.IsTrue (cap.IsAllowed (p4532), "file / 4532");
			Assert.IsTrue (cap.IsAllowed (p4533), "file / 4533");
			Assert.IsTrue (cap.IsAllowed (p4534), "file / 4534");
			Assert.IsFalse (cap.IsAllowed (p4535), "file / 4535");
		}

		[TestMethod]
		public void SocketInvalidProtocol ()
		{
			string policy = @"<?xml version=""1.0"" encoding=""utf-8""?>
<access-policy>
  <cross-domain-access>
    <policy>
      <allow-from>
        <domain uri=""*"" />
      </allow-from>
      <grant-to>
        <socket-resource port=""4529-4531"" protocol=""udp"" />
      </grant-to>
    </policy>
  </cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);

			ClientAccessPolicy.ApplicationUri = http;
			Assert.IsFalse (cap.IsAllowed (p4529), "http / 4529");

			ClientAccessPolicy.ApplicationUri = https;
			Assert.IsFalse (cap.IsAllowed (p4530), "http / 4530");

			ClientAccessPolicy.ApplicationUri = file;
			Assert.IsFalse (cap.IsAllowed (p4531), "file / 4531");
		}

		[TestMethod]
		public void SocketNoProtocolSpecified ()
		{
			string policy = @"<?xml version=""1.0"" encoding=""utf-8""?>
<access-policy>
  <cross-domain-access>
    <policy>
      <grant-to>
        <socket-resource port='4529-4531'/>
      </grant-to>
      <allow-from>
        <domain uri='*'/>
      </allow-from>
    </policy>
  </cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);

			ClientAccessPolicy.ApplicationUri = http;
			Assert.IsFalse (cap.IsAllowed (p4529), "http / 4529");

			ClientAccessPolicy.ApplicationUri = https;
			Assert.IsFalse (cap.IsAllowed (p4530), "http / 4530");

			ClientAccessPolicy.ApplicationUri = file;
			Assert.IsFalse (cap.IsAllowed (p4531), "file / 4531");
		}

		[TestMethod]
		public void SocketProtocolCasing ()
		{
			string policy = @"<?xml version=""1.0"" encoding=""utf-8""?>
<access-policy>
  <cross-domain-access>
    <policy>
      <grant-to>
        <socket-resource port='4529-4531' protocol='Tcp'/>
      </grant-to>
      <allow-from>
        <domain uri='*'/>
      </allow-from>
    </policy>
  </cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);

			ClientAccessPolicy.ApplicationUri = http;
			Assert.IsFalse (cap.IsAllowed (p4529), "http / 4529");

			ClientAccessPolicy.ApplicationUri = https;
			Assert.IsFalse (cap.IsAllowed (p4530), "http / 4530");

			ClientAccessPolicy.ApplicationUri = file;
			Assert.IsFalse (cap.IsAllowed (p4531), "file / 4531");
		}

		[TestMethod]
		public void SocketSingleAllowHttpOnly ()
		{
			string policy = @"<?xml version=""1.0"" encoding=""utf-8""?>
<access-policy>
  <cross-domain-access>
    <policy>
      <allow-from>
        <domain uri=""http://*"" />
      </allow-from>
      <grant-to>
        <socket-resource port=""4502"" protocol=""tcp"" />
      </grant-to>
    </policy>
  </cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);

			ClientAccessPolicy.ApplicationUri = http;
			Assert.IsTrue (cap.IsAllowed (p4502), "http / 4502");
			Assert.IsFalse (cap.IsAllowed (p4503), "http / 4503");

			ClientAccessPolicy.ApplicationUri = https;
			Assert.IsFalse (cap.IsAllowed (p4502), "https / 4502");
			Assert.IsFalse (cap.IsAllowed (p4503), "https / 4503");

			ClientAccessPolicy.ApplicationUri = file;
			Assert.IsFalse (cap.IsAllowed (p4502), "file / 4502");
			Assert.IsFalse (cap.IsAllowed (p4503), "file / 4503");

			ClientAccessPolicy.ApplicationUri = http_non_standard_port;
			Assert.IsFalse (cap.IsAllowed (p4502), "http:8080 / 4502");
			Assert.IsFalse (cap.IsAllowed (p4503), "http:8080 / 4503");
		}

		[TestMethod]
		public void SocketSingleAllowHttpsOnly ()
		{
			string policy = @"<?xml version=""1.0"" encoding=""utf-8""?>
<access-policy>
  <cross-domain-access>
    <policy>
      <allow-from>
        <domain uri=""https://*"" />
      </allow-from>
      <grant-to>
        <socket-resource port=""4534"" protocol=""tcp"" />
      </grant-to>
    </policy>
  </cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);

			ClientAccessPolicy.ApplicationUri = http;
			Assert.IsFalse (cap.IsAllowed (p4533), "http / 4533");
			Assert.IsFalse (cap.IsAllowed (p4534), "http / 4534");

			ClientAccessPolicy.ApplicationUri = https;
			Assert.IsFalse (cap.IsAllowed (p4533), "https / 4533");
			Assert.IsTrue (cap.IsAllowed (p4534), "https / 4534");

			ClientAccessPolicy.ApplicationUri = file;
			Assert.IsFalse (cap.IsAllowed (p4533), "file / 4533");
			Assert.IsFalse (cap.IsAllowed (p4534), "file / 4534");
		}

		[TestMethod]
		public void SocketSingleAllowLocalOnly ()
		{
			string policy = @"<?xml version=""1.0"" encoding=""utf-8""?>
<access-policy>
  <cross-domain-access>
    <policy>
      <allow-from>
        <domain uri=""file:///"" />
      </allow-from>
      <grant-to>
        <socket-resource port=""4520"" protocol=""tcp"" />
      </grant-to>
    </policy>
  </cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);

			ClientAccessPolicy.ApplicationUri = http;
			Assert.IsFalse (cap.IsAllowed (p4519), "http / 4519");
			Assert.IsFalse (cap.IsAllowed (p4520), "http / 4520");

			ClientAccessPolicy.ApplicationUri = https;
			Assert.IsFalse (cap.IsAllowed (p4519), "https / 4519");
			Assert.IsFalse (cap.IsAllowed (p4520), "https / 4520");

			ClientAccessPolicy.ApplicationUri = file;
			Assert.IsFalse (cap.IsAllowed (p4519), "file / 4519");
			Assert.IsTrue (cap.IsAllowed (p4520), "file / 4520");
		}

		[TestMethod]
		public void SocketPortWildcard ()
		{
			string policy = @"<?xml version=""1.0"" encoding=""utf-8""?>
<access-policy>
  <cross-domain-access>
    <policy>
      <allow-from>
        <domain uri=""*"" />
      </allow-from>
      <grant-to>
        <socket-resource port=""*"" protocol=""tcp"" />
      </grant-to>
    </policy>
  </cross-domain-access>
</access-policy>";
			Assert.Throws<XmlException> (delegate {
				ClientAccessPolicy cap = GetPolicy (policy);
			}, "wildcard");
		}

		[TestMethod]
		public void SocketPortOutOfRange ()
		{
			string policy = @"<?xml version=""1.0"" encoding=""utf-8""?>
<access-policy>
  <cross-domain-access>
    <policy>
      <allow-from>
        <domain uri=""*"" />
      </allow-from>
      <grant-to>
        <socket-resource port=""4501-4531"" protocol=""tcp"" />
      </grant-to>
    </policy>
  </cross-domain-access>
</access-policy>";
			Assert.Throws<XmlException> (delegate {
				ClientAccessPolicy cap = GetPolicy (policy);
			}, "out of range");
		}

		[TestMethod]
		public void NoDomainSpecified ()
		{
			string policy = @"<?xml version=""1.0"" encoding=""utf-8""?>
<access-policy>
  <cross-domain-access>
    <policy>
      <allow-from>
      </allow-from>
      <grant-to>
        <socket-resource port=""4502-4531"" protocol=""tcp"" />
      </grant-to>
    </policy>
  </cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);

			ClientAccessPolicy.ApplicationUri = http;
			Assert.IsFalse (cap.IsAllowed (p4519), "http / 4502");
			Assert.IsFalse (cap.IsAllowed (p4520), "http / 4531");

			ClientAccessPolicy.ApplicationUri = https;
			Assert.IsFalse (cap.IsAllowed (p4519), "https / 4503");
			Assert.IsFalse (cap.IsAllowed (p4520), "https / 4530");

			ClientAccessPolicy.ApplicationUri = file;
			Assert.IsFalse (cap.IsAllowed (p4519), "file / 4504");
			Assert.IsFalse (cap.IsAllowed (p4520), "file / 4529");

			ClientAccessPolicy.ApplicationUri = http_non_standard_port;
			Assert.IsFalse (cap.IsAllowed (p4502), "http:8080 / 4505");
			Assert.IsFalse (cap.IsAllowed (p4503), "http:8080 / 4528");
		}

		[TestMethod]
		public void NoAllowFromSpecified ()
		{
			string policy = @"<?xml version=""1.0"" encoding=""utf-8""?>
<access-policy>
  <cross-domain-access>
    <policy>
      <grant-to>
        <socket-resource port=""4502-4531"" protocol=""tcp"" />
      </grant-to>
    </policy>
  </cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);

			ClientAccessPolicy.ApplicationUri = http;
			Assert.IsFalse (cap.IsAllowed (p4519), "http / 4502");
			Assert.IsFalse (cap.IsAllowed (p4520), "http / 4531");

			ClientAccessPolicy.ApplicationUri = https;
			Assert.IsFalse (cap.IsAllowed (p4519), "https / 4503");
			Assert.IsFalse (cap.IsAllowed (p4520), "https / 4530");

			ClientAccessPolicy.ApplicationUri = file;
			Assert.IsFalse (cap.IsAllowed (p4519), "file / 4504");
			Assert.IsFalse (cap.IsAllowed (p4520), "file / 4529");

			ClientAccessPolicy.ApplicationUri = http_non_standard_port;
			Assert.IsFalse (cap.IsAllowed (p4502), "http:8080 / 4505");
			Assert.IsFalse (cap.IsAllowed (p4503), "http:8080 / 4528");
		}

		[TestMethod]
		public void NoSocketResource ()
		{
			string policy = @"<?xml version=""1.0"" encoding=""utf-8""?>
<access-policy>
  <cross-domain-access>
    <policy>
      <allow-from>
        <domain uri=""*"" />
      </allow-from>
      <grant-to>
      </grant-to>
    </policy>
  </cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);

			ClientAccessPolicy.ApplicationUri = http;
			Assert.IsFalse (cap.IsAllowed (p4519), "http / 4502");
			Assert.IsFalse (cap.IsAllowed (p4520), "http / 4531");

			ClientAccessPolicy.ApplicationUri = https;
			Assert.IsFalse (cap.IsAllowed (p4519), "https / 4503");
			Assert.IsFalse (cap.IsAllowed (p4520), "https / 4530");

			ClientAccessPolicy.ApplicationUri = file;
			Assert.IsFalse (cap.IsAllowed (p4519), "file / 4504");
			Assert.IsFalse (cap.IsAllowed (p4520), "file / 4529");

			ClientAccessPolicy.ApplicationUri = http_non_standard_port;
			Assert.IsFalse (cap.IsAllowed (p4502), "http:8080 / 4505");
			Assert.IsFalse (cap.IsAllowed (p4503), "http:8080 / 4528");
		}

		[TestMethod]
		public void NoGrantTo ()
		{
			string policy = @"<?xml version=""1.0"" encoding=""utf-8""?>
<access-policy>
  <cross-domain-access>
    <policy>
      <allow-from>
        <domain uri=""*"" />
      </allow-from>
    </policy>
  </cross-domain-access>
</access-policy>";
			ClientAccessPolicy cap = GetPolicy (policy);

			ClientAccessPolicy.ApplicationUri = http;
			Assert.IsFalse (cap.IsAllowed (p4519), "http / 4502");
			Assert.IsFalse (cap.IsAllowed (p4520), "http / 4531");

			ClientAccessPolicy.ApplicationUri = https;
			Assert.IsFalse (cap.IsAllowed (p4519), "https / 4503");
			Assert.IsFalse (cap.IsAllowed (p4520), "https / 4530");

			ClientAccessPolicy.ApplicationUri = file;
			Assert.IsFalse (cap.IsAllowed (p4519), "file / 4504");
			Assert.IsFalse (cap.IsAllowed (p4520), "file / 4529");

			ClientAccessPolicy.ApplicationUri = http_non_standard_port;
			Assert.IsFalse (cap.IsAllowed (p4502), "http:8080 / 4505");
			Assert.IsFalse (cap.IsAllowed (p4503), "http:8080 / 4528");
		}
	}
}

