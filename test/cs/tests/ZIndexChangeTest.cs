using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;


public class ZIndexChangeTest  : TestBase
{
	public ZIndexChangeTest ()
	{
		Rectangle r1, r2;

		canvas = new Canvas();
		r1 = CreateRectangle(new Point(1, 1), new Point(0,0));
		canvas.Children.Add(r1);

		r2 = CreateRectangle(new Point(0, 0), new Point(1,0));
		r2.SetValue<double>(Canvas.LeftProperty, 10);
		canvas.Children.Add(r2);

		/* by default, r2 should draw on top of r1, since they both have a
		   zindex of 0 and r2 was added later. */
		r1.SetValue<int>(UIElement.ZIndexProperty, 10);
	}

	private Rectangle CreateRectangle(Point start, Point end)
	{
		LinearGradientBrush brush;
		Rectangle rect;
		GradientStop stop;

		brush = new LinearGradientBrush();
		brush.StartPoint = start;
		brush.EndPoint = end;

		stop = new GradientStop();
		stop.Color = Colors.Red;
		stop.Offset = 0;
		brush.GradientStops.Add(stop);

		stop = new GradientStop();
		stop.Color = Colors.Green;
		stop.Offset = 0.5;
		brush.GradientStops.Add(stop);

		stop = new GradientStop();
		stop.Color = Colors.Blue;
		stop.Offset = 1;
		brush.GradientStops.Add(stop);

		rect = new Rectangle();
		rect.Fill = brush;
		rect.Width = 100;
		rect.Height = 100;
		return rect;
	}
}
