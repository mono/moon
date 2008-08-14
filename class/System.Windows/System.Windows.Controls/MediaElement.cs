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
using System.Security;
using System.Windows;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Runtime.InteropServices;

namespace System.Windows.Controls {
	public sealed partial class MediaElement : FrameworkElement {
		// These are defined on MediaBase for both Image and MediaElement.
		// The generator can't expand one DP into two managed ones yet.
		public static readonly DependencyProperty DownloadProgressProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "DownloadProgress", typeof (double));
		
		public static readonly DependencyProperty SourceProperty =
			DependencyProperty.Lookup (Kind.MEDIABASE, "Source", typeof (Uri));
		
		public static readonly DependencyProperty StretchProperty =
			DependencyProperty.Lookup (Kind.MEDIABASE, "Stretch", typeof (Stretch));
				
		public override object GetValue (DependencyProperty dp)
		{
			return base.GetValue (dp);
		}
		
#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void Pause ()
		{
			NativeMethods.media_element_pause (native);
		}
		
#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void Play ()
		{
			NativeMethods.media_element_play (native);
		}
		
#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void SetSource (Stream stream)
		{
			throw new NotImplementedException ();
		}
		
#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void SetSource (MediaStreamSource mediaStreamSource)
		{
			throw new NotImplementedException ();
		}
		
#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void Stop ()
		{
			NativeMethods.media_element_stop (native);
		}
		
		public double DownloadProgress { 
			get {
				return (double) GetValue (DownloadProgressProperty);
			}
		}
		
		public Uri Source {
			get {
				return (Uri) GetValue (SourceProperty);
			}
			set {
				SetValue (SourceProperty, value); 
			}
		}
		
