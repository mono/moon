// Author:
//   Rolf Bjarne Kvinge  (RKvinge@novell.com)
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
using System;

namespace System.Windows.Media.Animation 
{
	public sealed class ColorAnimationUsingKeyFrames : ColorAnimation {
		
		public static readonly DependencyProperty KeyFramesProperty = 
			DependencyProperty.Lookup (Kind.COLORANIMATIONUSINGKEYFRAMES, "KeyFrames", typeof (ColorKeyFrameCollection)); 
		
		public ColorAnimationUsingKeyFrames() : base (NativeMethods.color_animation_using_key_frames_new ()) 
		{
		}
	
		internal ColorAnimationUsingKeyFrames (IntPtr raw) : base (raw) 
		{
		}
		
		public ColorKeyFrameCollection KeyFrames { 
			get {
				return (ColorKeyFrameCollection) GetValue (KeyFramesProperty);
			}
			set {
				SetValue (KeyFramesProperty, value);
			}
		}

		internal override Kind GetKind ()
		{
			return Kind.COLORANIMATIONUSINGKEYFRAMES;
		}
	}
}
