//
// ComAutomationFactory.cs
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

#if NET_2_1 // couldn't make it build with the 3.0 profile

using System;
using System.ComponentModel;

namespace System.Runtime.InteropServices.Automation {
	public static class AutomationFactory {
		public static dynamic CreateObject (string progID)
		{
			Console.WriteLine ("System.Runtime.InteropServices.Automation.AutomationFactory.CreateObject: NIEX");
			throw new NotImplementedException ();
		}

		[EditorBrowsable (EditorBrowsableState.Never)]
		public static T CreateObject<T> ()
		{
			Console.WriteLine ("System.Runtime.InteropServices.Automation.AutomationFactory.CreateObject<T>: NIEX");
			throw new NotImplementedException ();
		}

		public static dynamic GetObject (string progID)
		{
			Console.WriteLine ("System.Runtime.InteropServices.Automation.AutomationFactory.GetObject: NIEX");
			throw new NotImplementedException ();
		}

		public static AutomationEvent GetEvent (object comAutomationObject, string eventName)
		{
			Console.WriteLine ("System.Runtime.InteropServices.Automation.AutomationFactory.GetEvent: NIEX");
			throw new NotImplementedException ();
		}

		public static bool IsAvailable {
			get {
				Console.WriteLine ("System.Runtime.InteropServices.Automation.AutomationFactory.get_IsAvailable: not implemented (returning false);");
				return false;
			}
		}
	}
}

#endif
