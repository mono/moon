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
using System.ComponentModel;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Messaging;
using System.Windows.Navigation;
using System.Windows.Printing;
using System.Runtime.InteropServices;

namespace Mono {

	[StructLayout(LayoutKind.Sequential)]
	struct UnmanagedKeyEventArgs {
		public int state;
		public int platformcode;
		public int key;
	}

	internal static partial class Events {

		public static void SafeAction (Action action)
		{
			try {
				action ();
			}
			catch (Exception ex) {
				try {
					Application.OnUnhandledException (Application.Current, ex);
					Console.WriteLine ("Moonlight: Unhandled exception in Events.SafeAction: {0}", ex);
				} catch {
					// Ignore
				}
			}
		}

		public static UnmanagedEventHandler SafeDispatcher (UnmanagedEventHandler handler)
		{
			return (sender, calldata, closure) => SafeAction (() => handler (sender, calldata, closure));
		}

		public static UnmanagedEventHandler CreateCheckAndDownloadUpdateCompletedEventHandlerDispatcher (object sender, CheckAndDownloadUpdateCompletedEventHandler handler)
		{
			return SafeDispatcher ( (_sender, calldata, closure) 
						=> handler (sender, new CheckAndDownloadUpdateCompletedEventArgs (calldata)) );
		}
		
