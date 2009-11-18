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
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Runtime.InteropServices;

namespace Mono {

	[StructLayout(LayoutKind.Sequential)]
	struct UnmanagedKeyEventArgs {
		public int state;
		public int platformcode;
		public int key;
	}

	internal partial class Events {

		public static void SafeAction (Action action)
		{
			try {
				action ();
			}
			catch (Exception ex) {
				try {
					Application.OnUnhandledException (Application.Current, ex);
#if DEBUG
					if (IsPlugin ())
						ReportException (ex);
					else
#endif
						Console.WriteLine ("Moonlight: Unhandled exception in Events.SafeDispatcher: {0}", ex);
				} catch {
					// Ignore
				}
			}
		}

		public static UnmanagedEventHandler SafeDispatcher (UnmanagedEventHandler handler)
		{
			return (sender, calldata, closure) => SafeAction (() => handler (sender, calldata, closure));
		}
		
		public static UnmanagedEventHandler CreateLogReadyRoutedEventHandlerDispatcher (LogReadyRoutedEventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure) 
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    NativeDependencyObjectHelper.FromIntPtr (calldata) as LogReadyRoutedEventArgs ?? new LogReadyRoutedEventArgs (calldata)) );
		}

		public static UnmanagedEventHandler CreateSizeChangedEventHandlerDispatcher (SizeChangedEventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure) 
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    NativeDependencyObjectHelper.FromIntPtr (calldata) as SizeChangedEventArgs ?? new SizeChangedEventArgs (calldata)) );
		}

		// ignores the sender parameter and passes null along to handler
		public static UnmanagedEventHandler CreateNullSenderEventHandlerDispatcher (EventHandler handler)
		{
			return CreateExplicitSenderEventHandlerDispatcher (null, handler);
		}

		// ignores the sender passed to the delegate and replaces it with the @sender when calling handler
		public static UnmanagedEventHandler CreateExplicitSenderEventHandlerDispatcher (object sender, EventHandler handler)
		{
			return SafeDispatcher ( (_sender, calldata, closure)
						=> handler (sender,
							    EventArgs.Empty) );
		}

		public static UnmanagedEventHandler CreateEventHandlerDispatcher (EventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    EventArgs.Empty) );
		}

		// this one is screwy, since there's no
		// RenderingEventHandler.  it's just an EventHandler
		// that gets passed some RenderingEventArgs.
		public static UnmanagedEventHandler CreateRenderingEventHandlerDispatcher (EventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (null /* sender is a TimeManager, which has no managed peer */,
							    new RenderingEventArgs (calldata)) );
		}
		
		public static UnmanagedEventHandler CreateRoutedEventHandlerDispatcher (RoutedEventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> { object o = NativeDependencyObjectHelper.FromIntPtr (closure);
						     handler (o,
							      NativeDependencyObjectHelper.FromIntPtr (calldata) as RoutedEventArgs ?? new RoutedEventArgs (calldata, false)); } );
		}

		public static UnmanagedEventHandler CreateKeyEventHandlerDispatcher (KeyEventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    NativeDependencyObjectHelper.FromIntPtr (calldata) as KeyEventArgs ?? new KeyEventArgs (calldata)) );
		}

		public static UnmanagedEventHandler CreateMouseEventHandlerDispatcher (MouseEventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseEventArgs ?? new MouseEventArgs (calldata)) );
		}

		public static UnmanagedEventHandler CreateMouseButtonEventHandlerDispatcher (MouseButtonEventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseButtonEventArgs ?? new MouseButtonEventArgs (calldata)) );
		}

		public static UnmanagedEventHandler CreateMouseWheelEventHandlerDispatcher (MouseWheelEventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    NativeDependencyObjectHelper.FromIntPtr (calldata) as MouseWheelEventArgs ?? new MouseWheelEventArgs (calldata)) );
		}

		public static UnmanagedEventHandler CreateTimelineMarkerRoutedEventHandlerDispatcher (TimelineMarkerRoutedEventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    NativeDependencyObjectHelper.FromIntPtr (calldata) as TimelineMarkerRoutedEventArgs ?? new TimelineMarkerRoutedEventArgs (calldata, false)) );
		}

		public static UnmanagedEventHandler CreateExceptionRoutedEventArgsEventHandlerDispatcher (EventHandler <ExceptionRoutedEventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> { object o = NativeDependencyObjectHelper.FromIntPtr (closure);
						     ExceptionRoutedEventArgs args = ExceptionRoutedEventArgs.FromErrorEventArgs (calldata);
						     args.OriginalSource = o;
						     handler (o, args); } );
		}

		public static UnmanagedEventHandler CreateRoutedEventArgsEventHandlerDispatcher (EventHandler <RoutedEventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> { object o = NativeDependencyObjectHelper.FromIntPtr (closure);
						     handler (o,
							      NativeDependencyObjectHelper.FromIntPtr (calldata) as RoutedEventArgs ?? new RoutedEventArgs (calldata, false) { OriginalSource = o }); } );
		}

		public static UnmanagedEventHandler CreateDownloadProgressEventArgsEventHandlerDispatcher (EventHandler <DownloadProgressEventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    new DownloadProgressEventArgs (calldata)) );
		}

		public static UnmanagedEventHandler CreateDependencyPropertyChangedEventHandlerDispatcher (DependencyPropertyChangedEventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    new DependencyPropertyChangedEventArgs (calldata)) );
		}

		public static UnmanagedEventHandler CreateCursorPositionChangedEventHandlerDispatcher (CursorPositionChangedEventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    new CursorPositionChangedEventArgs (calldata)) );
		}

		public static UnmanagedEventHandler CreateTextChangedEventHandlerDispatcher (TextChangedEventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    NativeDependencyObjectHelper.FromIntPtr (calldata) as TextChangedEventArgs ?? new TextChangedEventArgs (calldata)) );
		}

		public static void AddOnEventHandler (DependencyObject obj, int eventId, UnmanagedEventHandler handler)
		{
			NativeMethods.event_object_add_on_event_handler (obj.native, eventId, handler, obj.native, IntPtr.Zero);
		}

		public static void RemoveOnEventHandler (DependencyObject obj, int eventId, UnmanagedEventHandler handler)
		{
			NativeMethods.event_object_remove_on_event_handler (obj.native, eventId, handler, obj.native);
		}

		public static int AddHandler (DependencyObject obj, int eventId, UnmanagedEventHandler handler)
		{
			return AddHandler (obj.native, eventId, handler);
		}

		public static int AddHandler (IntPtr raw, int eventId, UnmanagedEventHandler handler)
		{
			return NativeMethods.event_object_add_handler (raw, eventId, handler, raw, IntPtr.Zero);
		}

		public static void RemoveHandler (DependencyObject obj, int eventId, UnmanagedEventHandler handler)
		{
			RemoveHandler (obj.native, eventId, handler);
		}

		public static void RemoveHandler (IntPtr raw, int eventId, UnmanagedEventHandler handler)
		{
			NativeMethods.event_object_remove_handler (raw, eventId, handler, raw);
		}

		public static bool IsPlugin () {
			return System.Windows.Interop.PluginHost.Handle != IntPtr.Zero;
		}
		
		public static void ReportException (Exception ex) {
			String msg = ex.Message;
			System.Text.StringBuilder sb = new StringBuilder (ex.GetType ().FullName);
			sb.Append (": ").Append (ex.Message);
			String details = sb.ToString ();
			String[] stack_trace = ex.StackTrace.Split (new [] { Environment.NewLine }, StringSplitOptions.None);

			NativeMethods.plugin_instance_report_exception (System.Windows.Interop.PluginHost.Handle, msg, details, stack_trace, stack_trace.Length);
		}
	}
}
