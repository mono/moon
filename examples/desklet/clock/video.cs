//
// video.cs: video player desklet
//
// Authors:
//   Rodrigo Kumpera (rkumpera@novell.com)
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
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Desklet.VideoPlayer
{
	public partial class VideoPlayer : Canvas 
	{
		public VideoPlayer()
		{
			InitializeComponent();
		}
		public void PageLoaded (object o, EventArgs e)
		{
			MediaElement media = FindName ("Video") as MediaElement;
			Shape play = FindName ("Play") as Shape;
			Canvas pause = FindName ("Pause") as Canvas;
			Shape stop = FindName ("Stop") as Shape;
			bool stopped = true;
			bool paused = false;

			play.MouseLeftButtonUp += delegate {
				Console.WriteLine ("play pause "+paused+ " stop "+stopped);
				if (!stopped && !paused)
					return;
				media.Play ();
				paused = stopped = false;
			};

			pause.MouseLeftButtonUp += delegate {
				Console.WriteLine ("pause "+paused+ " stop "+stopped);
				if (stopped)
					return;
				if (paused)
					media.Play ();
				else
					media.Pause ();
				paused = !paused;
			};

			stop.MouseLeftButtonUp += delegate {
				Console.WriteLine ("stop pause "+paused+ " stop "+stopped);
				if (stopped)
					return;
				media.Stop ();
				paused = false;
				stopped = true;
			};
		}
	}
}
