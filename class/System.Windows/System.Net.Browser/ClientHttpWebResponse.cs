//
// System.Net.Browser.ClientHttpWebResponse
//
// Authors:
//	Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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

using System.Collections;
using System.Collections.Specialized;
using System.IO;
using System.Reflection;

namespace System.Net.Browser {

	internal class ClientHttpWebResponse : HttpWebResponseCore {

		static MethodInfo get_response_stream;
		static MethodInfo get_content_length;
		static MethodInfo get_content_type;
		static MethodInfo get_headers;
		static MethodInfo get_method;
		static MethodInfo get_response_uri;
		static MethodInfo get_status_code;
		static MethodInfo get_status_description;

		static ClientHttpWebResponse ()
		{
			Type web_response = ClientReflectionHelper.SystemAssembly.GetType ("System.Net.HttpWebResponse");

			get_response_stream = web_response.GetMethod ("GetResponseStream");

			PropertyInfo content_length = web_response.GetProperty ("ContentLength");
			get_content_length = content_length.GetGetMethod ();

			PropertyInfo content_type = web_response.GetProperty ("ContentType");
			get_content_type = content_type.GetGetMethod ();

			PropertyInfo headers = web_response.GetProperty ("Headers");
			get_headers = headers.GetGetMethod ();

			PropertyInfo method = web_response.GetProperty ("Method");
			get_method = method.GetGetMethod ();

			PropertyInfo response_uri = web_response.GetProperty ("ResponseUri");
			get_response_uri = response_uri.GetGetMethod ();

			PropertyInfo status_code = web_response.GetProperty ("StatusCode");
			get_status_code = status_code.GetGetMethod ();

			PropertyInfo status_description = web_response.GetProperty ("StatusDescription");
			get_status_description = status_description.GetGetMethod ();
		}

		private long content_length;
		private string content_type;
		private string method;
		private Uri response_uri;
		private HttpStatusCode status_code;
		private string status_description;
		private WebHeaderCollection headers;
		private Stream stream;
		private CookieCollection jar;

		internal ClientHttpWebResponse (HttpWebRequest request, object response)
		{
			try {
				content_length = (long) get_content_length.Invoke (response, null);
				content_type = (string) get_content_type.Invoke (response, null);
				response_uri = (Uri) get_response_uri.Invoke (response, null);

				SetMethod ((string) get_method.Invoke (response, null));

				SetStatus ((HttpStatusCode) (int) get_status_code.Invoke (response, null),
					(string) get_status_description.Invoke (response, null));

				jar = new CookieCollection ();
				headers = new WebHeaderCollection ();
				object header_collection = get_headers.Invoke (response, null);
				string[] keys = ClientReflectionHelper.GetHeaderKeys (header_collection);
				foreach (string key in keys) {
					switch (key) {
					case "Set-Cookie":
						string[] values = ClientReflectionHelper.GetHeaderValues (header_collection, key);
						foreach (string cookie in values)
							SetCookie (cookie);
						break;
					default:
						string value = ClientReflectionHelper.GetHeader (header_collection, key);
						headers.SetHeader (key, value);
						break;
					}
				}

				using (Stream response_stream = (Stream) get_response_stream.Invoke (response, null)) {
					MemoryStream ms = new MemoryStream ();
					response_stream.CopyTo (ms);
					stream = new InternalWebResponseStreamWrapper (ms, request.AllowReadStreamBuffering);
				}

				(response as IDisposable).Dispose ();
			}
			catch (TargetInvocationException tie) {
				throw tie.InnerException;
			}
		}

		public override long ContentLength {
			get { return content_length; }
		}

		public override string ContentType {
			get { return content_type; }
		}

		public override CookieCollection Cookies {
			get { return jar; }
		}

		// note: SL System.Net's WebHeaderCollection is a different type than (regular) System.dll's WebHeaderCollection
		public override WebHeaderCollection Headers {
			get { return headers; }
		}

		public override Uri ResponseUri {
			get { return response_uri; }
		}

		public override bool SupportsHeaders {
			get { return true; }
		}


		public override void Close ()
		{
		}

		public override Stream GetResponseStream ()
		{
			return stream;
		}

		// copy-pasted (and a bit adapted) from HttpWebResponse
		void SetCookie (string header)
		{
			string name, val;
			Cookie cookie = null;
			CookieParser parser = new CookieParser (header);

			while (parser.GetNextNameValue (out name, out val)) {
				if (String.IsNullOrEmpty (name) && cookie == null)
					continue;

				if (cookie == null) {
					cookie = new Cookie (name, val);
					continue;
				}

				name = name.ToUpper ();
				switch (name) {
				case "COMMENT":
					if (cookie.Comment == null)
						cookie.Comment = val;
					break;
				case "COMMENTURL":
					if (cookie.CommentUri == null)
						cookie.CommentUri = new Uri (val);
					break;
				case "DISCARD":
					cookie.Discard = true;
					break;
				case "DOMAIN":
					if (cookie.Domain.Length == 0)
						cookie.Domain = val;
					break;
				case "HTTPONLY":
					cookie.HttpOnly = true;
					break;
				case "MAX-AGE": // RFC Style Set-Cookie2
					if (cookie.Expires == DateTime.MinValue) {
						try {
						cookie.Expires = cookie.TimeStamp.AddSeconds (UInt32.Parse (val));
						} catch {}
					}
					break;
				case "EXPIRES": // Netscape Style Set-Cookie
					if (cookie.Expires != DateTime.MinValue)
						break;

					cookie.Expires = CookieParser.TryParseCookieExpires (val);
					break;
				case "PATH":
					cookie.Path = val;
					break;
				case "PORT":
					if (cookie.Port == null)
						cookie.Port = val;
					break;
				case "SECURE":
					cookie.Secure = true;
					break;
				case "VERSION":
					try {
						cookie.Version = (int) UInt32.Parse (val);
					} catch {}
					break;
				}
			}

			if (cookie.Domain.Length == 0)
				cookie.Domain = response_uri.Host;

			jar.Add (cookie);
		}
	}
}

#endif

