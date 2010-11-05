//
// System.Net.Browser.ClientReflectionHelper
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

using System.IO;
using System.Reflection;

namespace System.Net.Browser {

	internal class ClientReflectionHelper {

		internal static Assembly SystemAssembly;
		static Type web_header_collection;
		static MethodInfo headers_all_keys;
		static MethodInfo headers_get;
		static MethodInfo headers_get_values;
		static MethodInfo headers_set;

		static ConstructorInfo network_credential_ctor;

		static ClientReflectionHelper ()
		{
			SystemAssembly = typeof (Uri).Assembly;
			web_header_collection = SystemAssembly.GetType ("System.Net.WebHeaderCollection");
			headers_all_keys = web_header_collection.GetProperty ("AllKeys").GetGetMethod ();

			headers_get = web_header_collection.GetMethod ("Get", new Type [] { typeof (string) });
			headers_get_values = web_header_collection.GetMethod ("GetValues", new Type [] { typeof (string) });
			headers_set = web_header_collection.GetMethod ("Set", new Type [] { typeof (string), typeof (string) });

			Type network_credential = SystemAssembly.GetType ("System.Net.NetworkCredential");
			network_credential_ctor = network_credential.GetConstructor (new Type [] { typeof (string), typeof (string), typeof (string) });
		}

		static public string[] GetHeaderKeys (object headerCollection)
		{
			try {
				return (string[]) headers_all_keys.Invoke (headerCollection, null);
			}
			catch (TargetInvocationException tie) {
				throw tie.InnerException;
			}
		}

		static public string GetHeader (object headerCollection, string name)
		{
			try {
				return (string) headers_get.Invoke (headerCollection, new object [] { name });
			}
			catch (TargetInvocationException tie) {
				throw tie.InnerException;
			}
		}

		static public string[] GetHeaderValues (object headerCollection, string name)
		{
			try {
				return (string []) headers_get_values.Invoke (headerCollection, new object [] { name });
			}
			catch (TargetInvocationException tie) {
				throw tie.InnerException;
			}
		}

		static public void SetHeader (object headerCollection, string name, string value)
		{
			try {
				headers_set.Invoke (headerCollection, new object [] { name, value });
			}
			catch (TargetInvocationException tie) {
				throw tie.InnerException;
			}
		}

		static public object[] BuildCredentials (string username, string password, string domain)
		{
			try {
				return new object[] { 
					network_credential_ctor.Invoke (new object[] { username, password, domain })
				};
			}
			catch (TargetInvocationException tie) {
				throw tie.InnerException;
			}
		}
	}
}

#endif

