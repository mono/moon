using System.Windows.Controls;
using System.Windows.Shapes;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows;
using System.Windows.Input;
using System;


public class LineColorTest : TestBase
{	
	public LineColorTest()
	{
		canvas = new Canvas();
		Rectangle rect = new Rectangle();
		SolidColorBrush brush = new SolidColorBrush();
		

		rect.Stroke = new SolidColorBrush(Colors.Black);
		rect.StrokeThickness = 3;
		rect.Height = 100; rect.Width = 100;
		rect.SetValue<double>(Canvas.TopProperty, 10);
		rect.SetValue<double>(Canvas.LeftProperty, 10);

		brush.SetValue<string>(DependencyObject.NameProperty, "rect-brush");
		brush.Color = Colors.Red;
		rect.Fill = brush;
		rect.MouseLeftButtonDown += new MouseEventHandler(rect_Loaded);
		rect.MouseMove += new MouseEventHandler(rect_Loaded);
		canvas.Children.Add(rect);

		rect = new Rectangle();
		canvas.Children.Add(rect);

		rect.SetValue<string>(DependencyObject.NameProperty, "rect");
		rect.Fill = new SolidColorBrush(Colors.Yellow);
		rect.Stroke = new SolidColorBrush(Colors.Orange);
		rect.StrokeThickness = 3;
		rect.Height = 100; rect.Width = 100;
		rect.SetValue<double>(Canvas.TopProperty, 150);
		rect.SetValue<double>(Canvas.LeftProperty, 150);
		//rect.Loaded += new EventHandler(rect2_Loaded);
	}

	
	private void rect_Loaded(object sender, MouseEventArgs e)
	{
		Console.WriteLine("Starting the rect thingy");
		BeginStoryboard story = new BeginStoryboard();
		ColorAnimation anim = new ColorAnimation();
		anim.SetValue<string>(Storyboard.TargetNameProperty, "rect-brush");
		anim.SetValue<string>(Storyboard.TargetPropertyProperty, "Color");
		anim.From = Colors.Red;
		anim.To = Colors.Blue;
		anim.Duration = new TimeSpan(0, 0 ,30);
		story.Storyboard.Children.Add(anim);
		story.Storyboard.Begin();
	}

	private void rect2_Loaded(object sender, EventArgs e)
	{
		BeginStoryboard story = new BeginStoryboard();
		ColorAnimation anim = new ColorAnimation();
		anim.SetValue<string>(Storyboard.TargetNameProperty, "rect");
		anim.SetValue<string>(Storyboard.TargetPropertyProperty, "(Fill).(Color)");
		anim.From = Colors.Yellow;
		anim.To = Colors.Green;
		anim.Duration = new TimeSpan(0, 0 ,30);
		story.Storyboard.Begin();
	}
}
