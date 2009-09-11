// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
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

using System;
using System.Threading;
using System.Windows;
using System.Windows.Media;
using System.Windows.Input;
using System.Runtime.InteropServices;
using Mono;

namespace System.Windows.Input
{
	public struct StylusPoint : INativeDependencyObjectWrapper
	{
		// FIXME: we shouldn't implement INativeDependencyObjectWrapper and have toggle ref behaviour,
		// since a struct can't have a destructor, we'll always end up leaking the native object.
		// The right thing to do is probably to just have a managed struct with no native representation.
		internal StylusPoint (IntPtr raw)
		{
			NativeHandle = raw;
		}
		
		public StylusPoint (double x, double y) : this (NativeMethods.stylus_point_new ())
		{
			X = x;
			Y = y;
		}
		
		public float PressureFactor {
			get { return (float) ((INativeDependencyObjectWrapper)this).GetValue (PressureFactorProperty); }
			set { ((INativeDependencyObjectWrapper)this).SetValue (PressureFactorProperty, value); }
		}

		public double X {
			get { return (double) ((INativeDependencyObjectWrapper)this).GetValue (XProperty); }
			set { ((INativeDependencyObjectWrapper)this).SetValue (XProperty, value); }
		}

		public double Y {
			get { return (double) ((INativeDependencyObjectWrapper)this).GetValue (YProperty); }
			set { ((INativeDependencyObjectWrapper)this).SetValue (YProperty, value); }
		}

		private static readonly DependencyProperty PressureFactorProperty =
			DependencyProperty.Lookup (Kind.STYLUSPOINT, "PressureFactor", typeof (float));

		private static readonly DependencyProperty XProperty =
			DependencyProperty.Lookup (Kind.STYLUSPOINT, "X", typeof (double));

		private static readonly DependencyProperty YProperty =
			DependencyProperty.Lookup (Kind.STYLUSPOINT, "Y", typeof (double));

#region "INativeDependencyObjectWrapper interface"
		IntPtr _native;

		internal IntPtr NativeHandle {
			get { return _native; }
			set {
				if (_native != IntPtr.Zero) {
					throw new InvalidOperationException ("Application.native is already set");
				}

				_native = value;

				NativeDependencyObjectHelper.AddNativeMapping (value, this);
			}
		}

		IntPtr INativeEventObjectWrapper.NativeHandle {
			get { return NativeHandle; }
			set { NativeHandle = value; }
		}

		object INativeDependencyObjectWrapper.GetValue (DependencyProperty dp)
		{
			return NativeDependencyObjectHelper.GetValue (this, dp);
		}

		void INativeDependencyObjectWrapper.SetValue (DependencyProperty dp, object value)
		{
			NativeDependencyObjectHelper.SetValue (this, dp, value);
		}

		object INativeDependencyObjectWrapper.GetAnimationBaseValue (DependencyProperty dp)
		{
			return NativeDependencyObjectHelper.GetAnimationBaseValue (this, dp);
		}

		object INativeDependencyObjectWrapper.ReadLocalValue (DependencyProperty dp)
		{
			return NativeDependencyObjectHelper.ReadLocalValue (this, dp);
		}

		void INativeDependencyObjectWrapper.ClearValue (DependencyProperty dp)
		{
			NativeDependencyObjectHelper.ClearValue (this, dp);
		}

		Kind INativeEventObjectWrapper.GetKind ()
		{
			return Kind.STYLUSPOINT;
		}

		bool INativeDependencyObjectWrapper.CheckAccess ()
		{
			return Thread.CurrentThread == DependencyObject.moonlight_thread;
		}
#endregion
	}
}
