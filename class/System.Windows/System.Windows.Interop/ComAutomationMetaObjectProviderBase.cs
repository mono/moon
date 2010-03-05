// 
// ComAutomationMetaObjectProviderBase.cs
// 
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
// 
// Copyright 2010 Novell, Inc.
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

#if notyet // need to update System.Core
#if NET_2_1 // couldn't make it build with the 3.0 profile

using System;
using System.Collections.Generic;
using System.Dynamic;

namespace System.Windows.Interop {
	public abstract class ComAutomationMetaObjectProviderBase : IDynamicMetaObjectProvider {
		internal ComAutomationMetaObjectProviderBase ()
		{
		}

		public virtual bool TryGetMember (GetMemberBinder binder, out object result)
		{
			Console.WriteLine ("System.Windows.Interop.ComAutomationMetaObjectProviderBase.TryGetMember: NIEX");
			throw new NotImplementedException ();
		}

		public virtual bool TrySetMember (SetMemberBinder binder, object value)
		{
			Console.WriteLine ("System.Windows.Interop.ComAutomationMetaObjectProviderBase.TrySetMember: NIEX");
			throw new NotImplementedException ();
		}

		public virtual bool TryInvokeMember (InvokeMemberBinder binder, object [] args, out object result)
		{
			Console.WriteLine ("System.Windows.Interop.ComAutomationMetaObjectProviderBase.TryInvokeMember: NIEX");
			throw new NotImplementedException ();
		}

		public virtual bool TryConvert (ConvertBinder binder, out object result)
		{
			Console.WriteLine ("System.Windows.Interop.ComAutomationMetaObjectProviderBase.TryConvert: NIEX");
			throw new NotImplementedException ();
		}

		public virtual bool TryCreateInstance (CreateInstanceBinder binder, object [] args, out object result)
		{
			Console.WriteLine ("System.Windows.Interop.ComAutomationMetaObjectProviderBase.TryCreateInstance: NIEX");
			throw new NotImplementedException ();
		}

		public virtual bool TryInvoke (InvokeBinder binder, object [] args, out object result)
		{
			Console.WriteLine ("System.Windows.Interop.ComAutomationMetaObjectProviderBase.TryInvoke: NIEX");
			throw new NotImplementedException ();
		}

		public virtual bool TryBinaryOperation (BinaryOperationBinder binder, object arg, out object result)
		{
			Console.WriteLine ("System.Windows.Interop.ComAutomationMetaObjectProviderBase.TryBinaryOperation: NIEX");
			throw new NotImplementedException ();
		}

		public virtual bool TryUnaryOperation (UnaryOperationBinder binder, out object result)
		{
			Console.WriteLine ("System.Windows.Interop.ComAutomationMetaObjectProviderBase.TryUnaryOperation: NIEX");
			throw new NotImplementedException ();
		}

		public virtual bool TryGetIndex (GetIndexBinder binder, object [] indexes, out object result)
		{
			Console.WriteLine ("System.Windows.Interop.ComAutomationMetaObjectProviderBase.TryGetIndex: NIEX");
			throw new NotImplementedException ();
		}

		public virtual bool TrySetIndex (SetIndexBinder binder, object [] indexes, object value)
		{
			Console.WriteLine ("System.Windows.Interop.ComAutomationMetaObjectProviderBase.TrySetIndex: NIEX");
			throw new NotImplementedException ();
		}

		public virtual IEnumerable<string> GetDynamicMemberNames ()
		{
			Console.WriteLine ("System.Windows.Interop.ComAutomationMetaObjectProviderBase.GetDynamicMemberNames: NIEX");
			throw new NotImplementedException ();
		}

		public System.Dynamic.DynamicMetaObject GetMetaObect (System.Linq.Expressions.Expression parameter)
		{
			return ((System.Dynamic.IDynamicMetaObjectProvider) this).GetMetaObject (parameter);
		}
		
		System.Dynamic.DynamicMetaObject System.Dynamic.IDynamicMetaObjectProvider.GetMetaObject(System.Linq.Expressions.Expression parameter)
		{
			Console.WriteLine ("System.Windows.Interop.ComAutomationMetaObjectProviderBase.GetMetaObject: NIEX");
			throw new NotImplementedException ();
		}
	}
}

#endif
#endif