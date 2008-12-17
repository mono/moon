//
// BindingExpression.cs
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
using Mono;

namespace System.Windows.Data
{
	class BindingExpression : BindingExpressionBase
	{
//		GetValueCallback gv_callback;
//		SetValueCallback sv_callback;
		
		public BindingExpression()
//			: this (NativeMethods.binding_expression_new ())
		{
//			gv_callback = new GetValueCallback (GetValueOverride);
//			sv_callback = new SetValueCallback (UpdateSourceOverride);
//			NativeMethods.binding_expression_base_register_managed_overrides (Native, gv_callback, sv_callback);
		}

		
//		internal BindingExpression (IntPtr native)
//			: base (native)
//		{
//			
//		}
//
//		Value GetValueOverride ()
//		{
//			Value v;
//			object o;
//			if (base.TryGetValue (out o) && o != null) {
//				v = DependencyObject.GetAsValue (o);
//			} else {
//				v = new Value { k = Kind.INVALID };
//			}
//
//			return v;
//		}
//
//		void UpdateSourceOverride (IntPtr value)
//		{
//			base.SetValue (DependencyObject.ValueToObject (base.Property.PropertyType, value));
//		}
	}
}
