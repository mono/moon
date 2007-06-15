using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;


public class RadialBrushTest : TestBase
{
	public RadialBrushTest()
	{
		canvas = new Canvas();
		Rectangle rect = CreateRectangle(new Point(0.5, 0.5), new Point(0.5, 0.5), new Point(0.5, 0.5));
		rect.Width = 200;
		rect.Height = 200;
		canvas.Children.Add(rect);

		rect = CreateRectangle(new Point(0.5, 0.5), new Point(0.5, 0.5), new Point(0.5, 0.5));
		rect.Width = 100;
		rect.Height = 200;
		rect.SetValue<double>(Canvas.LeftProperty, 220);
		canvas.Children.Add(rect);

		rect = CreateRectangle(new Point(0.5, 0.5), new Point(0.5, 0.5), new Point(0.5, 0.5));
		rect.Width = 100;
		rect.Height = 200;
		rect.SetValue<double>(Canvas.TopProperty, 220);
		canvas.Children.Add(rect);

		rect = CreateRectangle(new Point(0, 0), new Point(0.75, 0.75), new Point(0.9, 0.9));
		rect.Width = 100;
		rect.Height = 100;
		rect.SetValue<double>(Canvas.TopProperty, 220);
		rect.SetValue<double>(Canvas.LeftProperty, 220);
		canvas.Children.Add(rect);
	}

	private Rectangle CreateRectangle(Point origin, Point centre, Point radius)
	{
		RadialGradientBrush brush;
		Rectangle rect;
		GradientStop stop;

		brush = new RadialGradientBrush();
		brush.GradientOrigin = origin;
		brush.Center = centre;
		brush.RadiusX = radius.X;
		brush.RadiusY = radius.Y;

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
		return rect;
	}
}
