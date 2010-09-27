//
// XamlLoaderFactory.cs
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
using System.Reflection;

using Mono;
using System.Windows;
using System.Windows.Interop;



namespace Mono.Xaml {

	internal class XamlLoaderFactory {

		private static bool? use_managed;

		static XamlLoaderFactory ()
		{
			bool force_managed = (Environment.GetEnvironmentVariable ("MOON_USE_MANAGED_XAML_PARSER") != null);
			if (force_managed) {
				UseManaged (true);
			}

			bool dont_force_managed = (Environment.GetEnvironmentVariable ("MOON_DONT_USE_MANAGED_XAML_PARSER") != null);
			if (dont_force_managed) {
				UseManaged (false);
			}
		}

		static void UseManaged (bool? value)
		{
			if (value != use_managed) {
				if (value ?? false)
					Console.WriteLine ("Using managed xaml parser.");
				else
					Console.WriteLine ("Using native xaml parser.");
			}

			use_managed = value;
		}

		public static XamlLoader CreateLoader (Assembly assembly, Uri resourceBase, IntPtr surface, IntPtr plugin)
		{
			if (use_managed == null)
				UseManaged (ShouldUseManagedParser ());

			if (use_managed != null && use_managed == true)
				return new SL4XamlLoader (resourceBase);
			return new ManagedXamlLoader (assembly, resourceBase, surface, plugin);
		}

		public static XamlLoader CreateLoader ()
		{
			return CreateLoader (Deployment.Current.EntryAssembly, null, Deployment.Current.Surface.Native, PluginHost.Handle);
		}

		public static bool? ShouldUseManagedParser ()
		{
			Deployment c = Deployment.Current;

			/* '1.0' is the default value, treat it as no version, since the real version might not be set yet. */
			if (c == null || c.RuntimeVersion == null || c.RuntimeVersion == "1.0")
				return null;

			string [] version_parts = c.RuntimeVersion.Split ('.');
			if (version_parts.Length < 1)
				return null;

			int major = Int32.Parse (version_parts [0]);

			if (major >= 4)
				return true;

			return false;
		}
	}
}

