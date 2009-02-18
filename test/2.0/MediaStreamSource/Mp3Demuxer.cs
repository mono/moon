using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace MediaStreamSource
{
	public class Mp3Demuxer : System.Windows.Media.MediaStreamSource
	{
		private BinaryReader stream;
		private Mp3Frame frame;
		private MediaStreamDescription description;
		private long current_pts;
		private DateTime opened = DateTime.MinValue;

		public Mp3Demuxer (Stream stream)
		{
			this.stream = new BinaryReader (stream);
		}

		protected override void CloseMedia ()
		{
			if (stream != null) {
				stream.Close ();
				stream = null;
			}
		}

		protected override void GetDiagnosticAsync (MediaStreamSourceDiagnosticKind diagnosticKind)
		{
			throw new NotImplementedException ();
		}

		protected override void GetSampleAsync (MediaStreamType mediaStreamType)
		{
			Mp3Frame frame;
			MediaStreamSample sample;
			Dictionary<MediaSampleAttributeKeys, string> attribs = new Dictionary<MediaSampleAttributeKeys, string> ();

			//string format = "HH:mm:ss.ffff";
			//if (opened == DateTime.MinValue)
			//    opened = DateTime.Now;
			//Debug.WriteLine ("{0} GetSampleAsync stamp: {1}", (DateTime.Now - opened).ToString (), TimeSpan.FromMilliseconds (current_pts / 10000).ToString ());

			try {
				if (this.frame != null) {
					frame = this.frame;
					this.frame = null;
				} else {
					frame = Mp3Frame.Read (stream);
				}

				sample = new MediaStreamSample (description, new MemoryStream (frame.data), 0, frame.data.Length, current_pts, attribs);

				current_pts += frame.Duration;

				ReportGetSampleCompleted (sample);
			} catch (System.IO.EndOfStreamException ex) {
				Console.WriteLine (ex);
				sample = new MediaStreamSample (description, null, 0, 0, 0, attribs);
				ReportGetSampleCompleted (sample);
			} catch (Exception ex) {
				Console.WriteLine (ex);
				ReportGetSampleCompleted (null);
			}
		}

		protected override void OpenMediaAsync ()
		{
			Dictionary<MediaSourceAttributesKeys, string> media_attributes = new Dictionary<MediaSourceAttributesKeys, string> ();
			List<MediaStreamDescription> media_streams = new List<MediaStreamDescription> ();
			Dictionary<MediaStreamAttributeKeys, string> stream_attributes = new Dictionary<MediaStreamAttributeKeys,string> ();
			MediaStreamDescription media_stream = new MediaStreamDescription (MediaStreamType.Audio, stream_attributes);
			long duration = 60 * 10000;
			WaveFormatEx wave = new WaveFormatEx ();
			Mp3Frame frame = Mp3Frame.Read (stream);

			wave.FormatTag = 85;
			wave.AvgBytesPerSec = (uint) frame.Bitrate / 8;
			wave.BitsPerSample = 0;
			wave.BlockAlign = 1;
			wave.Channels = (ushort) frame.Channels;
			wave.SamplesPerSec = (ushort) frame.SampleRate;
			wave.Size = 12;

			media_attributes.Add (MediaSourceAttributesKeys.CanSeek, "0");
			media_attributes.Add (MediaSourceAttributesKeys.Duration, duration.ToString ());
			stream_attributes [MediaStreamAttributeKeys.CodecPrivateData] = wave.Encoded;
			
			media_streams.Add (media_stream);

			try {
				this.frame = frame;
				this.description = media_stream;
				ReportOpenMediaCompleted (media_attributes, media_streams);
				opened = DateTime.Now;
			} catch (Exception ex) {
				Console.WriteLine (ex);
			}
		}

		protected override void SeekAsync (long seekToTime)
		{
			if (seekToTime == 0) {
				this.ReportSeekCompleted (seekToTime);
				return;
			}

			throw new NotImplementedException ();
		}

		protected override void SwitchMediaStreamAsync (MediaStreamDescription mediaStreamDescription)
		{
			throw new NotImplementedException ();
		}
	}
}
