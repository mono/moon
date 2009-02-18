using System;
using System.Net;
using System.Text;
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
	internal class WaveFormatEx
	{
		public ushort FormatTag;
		public ushort Channels;
		public uint SamplesPerSec;
		public uint AvgBytesPerSec;
		public ushort BlockAlign;
		public ushort BitsPerSample;
		public ushort Size;

		private int index;
		private string encoded;

		public WaveFormatEx ()
		{
		}

		public WaveFormatEx (string encoded)
		{
			this.encoded = encoded;
			FormatTag = ReadUInt16 ();
			Channels = ReadUInt16 ();
			SamplesPerSec = ReadUInt32 ();
			AvgBytesPerSec = ReadUInt32 ();
			BlockAlign = ReadUInt16 ();
			BitsPerSample = ReadUInt16 ();
			Size = ReadUInt16 ();
		}

		public string Encoded
		{
			get
			{
				StringBuilder result = new StringBuilder ();
				Write (result, FormatTag);
				Write (result, Channels);
				Write (result, SamplesPerSec);
				Write (result, AvgBytesPerSec);
				Write (result, BlockAlign);
				Write (result, BitsPerSample);
				Write (result, Size);
				/*
				Console.WriteLine ("WaveFormatEx, FormatTag: {0}, Channels: {1}, SamplesPerSec: {2}, AvgBytesPerSec: {3}, BlockAlign: {4}, BitsPerSample: {5}, Size: {6} result: {7}",
					FormatTag, Channels, SamplesPerSec, AvgBytesPerSec, BlockAlign, BitsPerSample, Size, result.ToString ());
				*/	
				return result.ToString ();
			}
		}
		private void Write (StringBuilder result, byte value)
		{
			result.AppendFormat ("{0:X2}", value);
		}

		private void Write (StringBuilder result, ushort value)
		{
			Write (result, (byte) (value & 0xFF));
			Write (result, (byte) (value >> 8));
		}

		private void Write (StringBuilder result, uint value)
		{
			Write (result, (ushort) (value & 0xFFFF));
			Write (result, (ushort) (value >> 16));
		}

		private byte ReadByte ()
		{
			char c = char.ToUpper (Encoded [index++], System.Globalization.CultureInfo.InvariantCulture);

			if (c >= 'A' && c <= 'F')
				return (byte) ((byte) c - (byte) 'A');

			if (c >= '0' && c <= '9')
				return (byte) ((byte) c - (byte) '0');

			throw new ArgumentException (string.Format ("Invalid hex character: '{0}'", c));
		}

		private ushort ReadUInt16 ()
		{
			return (ushort) (ReadByte () + ReadByte () << 8);
		}

		private uint ReadUInt32 ()
		{
			return (uint) (ReadUInt16 () + ReadUInt16 () << 16);
		}
	}
}
