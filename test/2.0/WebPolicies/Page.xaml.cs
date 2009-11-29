using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Security;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Threading;

namespace WebPolicies {
	public partial class Page : UserControl {
		public Page ()
		{
			InitializeComponent ();
			this.WebClientButton.Click += delegate {
				WebClientTest ();
			};
			this.WebRequestButton.Click += delegate {
				WebRequestTest ();
			};
		}

		static string CheckOk (string status)
		{
			if (status == "OK") {
				pass++;
				return "PASS (OK)";
			} else {
				fail++;
				return "FAIL (" + status + ")";
			}
		}

		static string CheckSecurityException (string status)
		{
			if (status == "SecurityException") {
				pass++;
				return "PASS (SecurityException)";
			} else {
				fail++;
				return "FAIL (" + status + ")";
			}
		}

		// TODO
		// ? extend to POST (not just GET)
		// ? support Headers
		// ? support HTTPS (actually it's just using a second web server)

		Dictionary<string, Func<string, string>> test_cases = new Dictionary<string, Func<string, string>> () {
			// flash-1 accept everything
			{ "http://flash-1.moonlight.test/test/allow-all-domains-simplest", CheckOk },
			{ "http://flash-1.moonlight.test/crossdomain.xml", CheckOk },						// we can get retrieve the policy
			{ "http://flash-1.moonlight.test/../using-double-dot", CheckOk },
			{ "http://flash-1.moonlight.test/./using-single-dot", CheckOk },
			{ "http://flash-1.moonlight.test/using%20percent-sign", CheckOk },
			{ "http://flash-1.moonlight.test/test/../using-double-dot", CheckOk },
			{ "http://flash-1.moonlight.test/test/./using-single-dot", CheckOk },
			{ "http://flash-1.moonlight.test/test/using%20percent-sign", CheckOk },
			{ "http://flash-1.moonlight.test/test/path?query=../using-double-dot", CheckOk },
			{ "http://flash-1.moonlight.test/test/path?query=./using-single-dot", CheckOk },
			{ "http://flash-1.moonlight.test/test/path?query=using%20percent-sign", CheckOk },

			{ "http://flash-2.moonlight.test/test/allow-caller-domain-only-using-url", CheckSecurityException },
			{ "http://flash-3.moonlight.test/test/allow-caller-domain-only-using-ipaddress", CheckSecurityException },
			// flash-4 accept everything but add a "X-Permitted-Cross-Domain-Policies: none" header
			{ "http://flash-4.moonlight.test/test/allow-all-domains-X-Permitted-Cross-Domain-Policies-none", CheckSecurityException },
			{ "http://flash-5.moonlight.test/test/allow-all-domains-X-Permitted-Cross-Domain-Policies-master-only", CheckOk },
			{ "http://flash-6.moonlight.test/test/allow-all-domains-X-Permitted-Cross-Domain-Policies-by-content-type", CheckSecurityException },
			{ "http://flash-7.moonlight.test/test/allow-all-domains-X-Permitted-Cross-Domain-Policies-by-ftp-filename", CheckOk },
			{ "http://flash-8.moonlight.test/test/allow-all-domains-X-Permitted-Cross-Domain-Policies-all", CheckOk },
			{ "http://flash-9.moonlight.test/test/allow-all-domains-X-Permitted-Cross-Domain-Policies-invalid-value", CheckSecurityException },
			// flash-1 accept everything and includes a DTD declaration in the XML
			{ "http://flash-10.moonlight.test/test/allow-all-domains-with-DTD", CheckOk },
			{ "http://flash-11.moonlight.test/test/allow-all-domains-with-DTD-secure-true", CheckOk },
			{ "http://flash-12.moonlight.test/test/allow-all-domains-with-DTD-secure-false", CheckOk },
			{ "http://flash-13.moonlight.test/test/allow-all-domains-with-DTD-secure-invalid-value", CheckSecurityException },
			// flash-14 has no crossdomain.xml (nor a clientaccesspolicy.xml)
			{ "http://flash-14.moonlight.test/", CheckSecurityException },
			// flash-15 has an invalid (but accepted) XML policy with whitespace preceding the XML declaration
			{ "http://flash-15.moonlight.test/", CheckOk },
			// flash-16-21 (same as 4-9 except it use XML <site-control permitted-cross-domain-policies=X>
			{ "http://flash-16.moonlight.test/test/allow-all-domains-permitted-cross-domain-policies-none", CheckSecurityException },
			{ "http://flash-17.moonlight.test/test/allow-all-domains-permitted-cross-domain-policies-master-only", CheckOk },
			{ "http://flash-18.moonlight.test/test/allow-all-domains-permitted-cross-domain-policies-by-content-type", CheckSecurityException },
			{ "http://flash-19.moonlight.test/test/allow-all-domains-permitted-cross-domain-policies-by-ftp-filename", CheckOk },
			{ "http://flash-20.moonlight.test/test/allow-all-domains-permitted-cross-domain-policies-all", CheckOk },
			{ "http://flash-21.moonlight.test/test/allow-all-domains-permitted-cross-domain-policies-invalid-value", CheckSecurityException },
			// another test where we check Domain!="*" using policy-client
			{ "http://flash-22.moonlight.test/test/allow-caller-domain-policy-client", CheckSecurityException },	// fully named
			{ "http://flash-23.moonlight.test/test/allow-caller-domain-wildcard", CheckSecurityException },		// using wildcard *

			// add no policy test
			// add redirection test (not allowed on policy files)
			// return some binary file
			// add policy inside comment
			// add policy embedded inside another tag

			// silverlight-1 grant every path under /
			{ "http://silverlight-1.moonlight.test/test/allow-all-domains-simplest", CheckOk },
			{ "http://silverlight-1.moonlight.test/clientaccesspolicy.xml", CheckOk },				// we can get retrieve the policy
			{ "http://silverlight-1.moonlight.test/../using-double-dot", CheckOk },
			{ "http://silverlight-1.moonlight.test/./using-single-dot", CheckOk },
			{ "http://silverlight-1.moonlight.test/using%20percent-sign", CheckOk },
			{ "http://silverlight-1.moonlight.test/test/../using-double-dot", CheckOk },
			{ "http://silverlight-1.moonlight.test/test/./using-single-dot", CheckOk },
			{ "http://silverlight-1.moonlight.test/test/using%20percent-sign", CheckOk },
			// silverlight-2 grant every path under "/test"
			{ "http://silverlight-2.moonlight.test/test/allow-all-domains-under-test", CheckOk },
			{ "http://silverlight-2.moonlight.test/test/", CheckOk },
			{ "http://silverlight-2.moonlight.test/test", CheckOk },
			{ "http://silverlight-2.moonlight.test/test-file", CheckSecurityException },
			{ "http://silverlight-2.moonlight.test/test-dir/test", CheckSecurityException },
			{ "http://silverlight-2.moonlight.test/TEST/allow-all-domains-under-test", CheckSecurityException },	// policy is case sensitive
			{ "http://silverlight-2.moonlight.test/clientaccesspolicy.xml", CheckSecurityException },		// we cannot retrieve the policy
			{ "http://silverlight-2.moonlight.test/test/../using-double-dot", CheckSecurityException },
			{ "http://silverlight-2.moonlight.test/test/./using-single-dot", CheckSecurityException },
			{ "http://silverlight-2.moonlight.test/test/using%20percent-sign", CheckSecurityException },
			{ "http://silverlight-2.moonlight.test/test/path?query=../using-double-dot", CheckOk },		// invalid path characters in query
			{ "http://silverlight-2.moonlight.test/test/path?query=./using-single-dot", CheckOk },
			{ "http://silverlight-2.moonlight.test/test/path?query=using%20percent-sign", CheckOk },
			// silverlight-3 grant every path under "/test/again"
			{ "http://silverlight-3.moonlight.test/test/again/allow-all-domains-under-test-again", CheckOk },
			{ "http://silverlight-3.moonlight.test/test/allow-all-domains-under-test", CheckSecurityException },
			{ "http://silverlight-3.moonlight.test/test/again/deeper/../using-double-dot", CheckSecurityException },
			{ "http://silverlight-3.moonlight.test/test/again/deeper/./using-single-dot", CheckSecurityException },
			{ "http://silverlight-3.moonlight.test/test/again/deeper/using%20percent-sign", CheckSecurityException },
			// silverlight-4 grant / but no subpaths
			{ "http://silverlight-4.moonlight.test/clientaccesspolicy.xml", CheckSecurityException },
			{ "http://silverlight-4.moonlight.test/../", CheckSecurityException },
			{ "http://silverlight-4.moonlight.test/./", CheckSecurityException },
			{ "http://silverlight-4.moonlight.test/%20", CheckSecurityException },
			{ "http://silverlight-4.moonlight.test/", CheckOk },
			// silverlight-5 grant everything under ""
			{ "http://silverlight-5.moonlight.test/", CheckSecurityException },
			// silverlight-6 grant everything under "" and "/test"
			{ "http://silverlight-6.moonlight.test/", CheckSecurityException },
			{ "http://silverlight-6.moonlight.test/test", CheckSecurityException },		// fail because "" is invalid
			{ "http://silverlight-6.moonlight.test/test/", CheckSecurityException },	// fail because "" is invalid
			{ "http://silverlight-6.moonlight.test/test/file", CheckSecurityException },	// fail because "" is invalid
			// silverlight-7 has an invalid (but accepted) XML policy with whitespace preceding the XML declaration
			{ "http://silverlight-7.moonlight.test/", CheckOk },
			// test cases where Domain!='*'
			{ "http://silverlight-8.moonlight.test/", CheckSecurityException },	//  (without scheme) is invalid
			{ "http://silverlight-9.moonlight.test/", CheckOk },			// http://policy-client == OK
			{ "http://silverlight-9.moonlight.test/test/file", CheckOk },		// http://policy-client == OK
			{ "http://silverlight-10.moonlight.test/", CheckOk },			// http://policy-client:80 (default port) == OK
			{ "http://silverlight-10.moonlight.test/test/file", CheckOk },		// http://policy-client:80 (default port) == OK
			{ "http://silverlight-11.moonlight.test/", CheckSecurityException },	// http://policy-client:8080 (port) is invalid
			{ "http://silverlight-12.moonlight.test/", CheckSecurityException },	// https://policy-client is invalid (from http)
			{ "http://silverlight-13.moonlight.test/", CheckSecurityException },	// http://policy-client/WebPolicies.html is invalid
			{ "http://silverlight-14.moonlight.test/", CheckSecurityException },	// another http host
			{ "http://silverlight-15.moonlight.test/", CheckOk },			// http://policy-client/WebPolicies.xap == OK
			{ "http://silverlight-16.moonlight.test/", CheckSecurityException },	// http://policy-client/WebPolicies is invalid
			// test cases with wildcards
			{ "http://silverlight-17.moonlight.test/", CheckOk },				// http://*.moonlight.test/ == OK
			{ "http://silverlight-17.moonlight.test/test/file", CheckOk },			// http://*.moonlight.test/ == OK
			{ "http://silverlight-17.moonlight.test:80/test/file", CheckOk },		// http://*.moonlight.test:80/ == OK
			{ "http://silverlight-17.moonlight.test:8080/", CheckSecurityException },	// http://*.moonlight.test:8080/ is invalid
			{ "http://silverlight-18.moonlight.test/", CheckSecurityException },		// http://policy-client.*.test/ is invalid
			{ "http://silverlight-19.moonlight.test/", CheckSecurityException },		// http://policy-client.moonlight.*/ is invalid
			{ "http://silverlight-20.moonlight.test/", CheckSecurityException },		// http://*-client.moonlight.test/ is invalid
			{ "http://silverlight-21.moonlight.test/", CheckSecurityException },		// http://*.moonlight.test/test/file is invalid
			{ "http://silverlight-21.moonlight.test/test/file", CheckSecurityException },	// http://*.moonlight.test/test/file is invalid
			{ "http://silverlight-22.moonlight.test/file#fragment", CheckSecurityException },	// wildcard with fragment
			{ "http://silverlight-23.moonlight.test/file?query", CheckSecurityException },		// wildcard with query

			// add redirection test (not allowed on policy files)
			// return some binary file
			// add policy inside comment
			// add policy embedded inside another tag

			// ? infinite xml using a script ?
		};

