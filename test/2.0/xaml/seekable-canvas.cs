using System;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media.Animation;

public class SeekableCanvas : Canvas
{
	public void Canvas_Loaded (object sender, EventArgs ea) {
		sb = (Storyboard)FindName ("animation");
		sb.Begin ();
	}

	public void Canvas_OnMouseDown (object sender, MouseEventArgs me) {
		CaptureMouse ();
		active = true;
		sb.Pause ();
	}

	public void Canvas_OnMouseUp (object sender, MouseEventArgs me) {
		ReleaseMouseCapture ();
		active = false;
		sb.Resume ();
	}

	public void Canvas_OnMouseMove (object sender, MouseEventArgs me) {
		if (!active)
			return;

		double p = me.GetPosition(this).X / Width;

		sb.Seek (TimeSpan.FromTicks ((long)(p * sb.Duration.TimeSpan.Ticks)));
		sb.Pause ();
	}

	Storyboard sb;
	bool active;
}
