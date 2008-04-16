//
// DoubleAnimation.cs
//
// Author:
//   Alan McGovern (amcgovern@novell.com)
//
// Copyright 2007 Novell, Inc.
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
namespace System.Windows.Media.Animation 
{
	public class DoubleAnimation : System.Windows.Media.Animation.Animation 
	{
		public static readonly DependencyProperty ByProperty = 
			   DependencyProperty.Lookup (Kind.DOUBLEANIMATION, "By", typeof (Nullable <double>));
		public static readonly DependencyProperty FromProperty =
			   DependencyProperty.Lookup (Kind.DOUBLEANIMATION, "From", typeof (Nullable <double>));
		public static readonly DependencyProperty ToProperty =
			   DependencyProperty.Lookup (Kind.DOUBLEANIMATION, "To", typeof (Nullable <double>));


		public DoubleAnimation(): base (Mono.NativeMethods.double_animation_new ())
		{
		}

		internal DoubleAnimation (IntPtr raw) : base (raw)
		{
		}


		public Nullable<double> By {
			get { return (Nullable<double>) GetValue(ByProperty); }
			set { SetValue(ByProperty, value); }
		}

		public Nullable<double> From {
			get { return (Nullable<double>) GetValue (FromProperty); }
			set { SetValue (FromProperty, value); }
		}

		public Nullable<double> To {
			get { return (Nullable<double>) GetValue (ToProperty); }
			set { SetValue (ToProperty, value); }
		}

		internal override Kind GetKind()
		{
			return Kind.DOUBLEANIMATION;
		}
	}
}
