//
// UriHelper.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
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
using System.Runtime.InteropServices;
using System.Windows;

namespace Mono
{
	internal class UriHelper
	{
		static UriFunctions functions;

		private static IntPtr Ctor1 (string uri_string)
		{
			try {
				return ToGCHandlePtr (ToGCHandle (new Uri (uri_string)));
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.Ctor1 ({0}): {1}", uri_string, ex.Message);
				} catch {
				}
#endif
				return IntPtr.Zero;
			}
		}

		private static IntPtr Ctor2 (string uri_string, UriKind uri_kind)
		{
			try {
				return ToGCHandlePtr (ToGCHandle (new Uri (uri_string, uri_kind)));
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.Ctor2 ({0}, {2}): {1}", uri_string, ex.Message, uri_kind);
				} catch {
				}
#endif
				return IntPtr.Zero;
			}
		}

		private static IntPtr Ctor3 (IntPtr base_uri, string relative_uri)
		{
			try {
				return ToGCHandlePtr (ToGCHandle (new Uri (FromGCHandle (base_uri), relative_uri)));
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.Ctor3 ({0}, {2}): {1}", base_uri, ex.Message, relative_uri);
				} catch {
				}
#endif
				return IntPtr.Zero;
			}
		}

		private static IntPtr Ctor4 (IntPtr base_uri, IntPtr relative_uri)
		{
			try {
				return ToGCHandlePtr (ToGCHandle (new Uri (FromGCHandle (base_uri), FromGCHandle (relative_uri))));
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.Ctor4 ({0}, {2}): {1}", base_uri, ex.Message, relative_uri);
				} catch {
				}
#endif
				return IntPtr.Zero;
			}
		}

		private static IntPtr GetScheme (IntPtr instance)
		{
			try {
				Uri uri;

				if (instance == IntPtr.Zero)
					return IntPtr.Zero;

				uri = FromGCHandle (instance);

				return Value.StringToIntPtr (uri.Scheme);
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.GetScheme ({1}): {0}", ex.Message, FromGCHandle (instance).ToString ());
				} catch {
				}
#endif
				return IntPtr.Zero;
			}
		}

		private static IntPtr GetHost (IntPtr instance)
		{
			try {
				Uri uri;

				if (instance == IntPtr.Zero)
					return IntPtr.Zero;

				uri = FromGCHandle (instance);

				return Value.StringToIntPtr (uri.Host);
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.GetHost ({1}): {0}", ex.Message, FromGCHandle (instance).ToString ());
				} catch {
				}
#endif
				return IntPtr.Zero;
			}
		}

		private static int GetPort (IntPtr instance)
		{
			try {
				Uri uri;

				if (instance == IntPtr.Zero)
					return 0;

				uri = FromGCHandle (instance);

				return uri.Port;
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.GetPort ({1}): {0}", ex.Message, FromGCHandle (instance).ToString ());
				} catch {
				}
#endif
				return 0;
			}
		}

		private static IntPtr GetFragment (IntPtr instance)
		{
			try {
				Uri uri;

				if (instance == IntPtr.Zero)
					return IntPtr.Zero;

				uri = FromGCHandle (instance);

				return Value.StringToIntPtr (uri.Fragment);
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.GetFragment ({1}): {0}", ex.Message, FromGCHandle (instance).ToString ());
				} catch {
				}
#endif
				return IntPtr.Zero;
			}
		}

		private static IntPtr GetPath (IntPtr instance)
		{
			try {
				Uri uri;

				if (instance == IntPtr.Zero)
					return IntPtr.Zero;

				uri = FromGCHandle (instance);

				if (!uri.IsAbsoluteUri) {
					/* Can't get the path of relative uris with the managed Uri class */
					return Value.StringToIntPtr (uri.OriginalString);
				}

				return Value.StringToIntPtr (uri.AbsolutePath);
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.GetPath ({1}): {0}", ex.Message, FromGCHandle (instance).ToString ());
				} catch {
				}
#endif
				return IntPtr.Zero;
			}
		}

		private static IntPtr GetQuery (IntPtr instance)
		{
			try {
				Uri uri;

				if (instance == IntPtr.Zero)
					return IntPtr.Zero;

				uri = FromGCHandle (instance);

				return Value.StringToIntPtr (uri.Query);
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.GetQuery ({1}): {0}", ex.Message, FromGCHandle (instance).ToString ());
				} catch {
				}
#endif
				return IntPtr.Zero;
			}
		}

		private static IntPtr GetOriginalString (IntPtr instance)
		{
			try {
				Uri uri;

				if (instance == IntPtr.Zero)
					return IntPtr.Zero;

				uri = FromGCHandle (instance);

				return Value.StringToIntPtr (uri.OriginalString);
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.GetOriginalString ({1}): {0}", ex.Message, FromGCHandle (instance).ToString ());
				} catch {
				}
#endif
				return IntPtr.Zero;
			}
		}

		private static bool GetIsAbsolute (IntPtr instance)
		{
			try {
				Uri uri;

				if (instance == IntPtr.Zero)
					return false;

				uri = FromGCHandle (instance);

				return uri.IsAbsoluteUri;
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.GetIsAbsolute ({1}): {0}", ex.Message, FromGCHandle (instance).ToString ());
				} catch {
				}
#endif
				return false;
			}
		}

		private static IntPtr ToString (IntPtr instance)
		{
			try {
				Uri uri;

				if (instance == IntPtr.Zero)
					return IntPtr.Zero;

				uri = FromGCHandle (instance);

				return Value.StringToIntPtr (uri.ToString ());
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.ToString ({1}): {0}", ex.Message, FromGCHandle (instance).ToString ());
				} catch {
				}
#endif
				return IntPtr.Zero;
			}
		}

		private static IntPtr GetHttpRequestString (IntPtr instance)
		{
			try {
				Uri uri;

				if (instance == IntPtr.Zero)
					return IntPtr.Zero;

				uri = FromGCHandle (instance);

				return Value.StringToIntPtr (uri.GetComponents (UriComponents.HttpRequestUrl, UriFormat.UriEscaped));
			} catch (Exception ex) {
#if DEBUG
				try {
					Uri u = FromGCHandle (instance);
					Console.WriteLine ("UriHelper.GetHttpRequestString (ToString: '{1}' OriginalString: '{2}'): {0}", ex.Message, u.ToString (), u.OriginalString);
				} catch {
				}
#endif
				return IntPtr.Zero;
			}
		}

		private static bool Equals (IntPtr a, IntPtr b)
		{
			try {
				Uri aa;
				Uri bb;

				if (a == b)
					return true;

				if (a == IntPtr.Zero || b == IntPtr.Zero)
					return false;

				aa = FromGCHandle (a);
				bb = FromGCHandle (b);

				if (aa == null && bb == null)
					return true;
				if (aa == null || bb == null)
					return false;
				return aa.Equals (bb);
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.Equals ({1}, {2}): {0}", ex.Message, FromGCHandle (a).ToString (), FromGCHandle (b).ToString ());
				} catch {
				}
#endif
				return false;
			}
		}

		private static IntPtr Clone (IntPtr instance)
		{
			try {
				Uri uri;

				if (instance == IntPtr.Zero)
					return IntPtr.Zero;

				uri = FromGCHandle (instance);

				// Since Uri is immutable, we just create another gchandle to the same uri instance.
				return ToGCHandlePtr (ToGCHandle (uri));
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.Clone ({1}): {0}", ex.Message, FromGCHandle (instance).ToString ());
				} catch {
				}
#endif
				return IntPtr.Zero;
			}
		}

		private static IntPtr CloneWithScheme (IntPtr instance, string scheme)
		{
			try {
				Uri uri;
				UriBuilder builder;

				if (instance == IntPtr.Zero)
					return IntPtr.Zero;

				uri = FromGCHandle (instance);

				builder = new UriBuilder (uri);
				builder.Scheme = scheme;
				return ToGCHandlePtr (ToGCHandle (builder.Uri));
			} catch (Exception ex) {
#if DEBUG
				try {
					Console.WriteLine ("UriHelper.CloneWithScheme ({1}, {2}): {0}", ex.Message, FromGCHandle (instance).ToString (), scheme);
				} catch {
				}
#endif
				return IntPtr.Zero;
			}
		}

		public static void Initialize (Deployment deployment)
		{
			// Store delegates in static fields to make sure they aren't collected by the gc
			functions.ctor_1 = Ctor1;
			functions.ctor_2 = Ctor2;
			functions.ctor_3 = Ctor3;
			functions.ctor_4 = Ctor4;
			functions.get_scheme = GetScheme;
			functions.get_host = GetHost;
			functions.get_port = GetPort;
			functions.get_fragment = GetFragment;
			functions.get_path = GetPath;
			functions.get_query = GetQuery;
			functions.get_original_string = GetOriginalString;
			functions.get_is_absolute = GetIsAbsolute;
			functions.tostring = ToString;
			functions.equals = Equals;
			functions.clone = Clone;
			functions.clone_with_scheme = CloneWithScheme;
			functions.toescapedstring = GetHttpRequestString;

			NativeMethods.deployment_set_uri_functions (deployment.native, ref functions);
		}

		// Managed helpers
		public static Uri FromNativeUri (IntPtr native_uri)
		{
			if (native_uri == IntPtr.Zero)
				return null;
			return FromGCHandle (NativeMethods.uri_get_gchandle (native_uri));
		}

		public static Uri FromGCHandle (IntPtr gchandle)
		{
			if (gchandle == IntPtr.Zero)
				return new Uri (string.Empty, UriKind.Relative);
			return (Uri) GCHandle.FromIntPtr (gchandle).Target;
		}

		public static IntPtr ToNativeUri (Uri uri)
		{
			if (uri == null)
				return IntPtr.Zero;
			return NativeMethods.uri_new (ToGCHandlePtr (ToGCHandle (uri)));
		}

		public static IntPtr ToNativeUri (GCHandle? handle)
		{
			if (!handle.HasValue)
				return IntPtr.Zero;
			return NativeMethods.uri_new (ToGCHandlePtr (handle));
		}

		public static GCHandle? ToGCHandle (Uri uri)
		{
			if (uri == null)
				return null;
			return GCHandle.Alloc (uri);
		}

		public static IntPtr ToGCHandlePtr (GCHandle? handle)
		{
			if (!handle.HasValue)
				return IntPtr.Zero;
			return GCHandle.ToIntPtr (handle.Value);
		}
	}
}

