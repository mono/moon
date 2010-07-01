//
// INativeDependencyObjectWrapper.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
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
using System.Windows;
using System.Threading;

namespace Mono {

	static class INativeDependencyObjectWrapperExtensions
	{
		public static object GetValue (this INativeDependencyObjectWrapper native_do, DependencyProperty dp)
		{
			return NativeDependencyObjectHelper.GetValue (native_do, dp);
		}

		public static void SetValue (this INativeDependencyObjectWrapper native_do, DependencyProperty dp, object value)
		{
			NativeDependencyObjectHelper.SetValue (native_do, dp, value);
		}

		public static object ReadLocalValue (this INativeDependencyObjectWrapper native_do, DependencyProperty dp)
		{
			return NativeDependencyObjectHelper.ReadLocalValue (native_do, dp);
		}

		public static object GetAnimationBaseValue (this INativeDependencyObjectWrapper native_do, DependencyProperty dp)
		{
			return NativeDependencyObjectHelper.GetAnimationBaseValue (native_do, dp);
		}

		public static void ClearValue (this INativeDependencyObjectWrapper native_do, DependencyProperty dp)
		{
			NativeDependencyObjectHelper.ClearValue (native_do, dp);
		}

		public static bool CheckAccess (this INativeDependencyObjectWrapper native_do)
		{
			return Thread.CurrentThread == DependencyObject.moonlight_thread;
		}
	}

	interface INativeDependencyObjectWrapper : INativeEventObjectWrapper {

	}
}