		public static UnmanagedEventHandler CreateLogReadyRoutedEventHandlerDispatcher (LogReadyRoutedEventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure) 
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    NativeDependencyObjectHelper.FromIntPtr (calldata) as LogReadyRoutedEventArgs ?? new LogReadyRoutedEventArgs (calldata)) );
		}
		
		public static UnmanagedEventHandler CreateTextCompositionEventHandlerDispatcher (TextCompositionEventHandler handler)
		{
			return SafeDispatcher( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
								NativeDependencyObjectHelper.FromIntPtr (calldata) as TextCompositionEventArgs ?? new TextCompositionEventArgs (calldata, false)) );
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
						     NativeDependencyObjectHelper.FromIntPtr (calldata) as RoutedEventArgs ?? new RoutedEventArgs (calldata, false) ); } );
		}

		public static UnmanagedEventHandler CreateKeyEventHandlerDispatcher (KeyEventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    NativeDependencyObjectHelper.FromIntPtr (calldata) as KeyEventArgs ?? new KeyEventArgs (calldata)) );
		}

		public static UnmanagedEventHandler CreateNotifyEventHandlerDispatcher (NotifyEventHandler handler)
		{
			return SafeDispatcher( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
								new NotifyEventArgs ()) );
		}

		public static UnmanagedEventHandler CreateNotifyEventArgsEventHandlerDispatcher (EventHandler<NotifyEventArgs> handler)
		{
			return SafeDispatcher( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
								new NotifyEventArgs ()) );
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

		public static UnmanagedEventHandler CreateDragEventHandlerDispatcher (DragEventHandler handler)
		{
			return SafeDispatcher( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
								NativeDependencyObjectHelper.FromIntPtr (calldata) as DragEventArgs ?? new DragEventArgs (calldata, false)) );
		}

		public static UnmanagedEventHandler CreateEventArgsEventHandlerDispatcher (EventHandler<EventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
								new EventArgs ()) );
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

		public static UnmanagedEventHandler CreateContentChangedEventHandlerDispatcher (ContentChangedEventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    NativeDependencyObjectHelper.FromIntPtr (calldata) as ContentChangedEventArgs ?? new ContentChangedEventArgs (calldata)) );
		}

		public static UnmanagedEventHandler CreateTextChangedEventHandlerDispatcher (TextChangedEventHandler handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    NativeDependencyObjectHelper.FromIntPtr (calldata) as TextChangedEventArgs ?? new TextChangedEventArgs (calldata)) );
		}

		public static UnmanagedEventHandler CreateMessageReceivedEventArgsEventHandlerDispatcher (EventHandler <MessageReceivedEventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
							    new MessageReceivedEventArgs (calldata, false)) );
		}

		public static UnmanagedEventHandler CreateSampleReadyEventArgsEventHandlerDispatcher (EventHandler <SampleReadyEventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> { object o = NativeDependencyObjectHelper.FromIntPtr (closure);
						     SampleReadyEventArgs args = new SampleReadyEventArgs (calldata);
						     handler (o, args); } );
		}

		public static UnmanagedEventHandler CreateVideoFormatChangedEventArgsEventHandlerDispatcher (EventHandler <VideoFormatChangedEventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> { object o = NativeDependencyObjectHelper.FromIntPtr (closure);
						     VideoFormatChangedEventArgs args = new VideoFormatChangedEventArgs (calldata);
						     handler (o, args); } );
		}

		public static UnmanagedEventHandler CreateLoadCompletedEventHandlerDispatcher (LoadCompletedEventHandler handler)
		{
			return SafeDispatcher( (sender, calldata, closure)
						=> handler (NativeDependencyObjectHelper.FromIntPtr (closure),
								new NavigationEventArgs ()) );
		}

		// avoid having SSC code in anonymous methods since their name can change on a compiler's whim
		private static Exception SendCompletedEventArgsGetError (IntPtr calldata)
		{
			try {
				NativeMethods.send_completed_event_args_get_error (calldata);
				return null;
			}
			catch (Exception e) {
				return e;
			}
		}

		public static UnmanagedEventHandler CreateSendCompletedEventArgsEventHandlerDispatcher (EventHandler <SendCompletedEventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> {
							Exception exc = SendCompletedEventArgsGetError (calldata);

							handler (NativeDependencyObjectHelper.FromIntPtr (closure),
								 (SendCompletedEventArgs)NativeDependencyObjectHelper.Lookup (calldata) ?? new SendCompletedEventArgs (calldata, exc, false));
						} );
		}

		private static Exception CaptureImageCompletedEventArgsGetError (IntPtr calldata)
		{
			try {
				NativeMethods.capture_image_completed_event_args_get_error (calldata);
				return null;
			}
			catch (Exception e) {
				return e;
			}
		}

		public static UnmanagedEventHandler CreateCaptureImageCompletedEventArgsEventHandlerDispatcher (EventHandler <CaptureImageCompletedEventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> {
							Exception exc = CaptureImageCompletedEventArgsGetError (calldata);

							handler (NativeDependencyObjectHelper.FromIntPtr (closure),
								 (CaptureImageCompletedEventArgs)NativeDependencyObjectHelper.Lookup (calldata) ?? new CaptureImageCompletedEventArgs (calldata, exc, false));
						} );
		}

		public static UnmanagedEventHandler CreateEndPrintEventArgsEventHandlerDispatcher (EventHandler <EndPrintEventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler(NativeDependencyObjectHelper.FromIntPtr (closure),
								new EndPrintEventArgs ()) );
		}

		public static UnmanagedEventHandler CreateBeginPrintEventArgsEventHandlerDispatcher (EventHandler <BeginPrintEventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler(NativeDependencyObjectHelper.FromIntPtr (closure),
								new BeginPrintEventArgs ()) );
		}

		public static UnmanagedEventHandler CreatePrintPageEventArgsEventHandlerDispatcher (EventHandler <PrintPageEventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler(NativeDependencyObjectHelper.FromIntPtr (closure),
								new PrintPageEventArgs ()) );
		}

		public static UnmanagedEventHandler CreateManipulationStartedEventArgsEventHandlerDispatcher (EventHandler <ManipulationStartedEventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler(NativeDependencyObjectHelper.FromIntPtr (closure),
								new ManipulationStartedEventArgs ()) );
		}

		public static UnmanagedEventHandler CreateManipulationDeltaEventArgsEventHandlerDispatcher (EventHandler <ManipulationDeltaEventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler(NativeDependencyObjectHelper.FromIntPtr (closure),
								new ManipulationDeltaEventArgs ()) );
		}

		public static UnmanagedEventHandler CreateManipulationCompletedEventArgsEventHandlerDispatcher (EventHandler <ManipulationCompletedEventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler(NativeDependencyObjectHelper.FromIntPtr (closure),
								new ManipulationCompletedEventArgs ()) );
		}

		public static UnmanagedEventHandler CreateClosingEventArgsEventHandlerDispatcher (EventHandler <ClosingEventArgs> handler)
		{
			return SafeDispatcher ( (sender, calldata, closure)
						=> handler(NativeDependencyObjectHelper.FromIntPtr (closure),
								new ClosingEventArgs ()) );
		}

		public static void AddOnEventHandler (DependencyObject obj, int eventId, UnmanagedEventHandler handler)
		{
			NativeMethods.event_object_add_on_event_handler (obj.native, eventId, handler, obj.native, null);
		}

		public static void RemoveOnEventHandler (DependencyObject obj, int eventId, UnmanagedEventHandler handler)
		{
			NativeMethods.event_object_remove_on_event_handler (obj.native, eventId, handler, obj.native);
		}


		public static int AddHandler (INativeEventObjectWrapper obj, int eventId, UnmanagedEventHandler handler)
		{
			return AddHandler (obj.NativeHandle, eventId, handler);
		}

		public static int AddHandler (IntPtr raw, int eventId, UnmanagedEventHandler handler)
		{
			return NativeMethods.event_object_add_handler (raw, eventId, handler, raw, null);
		}

		public static int AddHandler (INativeEventObjectWrapper obj, int eventId, UnmanagedEventHandler handler, Action dtor_action)
		{
			return AddHandler (obj.NativeHandle, eventId, handler, dtor_action);
		}

		public static int AddHandler (IntPtr raw, int eventId, UnmanagedEventHandler handler, Action dtor_action)
		{
			GDestroyNotify call_dtor = (data) => SafeAction ( () => dtor_action () );

			return AddHandler (raw, eventId, handler, call_dtor);
		}

		public static int AddHandler (IntPtr raw, int eventId, UnmanagedEventHandler handler, GDestroyNotify data_dtor)
		{
			return NativeMethods.event_object_add_handler (raw, eventId, handler, raw, data_dtor);
		}

		public static void RemoveHandler (INativeEventObjectWrapper obj, int eventId, UnmanagedEventHandler handler)
		{
			RemoveHandler (obj.NativeHandle, eventId, handler);
		}

		public static void RemoveHandler (IntPtr raw, int eventId, UnmanagedEventHandler handler)
		{
			NativeMethods.event_object_remove_handler (raw, eventId, handler, raw);
		}
	}
}
