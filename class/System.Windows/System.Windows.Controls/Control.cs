//
// System.Windows.Controls.Control
//
// Authors:
//	Sebastien Pouliot  <sebastien@ximian.com>
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
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

using Mono;
using Mono.Xaml;
using System.Windows.Markup;

namespace System.Windows.Controls {

	public abstract class Control : FrameworkElement {

		public Control ()  : base (NativeMethods.control_new ())
		{
		}

		internal Control (IntPtr raw) : base (raw)
		{
		}

		protected FrameworkElement InitializeFromXaml (string xaml)
		{
			Kind kind;
			ManagedXamlLoader loader = new ManagedXamlLoader ();
			loader.LoadDepsSynch = true;
			loader.CreateNativeLoader (null, xaml);
			IntPtr native_child = NativeMethods.control_initialize_from_xaml_callbacks (native, xaml,
											  out kind, loader.NativeLoader);
			loader.FreeNativeLoader ();

			if (native_child == IntPtr.Zero)
				// FIXME: Add detail
				throw new XamlParseException ();

			DependencyObject o = DependencyObject.Lookup (kind, native_child);
			return (FrameworkElement) o;
		}

		internal override Kind GetKind ()
		{
			return Kind.CONTROL;
		}
	}
}
