//
// System.Windows.Browser.Net.BrowserHttpWebRequest class
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
using System.Net;
using System.Net.Browser;
using System.Security;

namespace System.Windows.Browser.Net {

	// This class maps with Silverlight exposed behavior.
	// One request can represent multiple connections with a server.
	// e.g. requesting a policy for cross-domain requests
	// e.g. http redirection

	sealed class BrowserHttpWebRequest : HttpWebRequest {
		Uri uri;
		bool aborted;
		bool allow_read_buffering;
		string method = "GET";

		ICrossDomainPolicy policy;

		internal InternalWebRequestStreamWrapper request;
		BrowserHttpWebResponse response;
		BrowserHttpWebAsyncResult async_result;
 		
		//NOTE: This field name needs to stay in sync with WebRequest_2_1.cs in System.Net
		// FIXME: how does this behave wrt redirection ?
 		Action<long,long,object> progress;

 		public BrowserHttpWebRequest (Uri uri)
 		{
 			this.uri = uri;
			aborted = false;
			allow_read_buffering = true;
		}

		~BrowserHttpWebRequest () /* thread-safe: no p/invokes */
		{
			Abort ();

			if (async_result != null)
				async_result.Dispose ();
		}

		// FIXME: to be moved to client stack only - but needed for SL3 as long as we share a single stack
		public override CookieContainer CookieContainer {
			get;
			set;
		}

		public override IWebRequestCreate CreatorInstance { 
			get { return WebRequestCreator.BrowserHttp; }
		}

		public override void Abort ()
		{
			if (response != null)
				response.InternalAbort ();

			InternalAbort ();
		}

		internal void InternalAbort ()
		{
			aborted = true;
		}

		public override IAsyncResult BeginGetRequestStream (AsyncCallback callback, object state)
		{
			Console.WriteLine ("BrowserHttpWebRequest.BeginGetRequestStream: Should be async, doing it sync for now.");
			BrowserHttpWebAsyncResult result = new BrowserHttpWebAsyncResult (callback, state);
			result.SetComplete ();
			return result;
		}

		// NOTE: the supplied callback must be called only once the request is complete
		//	 i.e. it's not called after the policy is downloaded
		//	 i.e. it's not called after each redirection we need to follow
		public override IAsyncResult BeginGetResponse (AsyncCallback callback, object state)
		{
			// we're not allowed to reuse an aborted request
			if (aborted)
				throw new WebException ("Aborted", WebExceptionStatus.RequestCanceled);

			// under SL the callback MUST call the EndGetResponse, so having no callback is BAD
			// this also means that faking a synch op using EndGetReponse(BeginGetReponse(null,null)) does NOT work
			if (callback == null)
				throw new NotSupportedException ();

			// we cannot issue 2 requests from the same instance
			if (async_result != null)
				throw new InvalidOperationException ();

			// this is the "global/total" IAsyncResult, it's also the public one
			async_result = new BrowserHttpWebAsyncResult (callback, state);

			GetResponse (this.Method, uri);
			return async_result;
		}

		private IAsyncResult GetResponse (string method, Uri uri)
		{
			// this is a same site (site of origin, SOO) request; or
			// we either already know the policy (previously downloaded); or
			// we try to download the policy
			if (!IsDownloadingPolicy ()) {
				policy = CrossDomainPolicyManager.GetCachedWebPolicy (uri);
				if (policy == null) {
					// we'll download the policy *then* proceed to the requested URI
					policy = CrossDomainPolicyManager.PolicyDownloadPolicy;

					Uri silverlight_policy_uri = CrossDomainPolicyManager.GetSilverlightPolicyUri (uri);
					BrowserHttpWebRequestInternal preq = new BrowserHttpWebRequestInternal (silverlight_policy_uri);
					return preq.BeginGetResponse (new AsyncCallback (SilverlightPolicyCallback), preq);
				}
			}

			// Console.WriteLine ("{0} '{1}' using policy: {2}", method, uri, policy);
			BrowserHttpWebRequestInternal wreq = new BrowserHttpWebRequestInternal (this, uri);
			wreq.Method = method;
			// store SecurityException, to throw later, if we have no policy or are not allowed by the policy
			if ((policy == null) || !policy.IsAllowed (wreq)) {
				async_result.Exception = new SecurityException ();
				async_result.SetComplete ();
				return async_result;
			}

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
				GetResponse (this.Method, uri);
			} else {
				// no policy but we get a second chance to try a Flash policy
				Uri flash_policy_uri = CrossDomainPolicyManager.GetFlashPolicyUri (wres.ResponseUri);
				BrowserHttpWebRequestInternal preq = new BrowserHttpWebRequestInternal (flash_policy_uri);
				preq.BeginGetResponse (new AsyncCallback (FlashPolicyCallback), preq);
			}
		}

