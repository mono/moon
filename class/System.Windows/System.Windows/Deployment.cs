//
// Deployment.cs
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

using Mono;
using System.Reflection;
using System.Security;

namespace System.Windows {

	public sealed partial class Deployment : DependencyObject {

		public static Deployment Current {
			[SecuritySafeCritical]
			get {
				IntPtr dep = NativeMethods.deployment_get_current ();
				return NativeDependencyObjectHelper.Lookup (Kind.DEPLOYMENT, dep) as Deployment;
			}

			[SecuritySafeCritical]
			internal set {
				NativeMethods.deployment_set_current (value == null ? IntPtr.Zero : value.native);
			}
		}
	
		[SecurityCritical]
		public static void RegisterAssembly (Assembly assembly)
		{
			throw new System.NotImplementedException ();
		}
		
		[SecurityCritical]
		public static void SetCurrentApplication (Application application)
		{
			NativeMethods.deployment_set_current_application (Current.native, application.NativeHandle);
			throw new System.NotImplementedException ();
		}

		Types types;
		internal Types Types {
			get {
				if (types == null)
					types = new Types (NativeMethods.deployment_get_types (native));
				return types;
			}
		}
	}
}