		static int pass;
		static int fail;
		private int uri_no;
		private string [] urls;
		private Uri current;
		private DateTime time;

		void Start ()
		{
			log.Text = String.Empty;
			uri_no = 1;
			pass = 0;
			fail = 0;
			urls = (string []) test_cases.Keys.ToArray<string> ();
			time = DateTime.UtcNow;
		}

		void ShowFinalStatus ()
		{
			TimeSpan elapsed = DateTime.UtcNow - time;
			status.Text = String.Format ("PASS: {0}    FAIL: {1}    TOTAL: {2}  ({3:F2}%)    {4:F2} seconds",
				pass, fail, urls.Length, (double) pass / urls.Length * 100, elapsed.TotalMilliseconds / 1000);
		}

		// WEBCLIENT

		void WebClientTest ()
		{
			Start ();
			WebClientNext ();
		}

		void WebClientNext ()
		{
			WebClient wc = new WebClient ();
			wc.OpenReadCompleted += delegate (object sender, OpenReadCompletedEventArgs e) {
				string status = "FAIL (no check found)";
				try {
					if (e.Result != null)
						status = "OK";
					else
						status = "null";
				}
				catch (TargetInvocationException tie) {
					if (tie.InnerException is SecurityException)
						status = "SecurityException";
					else
						status = String.Format ("{0} : {1}", tie.InnerException.GetType (), tie.InnerException);
				}
				catch (Exception ex) {
					status = String.Format ("{0} : {1}", ex.GetType (), ex);
				}
				finally {
					Func<string, string> check = null;
					if (test_cases.TryGetValue (current.OriginalString, out check)) {
						log.Text += check (status);
					} else {
						log.Text += status;
						fail++;
					}

					uri_no++;
					if (uri_no <= urls.Length)
						WebClientNext ();
					else
						ShowFinalStatus ();
				}
			};
			string uri = urls [uri_no - 1];
			log.Text += String.Format ("{0}WEBCLIENT {1}. {2} : ", Environment.NewLine, uri_no, uri);
			current = new Uri (uri);
			wc.OpenReadAsync (current);
		}

