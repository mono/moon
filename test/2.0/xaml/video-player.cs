using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Shapes;
using System.Windows.Controls;

public class VideoPlayer : Canvas {
	Rectangle progress, stop;
	Canvas play;
	MediaElement video;
	bool playing;
	
	public void OnLoaded (object sender, EventArgs e)
	{
		progress = (Rectangle) FindName ("ProgressBar");
		play = (Canvas) FindName ("PlayButton");
		stop = (Rectangle) FindName ("StopButton");
		video = (MediaElement) FindName ("Video");
		playing = video.AutoPlay;
		SetPlayShape ();
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
			Duration duration = video.NaturalDuration;
		
			if (duration.HasTimeSpan)	
				video.Position = new TimeSpan ((long) ((double) duration.TimeSpan.Ticks * percent));
		} else if (WasClicked (play, x, y)) {
			if (playing)
				Pause ();
			else
				Play ();
			playing = !playing;
		} else if (WasClicked (stop, x, y)) {
			Stop ();
			playing = false;
		}
	}

	private void SetPlayShape()
	{
		play.Children.Clear ();

		Path p = new Path();
		PathGeometry geometry = new PathGeometry ();
		PathFigure f = new PathFigure ();	
		f.Segments = new PathSegmentCollection ();

		p.Data = geometry;
		p.Fill = new SolidColorBrush(Colors.Red);
		p.Stroke = new SolidColorBrush(Colors.Black);
		geometry.Figures = new PathFigureCollection ();
		geometry.Figures.Add(f);

		LineSegment m = new LineSegment();
		m.Point = new Point(3, 2);
		f.Segments.Add(m);

		m = new LineSegment();	
		m.Point = new Point(14, 8.5);
		f.Segments.Add(m);

		m = new LineSegment();	
		m.Point = new Point(3, 15);
		f.Segments.Add(m);

		m = new LineSegment();	
		m.Point = new Point(3, 2);
		f.Segments.Add(m);

		play.Children.Add(p);
	}

	private void SetPauseShape()
	{
		play.Children.Clear ();

		Rectangle r = new Rectangle ();
		r.Width = 4;
		r.Height = 10;
		Canvas.SetTop (r, 3.5);
		Canvas.SetLeft (r, 2);
		r.Fill = new SolidColorBrush(Colors.Red);
		r.Stroke = new SolidColorBrush(Colors.Black);
		play.Children.Add (r);

		r = new Rectangle ();
		r.Width = 4;
		r.Height = 10;
		Canvas.SetTop (r, 3.5);
		Canvas.SetLeft(r, 9);
		r.Fill = new SolidColorBrush(Colors.Red);
		r.Stroke = new SolidColorBrush(Colors.Black);
		play.Children.Add (r);
	}

	private void Stop ()
	{
		SetPlayShape ();
		video.Stop ();
		video.Position = TimeSpan.Zero;
	}

	private void Play ()
	{
		SetPauseShape ();
		video.Play ();
	}

	private void Pause ()
	{
		SetPlayShape ();	
		video.Pause ();
	}
}
