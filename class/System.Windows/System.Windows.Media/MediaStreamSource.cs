//
// MediaStreamSource.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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
//

using Mono;
using System;
using System.Globalization;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;

namespace System.Windows.Media
{
	public abstract class MediaStreamSource
	{
		IntPtr demuxer;
		IntPtr media;
		MediaElement media_element;
		GCHandle handle;
		bool closed;
		bool opened;

		private static CloseDemuxerDelegate closeMediaCallback = CloseMediaInternal;
		private static GetDiagnosticAsyncDelegate getDiagnosticAsyncCallback = GetDiagnosticAsyncInternal;
		private static GetFrameAsyncDelegate getSampleAsyncCallback = GetSampleAsyncInternal;
		private static OpenDemuxerAsyncDelegate openMediaAsyncCallback = OpenMediaAsyncInternal;
		private static SeekAsyncDelegate seekAsyncCallback = SeekAsyncInternal;
		private static SwitchMediaStreamAsyncDelegate switchMediaStreamAsyncCallback = SwitchMediaStreamAsyncInternal;
		
		protected MediaStreamSource ()
		{
			
		}
		
		~MediaStreamSource ()
		{
			if (demuxer != IntPtr.Zero) {
				/* Note that when the appdomain is unloading we may get here
				 * with the gchandle still allocated, so there is no guarantee
				 * that CloseMediaInternal (which also clears the callbacks)
				 * has been called already */
				NativeMethods.external_demuxer_clear_callbacks (demuxer); /* thread-safe */
				NativeMethods.event_object_unref (demuxer); /* thread-safe */
				demuxer = IntPtr.Zero;
			}
			
			if (media != IntPtr.Zero) {
				NativeMethods.event_object_unref (media);
				media = IntPtr.Zero;
			}
		}
		
		internal bool Closed {
			get { return closed; }
		}

		internal bool Opened {
			get { return opened; }
		}
		
		internal void SetMediaElement (MediaElement mediaElement)
		{
			IntPtr demuxer = IntPtr.Zero;
					
			if (this.demuxer != IntPtr.Zero)
				throw new InvalidOperationException ("MediaStreamSource: this source has already been initialized.");
			
			if (handle.IsAllocated)
				throw new InvalidOperationException ("MediaStreamSource: this source has already been initialized.");
			
			media_element = mediaElement;
		
			handle = GCHandle.Alloc (this);
			demuxer = NativeMethods.media_element_set_demuxer_source (media_element.native, GCHandle.ToIntPtr (handle), closeMediaCallback, getDiagnosticAsyncCallback, getSampleAsyncCallback, openMediaAsyncCallback, seekAsyncCallback, switchMediaStreamAsyncCallback);
			
			if (demuxer == IntPtr.Zero)
				throw new InvalidOperationException ("MediaStreamSource: Could not create native demuxer.");
			
			if (this.media == IntPtr.Zero)
				this.media = NativeMethods.imedia_object_get_media_reffed (demuxer);
			this.demuxer = demuxer;
		}

		// private static callback methods
		
		static void CloseMediaInternal (IntPtr instance)
		{
			try {
				GCHandle handle = GCHandle.FromIntPtr (instance);
				((MediaStreamSource) handle.Target).CloseMediaInternal ();
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Unhandled exception in MediaStreamSource.CloseMediaInternal: {0}", ex);
				} catch {
				}
			}
		}

