//
// Events.cs
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
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

using System;
using System.Text;
using System.Windows;
using System.Windows.Input;
using System.Windows.Interop;
using System.Runtime.InteropServices;

namespace Mono {

	[StructLayout(LayoutKind.Sequential)]
	struct UnmanagedKeyboardEventArgs {
		public int state;
		public int platformcode;
		public int key;
	}

	internal class Events {
		internal static UnmanagedEventHandler mouse_motion = new UnmanagedEventHandler (mouse_motion_notify_callback);
		internal static UnmanagedEventHandler mouse_button_down = new UnmanagedEventHandler (mouse_button_down_callback);
		internal static UnmanagedEventHandler mouse_button_up = new UnmanagedEventHandler (mouse_button_up_callback);
		internal static UnmanagedEventHandler mouse_enter = new UnmanagedEventHandler (mouse_enter_callback);
		internal static UnmanagedEventHandler key_down = new UnmanagedEventHandler (key_down_callback);
		internal static UnmanagedEventHandler key_up = new UnmanagedEventHandler (key_up_callback);
		internal static UnmanagedEventHandler got_focus = new UnmanagedEventHandler (got_focus_callback);
		internal static UnmanagedEventHandler lost_focus = new UnmanagedEventHandler (lost_focus_callback);
		internal static UnmanagedEventHandler loaded = new UnmanagedEventHandler (loaded_callback);
		internal static UnmanagedEventHandler mouse_leave = new UnmanagedEventHandler (mouse_leave_callback);
		internal static UnmanagedEventHandler surface_resized = new UnmanagedEventHandler (surface_resized_callback);

		static void got_focus_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			try {
				UIElement e = (UIElement)Helper.GCHandleFromIntPtr (closure).Target;
				e.InvokeGotFocus ();
			}
			catch (Exception ex) {
				if (IsPlugin ())
					ReportException (ex);
				else
					throw;
			}
		}

		static void lost_focus_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			try {
				UIElement e = (UIElement)Helper.GCHandleFromIntPtr (closure).Target;
				e.InvokeLostFocus ();
			}
			catch (Exception ex) {
				if (IsPlugin ())
					ReportException (ex);
				else
					throw;
			}
		}

		static void loaded_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			try {
				UIElement e = (UIElement)Helper.GCHandleFromIntPtr (closure).Target;
				e.InvokeLoaded ();
				
				BrowserHost.InvokeResize ();
			}
			catch (Exception ex) {
				if (IsPlugin ())
					ReportException (ex);
				else
					throw;
			}
		}

		static void mouse_leave_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			try {
				UIElement e = (UIElement)Helper.GCHandleFromIntPtr (closure).Target;
				e.InvokeMouseLeave ();
			}
			catch (Exception ex) {
				if (IsPlugin ())
					ReportException (ex);
				else
					throw;
			}
		}

		static KeyboardEventArgs MarshalKeyboardEventArgs (IntPtr calldata)
		{
			UnmanagedKeyboardEventArgs args =
				(UnmanagedKeyboardEventArgs)Marshal.PtrToStructure (calldata,
										    typeof (UnmanagedKeyboardEventArgs));

			return new KeyboardEventArgs ((args.state & 4) != 0, (args.state & 1) != 0, args.key, args.platformcode);
		}

		static void key_up_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			try {
				UIElement e = (UIElement) Helper.GCHandleFromIntPtr (closure).Target;
				e.InvokeKeyUp (MarshalKeyboardEventArgs (calldata));
			}
			catch (Exception ex) {
				if (IsPlugin ())
					ReportException (ex);
				else
					throw;
			}
		}

		static void key_down_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			try {
				UIElement e = (UIElement) Helper.GCHandleFromIntPtr (closure).Target;
				e.InvokeKeyDown (MarshalKeyboardEventArgs (calldata));
			}
			catch (Exception ex) {
				if (IsPlugin ())
					ReportException (ex);
				else
					throw;
			}
		}

		static void mouse_motion_notify_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			try {
				UIElement e = (UIElement) Helper.GCHandleFromIntPtr (closure).Target;
				e.InvokeMouseMove (new MouseEventArgs (calldata));
			}
			catch (Exception ex) {
				if (IsPlugin ())
					ReportException (ex);
				else
					throw;
			}
		}
		
		static void mouse_button_down_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			try {
				UIElement e = (UIElement) Helper.GCHandleFromIntPtr (closure).Target;
				e.InvokeMouseButtonDown (new MouseEventArgs (calldata));
			}
			catch (Exception ex) {
				if (IsPlugin ())
					ReportException (ex);
				else
					throw;
			}
		}
		
		static void mouse_button_up_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			try {
				UIElement e = (UIElement) Helper.GCHandleFromIntPtr (closure).Target;
				e.InvokeMouseButtonUp (new MouseEventArgs (calldata));
			}
			catch (Exception ex) {
				if (IsPlugin ())
					ReportException (ex);
				else
					throw;
			}
		}
		
		static void mouse_enter_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			try {
				UIElement e = (UIElement) Helper.GCHandleFromIntPtr (closure).Target;
				e.InvokeMouseEnter (new MouseEventArgs (calldata));
			}
			catch (Exception ex) {
				if (IsPlugin ())
					ReportException (ex);
				else
					throw;
			}
		}

		static void surface_resized_callback (IntPtr target, IntPtr calldata, IntPtr clozure)
		{
			try {
				// Parameter ignored
				BrowserHost.InvokeResize ();
			}
			catch (Exception ex) {
				if (IsPlugin ())
					ReportException (ex);
				else
					throw;
			}
		}

		internal static void InitSurface (IntPtr surface)
		{
			// We don't really need a closure for this event
			NativeMethods.event_object_add_event_handler (surface, "Resize", surface_resized, (IntPtr) new GCHandle ());
		}

		internal static void AddHandler (DependencyObject obj, string eventName, UnmanagedEventHandler handler)
		{
			NativeMethods.event_object_add_event_handler (obj.native, eventName, handler, (IntPtr) obj.GCHandle);
		}

		internal static void RemoveHandler (DependencyObject obj, string eventName, UnmanagedEventHandler handler)
		{
			NativeMethods.event_object_remove_event_handler (obj.native, eventName, handler, (IntPtr) obj.GCHandle);
		}

		internal static bool IsPlugin () {
			return System.Windows.Interop.PluginHost.Handle != IntPtr.Zero;
		}
		
		internal static void ReportException (Exception ex) {
			String msg = ex.Message;
			System.Text.StringBuilder sb = new StringBuilder (ex.GetType ().FullName);
			sb.Append (": ").Append (ex.Message);
			String details = sb.ToString ();
			String[] stack_trace = Helper.Split (ex.StackTrace, new String [] { Environment.NewLine });

			NativeMethods.plugin_instance_report_exception (System.Windows.Interop.PluginHost.Handle, msg, details, stack_trace, stack_trace.Length);
		}
	}
}
