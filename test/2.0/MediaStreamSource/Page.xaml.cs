using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Resources;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace MediaStreamSource
{
	public partial class Page : UserControl
	{
		public Page ()
		{
			InitializeComponent ();

			DispatcherTimer timer = new DispatcherTimer ();
			timer.Tick += delegate (object sender, EventArgs args) { timer.Stop (); cmdPlay_Click (null, null); };
			timer.Interval = TimeSpan.FromMilliseconds(10);
			timer.Start ();
		}

		private void cmdPlay_Click (object sender, RoutedEventArgs e)
		{
			ResourceManager rm = new ResourceManager ("MediaStreamSource.g", typeof (Page).Assembly);
			Stream stream = rm.GetStream ("miguel.mp3");

			Mp3Demuxer demuxer = new Mp3Demuxer (stream);
			mediaElement.SetSource (demuxer);
			mediaElement.Play ();
		}
	}
}
