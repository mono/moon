//
// Application.cs
//
// Authors:
//   Miguel de Icaza (miguel@novell.com)
//
// Copyright 2008 Novell, Inc.
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
using System.Windows.Resources;

namespace System.Windows {

	public class Application {
		static Application current;

		DependencyObject root_visual;
		
		public event EventHandler Exit;
		public event StartupEventHandler Startup;
		public event EventHandler<ApplicationUnhandledExceptionEventArgs> UnhandledException;

		public Application ()
		{
		}
		
		//
		// component is used as the target type of the object
		// we are loading, makes no sense to me, sounds like a
		// hack.
		//
		public static void LoadComponent (object component, Uri xamlUri)
		{
			throw new NotImplementedException ();
		}

		public static StreamResourceInfo GetResourceStream (Uri resourceUri)
		{
			//
			// Needs to support:
			//   "pathname"                     resource file embedded in application package
			//   "AssemblyName;component/pathname"   embedded in AssemblyName, the file pathname

			throw new NotImplementedException ();
		}

		public static StreamResourceInfo GetResourceStream (StreamResourceInfo zipPakResourceStreamInfo, Uri resourceUri)
		{
			throw new NotImplementedException ();
		}

		public static Application Current {
			get {
				return current;
			}
		}

		public ResourceDictionary Resources {
			get {
				throw new NotImplementedException ();
			}
		}

		public DependencyObject RootVisual {
			get {
				return root_visual;
			}

			set {
				root_visual = value;
			}
		}
	}
}