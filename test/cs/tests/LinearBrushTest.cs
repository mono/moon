using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;


public class LinearBrushTest : TestBase
{
	public LinearBrushTest()
	{
		canvas = new Canvas();
		Rectangle rect = CreateRectangle(new Point(1, 1), new Point(0,0));
		canvas.Children.Add(rect);

		rect = CreateRectangle(new Point(0, 0), new Point(1,0));
		rect.SetValue<double>(Canvas.LeftProperty, 120);
		canvas.Children.Add(rect);

		rect = CreateRectangle(new Point(0, 0), new Point(0,1));
		rect.SetValue<double>(Canvas.TopProperty, 120);
		canvas.Children.Add(rect);

		rect = CreateRectangle(new Point(0, 0), new Point(1,1));
		rect.SetValue<double>(Canvas.LeftProperty, 120);
		rect.SetValue<double>(Canvas.TopProperty, 120);
		canvas.Children.Add(rect);
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
