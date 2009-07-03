// 
// Runtime.cs
// 
// Author:
//   Aaron Bockover <abockover@novell.com>
// 
// Copyright 2009 Aaron Bockover
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
using System.Windows;
using Mono;

namespace Moonlight.Gtk
{
	/// <summary>
	///    The Moonlight Runtime Engine
	/// </summary>
	/// <remarks>
	///    This class is used to access global functionality 
	///    in the Moonlight runtime engine
	/// </remarks>
	public static class MoonlightRuntime
	{
		static MoonlightRuntime ()
			{
				NativeMethods.runtime_init_desktop ();
				NativeMethods.downloader_set_functions (
					ManagedDownloader.CreateDownloader,
					ManagedDownloader.DestroyDownloader,
					ManagedDownloader.Open,
					ManagedDownloader.Send,
					ManagedDownloader.Abort,
					ManagedDownloader.Header,
					ManagedDownloader.Body,
					ManagedDownloader.CreateWebRequest,
					null,
					null);
            
				DependencyObject.Initialize ();
			}

		/// <summary>
		///    Initializes the Moonlight engine; this must be called
		///    before any calls are done to System.Windows.
		/// </summary>
		/// <remarks>
		///    <para>The System.Windows namespace for Silverlight
		///    requires a downloader engine to be registered before it
		///    can be used to satisfy assembly dependencies and
		///    images.  If your application will for some reason call
		///    into System.Windows before they create an instance of
		///    Moonlight.Gtk.Surface, they should call this method to ensure that
		///    the proper downloader has been registered with the
		///    Moonlight runtime.</para>
		///
		///    <para>Failure to call this method typically results in
		///    errors from the XAML parsing code when it tries to
		///    resolve assembly references, external classes or
		///    loading of external media.</para>
		/// </remarks>
		public static void Init ()
			{
				// Stub to trigger the static ctor
			}
	}
}
