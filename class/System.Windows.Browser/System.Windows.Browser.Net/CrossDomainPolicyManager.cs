//
// CrossDomainPolicyManager.cs
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
using System.Net.Sockets;
using System.Reflection;
using System.Security;
using System.Text;
using System.Threading;
using System.Windows.Interop;
using System.Xml;

namespace System.Windows.Browser.Net {

	class PolicyAsyncResult : BrowserHttpWebAsyncResult {

		public PolicyAsyncResult (AsyncCallback cb, Uri root) :
			base (cb, root)
		{
		}

		public ICrossDomainPolicy Policy { get; set; }

		public Uri PolicyUri { get; set; }

		public Uri RootUri { 
			get { return (AsyncState as Uri); }
		}
	}

	static class CrossDomainPolicyManager {

		public const string ClientAccessPolicyFile = "/clientaccesspolicy.xml";
		public const string CrossDomainFile = "/crossdomain.xml";

		const int Timeout = 10000;

		// Web Access Policy

		public static IAsyncResult BeginGetPolicy (WebRequest request, AsyncCallback cb)
		{
			WebClient wc = new WebClient ();
			wc.OpenReadCompleted += new OpenReadCompletedEventHandler (PolicyReadCompleted);

			// get the root of the URI from where we'll try to read the policy files
			Uri root = new Uri (GetRoot (request.RequestUri));
			Uri silverlight_policy_uri = new Uri (root, ClientAccessPolicyFile);

			PolicyAsyncResult async = new PolicyAsyncResult (cb, root);
			async.PolicyUri = silverlight_policy_uri;

			wc.OpenReadAsync (silverlight_policy_uri, async);
			return async;
		}

		static void PolicyReadCompleted (object sender, OpenReadCompletedEventArgs e)
		{
			PolicyAsyncResult async = (e.UserState as PolicyAsyncResult);
			Uri uri = async.PolicyUri;

			if (!e.Cancelled && e.Error == null && e.Result != null) {
				try {
					ICrossDomainPolicy policy = null;
					if (uri.LocalPath == ClientAccessPolicyFile) {
						policy = ClientAccessPolicy.FromStream (e.Result);
					} else if (uri.LocalPath == CrossDomainFile) {
						policy = BuildFlashPolicy (e.Result, (sender as WebClient).ResponseHeaders);
					}
					async.Policy = policy;
					policies.Add (async.RootUri.OriginalString, policy);
				} catch (Exception ex) {
					Console.WriteLine (String.Format ("CrossDomainAccessManager caught an exception while reading {0}: {1}", 
						uri.LocalPath, ex));
					// and ignore.
				}
				async.SetComplete ();
			} else if (uri.LocalPath == ClientAccessPolicyFile) {
				// we tried (and failed) retrieving the Silverlight policy file, trying the Flash file
				WebClient flash = new WebClient ();
				flash.OpenReadCompleted += new OpenReadCompletedEventHandler (PolicyReadCompleted);

				Uri flash_policy_uri = new Uri (async.RootUri, CrossDomainFile);
				async.PolicyUri = flash_policy_uri;

				flash.OpenReadAsync (flash_policy_uri, async);
			} else {
				// don't fire the callback
				async.Exception = e.Error;
				async.SetComplete ();
			}
		}

		static ICrossDomainPolicy BuildFlashPolicy (Stream stream, WebHeaderCollection headers)
		{
			FlashCrossDomainPolicy policy = (FlashCrossDomainPolicy) FlashCrossDomainPolicy.FromStream (stream);
			if (policy == null)
				return null;

			string site_control = headers ["X-Permitted-Cross-Domain-Policies"]; // see DRT# 864 and 865
			if (!String.IsNullOrEmpty (site_control))
				policy.SiteControl = site_control;

			return policy;
		}

		public static ICrossDomainPolicy EndGetPolicy (IAsyncResult result)
		{
			PolicyAsyncResult async = (result as PolicyAsyncResult);
			if (async == null || !async.AsyncWaitHandle.WaitOne (Timeout))
				return null;

			return async.Policy;
		}

		static public Dictionary<string,ICrossDomainPolicy> policies = new Dictionary<string,ICrossDomainPolicy> ();

		static public ICrossDomainPolicy PolicyDownloadPolicy = new PolicyDownloadPolicy ();
		static ICrossDomainPolicy site_of_origin_policy = new SiteOfOriginPolicy ();

		static string GetRoot (Uri uri)
		{
			if ((uri.Scheme == "http" && uri.Port == 80) || (uri.Scheme == "https" && uri.Port == 443) || (uri.Port == -1))
				return String.Format ("{0}://{1}", uri.Scheme, uri.DnsSafeHost);
			else
				return String.Format ("{0}://{1}:{2}", uri.Scheme, uri.DnsSafeHost, uri.Port);
		}

