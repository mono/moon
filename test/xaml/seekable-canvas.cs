using System;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media.Animation;

public class SeekableCanvas : Canvas
{
	public void Canvas_Loaded (object sender, EventArgs ea) {
		sb = (Storyboard)FindName ("animation");

		MouseMove += Canvas_OnMouseMove;
	}

	public void Canvas_OnMouseDown (object sender, MouseEventArgs me) {
		Console.WriteLine ("mousedown");
		CaptureMouse ();
		active = true;
	}

	public void Canvas_OnMouseUp (object sender, MouseEventArgs me) {
		Console.WriteLine ("mouseup");
		ReleaseMouseCapture ();
		active = false;
	}

	public void Canvas_OnMouseMove (object sender, MouseEventArgs me) {
		if (!active)
			return;

		double p = me.GetPosition(this).X / Width;

		Console.WriteLine ("updating storyboard to {0}", p);

		sb.Seek (TimeSpan.FromTicks ((long)(p * sb.Duration.TimeSpan.Ticks)));
	}

	Storyboard sb;
	bool active;
}
