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
//

using Mono;
using System;
using System.Collections.Generic;
using System.IO;
using System.Windows;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Runtime.InteropServices;

namespace System.Windows.Controls {

	public sealed partial class MediaElement : FrameworkElement {
		private StreamWrapper wrapper;

		private MediaStreamSource media_stream_source;
		
		public LicenseAcquirer LicenseAcquirer {
			get { throw new NotImplementedException (); }
			set { throw new NotImplementedException (); }
		}
		
		public void Pause ()
		{
			NativeMethods.media_element_pause (native);
		}
		
		public void Play ()
		{
			NativeMethods.media_element_play (native);
		}
		
		public void SetSource (Stream stream)
		{
			if (stream != null) {
				ManagedStreamCallbacks callbacks;
				
				wrapper = new StreamWrapper (stream);
				callbacks = wrapper.GetCallbacks ();
				
				NativeMethods.media_element_set_stream_source (this.native, ref callbacks);
			} else {
				Source = null;
			}
		}
		
		public void SetSource (MediaStreamSource mediaStreamSource)
		{
			if (media_stream_source != null) {
				media_stream_source.CloseMediaInternal ();
				media_stream_source = null;
			}

			if (mediaStreamSource.Closed)
				throw new InvalidOperationException ();
			
			media_stream_source = mediaStreamSource;
			media_stream_source.SetMediaElement (this);
		}
		
		public void Stop ()
		{
			NativeMethods.media_element_stop (native);
		}
		
		static object BufferingProgressChangedEvent = new object ();
		static object CurrentStateChangedEvent = new object ();
		static object DownloadProgressChangedEvent = new object ();
		static object MarkerReachedEvent = new object ();
		static object MediaOpenedEvent = new object ();
		static object MediaEndedEvent = new object ();
		static object MediaFailedEvent = new object ();
		
		public event RoutedEventHandler BufferingProgressChanged {
			add {
				RegisterEvent (BufferingProgressChangedEvent, "BufferingProgressChanged", buffering_progress_changed, value);
			}
			remove {
				UnregisterEvent (BufferingProgressChangedEvent, "BufferingProgressChanged", buffering_progress_changed, value);
			}
		}

		public event RoutedEventHandler CurrentStateChanged {
			add {
				RegisterEvent (CurrentStateChangedEvent, "CurrentStateChanged", current_state_changed, value);
			}
			remove {
				UnregisterEvent (CurrentStateChangedEvent, "CurrentStateChanged", current_state_changed, value);
			}
		}
		
		public event RoutedEventHandler DownloadProgressChanged {
			add {
				RegisterEvent (DownloadProgressChangedEvent, "DownloadProgressChanged", download_progress_changed, value);
			}
			remove {
				UnregisterEvent (DownloadProgressChangedEvent, "DownloadProgressChanged", download_progress_changed, value);;
			}
		}
		
		public event TimelineMarkerRoutedEventHandler MarkerReached {
			add {
				RegisterEvent (MarkerReachedEvent, "MarkerReached", marker_reached, value);
			}
			remove {
				UnregisterEvent (MarkerReachedEvent, "MarkerReached", marker_reached, value);
			}
		}

		public event RoutedEventHandler MediaOpened {
			add {
				RegisterEvent (MediaOpenedEvent, "MediaOpened", media_opened, value);
			}
			remove {
				UnregisterEvent (MediaOpenedEvent, "MediaOpened", media_opened, value);
			}
		}

		public event RoutedEventHandler MediaEnded {
			add {
				RegisterEvent (MediaEndedEvent, "MediaEnded", media_ended, value);
			}
			remove {
				UnregisterEvent (MediaEndedEvent, "MediaEnded", media_ended, value);
			}
		}

		public event EventHandler <ExceptionRoutedEventArgs> MediaFailed {
			add {
				RegisterEvent (MediaFailedEvent, "MediaFailed", media_failed, value);
			}
			remove {
				UnregisterEvent (MediaFailedEvent, "MediaFailed", media_failed, value);
			}
		}


		// the nasty event hookup stuff