		private void FlashPolicyCallback (IAsyncResult result)
		{
			WebRequest wreq = (result.AsyncState as WebRequest);
			BrowserHttpWebResponse wres = (BrowserHttpWebResponse) wreq.EndGetResponse (result);

			// we either got a Flash policy or (if none/bad) a NoAccessPolicy, either way we continue...
			policy = CrossDomainPolicyManager.BuildFlashPolicy (wres);
			GetResponse (this.Method, uri);
		}

		private void EndCallback (IAsyncResult result)
		{
			WebRequest wreq = (result.AsyncState as WebRequest);
			BrowserHttpWebResponse wres = (BrowserHttpWebResponse) wreq.EndGetResponse (result);

			//			Redirection	Error
			// Normal Request	allowed		throw
			// Policy Request	throw		ignore (no policy)
			if (IsRedirection (wres)) {
				if (IsDownloadingPolicy ()) {
					// redirection is NOT allowed for policy files
					async_result.Exception = new SecurityException ("Cannot redirect policy files");
					async_result.SetComplete ();
				} else {
					string location = wres.Headers ["Location"];
					GetResponse ("GET", new Uri (location));
				}
			} else if (wres.StatusCode != HttpStatusCode.OK) {
				// policy file could be missing, but then it means no policy
				if (!IsDownloadingPolicy ()) {
					async_result.Response = wres;
					async_result.Exception = new WebException ("NotFound", null, WebExceptionStatus.Success, wres);
					async_result.SetComplete ();
				} else {
					async_result.SetComplete ();
				}
			} else {
				wres.method = method;
				async_result.Response = wres;
				async_result.SetComplete ();
			}
		}

		public override Stream EndGetRequestStream (IAsyncResult asyncResult)
		{
			if (request == null)
				request = new InternalWebRequestStreamWrapper (new MemoryStream ());

			return request;
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

		bool IsRedirection (BrowserHttpWebResponse response)
		{
			// FIXME - there's likely a maximum number of redirection allowed because throwing an exception
			switch (response.RealStatusCode) {
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

		bool IsDownloadingPolicy ()
		{
			return (policy == CrossDomainPolicyManager.PolicyDownloadPolicy);
		}

		[MonoTODO ("value is unused, current implementation always works like it's true (default)")]
		public override bool AllowReadStreamBuffering {
			get { return allow_read_buffering; }
			set { allow_read_buffering = value; }
		}

		public override bool HaveResponse {
			get {
				if (response != null)
					return true;
				if (async_result != null && async_result.Response != null)
					return true;
				return false;
			}
		}

		public override string Method {
			get { return method; }
			set {
				if (String.IsNullOrEmpty (value))
					throw new NotSupportedException ("Method");

				switch (value.ToUpperInvariant ()) {
				case "GET":
				case "POST":
					method = value;
					break;
				default:
					throw new NotSupportedException ("Method " + value);
				}
			}
		}

		public override Uri RequestUri {
			get { return uri; }
		}

		static string[] bad_get_headers = { "Content-Encoding", "Content-Language", "Content-MD5", "Expires" };

		void CheckProtocolViolation ()
		{
			if (String.Compare (method, "GET", StringComparison.OrdinalIgnoreCase) != 0)
				return;

			// most headers are checked when set, but some are checked much later
			foreach (string header in bad_get_headers) {
				// case insensitive check to internal Headers dictionary
				if (Headers.headers.ContainsKey (header))
					throw new ProtocolViolationException ();
			}
		}
	}
}

#endif