		// WEBREQUEST

		SynchronizationContext syncContext;

		void WebRequestTest ()
		{
			Start ();
			syncContext = SynchronizationContext.Current;

			for (int i = 0; i < urls.Length; i++) {
				current = new Uri (urls [i]);
				WebRequest wreq = WebRequest.Create (current);
				wreq.BeginGetResponse (GetResponseCallback, wreq);
			}
		}

		void GetResponseCallback (IAsyncResult result)
		{
			string status = "FAIL (no check found)";
			WebRequest wreq = (result.AsyncState as WebRequest);
			try {
				HttpWebResponse wres = (HttpWebResponse) wreq.EndGetResponse (result);
				status = wres.StatusDescription;
			}
			catch (WebException) {
				status = "WebException";
			}
			catch (SecurityException) {
				status = "SecurityException";
			}
			catch (Exception e) {
				status = e.Message;
			}
			finally {
				Func<string, string> check = null;
				if (test_cases.TryGetValue (wreq.RequestUri.OriginalString, out check)) {
					status = check (status);
				} else {
					fail++;
				}

				string msg = String.Format ("{1} : {2}", Environment.NewLine, wreq.RequestUri.OriginalString, status);
				syncContext.Post (new SendOrPostCallback (PostCallback), msg);
			}
		}

		void PostCallback (object o)
		{
			log.Text += String.Format ("{0}WEBREQUEST {1}. {2}", Environment.NewLine, uri_no++, o as string);
			if (uri_no - 1 == urls.Length)
				ShowFinalStatus ();
		}
	}
}

