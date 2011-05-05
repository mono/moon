using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Net;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Effects;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace DemoApp
{
	public class CustomEffect : ShaderEffect
	{
		public CustomEffect ()
		{
			PixelShader ps = new PixelShader ();
			ps.UriSource = new Uri ("file:///data/tmp/local/invert.ps", UriKind.Absolute);

			this.PixelShader = ps;
		}
	}

	public partial class Page : UserControl
	{
	    bool customShaderApplied = false;
	    bool blurApplied = false;
	    bool customApplied = false;

	    public Page () {
		InitializeComponent ();

		mediaElement.MediaEnded += delegate {
		    mediaElement.Stop ();
		    mediaElement.Play ();
		};

		double lastX = 0, lastY = 0;
		double xfactor = 0, yfactor = 0;
		double xdelta = 0, ydelta = 0;

		DispatcherTimer timer = new DispatcherTimer () {
			Interval = TimeSpan.FromMilliseconds (60)
		};

		timer.Tick += delegate {
		    if (xdelta > 0) {
			xdelta -= 0.10;
			if (xdelta < 0)
			    xdelta = 0;
		    }
		    else {
			xdelta += 0.10;
			if (xdelta > 0)
			    xdelta = 0;
		    }

		    if (ydelta > 0) {
			ydelta -= 0.10;
			if (ydelta < 0)
			    ydelta = 0;
		    }
		    else {
			ydelta += 0.10;
			if (ydelta > 0)
			    ydelta = 0;
		    }

		    if (ydelta == 0 && xdelta == 0) {
			timer.Stop ();
			return;
		    }

		    yfactor += ydelta;
		    xfactor += xdelta;

		    ((PlaneProjection)mediaElement.Projection).RotationX = xfactor;
		    ((PlaneProjection)mediaElement.Projection).RotationY = yfactor;
		};

		grid.MouseLeftButtonDown += (o, e) => {
		    var pos = e.GetPosition (grid);
		    lastX = pos.X;
		    lastY = pos.Y;

		    timer.Stop ();
		};

		grid.MouseMove += (o, e) => {
		    var pos = e.GetPosition (grid);

		    ydelta = (double)(lastX - pos.X) / grid.Width * 360;
		    xdelta = (double)(lastY - pos.Y) / grid.Height * 360;

		    yfactor += ydelta;
		    xfactor += xdelta;

		    if (mediaElement.Projection == null)
			mediaElement.Projection = new PlaneProjection ();

		    ((PlaneProjection)mediaElement.Projection).RotationX = xfactor;
		    ((PlaneProjection)mediaElement.Projection).RotationY = yfactor;

		    lastX = pos.X;
		    lastY = pos.Y;
		};

		grid.MouseLeftButtonUp += (o, e) => {
		    timer.Start();
	        };

		reset.Click += (o,e) => {
		    timer.Stop();
		    if (blurApplied || customShaderApplied)
			mediaElement.Effect = null;
		    blurApplied = true;
		    customShaderApplied = true;

		    ((PlaneProjection)mediaElement.Projection).RotationX = 0;
		    ((PlaneProjection)mediaElement.Projection).RotationY = 0;
		};
	    }

	}
}
