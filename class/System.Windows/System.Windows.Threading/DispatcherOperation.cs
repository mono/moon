//
// DispatcherTimer.cs
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
using System.Reflection;

namespace System.Windows.Threading {

	[CLSCompliant (false)]
	public sealed class DispatcherOperation {
		Delegate d;
		object[] args;

		internal DispatcherOperation (Delegate d, object[] args)
		{
			this.d = d;
			this.args = args;
		}

		internal void Invoke ()
		{
			try {
				d.DynamicInvoke (args);
			} catch (TargetInvocationException tie) {
				// the unhandled exception is the inner exception, not the TargetInvocationException
				Application.OnUnhandledException (this, tie.InnerException);
			} catch (Exception ex) {
				Application.OnUnhandledException (this, ex);
			}
		}

		internal string GetDebugString ()
		{
			return (d != null && d.Method != null ? d.Method.ToString () : "null") + " (" + (args != null ? args.ToString () : "null") + ")";
		}
	}
}
