//
// A11yHelper.cs
//
// Contact:
//   Mono-A11Y List (mono-a11y@forge.novell.com)
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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
using System.IO;
using System.Reflection;

namespace Mono {

	internal static class A11yHelper {
		
		private const string BRIDGE_ASM_NAME = "MoonAtkBridge.dll";
		private const string BRIDGE_FULL_NAME = "Moonlight.AtkBridge.AutomationBridge";

		internal static void Initialize ()
		{
			string current_assembly_location = Assembly.GetExecutingAssembly ().Location;
			
			//please keep this lookup pattern in sync with the one in security.c (security_enable_coreclr() method)
			int pos = current_assembly_location.IndexOf ("moonlight@novell");
			if (pos < 0)
				return;

			string load_location = Path.Combine (current_assembly_location.Substring (0, pos), "moonlight-a11y@novell.com");
			if (!Directory.Exists (load_location))
				return;

			load_location = Path.Combine (Path.Combine (load_location, "components"), BRIDGE_ASM_NAME);

			Assembly bridge_asm;
			try {
				bridge_asm = Assembly.LoadFrom (load_location);
			} catch (Exception e) {
				return;
			}

			bridge_type = bridge_asm.GetType (BRIDGE_FULL_NAME);
			if (bridge_type == null) {
				Console.Error.WriteLine (String.Format ("Could not find type {0} in assembly {1}", BRIDGE_FULL_NAME, BRIDGE_ASM_NAME));
				return;
			}

			automation_bridge = bridge_type.GetMethod (
				"CreateAutomationBridge",
				BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.InvokeMethod)
				.Invoke (null, null);

			accessibility_enabled = (bool) bridge_type.GetMethod (
				"IsAccessibilityEnabled",
				BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.InvokeMethod)
				.Invoke (null, null);
		}

		internal static void Shutdown ()
		{
			if (bridge_type == null)
				return;

			var shutdown_mi = bridge_type.GetMethod ("Shutdown",
			        BindingFlags.Instance | BindingFlags.NonPublic
				| BindingFlags.InvokeMethod);
			if (shutdown_mi == null)
				return;

			shutdown_mi.Invoke (automation_bridge, null);
		}
		
		private static Type bridge_type;
		private static object automation_bridge;
		private static bool accessibility_enabled = false;
		private static IntPtr accessible;

		public static bool AccessibilityEnabled {
			get { return accessibility_enabled; }
		}
		
		public static IntPtr GetAccessible ()
		{
			if (!accessibility_enabled || automation_bridge == null)
				return IntPtr.Zero;

			if (accessible == IntPtr.Zero)
				accessible =  (IntPtr) bridge_type.GetMethod ("GetAccessibleHandle",
				                                               BindingFlags.Instance | 
				                                               BindingFlags.NonPublic | 
				                                               BindingFlags.InvokeMethod)
				                                               .Invoke (automation_bridge, null);
			return accessible;
		}
	}
}