		public Stretch Stretch {
			get { return (Stretch) GetValue (StretchProperty); }
			set { SetValue (StretchProperty, value); }
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
				if (events[BufferingProgressChangedEvent] == null)
					Events.AddHandler (this, "BufferingProgressChanged", buffering_progress_changed);
				events.AddHandler (BufferingProgressChangedEvent, value);
			}
			remove {
				events.RemoveHandler (BufferingProgressChangedEvent, value);
				if (events[BufferingProgressChangedEvent] == null)
					Events.RemoveHandler (this, "BufferingProgressChanged", buffering_progress_changed);
			}
		}

		public event RoutedEventHandler CurrentStateChanged {
			add {
				if (events[CurrentStateChangedEvent] == null)
					Events.AddHandler (this, "CurrentStateChanged", current_state_changed);
				events.AddHandler (CurrentStateChangedEvent, value);
			}
			remove {
				events.RemoveHandler (CurrentStateChangedEvent, value);
				if (events[CurrentStateChangedEvent] == null)
					Events.RemoveHandler (this, "CurrentStateChanged", current_state_changed);
			}
		}
		
		public event RoutedEventHandler DownloadProgressChanged {
			add {
				if (events[DownloadProgressChangedEvent] == null)
					Events.AddHandler (this, "DownloadProgressChanged", download_progress_changed);
				events.AddHandler (DownloadProgressChangedEvent, value);
			}
			remove {
				events.RemoveHandler (DownloadProgressChangedEvent, value);
				if (events[DownloadProgressChangedEvent] == null)
					Events.RemoveHandler (this, "DownloadProgressChanged", download_progress_changed);
			}
		}
		
		public event TimelineMarkerRoutedEventHandler MarkerReached {
			add {
				if (events[MarkerReachedEvent] == null)
					Events.AddHandler (this, "MarkerReached", marker_reached);
				events.AddHandler (MarkerReachedEvent, value);
			}
			remove {
				events.RemoveHandler (MarkerReachedEvent, value);
				if (events[MarkerReachedEvent] == null)
					Events.RemoveHandler (this, "MarkerReached", marker_reached);
			}
		}

		public event RoutedEventHandler MediaOpened {
			add {
				if (events[MediaOpenedEvent] == null)
					Events.AddHandler (this, "MediaOpened", media_opened);
				events.AddHandler (MediaOpenedEvent, value);
			}
			remove {
				events.RemoveHandler (MediaOpenedEvent, value);
				if (events[MediaOpenedEvent] == null)
					Events.RemoveHandler (this, "MediaOpened", media_opened);
			}
		}

		public event RoutedEventHandler MediaEnded {
			add {
				if (events[MediaEndedEvent] == null)
					Events.AddHandler (this, "MediaEnded", media_ended);
				events.AddHandler (MediaEndedEvent, value);
			}
			remove {
				events.RemoveHandler (MediaEndedEvent, value);
				if (events[MediaEndedEvent] == null)
					Events.RemoveHandler (this, "MediaEnded", media_ended);
			}
		}

		public event EventHandler <ExceptionRoutedEventArgs> MediaFailed {
			add {
				if (events[MediaFailedEvent] == null)
					Events.AddHandler (this, "MediaFailed", media_failed);
				events.AddHandler (MediaFailedEvent, value);
			}
			remove {
				events.RemoveHandler (MediaFailedEvent, value);
				if (events[MediaFailedEvent] == null)
					Events.RemoveHandler (this, "MediaFailed", media_failed);
			}
		}


		// the nasty event hookup stuff

		static UnmanagedEventHandler buffering_progress_changed = new UnmanagedEventHandler (buffering_progress_changed_cb);
		static UnmanagedEventHandler current_state_changed = new UnmanagedEventHandler (current_state_changed_cb);
		static UnmanagedEventHandler download_progress_changed = new UnmanagedEventHandler (download_progress_changed_cb);
		static UnmanagedEventHandler marker_reached = new UnmanagedEventHandler (marker_reached_cb);
		static UnmanagedEventHandler media_opened = new UnmanagedEventHandler (media_opened_cb);
		static UnmanagedEventHandler media_ended = new UnmanagedEventHandler (media_ended_cb);
		static UnmanagedEventHandler media_failed = new UnmanagedEventHandler (media_failed_cb);

		private static void buffering_progress_changed_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) Helper.GCHandleFromIntPtr (closure).Target).InvokeBufferingProgressChanged ();
		}
		
		private void InvokeBufferingProgressChanged ()
		{
			EventHandler h = (EventHandler) events[BufferingProgressChangedEvent];
			if (h != null)
				h (this, EventArgs.Empty);
		}
		
		private static void current_state_changed_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) Helper.GCHandleFromIntPtr (closure).Target).InvokeCurrentStateChanged ();
		}
		
		private void InvokeCurrentStateChanged ()
		{
			EventHandler h = (EventHandler) events[CurrentStateChangedEvent];
			if (h != null)
				h (this, EventArgs.Empty);
		}
		
		private static void download_progress_changed_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) Helper.GCHandleFromIntPtr (closure).Target).InvokeDownloadProgressChanged ();
		}
		
		private void InvokeDownloadProgressChanged ()
		{
			EventHandler h = (EventHandler) events[DownloadProgressChangedEvent];
			if (h != null)
				h (this, EventArgs.Empty);
		}

		private static void marker_reached_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) Helper.GCHandleFromIntPtr (closure).Target).InvokeMarkerReached (calldata);
		}
		
		private void InvokeMarkerReached (IntPtr calldata)
		{
			TimelineMarkerRoutedEventHandler h = (TimelineMarkerRoutedEventHandler) events[MarkerReachedEvent];
			
			if (h == null)
				return;
			
			TimelineMarker marker = new TimelineMarker (calldata);
			
			h (this, new TimelineMarkerRoutedEventArgs (marker));
		}
		
		private static void media_opened_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) Helper.GCHandleFromIntPtr (closure).Target).InvokeMediaOpened ();
		}
		
		private void InvokeMediaOpened ()
		{
			RoutedEventHandler h = (RoutedEventHandler) events[MediaOpenedEvent];
			if (h != null)
				h (this, null);
		}
		
		private static void media_ended_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) Helper.GCHandleFromIntPtr (closure).Target).InvokeMediaEnded ();
		}
		
		private void InvokeMediaEnded ()
		{
			RoutedEventHandler h = (RoutedEventHandler) events[MediaEndedEvent];
			if (h != null)
				h (this, null);
		}
		
		private static void media_failed_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) Helper.GCHandleFromIntPtr (closure).Target).InvokeMediaFailed ();
		}
		
		private void InvokeMediaFailed ()
		{
			EventHandler h = (EventHandler) events[MediaFailedEvent];
			if (h != null)
				h (this, null);
		}
	}
}
