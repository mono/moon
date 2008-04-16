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

using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Input;
using System.Runtime.InteropServices;
using Mono;

namespace System.Windows.Input
{
	public sealed class StylusInfo : DependencyObject
	{
		public static readonly DependencyProperty DeviceTypeProperty = 
			DependencyProperty.Lookup (Kind.STYLUSINFO, "DeviceType", typeof (TabletDeviceType));
		
		public static readonly DependencyProperty IsInvertedProperty = 
			DependencyProperty.Lookup (Kind.STYLUSINFO, "IsInverted", typeof (bool));
		
		public StylusInfo() : base (NativeMethods.stylus_info_new ())
		{
		}
		
		internal StylusInfo (IntPtr raw) : base (raw)
		{
		}

		public TabletDeviceType DeviceType { 
			get {
				return (TabletDeviceType) GetValue (DeviceTypeProperty);
			}
			set {
				SetValue (DeviceTypeProperty, value);
			}
		}
		
		public bool IsInverted {
			get {
				return (bool) GetValue (IsInvertedProperty);
			}
			set {
				SetValue (IsInvertedProperty, value);
			}
		}

		internal override Kind GetKind ()
		{
			return Kind.STYLUSINFO;
		}
	}
}
