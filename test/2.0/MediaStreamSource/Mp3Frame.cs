using System;
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
	public class Mp3Frame
	{
		public int Channels;
		public int SamplesPerSec;
		public int Bitrate;
		public int BlockAlign;
		public int Size;
		public int SampleRate;
		public byte [] data;
		public long Duration;

		int FrameSize;
		int Layer;
		int Version;  // 1 = 1, 2 = 2, 3 = 2.5
		int Padding;

		private static uint sync_mask = 0xFFE00000;
		private static int sync_shift = 21;
		private static uint version_mask = 0x00180000;
		private static int version_shift = 19;
		private static uint layer_mask = 0x00060000;
		private static int layer_shift = 17;
		private static uint protection_mask = 0x00010000;
		private static int protection_shift = 16;
		private static uint bitrate_mask = 0x0000F000;
		private static int bitrate_shift = 12;
		private static uint samplerate_mask = 0x00000C00;
		private static int samplerate_shift = 10;
		private static uint padding_mask = 0x00000200;
		private static int padding_shift = 9;
		private static uint channel_mask = 0x000000C0;
		private static int channel_shift = 6;

		private static int [,] mpeg1_bitrates = new int [,] {
			/* version 1, layer 1 */
			{ 0, 32000, 48000, 56000, 128000, 160000, 192000, 224000, 256000, 288000, 320000, 352000, 384000, 416000, 448000 },
			/* version 1, layer 2 */
			{ 0, 32000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000, 384000 },
			/* version 1, layer 3 */
			{ 0, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000 },
		};

		private static int [,] mpeg2_bitrates = new int [,] {
			/* version 2, layer 1 */
			{ 0, 32000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000, 176000, 192000, 224000, 256000 },
			/* version 2, layer 2 */
			{ 0, 8000, 16000, 24000, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000 },
			/* version 2, layer 3 */
			{ 0, 8000, 16000, 24000, 32000, 40000, 48000, 56000, 64000, 80000, 96000, 112000, 128000, 144000, 160000 }
		};

		private static int [,] sample_rates = new int [,]
        {   
            { 44100, 48000, 32000 }, // version 1
            { 22050, 24000, 16000 }, // version 2
            { 11025, 12000, 8000 }   // version 2.5
        };

		private static int [,] block_sizes = new int [,]
		{
			{ 384, 1152, 1152 }, // version 1
			{ 384, 1152,  576 }, // version 2
			{ 384, 1152,  576 }  // version 2.5
		};

		public static Mp3Frame Read (BinaryReader stream)
		{
			Mp3Frame result = new Mp3Frame ();
			uint header = 0;
			uint version, layer, bitrate, samplerate, padding, channel;

			do {
				header <<= 8;
				header += stream.ReadByte ();
			} while ((header & sync_mask) != sync_mask);

			version = (header & version_mask) >> version_shift;
			layer = (header & layer_mask) >> layer_shift;
			bitrate = (header & bitrate_mask) >> bitrate_shift;
			samplerate = (header & samplerate_mask) >> samplerate_shift;
			padding = (header & padding_mask) >> padding_shift;
			channel = (header & channel_mask) >> channel_shift;

			result.Padding = (int) padding;
			result.Channels = channel == 3 ? 1 : 2;

			switch (layer) {
			case 1:
				result.Layer = 3;
				break;
			case 2:
				result.Layer = 2;
				break;
			case 3:
				result.Layer = 1;
				break;
			default:
				throw new InvalidOperationException ();
			}

			switch (version) {
			case 1:
				result.Version = 3; // 2.5
				break;
			case 2:
				result.Version = 2; // 2
				break;
			case 3:
				result.Version = 1; // 1
				break;
			default: 
				throw new InvalidOperationException ();
			}

			result.SampleRate = sample_rates [result.Version - 1, samplerate];

			switch (result.Version) {
			case 1:
				result.Bitrate = mpeg1_bitrates [result.Layer - 1, bitrate];
				break;
			case 2:
			case 3:
				result.Bitrate = mpeg2_bitrates [result.Layer - 1, bitrate];
				break;
			default:
				throw new InvalidOperationException ();
			}

			switch (result.Layer) {
			case 1:
				result.FrameSize = ((12 * result.Bitrate / result.SampleRate) + result.Padding) * 4;
				break;
			case 2:
			case 3:
				result.FrameSize = (144 * result.Bitrate / result.SampleRate) + result.Padding;
				break;
            default:
				throw new InvalidOperationException ();
            }

			result.Duration = block_sizes [result.Version - 1, result.Layer - 1];
			result.Duration *= 10000000;
			result.Duration /= result.SampleRate;

			//System.Diagnostics.Debug.WriteLine ("Found header: {0:X8} at {7}, version: {1}, layer: {2}, bitrate: {3}, samplerate: {4}, padding: {5}, channel: {6} Duration: {8}",
			//	header, version, layer, bitrate, samplerate, padding, channel, stream.BaseStream.Position, result.Duration); 

			using (MemoryStream memory = new MemoryStream ()) {
				using (BinaryWriter writer = new BinaryWriter (memory)) {

					writer.Write ((byte) ((header & 0xFF000000) >> 24));
					writer.Write ((byte) ((header & 0x00FF0000) >> 16));
					writer.Write ((byte) ((header & 0x0000FF00) >> 8));
					writer.Write ((byte) ((header & 0x000000FF)));
					for (int i = 0; i < result.FrameSize - 4; i++)
						writer.Write (stream.ReadByte ());

					writer.Flush ();
					result.data = memory.ToArray ();
				}
			}

			return result;
		}
	}
}
