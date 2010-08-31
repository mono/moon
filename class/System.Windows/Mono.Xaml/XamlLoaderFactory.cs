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

		private static bool use_managed;

		static XamlLoaderFactory ()
		{
			use_managed = (Environment.GetEnvironmentVariable ("MOON_USE_MANAGED_XAML_PARSER") != null);

			if (use_managed)
				Console.WriteLine ("using managed xaml parser.");
		}

		public static XamlLoader CreateLoader (Assembly assembly, Uri resourceBase, IntPtr surface, IntPtr plugin)
		{
			if (use_managed)
				return new SL4XamlLoader ();
			return new ManagedXamlLoader (assembly, resourceBase, surface, plugin);
		}

		public static XamlLoader CreateLoader ()
		{
			return CreateLoader (Deployment.Current.EntryAssembly, null, Deployment.Current.Surface.Native, PluginHost.Handle);
		}
	}
}

