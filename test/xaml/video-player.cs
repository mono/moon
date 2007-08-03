using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;
using System.Windows.Controls;

public class VideoPlayer : Canvas {
	Rectangle progress, play, stop;
	MediaElement video;
	bool playing;
	
	public void OnLoaded (object sender, EventArgs e)
	{
		progress = (Rectangle) FindName ("ProgressBar");
		play = (Rectangle) FindName ("PlayButton");
		stop = (Rectangle) FindName ("StopButton");
		video = (MediaElement) FindName ("Video");
		playing = video.AutoPlay;
	}
	
	bool WasClicked (FrameworkElement item, double x, double y)
	{
		double left = (double) item.GetValue (Canvas.LeftProperty);
		double top = (double) item.GetValue (Canvas.TopProperty);
		
		return (x >= left && x <= (left + item.Width)
			&& y >= top && y <= (top + item.Height));
	}
	
	public void OnMouseDown (object sender, MouseEventArgs mouse)
	{
		Console.WriteLine ("Canvas OnMouseDown");
		
		int x = (int) mouse.GetPosition (this).X;
		int y = (int) mouse.GetPosition (this).Y;
		
		if (WasClicked (progress, x, y)) {
			double percent = mouse.GetPosition (progress).X / progress.Width;
			TimeSpan duration = video.NaturalDuration;
			
			video.Position = new TimeSpan ((long) ((double) duration.Ticks * percent));
		} else if (WasClicked (play, x, y)) {
			if (playing)
				video.Pause ();
			else
				video.Play ();
			playing = !playing;
		} else if (WasClicked (stop, x, y)) {
			video.Stop ();
			playing = false;
		}
	}
}