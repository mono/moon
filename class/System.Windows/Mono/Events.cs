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
					try {
						Application.OnUnhandledException (Application.Current, ex);
						if (IsPlugin ())
							ReportException (ex);
						else
							Console.WriteLine ("Moonlight: Unhandled exception in Events.CreateSafeHandler: {0}", ex);
					} catch {
						// Ignore
					}
				}
			};
		}
		
		internal static UnmanagedEventHandler binding_validation_error = CreateSafeHandler (binding_validation_error_callback);
		internal static UnmanagedEventHandler current_state_changing = CreateSafeHandler (current_state_changing_callback);
		internal static UnmanagedEventHandler current_state_changed = CreateSafeHandler (current_state_changed_callback);
		internal static UnmanagedEventHandler mouse_motion = CreateSafeHandler (mouse_motion_notify_callback);
		internal static UnmanagedEventHandler mouse_left_button_down = CreateSafeHandler (mouse_left_button_down_callback);
		internal static UnmanagedEventHandler mouse_left_button_up = CreateSafeHandler (mouse_left_button_up_callback);
#if NET_3_0
		internal static UnmanagedEventHandler mouse_right_button_down = CreateSafeHandler (mouse_right_button_down_callback);
		internal static UnmanagedEventHandler mouse_right_button_up = CreateSafeHandler (mouse_right_button_up_callback);
		internal static UnmanagedEventHandler mouse_wheel = CreateSafeHandler (mouse_wheel_callback);
#endif
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
		internal static UnmanagedEventHandler surface_full_screen_changed = CreateSafeHandler (surface_full_screen_changed_callback);
		internal static UnmanagedEventHandler template_applied = CreateSafeHandler (template_applied_callback);

		static void template_applied_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			Control c = (Control) Helper.ObjectFromIntPtr (closure);
			c.InvokeOnApplyTemplate ();
		}


		static void binding_validation_error_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			// XXX
			throw new NotImplementedException ();
		}

		static void current_state_changing_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			VisualStateGroup e = (VisualStateGroup) Helper.ObjectFromIntPtr (closure);
			e.RaiseCurrentStateChanging (new VisualStateChangedEventArgs (calldata));
		}
		
		static void current_state_changed_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			VisualStateGroup e = (VisualStateGroup) Helper.ObjectFromIntPtr (closure);
			e.RaiseCurrentStateChanged (new VisualStateChangedEventArgs (calldata));
		}
		
		static void got_focus_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement)Helper.ObjectFromIntPtr (closure);
			e.InvokeGotFocus (new RoutedEventArgs (calldata));
		}

		static void lost_focus_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement)Helper.ObjectFromIntPtr (closure);
			e.InvokeLostFocus (new RoutedEventArgs (calldata));
		}

		static void lost_mouse_capture_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.ObjectFromIntPtr (closure);
			e.InvokeLostMouseCapture (new MouseEventArgs (calldata));
		}
		
		static void layout_updated_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			FrameworkElement e = (FrameworkElement)Helper.ObjectFromIntPtr (closure);
			e.InvokeLayoutUpdated ();
		}

		static void loaded_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			FrameworkElement e = (FrameworkElement)Helper.ObjectFromIntPtr (closure);
			e.InvokeLoaded ();
			
			Content.InvokeResize ();
		}

		static void mouse_leave_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement)Helper.ObjectFromIntPtr (closure);
			e.InvokeMouseLeave (new MouseEventArgs (calldata));
		}

		static void key_up_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.ObjectFromIntPtr (closure);
			e.InvokeKeyUp (new KeyEventArgs (calldata));
		}

		static void key_down_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.ObjectFromIntPtr (closure);
			e.InvokeKeyDown (new KeyEventArgs (calldata));
		}

		static void mouse_motion_notify_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.ObjectFromIntPtr (closure);
			e.InvokeMouseMove (new MouseEventArgs (calldata));
		}
		
		static void mouse_left_button_down_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.ObjectFromIntPtr (closure);
			e.InvokeMouseLeftButtonDown (new MouseButtonEventArgs (calldata));
		}
		
		static void mouse_left_button_up_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.ObjectFromIntPtr (closure);
			e.InvokeMouseLeftButtonUp (new MouseButtonEventArgs (calldata));
		}

#if NET_3_0
		static void mouse_right_button_down_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.ObjectFromIntPtr (closure);
			e.InvokeMouseRightButtonDown (new MouseButtonEventArgs (calldata));
		}
		
		static void mouse_right_button_up_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.ObjectFromIntPtr (closure);
			e.InvokeMouseRightButtonUp (new MouseButtonEventArgs (calldata));
		}

		static void mouse_wheel_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.ObjectFromIntPtr (closure);
			e.InvokeMouseWheel (new MouseWheelEventArgs (calldata));
		}
#endif
		
		static void mouse_enter_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			UIElement e = (UIElement) Helper.ObjectFromIntPtr (closure);
			e.InvokeMouseEnter (new MouseEventArgs (calldata));
		}

		static void size_changed_callback (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			FrameworkElement e = (FrameworkElement)Helper.ObjectFromIntPtr (closure);
			e.InvokeSizeChanged (new SizeChangedEventArgs (calldata));
		}

		static void surface_resized_callback (IntPtr target, IntPtr calldata, IntPtr clozure)
		{
			Content.InvokeResize ();
		}
		
		static void surface_full_screen_changed_callback (IntPtr target, IntPtr calldata, IntPtr clozure)
		{
			Content.InvokeFullScreenChange ();
		}

		internal static void InitSurface (IntPtr surface)
		{
			NativeMethods.event_object_add_handler (surface, "Resize", surface_resized, IntPtr.Zero, IntPtr.Zero);
			NativeMethods.event_object_add_handler (surface, "FullScreenChange", surface_full_screen_changed, IntPtr.Zero, IntPtr.Zero);
		}

		internal static void AddHandler (DependencyObject obj, string eventName, UnmanagedEventHandler handler)
		{
			NativeMethods.event_object_add_handler (obj.native, eventName, handler, obj.native, IntPtr.Zero);
		}

		internal static void RemoveHandler (DependencyObject obj, string eventName, UnmanagedEventHandler handler)
		{
			NativeMethods.event_object_remove_handler (obj.native, eventName, handler, obj.native);
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
