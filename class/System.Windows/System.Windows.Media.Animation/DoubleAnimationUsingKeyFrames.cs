//
// DoubleAnimation.cs
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
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
	public sealed class DoubleAnimationUsingKeyFrames : DoubleAnimation 
	{
		public static readonly DependencyProperty KeyFramesProperty = 
			   DependencyProperty.Lookup (Kind.DOUBLEANIMATIONUSINGKEYFRAMES, "KeyFrames", typeof (DoubleKeyFrameCollection));

		public DoubleAnimationUsingKeyFrames (): base (Mono.NativeMethods.double_animation_using_key_frames_new ())
		{
		}

		internal DoubleAnimationUsingKeyFrames (IntPtr raw) : base (raw)
		{
		}


		public DoubleKeyFrameCollection KeyFrames {
			get { return (DoubleKeyFrameCollection) GetValue (KeyFramesProperty); }
			set { SetValue (KeyFramesProperty, value); }
		}

		internal override Kind GetKind()
		{
			return Kind.DOUBLEANIMATIONUSINGKEYFRAMES;
		}
	}
}
