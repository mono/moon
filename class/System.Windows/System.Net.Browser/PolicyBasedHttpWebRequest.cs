//
// System.Windows.Browser.Net.BaseHttpWebRequest class
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007,2009-2010 Novell, Inc (http://www.novell.com)
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

using System.IO;
using System.Net.Policy;
using System.Security;

namespace System.Net.Browser {

	// This class maps with Silverlight exposed behavior.
	// One request can represent multiple connections with a server.
	// e.g. requesting a policy for cross-domain requests
	// e.g. http redirection

	abstract class PolicyBasedWebRequest : HttpWebRequest {
		private Uri uri;
		private bool aborted;
		private bool allow_read_buffering;
		private string method;
		private long content_length;

		private ICrossDomainPolicy policy;

		protected HttpWebAsyncResult async_result;
		protected WebResponse response;

 		public PolicyBasedWebRequest (Uri uri)
 		{
			this.uri = uri;
			allow_read_buffering = true;
			method = "GET";
			content_length = -1;
		}

		~PolicyBasedWebRequest () /* thread-safe: no p/invokes */
		{
			try {
				Abort ();
			}
			finally {
				if (async_result != null)
					async_result.Dispose ();
			}
		}

		[MonoTODO ("value is unused, current implementation always works like it's true (default)")]
		public override bool AllowReadStreamBuffering {
			get { return allow_read_buffering; }
			set { allow_read_buffering = value; }
		}

		public override long ContentLength {
			get { return content_length; }
			set { content_length = value; }
		}

		protected bool IsAborted {
			get { return aborted; }
		}

		bool IsDownloadingPolicy ()
		{
			return (policy == CrossDomainPolicyManager.PolicyDownloadPolicy);
		}

		internal bool IsSiteOfOrigin ()
		{
			return (policy is SiteOfOriginPolicy);
		}

		public override string Method {
			get { return method; }
			set {
				CheckMethod (value);
				method = value;
			}
		}

		public override Uri RequestUri {
			get { return uri; }
		}


		public override void Abort ()
		{
			aborted = true;
		}

		// NOTE: the supplied callback must be called only once the request is complete
		//	 i.e. it's not called after the policy is downloaded
		//	 i.e. it's not called after each redirection we need to follow
		public override IAsyncResult BeginGetResponse (AsyncCallback callback, object state)
		{
			// Console.WriteLine ("{0} {1} {2}", GetType (), method, uri);
			// we're not allowed to reuse an aborted request
			if (IsAborted)
				throw new WebException ("Aborted", WebExceptionStatus.RequestCanceled);

			// under SL the callback MUST call the EndGetResponse, so having no callback is BAD
			// this also means that faking a synch op using EndGetReponse(BeginGetReponse(null,null)) does NOT work
			if (callback == null)
				throw new NotSupportedException ();

			// we cannot issue 2 requests from the same instance
			if (async_result != null)
				throw new InvalidOperationException ();

			// this is the "global/total" IAsyncResult, it's also the public one
			async_result = new HttpWebAsyncResult (callback, state);

			GetResponse (this.Method, uri, true);
			return async_result;
		}

		private IAsyncResult GetResponse (string method, Uri uri, bool sendHeaders)
		{
			if ((uri.Scheme != "http") && (uri.Scheme != "https")) {
				async_result.Exception = new SecurityException ("Bad scheme");
				async_result.SetComplete ();
				return async_result;
			}

			// this is a same site (site of origin, SOO) request; or
			// we either already know the policy (previously downloaded); or
			// we try to download the policy
			if (!IsDownloadingPolicy ()) {
				policy = CrossDomainPolicyManager.GetCachedWebPolicy (uri);
				if (policy == null) {
					// we'll download the policy *then* proceed to the requested URI
					policy = CrossDomainPolicyManager.PolicyDownloadPolicy;

					Uri silverlight_policy_uri = CrossDomainPolicyManager.GetSilverlightPolicyUri (uri);
					BrowserHttpWebRequestInternal preq = new BrowserHttpWebRequestInternal (null, silverlight_policy_uri);
					return preq.BeginGetResponse (new AsyncCallback (SilverlightPolicyCallback), preq);
				}
			}

			// Console.WriteLine ("{0} '{1}' using policy: {2}", method, uri, policy);
			HttpWebRequest wreq = GetHttpWebRequest (uri);
			wreq.Method = method;
			// store exception, to throw later, if we have no policy or are not allowed by the policy
#if !ANDROID_HACK
			if ((policy == null) || !policy.IsAllowed (wreq)) {
				if ((policy == null) || (policy.Exception == null))
					async_result.Exception = new SecurityException ();
				else
					async_result.Exception = policy.Exception;
				async_result.SetComplete ();
				return async_result;
			}
#endif

			if (!sendHeaders)
				wreq.Headers.Clear ();
			wreq.progress = progress;

			return wreq.BeginGetResponse (new AsyncCallback (EndCallback), wreq);
		}

		private void SilverlightPolicyCallback (IAsyncResult result)
		{
			WebRequest wreq = (result.AsyncState as WebRequest);
			BrowserHttpWebResponse wres = (BrowserHttpWebResponse) wreq.EndGetResponse (result);

			policy = CrossDomainPolicyManager.BuildSilverlightPolicy (wres);
			if (policy != null) {
				// we got our policy so we can proceed with the main request
				GetResponse (this.Method, uri, true);
			} else {
				// no policy but we get a second chance to try a Flash policy
				Uri flash_policy_uri = CrossDomainPolicyManager.GetFlashPolicyUri (wres.ResponseUri);
				BrowserHttpWebRequestInternal preq = new BrowserHttpWebRequestInternal (null, flash_policy_uri);
				preq.BeginGetResponse (new AsyncCallback (FlashPolicyCallback), preq);
			}
		}

		private void FlashPolicyCallback (IAsyncResult result)
		{
			WebRequest wreq = (result.AsyncState as WebRequest);
			BrowserHttpWebResponse wres = (BrowserHttpWebResponse) wreq.EndGetResponse (result);

			// we either got a Flash policy or (if none/bad) a NoAccessPolicy, either way we continue...
			policy = CrossDomainPolicyManager.BuildFlashPolicy (wres);
			GetResponse (this.Method, uri, true);
		}

		internal Exception NotFound (string scheme, WebResponse wres)
		{
			if (scheme == "https")
				return new SecurityException ();

			return new WebException ("NotFound", null, WebExceptionStatus.UnknownError, wres);
		}

		private void EndCallback (IAsyncResult result)
		{
			WebRequest wreq = (result.AsyncState as WebRequest);
			// new in SL4 - unlike others it can be set (earlier) and is not checked later (CheckProtocolViolation)
			if (wreq.Headers.ContainsKey ("Proxy-Authorization")) {
				if (!SecurityManager.HasElevatedPermissions) {
					async_result.Exception = new SecurityException ("'Proxy-Authorization' cannot be set unless running in elevated trust");
					async_result.SetComplete ();
					return;
				}
			}

			try {
				HttpWebResponseCore wres = (HttpWebResponseCore) wreq.EndGetResponse (result);
				//			Redirection	Error
				// Normal Request	allowed		throw
				// Policy Request	throw		ignore (no policy)
				if (IsRedirection (wres)) {
					if (IsDownloadingPolicy ()) {
						// redirection is NOT allowed for policy files
						async_result.Exception = new SecurityException ("Cannot redirect policy files");
						async_result.SetComplete ();
					} else {
						string location = wres.InternalHeaders ["Location"];
						Uri redirect = new Uri (location, UriKind.RelativeOrAbsolute);
						if (!redirect.IsAbsoluteUri)
							redirect = new Uri (wreq.RequestUri, redirect);
						// Silverlight does NOT redirect POST as POST to avoid cross site attacks - see DRT #866 or
						// http://blogs.msdn.com/jackgr/archive/2010/04/19/silverlight-clients-and-appfabric-access-control.aspx
						if ((String.Compare (method, "HEAD", StringComparison.OrdinalIgnoreCase) == 0) || 
							(String.Compare (method, "GET", StringComparison.OrdinalIgnoreCase) == 0)) {
							GetResponse (method, redirect, true);
						} else {
							GetResponse ("GET", redirect, false);
						}
					}
				} else if (wres.StatusCode != HttpStatusCode.OK) {
					// policy file could be missing, but then it means no policy
					if (!IsDownloadingPolicy ()) {
						async_result.Response = wres;
						async_result.Exception = NotFound (wres.ResponseUri.Scheme, wres);
						async_result.SetComplete ();
					} else {
						async_result.SetComplete ();
					}
				} else {
					wres.SetMethod (Method);
					async_result.Response = wres;
					async_result.SetComplete ();
				}
			}
			catch (Exception e) {
				async_result.Exception = NotFound (wreq.RequestUri.Scheme, async_result.Response ?? new NotFoundWebResponse ());
				async_result.SetComplete ();
			}
		}

		public override WebResponse EndGetResponse (IAsyncResult asyncResult)
		{
			try {
				CheckProtocolViolation ();

				if (async_result != asyncResult)
					throw new ArgumentException ("asyncResult");

				if (aborted) {
					throw new WebException ("Aborted", WebExceptionStatus.RequestCanceled);
				}

				// we could already have an exception waiting for us
				if (async_result.HasException)
					throw async_result.Exception;

				if (!async_result.IsCompleted)
					async_result.AsyncWaitHandle.WaitOne ();

				// (again) exception could occur during the wait
				if (async_result.HasException)
					throw async_result.Exception;

				response = async_result.Response;
			}
			finally {
				async_result.Dispose ();
			}

			return response;
		}

		bool IsRedirection (HttpWebResponseCore response)
		{
			// FIXME - there's likely a maximum number of redirection allowed because throwing an exception
			switch ((int) response.RealStatusCode) {
			case 301:	// Moved Permanently, RFC2616 10.3.2
					// Silverlight always redirect (i.e. not just POST requests)
			case 302:	// Found, RFC2616 10.3.3
					// main one used by ASP/ASPX Redirect
			case 303:	// See Other, RFC2616 10.3.4
			case 304:	// Not Modified, RFC2616 10.3.5
			case 305:	// Use Proxy, RFC2616 10.3.7
			case 307:	// Temporaray Redirect, RFC2616 10.3.8
					// see DRT 867
				return true;
			default:
				return false;
			}
		}

		static char[] NewLine = new char[] { '\n', '\r' };
		static char[] Prefix = new char[] { ' ', '\t' };

		internal static bool IsMultilineValue (string value)
		{
			if ((value == null) || (value.IndexOfAny (NewLine) == -1))
				return false;
			return (value.IndexOfAny (Prefix) != -1);
		}

		abstract protected void CheckMethod (string method);
		abstract protected void CheckProtocolViolation ();

		abstract protected HttpWebRequest GetHttpWebRequest (Uri uri);
	}
}

#endif
