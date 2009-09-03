<%@ Application Language="C#" %>
<%@ Import Namespace="System.IO" %>

<script RunAt="server">
	private void NotFound ()
	{
		Response.StatusCode = 404;
		Response.Write ("404");
	}

	private void FlashDefault (string path)
	{
		// if we're testing a Flash policy file it means we won't provide a Silverlight one
		if (path == "clientaccesspolicy.xml") {
			NotFound ();
		} else {
			Default (path);
		}
	}

	private void Default (string path)
	{
		Response.ContentType = "text/html";
		if (String.IsNullOrEmpty (path)) {
			Response.Write ("'empty'");
		} else {
			Response.Write (path);
		}
	}

	protected void Application_BeginRequest(Object sender, EventArgs e) {
		string SERVER_NAME = Context.Request ["SERVER_NAME"];
		string path = Context.Request.Path.Substring (1);

		switch (SERVER_NAME) {
		case "policy-client":
			switch (path) {
			case "WebPolicies.html":
			case "WebPolicies.xap":
				Response.TransmitFile (path);
				break;
			default:
				NotFound ();
				break;
			}
			break;

		case "flash-1":
			// Description:	simplest flash policy
			// Result:	OK
			if (path == "crossdomain.xml") {
				Response.Write ("<?xml version='1.0'?><cross-domain-policy><allow-access-from domain='*'/></cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-2":
			// Description:	allow domain name using host name (MS docs says only '*' is allowed)
			// Result:	SecurityException
			// Notes:	works only if UserHostName returns the name, not the IP address
			if (path == "crossdomain.xml") {
				string s = String.Format ("<?xml version='1.0'?><cross-domain-policy><allow-access-from domain='{0}'/></cross-domain-policy>", 
					Context.Request.UserHostName);
				Response.Write (s);
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-3":
			// Description:	allow domain name using host IP address (MS docs says only '*' is allowed)
			// Result:	SecurityException
			if (path == "crossdomain.xml") {
				string s = String.Format ("<?xml version='1.0'?><cross-domain-policy><allow-access-from domain='{0}'/></cross-domain-policy>", 
					Context.Request.UserHostAddress);
				Response.Write (s);
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-4":
			// Description:	allow all domains with X-Permitted-Cross-Domain-Policies == none
			// Result:	SecurityException
			if (path == "crossdomain.xml") {
				Response.AddHeader ("X-Permitted-Cross-Domain-Policies", "none");
				Response.Write ("<?xml version='1.0'?><cross-domain-policy><allow-access-from domain='*'/></cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-5":
			// Description:	allow all domains with X-Permitted-Cross-Domain-Policies == master-only
			// Result:	OK
			if (path == "crossdomain.xml") {
				Response.AddHeader ("X-Permitted-Cross-Domain-Policies", "master-only");
				Response.Write ("<?xml version='1.0'?><cross-domain-policy><allow-access-from domain='*'/></cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-6":
			// Description:	allow all domains with X-Permitted-Cross-Domain-Policies == by-content-type
			// Result:	SecurityException
			if (path == "crossdomain.xml") {
				Response.AddHeader ("X-Permitted-Cross-Domain-Policies", "by-content-type");
				Response.Write ("<?xml version='1.0'?><cross-domain-policy><allow-access-from domain='*'/></cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-7":
			// Description:	allow all domains with X-Permitted-Cross-Domain-Policies == by-ftp-filename
			// Result:	OK
			if (path == "crossdomain.xml") {
				Response.AddHeader ("X-Permitted-Cross-Domain-Policies", "by-ftp-filename");
				Response.Write ("<?xml version='1.0'?><cross-domain-policy><allow-access-from domain='*'/></cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-8":
			// Description:	allow all domains with X-Permitted-Cross-Domain-Policies == all
			// Result:	OK
			if (path == "crossdomain.xml") {
				Response.AddHeader ("X-Permitted-Cross-Domain-Policies", "all");
				Response.Write ("<?xml version='1.0'?><cross-domain-policy><allow-access-from domain='*'/></cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-9":
			// Description:	allow all domains with X-Permitted-Cross-Domain-Policies == invalid-value
			// Result:	SecurityException
			if (path == "crossdomain.xml") {
				Response.AddHeader ("X-Permitted-Cross-Domain-Policies", "invalid-value");
				Response.Write ("<?xml version='1.0'?><cross-domain-policy><allow-access-from domain='*'/></cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-10":
			// Description:	allow all with DTD
			// Result:	OK
			if (path == "crossdomain.xml") {
				Response.Write (@"<?xml version=""1.0""?>
<!DOCTYPE cross-domain-policy SYSTEM ""http://www.macromedia.com/xml/dtds/cross-domain-policy.dtd"">
<cross-domain-policy>
	<allow-access-from domain=""*"" />
</cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-11":
			// Description:	allow all with secure == true
			// Result:	OK
			if (path == "crossdomain.xml") {
				Response.Write (@"<?xml version=""1.0""?>
<!DOCTYPE cross-domain-policy SYSTEM ""http://www.macromedia.com/xml/dtds/cross-domain-policy.dtd"">
<cross-domain-policy>
	<allow-access-from domain=""*"" secure=""true"" />
</cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-12":
			// Description:	allow all with secure == false
			// Result:	OK
			if (path == "crossdomain.xml") {
				Response.Write (@"<?xml version=""1.0""?>
<!DOCTYPE cross-domain-policy SYSTEM ""http://www.macromedia.com/xml/dtds/cross-domain-policy.dtd"">
<cross-domain-policy>
	<allow-access-from domain=""*"" secure=""false"" />
</cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-13":
			// Description:	allow all with secure == invalid-value
			// Result:	SecurityException
			if (path == "crossdomain.xml") {
				Response.Write (@"<?xml version=""1.0""?>
<!DOCTYPE cross-domain-policy SYSTEM ""http://www.macromedia.com/xml/dtds/cross-domain-policy.dtd"">
<cross-domain-policy>
	<allow-access-from domain=""*"" secure=""invalid-value"" />
</cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-14":
			// Description:	no policy (404)
			NotFound ();
			break;
		case "flash-15":
			// Description:	simplest flash policy with some spaces before the XML (technically invalid XML)
			// Result:	OK
			if (path == "crossdomain.xml") {
				Response.Write ("  <?xml version='1.0'?><cross-domain-policy><allow-access-from domain='*'/></cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-16":
			// Description:	allow all domains with site-control permitted-cross-domain-policie == none
			// Result:	SecurityException
			if (path == "crossdomain.xml") {
				Response.Write ("<?xml version='1.0'?><cross-domain-policy><site-control permitted-cross-domain-policies='none' /><allow-access-from domain='*'/></cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-17":
			// Description:	allow all domains with site-control permitted-cross-domain-policie == master-only
			// Result:	OK
			if (path == "crossdomain.xml") {
				Response.Write ("<?xml version='1.0'?><cross-domain-policy><site-control permitted-cross-domain-policies='master-only' /><allow-access-from domain='*'/></cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-18":
			// Description:	allow all domains with site-control permitted-cross-domain-policie == by-content-type
			// Result:	SecurityException
			if (path == "crossdomain.xml") {
				Response.Write ("<?xml version='1.0'?><cross-domain-policy><site-control permitted-cross-domain-policies='by-content-type' /><allow-access-from domain='*'/></cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-19":
			// Description:	allow all domains with site-control permitted-cross-domain-policie == by-ftp-filename
			// Result:	OK
			if (path == "crossdomain.xml") {
				Response.Write ("<?xml version='1.0'?><cross-domain-policy><site-control permitted-cross-domain-policies='by-ftp-filename' /><allow-access-from domain='*'/></cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-20":
			// Description:	allow all domains with site-control permitted-cross-domain-policie == all
			// Result:	OK
			if (path == "crossdomain.xml") {
				Response.Write ("<?xml version='1.0'?><cross-domain-policy><site-control permitted-cross-domain-policies='all' /><allow-access-from domain='*'/></cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-21":
			// Description:	allow all domains with site-control permitted-cross-domain-policie == invalid-value
			// Result:	SecurityException
			if (path == "crossdomain.xml") {
				Response.Write ("<?xml version='1.0'?><cross-domain-policy><site-control permitted-cross-domain-policies='invalid-value' /><allow-access-from domain='*'/></cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;
		case "flash-22":
			// Description:	allow domain name using host name (MS docs says only '*' is allowed)
			// Result:	SecurityException
			if (path == "crossdomain.xml") {
				Response.Write ("<?xml version='1.0'?><cross-domain-policy><allow-access-from domain='policy-client'/></cross-domain-policy>");
			} else {
				FlashDefault (path);
			}
			break;


		case "silverlight-1":
			// Description:	simplest, allow everything, silverlight policy
			if (path == "clientaccesspolicy.xml") {
				Response.Write (@"<?xml version='1.0'?>
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
</access-policy>");
			} else {
				Default (path);
			}
			break;
		case "silverlight-2":
			// Description:	Allow everything under /test
			if (path == "clientaccesspolicy.xml") {
				Response.Write (@"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from>
				<domain uri='*'/>
			</allow-from>
			<grant-to>
				<resource path='/test' include-subpaths='true'/>
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>");
			} else {
				Default (path);
			}
			break;
		case "silverlight-3":
			// Description:	simplest silverlight policy
			if (path == "clientaccesspolicy.xml") {
				Response.Write (@"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from>
				<domain uri='*'/>
			</allow-from>
			<grant-to>
				<resource path='/test/again/' include-subpaths='true'/>
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>");
			} else {
				Default (path);
			}
			break;
		case "silverlight-4":
			// Description:	grant / but no subpaths
			if (path == "clientaccesspolicy.xml") {
				Response.Write (@"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from>
				<domain uri='*'/>
			</allow-from>
			<grant-to>
				<resource path='/' include-subpaths='false'/>
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>");
			} else {
				Default (path);
			}
			break;
		case "silverlight-5":
			// Description:	grant everything under ""
			if (path == "clientaccesspolicy.xml") {
				Response.Write (@"<?xml version='1.0'?>
<access-policy><cross-domain-access><policy>
	<allow-from><domain uri='*'/></allow-from>
	<grant-to><resource path='' include-subpaths='true'/></grant-to>
</policy></cross-domain-access></access-policy>");
			} else if (path == "crossdomain.xml") {
				// policy is invalid and we don't provide a flash policy alternative
				NotFound ();
			} else {
				Default (path);
			}
			break;
		case "silverlight-6":
			// Description:	grant everything under "" or "/test"
			if (path == "clientaccesspolicy.xml") {
				Response.Write (@"<?xml version='1.0'?>
<access-policy><cross-domain-access><policy>
	<allow-from><domain uri='*'/></allow-from>
	<grant-to><resource path='' include-subpaths='true'/><resource path='test' include-subpaths='true'/></grant-to>
</policy></cross-domain-access></access-policy>");
			} else if (path == "crossdomain.xml") {
				// policy is invalid and we don't provide a flash policy alternative
				NotFound ();
			} else {
				Default (path);
			}
			break;
		case "silverlight-7":
			// Description:	simplest silverlight policy with some spaces before the XML (technically invalid XML)
			if (path == "clientaccesspolicy.xml") {
				Response.Write (@"  <?xml version='1.0'?>
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
</access-policy>");
			} else {
				Default (path);
			}
			break;
		case "silverlight-8":
			// Description:	give access only to 'policy-client' (missing scheme)
			// Result:	SecurityException
			if (path == "clientaccesspolicy.xml") {
				Response.Write (@"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from>
				<domain uri='policy-client'/>
			</allow-from>
			<grant-to>
				<resource path='/' include-subpaths='true'/>
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>");
			} else {
				Default (path);
			}
			break;
		case "silverlight-9":
			// Description:	give access only to 'http://policy-client'
			// Result:	OK
			if (path == "clientaccesspolicy.xml") {
				Response.Write (@"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from>
				<domain uri='http://policy-client'/>
			</allow-from>
			<grant-to>
				<resource path='/' include-subpaths='true'/>
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>");
			} else {
				Default (path);
			}
			break;
		case "silverlight-10":
			// Description:	give access only to 'http://policy-client:80' (default port)
			// Result:	OK
			if (path == "clientaccesspolicy.xml") {
				Response.Write (@"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from>
				<domain uri='http://policy-client:80'/>
			</allow-from>
			<grant-to>
				<resource path='/' include-subpaths='true'/>
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>");
			} else {
				Default (path);
			}
			break;
		case "silverlight-11":
			// Description:	give access only to 'http://policy-client:8080' (alternate port)
			// Result:	SecurityException
			if (path == "clientaccesspolicy.xml") {
				Response.Write (@"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from>
				<domain uri='http://policy-client:8080'/>
			</allow-from>
			<grant-to>
				<resource path='/' include-subpaths='true'/>
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>");
			} else {
				Default (path);
			}
			break;
		case "silverlight-12":
			// Description:	give access only to 'https://policy-client'
			// Result:	SecurityException
			if (path == "clientaccesspolicy.xml") {
				Response.Write (@"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from>
				<domain uri='https://policy-client'/>
			</allow-from>
			<grant-to>
				<resource path='/' include-subpaths='true'/>
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>");
			} else {
				Default (path);
			}
			break;
		case "silverlight-13":
			// Description:	give access only to 'http://policy-client/WebPolicies.html' (full path)
			// Result:	SecurityException
			if (path == "clientaccesspolicy.xml") {
				Response.Write (@"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from>
				<domain uri='http://policy-client/WebPolicies.html'/>
			</allow-from>
			<grant-to>
				<resource path='/' include-subpaths='true'/>
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>");
			} else {
				Default (path);
			}
			break;
		case "silverlight-14":
			// Description:	give access only to 'unexisting-client'
			// Result:	SecurityException
			if (path == "clientaccesspolicy.xml") {
				Response.Write (@"<?xml version='1.0'?>
<access-policy>
	<cross-domain-access>
		<policy>
			<allow-from>
				<domain uri='http://unexisting-client'/>
			</allow-from>
			<grant-to>
				<resource path='/' include-subpaths='true'/>
			</grant-to>
		</policy>
	</cross-domain-access>
</access-policy>");
			} else {
				Default (path);
			}
			break;

		default:
			string s = String.Format ("<h1>Unknown server name: {0}</h1>", SERVER_NAME);
			Response.ContentType = "text/html";
			Response.Write (s);
			break;
		}

		Response.Flush ();
		Response.Close ();
		Response.End ();
	}
</script>