		public static ICrossDomainPolicy GetCachedWebPolicy (Uri uri)
		{
			// if we request an Uri from the same site then we return an "always positive" policy
			if (SiteOfOriginPolicy.HasSameOrigin (uri, PluginHost.SourceUri))
				return site_of_origin_policy;

			// otherwise we search for an already downloaded policy for the web site
			string root = GetRoot (uri);
			ICrossDomainPolicy policy = null;
			policies.TryGetValue (root, out policy);
			// and we return it (if we have it) or null (if we dont)
			return policy;
		}

		// Socket Policy
		//
		// - we connect once to a site for the entire application life time
		// - this returns us a policy file (silverlight format only) or else no access is granted
		// - this policy file
		// 	- can contain multiple policies
		// 	- can apply to multiple domains
		//	- can grant access to several resources

		static Dictionary<string,ClientAccessPolicy> socket_policies = new Dictionary<string,ClientAccessPolicy> ();
		static byte [] socket_policy_file_request = Encoding.UTF8.GetBytes ("<policy-file-request/>");
		const int PolicyPort = 943;

		// make sure this work in a IPv6-only environment
		static AddressFamily GetBestFamily ()
		{
			if (Socket.OSSupportsIPv4)
				return AddressFamily.InterNetwork;
			else if (Socket.OSSupportsIPv6)
				return AddressFamily.InterNetworkV6;
			else
				return AddressFamily.Unspecified;
		}

		static Stream GetPolicyStream (IPEndPoint endpoint)
		{
			MemoryStream ms = new MemoryStream ();
			ManualResetEvent mre = new ManualResetEvent (false);
			// Silverlight only support TCP
			Socket socket = new Socket (GetBestFamily (), SocketType.Stream, ProtocolType.Tcp);

			SocketAsyncEventArgs saea = new SocketAsyncEventArgs ();
			saea.RemoteEndPoint = new IPEndPoint (endpoint.Address, PolicyPort);
			saea.Completed += delegate (object sender, SocketAsyncEventArgs e) {
				if (e.SocketError != SocketError.Success) {
					mre.Set ();
					return;
				}

				switch (e.LastOperation) {
				case SocketAsyncOperation.Connect:
					e.SetBuffer (socket_policy_file_request, 0, socket_policy_file_request.Length);
					socket.SendAsync (e);
					break;
				case SocketAsyncOperation.Send:
					byte [] buffer = new byte [256];
					e.SetBuffer (buffer, 0, buffer.Length);
					socket.ReceiveAsync (e);
					break;
				case SocketAsyncOperation.Receive:
					int transfer = e.BytesTransferred;
					if (transfer > 0) {
						ms.Write (e.Buffer, 0, transfer);
						// Console.Write (Encoding.UTF8.GetString (e.Buffer, 0, transfer));
					}

					if ((transfer == 0) || (transfer < e.Buffer.Length)) {
						ms.Position = 0;
						mre.Set ();
					} else {
						socket.ReceiveAsync (e);
					}
					break;
				}
			};

			// Application code can't connect to port 943, so we need a special/internal API to allow this
			socket.ConnectAsync (saea, false);

			// behave like there's no policy (no socket access) if we timeout
			if (!mre.WaitOne (Timeout))
				return null;

			return ms;
		}

		public static ClientAccessPolicy CreateForEndPoint (IPEndPoint endpoint)
		{
			Stream s = GetPolicyStream (endpoint);
			if (s == null)
				return null;

			ClientAccessPolicy policy = null;
			try {
				policy = (ClientAccessPolicy) ClientAccessPolicy.FromStream (s);
			} catch (Exception ex) {
				Console.WriteLine (String.Format ("CrossDomainAccessManager caught an exception while reading {0}: {1}", 
					endpoint, ex.Message));
				// and ignore.
			}

			return policy;
		}

		static public bool CheckEndPoint (EndPoint endpoint)
		{
			// if needed transform the DnsEndPoint into a usable IPEndPoint
			IPEndPoint ip = (endpoint as IPEndPoint);
			if (ip == null) {
				DnsEndPoint dep = (endpoint as DnsEndPoint);
				if (dep != null)
					ip = dep.AsIPEndPoint ();
			}
			if (ip == null)
				throw new ArgumentException ("endpoint");

			// find the policy (cached or to be downloaded) associated with the endpoint
			string address = ip.Address.ToString ();
			ClientAccessPolicy policy = null;
			if (!socket_policies.TryGetValue (address, out policy)) {
				policy = CreateForEndPoint (ip);
				socket_policies.Add (address, policy);
			}

			// no access granted if no policy is available
			if (policy == null)
				return false;

			// does the policy allows access ?
			return policy.IsAllowed (ip);
		}
	}
}

#endif

