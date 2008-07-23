// Author:
//   Rolf Bjarne Kvinge  (RKvinge@novell.com)
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
using System.Windows;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Runtime.InteropServices;

namespace System.Windows.Controls
{
	public sealed class MediaElement : FrameworkElement {
		
		public static readonly DependencyProperty AttributesProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "Attributes", typeof (MediaAttributeCollection));
		
		public static readonly DependencyProperty AudioStreamCountProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "AudioStreamCount", typeof (int));
		
		public static readonly DependencyProperty AudioStreamIndexProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "AudioStreamIndex", typeof (int));
		
		public static readonly DependencyProperty AutoPlayProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "AutoPlay", typeof (bool));
		
		public static readonly DependencyProperty BalanceProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "Balance", typeof (double));
		
		public static readonly DependencyProperty BufferingProgressProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "BufferingProgress", typeof (double));
		
		public static readonly DependencyProperty BufferingTimeProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "BufferingTime", typeof (TimeSpan));
		
		public static readonly DependencyProperty CanPauseProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "CanPause", typeof (bool));
		
		public static readonly DependencyProperty CanSeekProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "CanSeek", typeof (bool));
		
		public static readonly DependencyProperty CurrentStateProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "CurrentState", typeof (string));
		
		public static readonly DependencyProperty DownloadProgressProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "DownloadProgress", typeof (double));
		
		public static readonly DependencyProperty IsMutedProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "IsMuted", typeof (bool));
		
		public static readonly DependencyProperty MarkersProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "Markers", typeof (TimelineMarkerCollection));
		
		public static readonly DependencyProperty NaturalDurationProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "NaturalDuration", typeof (Duration));
		
		public static readonly DependencyProperty NaturalVideoHeightProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "NaturalVideoHeight", typeof (double));
		
		public static readonly DependencyProperty NaturalVideoWidthProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "NaturalVideoWidth", typeof (double));
		
		public static readonly DependencyProperty PositionProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "Position", typeof (TimeSpan));
		
		public static readonly DependencyProperty SourceProperty =
			DependencyProperty.Lookup (Kind.MEDIABASE, "Source", typeof (string));
		
		public static readonly DependencyProperty StretchProperty =
			DependencyProperty.Lookup (Kind.MEDIABASE, "Stretch", typeof (Stretch));
		
		public static readonly DependencyProperty VolumeProperty = 
			DependencyProperty.Lookup (Kind.MEDIAELEMENT, "Volume", typeof (double));
		
		
		public MediaElement () : base (NativeMethods.media_element_new ()) 
		{
		}
	
		internal MediaElement (IntPtr raw) : base (raw) 
		{
		}
		
		public void Pause ()
		{
			NativeMethods.media_element_pause (native);
		}
		
		public void Play ()
		{
			NativeMethods.media_element_play (native);
		}
		
		public void SetSource (DependencyObject Downloader, string PartName)
		{
			NativeMethods.media_element_set_source (native, Downloader.native, PartName);
		}
		
		public void Stop ()
		{
			NativeMethods.media_element_stop (native);
		}
		
		public MediaAttributeCollection Attributes { 
			get {
				return (MediaAttributeCollection) GetValue (AttributesProperty);
			}
			set {
				SetValue (AttributesProperty, value);
			}
		}
		
		public int AudioStreamCount {
			get {
				return (int) GetValue (AudioStreamCountProperty);
			}
			set {
				SetValue (AudioStreamCountProperty, value);
			}
		}
		
		public int? AudioStreamIndex {
			get {
				return (int?) GetValue (AudioStreamIndexProperty);
			}
			set {
				SetValue (AudioStreamIndexProperty, value);
			}
		}
		
		public bool AutoPlay { 
			get {
				return (bool) GetValue (AutoPlayProperty);
			}
			set {
				SetValue (AutoPlayProperty, value);
			}
		}
		
		public double Balance { 
			get {
				return (double) GetValue (BalanceProperty); 
			}
			set {
				SetValue (BalanceProperty, value);
			}
		}
		
		public double BufferingProgress {
			get {
				return (double) GetValue (BufferingProgressProperty); 
			}
			set {
				SetValue (BufferingProgressProperty, value);
			}
		}
		
		public TimeSpan BufferingTime { 
			get {
				return (TimeSpan) GetValue (BufferingTimeProperty);
			}
			set {
				SetValue (BufferingTimeProperty, value);
			}
		}
		
		public bool CanPause { 
			get {
				return (bool) GetValue (CanPauseProperty);
			}
		}
		
		public bool CanSeek { 
			get {
				return (bool) GetValue (CanSeekProperty);
			}
		}
		
		public string CurrentState { 
			get {
				return (string) GetValue (CurrentStateProperty);
			}
		}
		
		public double DownloadProgress { 
			get {
				return (double) GetValue (DownloadProgressProperty);
			}
			set {
				SetValue (DownloadProgressProperty, value);
			}
		}
		
		public bool IsMuted { 
			get {
				return (bool) GetValue (IsMutedProperty);
			}
			set {
				SetValue (IsMutedProperty, value);
			}
		}
		
		public TimelineMarkerCollection Markers { 
			get {
				return (TimelineMarkerCollection) GetValue (MarkersProperty);
			}
		}
		
		public Duration NaturalDuration { 
			get {
				return (Duration) GetValue (NaturalDurationProperty);
			}
			set {
				SetValue (NaturalDurationProperty, value);
			}
		}
		
		public int NaturalVideoHeight { 
			get {
				return (int) GetValue (NaturalVideoHeightProperty);
			}
			set {
				SetValue (NaturalVideoHeightProperty, (double) value);
			}
		}
		
		public int NaturalVideoWidth { 
			get {
				return (int) GetValue (NaturalVideoWidthProperty);
			}
			set {
				SetValue (NaturalVideoWidthProperty, (double) value);
			}
		}
		
		public TimeSpan Position { 
			get {
				return (TimeSpan) GetValue (PositionProperty);
			}
			set {
				SetValue (PositionProperty, value);
			}
		}
		
		public Uri Source {
			get {
				// Uri is not a DependencyObject, we save it as a string
				string uri = (string) GetValue (SourceProperty);
				return new Uri (uri, UriKind.RelativeOrAbsolute);
			}
			set {
				string uri = value.OriginalString;
				SetValue (SourceProperty, uri); 
			}
		}
		
		public Stretch Stretch {
			get { return (Stretch) GetValue (StretchProperty); }
			set { SetValue (StretchProperty, value); }
		}
		
		public double Volume { 
			get {
				return (double) GetValue (VolumeProperty);
			}
			set {
				SetValue (VolumeProperty, value);
			}
		}
		
		internal override Kind GetKind ()
		{
			return Kind.MEDIAELEMENT;
		}

		static object BufferingProgressChangedEvent = new object ();
		static object CurrentStateChangedEvent = new object ();
		static object DownloadProgressChangedEvent = new object ();
		static object MarkerReachedEvent = new object ();
		static object MediaOpenedEvent = new object ();
		static object MediaEndedEvent = new object ();
		static object MediaFailedEvent = new object ();
		
		public event EventHandler BufferingProgressChanged {
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

		public event EventHandler CurrentStateChanged {
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
		
		public event EventHandler DownloadProgressChanged {
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
		
		public event TimelineMarkerEventHandler MarkerReached {
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

		public event EventHandler MediaOpened {
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

		public event EventHandler MediaEnded {
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
			TimelineMarkerEventHandler h = (TimelineMarkerEventHandler) events[MarkerReachedEvent];
			
			if (h == null)
				return;
			
			TimelineMarker marker = new TimelineMarker (calldata);
			
			h (this, new TimelineMarkerEventArgs (marker));
		}
		
		private static void media_opened_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) Helper.GCHandleFromIntPtr (closure).Target).InvokeMediaOpened ();
		}
		
		private void InvokeMediaOpened ()
		{
			EventHandler h = (EventHandler) events[MediaOpenedEvent];
			if (h != null)
				h (this, null);
		}
		
		private static void media_ended_cb (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			((MediaElement) Helper.GCHandleFromIntPtr (closure).Target).InvokeMediaEnded ();
		}
		
		private void InvokeMediaEnded ()
		{
			EventHandler h = (EventHandler) events[MediaEndedEvent];
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