		static UnmanagedEventHandler buffering_progress_changed = Events.CreateSafeHandler (buffering_progress_changed_cb);
		static UnmanagedEventHandler current_state_changed = Events.CreateSafeHandler (current_state_changed_cb);
		static UnmanagedEventHandler download_progress_changed = Events.CreateSafeHandler (download_progress_changed_cb);
		static UnmanagedEventHandler marker_reached = Events.CreateSafeHandler (marker_reached_cb);
		static UnmanagedEventHandler media_opened = Events.CreateSafeHandler (media_opened_cb);
		static UnmanagedEventHandler media_ended = Events.CreateSafeHandler (media_ended_cb);
		static UnmanagedEventHandler media_failed = Events.CreateSafeHandler (media_failed_cb);

		private static void buffering_progress_changed_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) NativeDependencyObjectHelper.FromIntPtr (closure)).InvokeBufferingProgressChanged ();
		}
		
		private void InvokeBufferingProgressChanged ()
		{
			RoutedEventHandler h = (RoutedEventHandler) EventList [BufferingProgressChangedEvent];
			if (h != null) {
				RoutedEventArgs args = new RoutedEventArgs ();
				args.OriginalSource = this;
				h (this, args);
			}
		}
		
		private static void current_state_changed_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) NativeDependencyObjectHelper.FromIntPtr (closure)).InvokeCurrentStateChanged ();
		}
		
		private void InvokeCurrentStateChanged ()
		{
			RoutedEventHandler h = (RoutedEventHandler) EventList [CurrentStateChangedEvent];
			if (h != null) {
				RoutedEventArgs args = new RoutedEventArgs ();
				args.OriginalSource = this;
				h (this, args);
			}
		}
		
		private static void download_progress_changed_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) NativeDependencyObjectHelper.FromIntPtr (closure)).InvokeDownloadProgressChanged ();
		}
		
		private void InvokeDownloadProgressChanged ()
		{
			RoutedEventHandler h = (RoutedEventHandler) EventList [DownloadProgressChangedEvent];
			if (h != null) {
				RoutedEventArgs args = new RoutedEventArgs ();
				args.OriginalSource = this;
				h (this, args);
			}
		}

		private static void marker_reached_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) NativeDependencyObjectHelper.FromIntPtr (closure)).InvokeMarkerReached (calldata);
		}
		
		private void InvokeMarkerReached (IntPtr calldata)
		{
			TimelineMarkerRoutedEventHandler h = (TimelineMarkerRoutedEventHandler) EventList [MarkerReachedEvent];
			
			if (h == null)
				return;
			
			IntPtr timeline_marker = NativeMethods.marker_reached_event_args_get_marker (calldata);
			
			TimelineMarker marker = new TimelineMarker (timeline_marker, false);
			
			h (this, new TimelineMarkerRoutedEventArgs (marker));
		}
		
		private static void media_opened_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) NativeDependencyObjectHelper.FromIntPtr (closure)).InvokeMediaOpened ();
		}
		
		private void InvokeMediaOpened ()
		{
			RoutedEventHandler h = (RoutedEventHandler) EventList [MediaOpenedEvent];
			if (h != null) {
				RoutedEventArgs args = new RoutedEventArgs ();
				args.OriginalSource = this;
				h (this, args);
			}
		}
		
		private static void media_ended_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) NativeDependencyObjectHelper.FromIntPtr (closure)).InvokeMediaEnded ();
		}
		
		private void InvokeMediaEnded ()
		{
			RoutedEventHandler h = (RoutedEventHandler) EventList [MediaEndedEvent];
			if (h != null) {
				RoutedEventArgs args = new RoutedEventArgs ();
				args.OriginalSource = this;
				h (this, args);
			}
		}
		
		private static void media_failed_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) NativeDependencyObjectHelper.FromIntPtr (closure)).InvokeMediaFailed (calldata);
		}
		
		private void InvokeMediaFailed (IntPtr calldata)
		{
			EventHandler <ExceptionRoutedEventArgs> h = (EventHandler <ExceptionRoutedEventArgs>) EventList [MediaFailedEvent];
			if (h != null) {
				ExceptionRoutedEventArgs args = new ExceptionRoutedEventArgs (calldata);
				args.OriginalSource = this;
				h (this, args);
			}
		}
	}
}
