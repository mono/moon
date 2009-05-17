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

	class CrossDomainPolicyManager {

		const int Timeout = 10000;

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
				using (XmlReader xr = XmlReader.Create (s)) {
					policy = ClientAccessPolicy.Read (xr);
				}
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