		static void GetDiagnosticAsyncInternal (IntPtr instance, MediaStreamSourceDiagnosticKind diagnosticKind)
		{
			try {
				GCHandle handle = GCHandle.FromIntPtr (instance);
				((MediaStreamSource) handle.Target).GetDiagnosticAsyncInternal (diagnosticKind);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Unhandled exception in MediaStreamSource.GetDiagnosticAsyncInternal: {0}", ex);
				} catch {
				}
			}
		}
		
		static void GetSampleAsyncInternal (IntPtr instance, MediaStreamType mediaStreamType)
		{
			try {
				GCHandle handle = GCHandle.FromIntPtr (instance);
				((MediaStreamSource) handle.Target).GetSampleAsyncInternal (mediaStreamType);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Unhandled exception in MediaStreamSource.GetSampleAsyncInternal: {0}", ex);
				} catch {
				}
			}
		}
		
		static void OpenMediaAsyncInternal (IntPtr instance, IntPtr demuxer)
		{
			try {
				GCHandle handle = GCHandle.FromIntPtr (instance);
				((MediaStreamSource) handle.Target).OpenMediaAsyncInternal (demuxer);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Unhandled exception in MediaStreamSource.OpenMediaAsyncInternal: {0}", ex);
				} catch {
				}
			}
		}
		
		static void SeekAsyncInternal (IntPtr instance, long seekToTime)
		{
			try {
				GCHandle handle = GCHandle.FromIntPtr (instance);
				((MediaStreamSource) handle.Target).SeekAsyncInternal (seekToTime);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Unhandled exception in MediaStreamSource.SeekAsyncInternal: {0}", ex);
				} catch {
				}
			}
		}

		static void SwitchMediaStreamAsyncInternal (IntPtr instance, MediaStreamDescription mediaStreamDescription)
		{
			try {
				GCHandle handle = GCHandle.FromIntPtr (instance);
				((MediaStreamSource) handle.Target).SwitchMediaStreamAsyncInternal (mediaStreamDescription);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("Unhandled exception in MediaStreamSource.SwitchMediaStreamAsyncInternal: {0}", ex);
				} catch {
				}
			}
		}
		
		// internal methods calling the abstract methods
		
		internal void CloseMediaInternal ()
		{
			if (!closed) {
				closed = true;
				CloseMedia ();
				
				if (handle.IsAllocated)
					handle.Free ();
				
				if (demuxer != IntPtr.Zero)
					NativeMethods.external_demuxer_clear_callbacks (demuxer); /* thread-safe */
			}
		}		
		
		internal void GetDiagnosticAsyncInternal (MediaStreamSourceDiagnosticKind diagnosticKind)
		{
			GetDiagnosticAsync (diagnosticKind);
		}

		internal void GetSampleAsyncInternal (MediaStreamType mediaStreamType)
		{
			GetSampleAsync (mediaStreamType);
		}

		internal void OpenMediaAsyncInternal (IntPtr demuxer)
		{
			this.demuxer = demuxer;
			OpenMediaAsync ();
		}

		internal void SeekAsyncInternal (long seekToTime)
		{
			SeekAsync (seekToTime);
		}

		internal void SwitchMediaStreamAsyncInternal (MediaStreamDescription mediaStreamDescription)
		{
			SwitchMediaStreamAsync (mediaStreamDescription);
		}

		// Methods to be called by the derived class
		
		protected void ErrorOccurred (string errorDescription)
		{
			if (closed || media_element == null || demuxer == IntPtr.Zero)
				throw new InvalidOperationException ();
			
			NativeMethods.media_element_report_error_occurred (media_element.native, errorDescription);
		}
		
		protected void ReportGetDiagnosticCompleted (MediaStreamSourceDiagnosticKind diagnosticKind, long diagnosticValue)
		{
			// according to http://www.letstakeovertheworld.com/blog/index.cgi/MSSPrimer2_10252008.html
			// SL2 never calls GetDiagnosticAsync, 
			// so user code should never call this method either
			
			if (closed || media_element == null || demuxer == IntPtr.Zero)
				throw new InvalidOperationException ();

			// NativeMethods.media_element_report_get_diagnostic_completed (media_element.native, diagnosticKind, diagnosticValue);
		}
		
		protected void ReportGetSampleCompleted (MediaStreamSample mediaStreamSample)
		{
			IntPtr frame;
			IntPtr buffer;
			uint buflen;
			byte [] buf;
			
			if (closed || media_element == null || demuxer == IntPtr.Zero)
				throw new InvalidOperationException ();
			
			if (mediaStreamSample.MediaStreamDescription.NativeStream == IntPtr.Zero)
				throw new InvalidOperationException ();
	
			// A null stream means the end has been reached.
			if (mediaStreamSample.Stream == null) {
				NativeMethods.imedia_demuxer_report_get_frame_completed (demuxer, IntPtr.Zero);
				return;
			}
			
			// TODO:
			// Fix this to not copy the data twice and have 3 managed/unmanaged switches.
			// The best would probably be to have the pipeline/mediaframe accept an IMediaStream as the 
			// buffer, this however requires changes in every demuxer/codecs we have.
			
			buflen = (uint) mediaStreamSample.Count;
			buf = new byte [buflen];
			mediaStreamSample.Stream.Seek (mediaStreamSample.Offset, System.IO.SeekOrigin.Begin);
			mediaStreamSample.Stream.Read (buf, 0, (int) buflen);
			
			buffer = Marshal.AllocHGlobal ((int) buflen);
			Marshal.Copy (buf, 0, buffer, (int) buflen);
			
			// TODO: check for KeyFrameFlag. Note that the pipeline must work even if it is never set, so I don't really see the point of it.
			frame = NativeMethods.media_frame_new (mediaStreamSample.MediaStreamDescription.NativeStream, buffer, buflen, (ulong) mediaStreamSample.Timestamp, false);
			
			NativeMethods.imedia_demuxer_report_get_frame_completed (demuxer, frame);
			
			NativeMethods.event_object_unref (frame);
		}
		
		protected void ReportGetSampleProgress (double bufferingProgress)
		{
			if (closed || media_element == null || demuxer == IntPtr.Zero)
				throw new InvalidOperationException ();
			
			// TODO (in mediaelement.h): NativeMethods.imedia_demuxer_report_get_sample_progress (media_element.native, bufferingProgress);
		}
		
		protected void ReportOpenMediaCompleted (IDictionary<MediaSourceAttributesKeys, string> mediaStreamAttributes, IEnumerable<MediaStreamDescription> availableMediaStreams)
		{
			IntPtr stream;
			string str_duration;
			string str_can_seek;
			bool can_seek;
			ulong duration;
			
			if (closed)
				throw new InvalidOperationException ("closed");
			
			if (media_element == null)
				throw new InvalidOperationException ("media_element");
			
			if (demuxer == IntPtr.Zero)
				throw new InvalidOperationException ("demuxer");
			
			if (mediaStreamAttributes == null)
				throw new ArgumentNullException ("mediaStreamAttributes");
			
			if (availableMediaStreams == null)
				throw new ArgumentNullException ("availableMediaStreams");
			
			if (media == IntPtr.Zero)
				media = NativeMethods.imedia_object_get_media_reffed (demuxer);
			
			if (mediaStreamAttributes.TryGetValue (MediaSourceAttributesKeys.Duration, out str_duration)) {
				duration = ulong.Parse (str_duration);
			} else {
				throw new ArgumentException ("mediaStreamAttributes.Duration is required.");
			}
			
			if (mediaStreamAttributes.TryGetValue (MediaSourceAttributesKeys.CanSeek, out str_can_seek)) {
				if (string.Equals (str_can_seek, "False", StringComparison.OrdinalIgnoreCase)) {
					can_seek = false;
				} else {
					can_seek = true;
				}
				
				NativeMethods.external_demuxer_set_can_seek (demuxer, can_seek);
			}
			
			if (mediaStreamAttributes.ContainsKey (MediaSourceAttributesKeys.DRMHeader))
				throw new ArgumentException ("DRM sources aren't supported yet.");
			
			foreach (MediaStreamDescription stream_description in availableMediaStreams) {
				string str_width, str_height;
				string str_fourcc, str_codec_private_data;
				uint width, height;
				int fourcc;
				IntPtr extra_data = IntPtr.Zero;
				uint extra_data_size = 0;
				
				if (stream_description == null)
					throw new ArgumentNullException ("availableMediaStreams");
				
				if (stream_description.MediaAttributes == null)
					throw new ArgumentNullException ("availableMediaStreams.MediaAttributes");
				
				switch (stream_description.Type) {
				case MediaStreamType.Video:
					if (stream_description.MediaAttributes.TryGetValue (MediaStreamAttributeKeys.VideoFourCC, out str_fourcc)) {
						if (str_fourcc == null || str_fourcc.Length != 4)
							throw new ArgumentOutOfRangeException ("availableMediaStreams.MediaAttributes.VideoFourCC", str_fourcc);
						fourcc = 0;
						for (int i = 0; i < str_fourcc.Length; i++)
							fourcc += ((byte) str_fourcc [i]) << (8 * i);
					} else {
						throw new ArgumentException ("availableMediaStreams.MediaAttributes.VideoFourCC");
					}
					
					if (stream_description.MediaAttributes.TryGetValue (MediaStreamAttributeKeys.Height, out str_height)) {
						height = uint.Parse (str_height);
					} else {
						throw new ArgumentException ("availableMediaStreams.MediaAttributes.Height");
					}
					
					if (stream_description.MediaAttributes.TryGetValue (MediaStreamAttributeKeys.Width, out str_width)) {
						width = uint.Parse (str_width);
					} else {
						throw new ArgumentException ("availableMediaStreams.MediaAttributes.Height");
					}

					if (stream_description.MediaAttributes.TryGetValue (MediaStreamAttributeKeys.CodecPrivateData, out str_codec_private_data)) {
						extra_data_size = (uint) str_codec_private_data.Length / 2;
						byte [] buf = new byte [extra_data_size]; 
						
						for (int i = 0; i < buf.Length; i++)
							buf[i] = byte.Parse (str_codec_private_data.Substring (i*2, 2), NumberStyles.HexNumber);

						extra_data = Marshal.AllocHGlobal ((int) extra_data_size);

						Marshal.Copy (buf, 0, extra_data, buf.Length);
					}
					
					stream = NativeMethods.video_stream_new (media, fourcc, width, height, (ulong) duration, extra_data, extra_data_size);
					break;
					
				case MediaStreamType.Audio:
					WAVEFORMATEX wave;
					
					if (stream_description.MediaAttributes.TryGetValue (MediaStreamAttributeKeys.CodecPrivateData, out str_codec_private_data)) {
						// str_codec_private_data is WAVEFORMATEX in base16 encoding
						if (str_codec_private_data == null || str_codec_private_data.Length < 36)
							throw new ArgumentOutOfRangeException ("availableMediaStreams.MediaAttributes.CodecPrivateData", str_codec_private_data);
						
						wave = new WAVEFORMATEX (str_codec_private_data);
						extra_data_size = wave.Size;
						byte [] buf = new byte [extra_data_size]; 
						
						for (int i = 0; i < buf.Length; i++)
							buf[i] = byte.Parse (str_codec_private_data.Substring (36 + i * 2, 2), NumberStyles.HexNumber);

						extra_data = Marshal.AllocHGlobal ((int) extra_data_size);

						Marshal.Copy (buf, 0, extra_data, buf.Length);
					} else {
						// CodecPrivateData is required for audio
						throw new ArgumentException ("availableMediaStreams.MediaAttributes.CodecPrivateData");
					}
					
					stream = NativeMethods.audio_stream_new (media, wave.FormatTag, wave.BitsPerSample, wave.BlockAlign, (int) wave.SamplesPerSec, wave.Channels, (int) wave.AvgBytesPerSec * 8, extra_data, extra_data_size);
					break;
					
				case MediaStreamType.Script:
					continue; // We don't care about these yet, SL probably doesn't either
				default:
					throw new ArgumentOutOfRangeException ("mediaStreamType");
				}
				
				stream_description.StreamId = NativeMethods.external_demuxer_add_stream (demuxer, stream);
				stream_description.NativeStream = stream;
			}
			
			NativeMethods.imedia_demuxer_report_open_demuxer_completed (demuxer);
		}
		
		protected void ReportSeekCompleted (long timeSeekedTo)
		{
			if (closed || media_element == null || demuxer == IntPtr.Zero)
				throw new InvalidOperationException ();
			
			NativeMethods.imedia_demuxer_report_seek_completed (demuxer, (ulong) timeSeekedTo);
		}
		
		protected void ReportSwitchMediaStreamCompleted (MediaStreamDescription mediaStreamDescription)
		{
			if (closed || media_element == null || demuxer == IntPtr.Zero)
				throw new InvalidOperationException ();
			
			NativeMethods.imedia_demuxer_report_get_frame_completed (demuxer, IntPtr.Zero);
		}

		private class WAVEFORMATEX
		{
		    public ushort FormatTag;
		    public ushort Channels;
		    public uint SamplesPerSec;
		    public uint AvgBytesPerSec;
		    public ushort BlockAlign;
		    public ushort BitsPerSample;
		    public ushort Size;
			
			private string encoded;
			private int index;
			
			public WAVEFORMATEX (string encoded)
			{
				this.encoded = encoded;
				FormatTag = ReadUInt16 ();
				Channels = ReadUInt16 ();
				SamplesPerSec = ReadUInt32 ();
				AvgBytesPerSec = ReadUInt32 ();
				BlockAlign = ReadUInt16 ();
				BitsPerSample = ReadUInt16 ();
				Size = ReadUInt16 ();
/*
				Console.WriteLine ("{0} => FormatTag: {1} Channels: {2} SamplesPerSec: {3} AvgBytesPerSec: {4} BlockAlign: {5} BitsPerSample: {6} Size: {7}", 
				                   encoded, FormatTag, Channels, SamplesPerSec, AvgBytesPerSec, BlockAlign, BitsPerSample, Size);
*/
			}
					
			private byte ReadChar ()
			{
				byte result;
				char c = encoded [index++];
				
				if (c >= 'A' && c <= 'F')
					result = (byte) (10 + ((byte) c - (byte) 'A'));
				else if (c >= 'a' && c <= 'f')
					result = (byte) (10 + ((byte) c - (byte) 'a'));
				else if (c >= '0' && c <= '9')
					result = (byte) ((byte) c - (byte) '0');
				else
					throw new ArgumentException (string.Format ("Invalid hex character: '{0}'", c));
				
				return result;
			}
			
			private byte ReadByte ()
			{
				int a = ReadChar () << 4;
				int b = ReadChar ();
				return (byte) (a + b);
			}
			
			private ushort ReadUInt16 ()
			{
				int a = ReadByte ();
				int b = ReadByte () << 8;
				return (ushort) (a + b);
			}
			
			private uint ReadUInt32 ()
			{
				uint a = ReadUInt16 ();
				uint b = (uint) (((uint) ReadUInt16 ()) << 16);
				return (uint) (a + b);
			}
		}
			
		internal delegate void CloseDemuxerDelegate (IntPtr instance);
		internal delegate void GetDiagnosticAsyncDelegate (IntPtr instance, MediaStreamSourceDiagnosticKind diagnosticKind);
		internal delegate void GetFrameAsyncDelegate (IntPtr instance, MediaStreamType mediaStreamType);
		internal delegate void OpenDemuxerAsyncDelegate (IntPtr instance, IntPtr demuxer);
		internal delegate void SeekAsyncDelegate (IntPtr instance, long seekToTime);
		internal delegate void SwitchMediaStreamAsyncDelegate (IntPtr instance, MediaStreamDescription mediaStreamDescription);
		
		protected abstract void CloseMedia ();
		protected abstract void GetDiagnosticAsync (MediaStreamSourceDiagnosticKind diagnosticKind);
		protected abstract void GetSampleAsync (MediaStreamType mediaStreamType);
		protected abstract void OpenMediaAsync ();
		protected abstract void SeekAsync (long seekToTime);
		protected abstract void SwitchMediaStreamAsync (MediaStreamDescription mediaStreamDescription);		
	}
}
