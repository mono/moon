//
// ClientAccessPolicy.cs
//
// Authors:
//	Atsushi Enomoto <atsushi@ximian.com>
//	Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc.  http://www.novell.com
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

#if NET_2_1

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Xml;
#if !TEST
using System.Windows.Interop;
#endif

/*
default namespace = ""

grammar {

start = access-policy

access-policy = element access-policy {
  element cross-domain-access {
    element policy { allow-from, grant-to }
  }
}

allow-from = element allow-from {
  attribute http-request-headers { text },
  element domain {
    attribute uri { text }
  }
}

grant-to = element grant-to {
  (resource | socket-resource)+
}

resource = element resource {
  attribute path { text },
  attribute include-subpaths { "true" | "false" }
}

socket-resource = element socket-resource {
  attribute port { text },
  attribute protocol { text }
}

}
*/

namespace System.Windows.Browser.Net
{
	class ClientAccessPolicy
	{
/*
public static void Main (string [] args)
{
	foreach (string s in args)
		using (var r = XmlReader.Create (s))
			ClientAccessPolicyReader.Read (r);
}
*/
		static public Uri ApplicationUri {
#if TEST
			get; set;
#else
			get { return PluginHost.RootUri; }
#endif
		}

		class AccessPolicy {

			public const short MinPort = 4502;
			public const short MaxPort = 4534;

			public List<AllowFrom> AllowedServices { get; private set; }
			public List<GrantTo> GrantedResources { get; private set; }
			public long PortMask { get; set; }

			public AccessPolicy ()
			{
				AllowedServices = new List<AllowFrom> ();
				GrantedResources = new List<GrantTo> ();
			}

			public bool PortAllowed (int port)
			{
				if ((port < MinPort) || (port > MaxPort))
					return false;

				return (((PortMask >> (port - MinPort)) & 1) == 1);
			}
		}

		public static ClientAccessPolicy Read (XmlReader reader)
		{
			var r = new ClientAccessPolicyReader (reader);
			r.Read ();
			return r.Result;
		}

		public ClientAccessPolicy ()
		{
			AccessPolicyList = new List<AccessPolicy> ();
		}

		List<AccessPolicy> AccessPolicyList { get; set; }

		public bool IsAllowed (IPEndPoint endpoint)
		{
			foreach (AccessPolicy policy in AccessPolicyList) {
				// does something allow our URI in this policy ?
				foreach (AllowFrom af in policy.AllowedServices) {
					if (af.IsAllowed (ApplicationUri, null)) {
						// if so, is our request port allowed ?
						if (policy.PortAllowed (endpoint.Port))
							return true;
					}
				}
			}
			// no policy allows this socket connection
			return false;
		}

		public class AllowFrom
		{
			public AllowFrom ()
			{
				Domains = new List<Uri> ();
				AllowAllHeaders = true;
				Scheme = String.Empty;
			}

			string [] headers;

			public bool AllowAllHeaders { get; private set; }

			public bool AllowAnyDomain { get; set; }

			public List<Uri> Domains { get; private set; }

			public string Scheme { get; internal set; }

			public void SetHttpRequestHeaders (string raw)
			{
				if (raw == "*")
					AllowAllHeaders = true;
				else if (raw != null) {
					headers = raw.Split (',');
					for (int i = 0; i < headers.Length; i++)
						headers [i] = headers [i].Trim ();
				}
			}

			public bool IsAllowed (Uri uri, string [] headerKeys)
			{
				// check headers
				if (!AllowAllHeaders && headerKeys.All (s => Array.IndexOf (headers, s) < 0))
					return false;
				// check domains
				if (AllowAnyDomain || (Scheme.Length > 0 && Scheme == uri.Scheme))
					return true;
				if (Domains.All (domain => domain.Host != uri.Host))
					return false;
				return true;
			}
		}

		public class GrantTo
		{
			public GrantTo ()
			{
				Resources = new List<Resource> ();
			}

			public List<Resource> Resources { get; private set; }

			public bool IsGranted (Uri uri)
			{
				foreach (var gr in Resources) {
					if (gr.IncludeSubpaths) {
						if (uri.LocalPath.StartsWith (gr.Path, StringComparison.Ordinal))
							return true;
					} else {
						if (uri.LocalPath == gr.Path)
							return true;
					}
				}
				return false;
			}
		}

		public class Resource
		{
			public string Path { get; set; }
			public bool IncludeSubpaths { get; set; }
		}

		public class ClientAccessPolicyReader
		{
			public ClientAccessPolicyReader (XmlReader reader)
			{
				this.reader = reader;
				cap = new ClientAccessPolicy ();
			}

			XmlReader reader;
			ClientAccessPolicy cap;

			public ClientAccessPolicy Result {
				get { return cap; }
			}

