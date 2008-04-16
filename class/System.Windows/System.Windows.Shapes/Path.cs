//
// System.Windows.Shapes.Path
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

using System.Windows.Media;
using Mono;

namespace System.Windows.Shapes {

	public sealed class Path : Shape {

		public static readonly DependencyProperty DataProperty = DependencyProperty.Lookup (Kind.PATH, "Data", typeof (Geometry));
		
		public Path () :  base (Mono.NativeMethods.path_new ())
		{
		}

		internal Path (IntPtr raw) : base (raw)
		{
		}

		public Geometry Data {
			get { return (Geometry) GetValue (DataProperty); }
			set { SetValue (DataProperty, value); }
		}
		
		internal override Kind GetKind ()
		{
			return Kind.PATH;
		}
	}
}
