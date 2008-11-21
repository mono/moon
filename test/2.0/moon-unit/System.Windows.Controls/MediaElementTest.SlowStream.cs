using System;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Controls
{
	public partial class MediaElementTest
	{

		private class SlowStream : Stream
		{
			private Stream real;

			public SlowStream (Stream real) 
			{
				this.real = real;
			}

			private void Log (string member)
			{
				Debug.WriteLine ("SlowStream." + member);
			}
			
			public override bool CanRead {
				get {
					Log ("CanRead: " + real.CanRead);
					return real.CanRead;
				}
			}

			public override bool CanSeek {
				get {
					Log ("CanSeek: " + real.CanSeek);
					return real.CanSeek;
				}
			}

			public override bool CanWrite {
				get {
					Log ("CanWrite: " + real.CanWrite);
					return real.CanWrite;
				}
			}

			public override void Flush () {
				Log ("Flush");
				real.Flush ();
			}

			public override long Length {
				get {
					Log ("get_Length: " + real.Length);
					return real.Length;
				}
			}

			public override long Position {
				get {
					Log ("get_Position: " + real.Position);
					return real.Position;
				}
				set {
					Log ("set_Position (" + value + ")");
					real.Position = value;
				}
			}

			public override int Read (byte [] buffer, int offset, int count) {
				Log ("Read (buffer, " + offset + ", " + count + ")");
				Thread.Sleep (1000);
				return real.Read (buffer, offset, count);
			}

			public override long Seek (long offset, SeekOrigin origin) {
				Log ("Seek (" + offset + ", " + origin + ")");
				return real.Seek (offset, origin);
			}

			public override void SetLength (long value) {
				Log ("SetLength: " + value.ToString ());
				real.SetLength (value);
			}

			public override void Write (byte [] buffer, int offset, int count) {
				Log ("Write (buffer, " + offset + ", " + count + ")");
				real.Write (buffer, offset, count);
			}
		}
	}
}