			public void Read ()
			{
				reader.MoveToContent ();
				if (reader.IsEmptyElement) {
					reader.Skip ();
					return;
				}
				reader.ReadStartElement ("access-policy", String.Empty);
				for (reader.MoveToContent (); reader.NodeType != XmlNodeType.EndElement; reader.MoveToContent ()) {
					if (reader.NodeType != XmlNodeType.Element)
						throw new XmlException (String.Format ("Unexpected access-policy content: {0}", reader.NodeType));
					if (reader.IsEmptyElement) {
						reader.Skip ();
						continue;
					}
					reader.ReadStartElement ("cross-domain-access", String.Empty);
					for (reader.MoveToContent (); reader.NodeType != XmlNodeType.EndElement; reader.MoveToContent ()) {
						if (reader.NodeType != XmlNodeType.Element)
							throw new XmlException (String.Format ("Unexpected access-policy content: {0}", reader.NodeType));
						ReadPolicyElement ();
					}
					reader.ReadEndElement ();
				}
				reader.ReadEndElement ();
			}

			void ReadPolicyElement ()
			{
				if (reader.IsEmptyElement) {
					reader.Skip ();
					return;
				}

				AccessPolicy policy = new AccessPolicy ();
				cap.AccessPolicyList.Add (policy);

				reader.ReadStartElement ("policy", String.Empty);
				for (reader.MoveToContent (); reader.NodeType != XmlNodeType.EndElement; reader.MoveToContent ()) {
					if (reader.NodeType != XmlNodeType.Element)
						throw new XmlException (String.Format ("Unexpected policy content: {0}", reader.NodeType));
					if (reader.IsEmptyElement || reader.NamespaceURI != String.Empty) {
						reader.Skip ();
						continue;
					}
					switch (reader.LocalName) {
					case "allow-from":
						ReadAllowFromElement (policy);
						break;
					case "grant-to":
						ReadGrantToElement (policy);
						break;
					default:
						reader.Skip ();
						continue;
					}
				}
				reader.ReadEndElement ();
			}

			void ReadAllowFromElement (AccessPolicy policy)
			{
				var v = new AllowFrom ();
				policy.AllowedServices.Add (v);

				if (reader.IsEmptyElement) {
					reader.Skip ();
					return;
				}

				v.SetHttpRequestHeaders (reader.GetAttribute ("http-request-headers"));
				reader.ReadStartElement ("allow-from", String.Empty);
				for (reader.MoveToContent (); reader.NodeType != XmlNodeType.EndElement; reader.MoveToContent ()) {
					if (reader.NodeType != XmlNodeType.Element)
						throw new XmlException (String.Format ("Unexpected allow-from content: {0}", reader.NodeType));
					if (reader.NamespaceURI != String.Empty) {
						reader.Skip ();
						continue;
					}
					switch (reader.LocalName) {
					case "domain":
						var d = reader.GetAttribute ("uri");
						switch (d) {
						case "*":
							v.AllowAnyDomain = true;
							break;
						case "http://*":
							v.Scheme = "http";
							break;
						case "https://*":
							v.Scheme = "https";
							break;
						case "file:///":
							v.Scheme = "file";
							break;
						default:
							// FIXME: other, more complex, wildcards needs to be supported
							v.Domains.Add (new Uri (d));
							break;
						}
						reader.Skip ();
						break;
					default:
						reader.Skip ();
						continue;
					}
				}
				reader.ReadEndElement ();
			}

			void ReadGrantToElement (AccessPolicy policy)
			{
				var v = new GrantTo ();
				policy.GrantedResources.Add (v);

				if (reader.IsEmptyElement) {
					reader.Skip ();
					return;
				}

				reader.ReadStartElement ("grant-to", String.Empty);
				for (reader.MoveToContent (); reader.NodeType != XmlNodeType.EndElement; reader.MoveToContent ()) {
					if (reader.NodeType != XmlNodeType.Element)
						throw new XmlException (String.Format ("Unexpected grant-to content: {0}", reader.NodeType));
					if (reader.NamespaceURI != String.Empty) {
						reader.Skip ();
						continue;
					}
					switch (reader.LocalName) {
					case "resource":
						var r = new Resource ();
						v.Resources.Add (r);
						r.Path = reader.GetAttribute ("path");
						if (reader.MoveToAttribute ("include-subpaths")) {
							r.IncludeSubpaths = XmlConvert.ToBoolean (reader.Value);
							reader.MoveToElement ();
						}
						break;
					case "socket-resource":
						// ignore everything that is not TCP
						if (reader.GetAttribute ("protocol") != "tcp")
							break;
						// we can merge them all together inside a policy
						policy.PortMask |= ParsePorts (reader.GetAttribute ("port"));
						break;
					}
					reader.Skip ();
				}
				reader.ReadEndElement ();
			}

			// e.g. reserved ? 4534-4502
			static long ParsePorts (string ports)
			{
				long mask = 0;
				int sep = ports.IndexOf ('-');
				if (sep >= 0) {
					// range
					ushort from = ParsePort (ports.Substring (0, sep));
					ushort to = ParsePort (ports.Substring (sep + 1));
					for (int port = from; port <= to; port++)
						mask |= (long) (1ul << (port - AccessPolicy.MinPort));
				} else {
					// single
					ushort port = ParsePort (ports);
					mask |= (long) (1ul << (port - AccessPolicy.MinPort));
				}
				return mask;
			}

			static ushort ParsePort (string s)
			{
				ushort port;
				if (!UInt16.TryParse (s, out port) || (port < AccessPolicy.MinPort) || (port > AccessPolicy.MaxPort))
					throw new XmlException ("Invalid port");
				return port;
			}
		}
	}
}

#endif

