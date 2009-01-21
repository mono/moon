//
// Events.cs
//
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

using System;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interop;
using System.Runtime.InteropServices;

namespace Mono {

	[StructLayout(LayoutKind.Sequential)]
	struct UnmanagedKeyEventArgs {
		public int state;
		public int platformcode;
		public int key;
	}

	internal class Events {
		
		internal static UnmanagedEventHandler CreateSafeHandler (UnmanagedEventHandler handler)
		{
			return delegate (IntPtr a, IntPtr b, IntPtr c) {
				try {
					handler (a, b, c);
				} catch (Exception ex) {
					if (IsPlugin ())
						ReportException (ex);
					else
						throw;
				}
			};
		}
		
		internal static UnmanagedEventHandler binding_validation_error = CreateSafeHandler (binding_validation_error_callback);
		internal static UnmanagedEventHandler current_state_changing = CreateSafeHandler (current_state_changing_callback);
		internal static UnmanagedEventHandler current_state_changed = CreateSafeHandler (current_state_changed_callback);
		internal static UnmanagedEventHandler mouse_motion = CreateSafeHandler (mouse_motion_notify_callback);
		internal static UnmanagedEventHandler mouse_button_down = CreateSafeHandler (mouse_button_down_callback);
		internal static UnmanagedEventHandler mouse_button_up = CreateSafeHandler (mouse_button_up_callback);
		internal static UnmanagedEventHandler mouse_enter = CreateSafeHandler (mouse_enter_callback);
		internal static UnmanagedEventHandler key_down = CreateSafeHandler (key_down_callback);
		internal static UnmanagedEventHandler key_up = CreateSafeHandler (key_up_callback);
		internal static UnmanagedEventHandler got_focus = CreateSafeHandler (got_focus_callback);
		internal static UnmanagedEventHandler lost_focus = CreateSafeHandler (lost_focus_callback);
		internal static UnmanagedEventHandler lost_mouse_capture = CreateSafeHandler (lost_mouse_capture_callback);
		internal static UnmanagedEventHandler layout_updated = CreateSafeHandler (layout_updated_callback);
		internal static UnmanagedEventHandler loaded = CreateSafeHandler (loaded_callback);
		internal static UnmanagedEventHandler mouse_leave = CreateSafeHandler (mouse_leave_callback);
		internal static UnmanagedEventHandler size_changed = CreateSafeHandler (size_changed_callback);
		internal static UnmanagedEventHandler surface_resized = CreateSafeHandler (surface_resized_callback);
		internal static UnmanagedEventHandler template_applied = CreateSafeHandler (template_applied_callback);

		static void template_applied_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			Control c = (Control) Helper.GCHandleFromIntPtr (closure).Target;
			c.OnApplyTemplate ();
		}


		static void binding_validation_error_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			// XXX
			throw new NotImplementedException ();
		}

		static void current_state_changing_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			VisualStateGroup e = (VisualStateGroup) Helper.GCHandleFromIntPtr (closure).Target;
			e.RaiseCurrentStateChanging (new VisualStateChangedEventArgs (calldata));
		}
		
		static void current_state_changed_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			VisualStateGroup e = (VisualStateGroup) Helper.GCHandleFromIntPtr (closure).Target;
			e.RaiseCurrentStateChanged (new VisualStateChangedEventArgs (calldata));
		}
		
		static void got_focus_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement)Helper.GCHandleFromIntPtr (closure).Target;
			e.InvokeGotFocus (new RoutedEventArgs (calldata));
		}

		static void lost_focus_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement)Helper.GCHandleFromIntPtr (closure).Target;
			e.InvokeLostFocus (new RoutedEventArgs (calldata));
		}

		static void lost_mouse_capture_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.GCHandleFromIntPtr (closure).Target;
			e.InvokeLostMouseCapture (new MouseEventArgs (calldata));
		}
		
		static void layout_updated_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			FrameworkElement e = (FrameworkElement)Helper.GCHandleFromIntPtr (closure).Target;
			e.InvokeLayoutUpdated ();
		}

		static void loaded_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			FrameworkElement e = (FrameworkElement)Helper.GCHandleFromIntPtr (closure).Target;
			e.InvokeLoaded ();
			
			//FIXME: BrowserHost is now replaced by SilverlightHost.Content
			BrowserHost.InvokeResize ();
		}

		static void mouse_leave_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement)Helper.GCHandleFromIntPtr (closure).Target;
			e.InvokeMouseLeave (new MouseEventArgs (calldata));
		}

		static void key_up_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.GCHandleFromIntPtr (closure).Target;
			e.InvokeKeyUp (new KeyEventArgs (calldata));
		}

		static void key_down_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.GCHandleFromIntPtr (closure).Target;
			e.InvokeKeyDown (new KeyEventArgs (calldata));
		}

		static void mouse_motion_notify_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.GCHandleFromIntPtr (closure).Target;
			e.InvokeMouseMove (new MouseEventArgs (calldata));
		}
		
		static void mouse_button_down_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.GCHandleFromIntPtr (closure).Target;
			e.InvokeMouseButtonDown (new MouseButtonEventArgs (calldata));
		}
		
		static void mouse_button_up_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.GCHandleFromIntPtr (closure).Target;
			e.InvokeMouseButtonUp (new MouseButtonEventArgs (calldata));
		}
		
		static void mouse_enter_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.GCHandleFromIntPtr (closure).Target;
			e.InvokeMouseEnter (new MouseEventArgs (calldata));
		}

		static void size_changed_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			FrameworkElement e = (FrameworkElement)Helper.GCHandleFromIntPtr (closure).Target;
			e.InvokeSizeChanged (new SizeChangedEventArgs (calldata));
		}

		static void surface_resized_callback (IntPtr target, IntPtr calldata, IntPtr clozure)
		{
			// Parameter ignored
			//FIXME: BrowserHost is now replaced by SilverlightHost.Content
			BrowserHost.InvokeResize ();
		}

		internal static void InitSurface (IntPtr surface)
		{
			NativeMethods.event_object_add_handler (surface, "Resize", surface_resized, IntPtr.Zero, IntPtr.Zero);
		}

		internal static void AddHandler (DependencyObject obj, string eventName, UnmanagedEventHandler handler)
		{
			NativeMethods.event_object_add_handler (obj.native, eventName, handler, (IntPtr) obj.GCHandle, IntPtr.Zero);
		}

		internal static void RemoveHandler (DependencyObject obj, string eventName, UnmanagedEventHandler handler)
		{
			NativeMethods.event_object_remove_handler (obj.native, eventName, handler, (IntPtr) obj.GCHandle);
		}

		internal static bool IsPlugin () {
			return System.Windows.Interop.PluginHost.Handle != IntPtr.Zero;
		}
		
		internal static void ReportException (Exception ex) {
			String msg = ex.Message;
			System.Text.StringBuilder sb = new StringBuilder (ex.GetType ().FullName);
			sb.Append (": ").Append (ex.Message);
			String details = sb.ToString ();
			String[] stack_trace = ex.StackTrace.Split (new [] { Environment.NewLine }, StringSplitOptions.None);

			NativeMethods.plugin_instance_report_exception (System.Windows.Interop.PluginHost.Handle, msg, details, stack_trace, stack_trace.Length);
		}
	}
}
