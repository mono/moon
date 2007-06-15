using System.Windows.Controls;
using System.Windows.Shapes;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows;
using System;


public class AnimationLineTest : TestBase
{
	public AnimationLineTest()
	{
		canvas = new Canvas();
		Line line = new Line();
		canvas.Children.Add(line);		
		
		line.X1 = 10; line.Y1 = 10;
		line.X2 = 10; line.Y2 = 200;
		line.SetValue<string>(DependencyObject.NameProperty, "TheAmazingMovingLine");
		line.Stroke = new SolidColorBrush(Color.FromArgb(255, 255, 255, 255));

		Storyboard story = new Storyboard();
		DoubleAnimation anim1 = new DoubleAnimation();
		DoubleAnimation anim2 = new DoubleAnimation();
		
		anim1.SetValue<string>(Storyboard.TargetNameProperty, "TheAmazingMovingLine");
		anim1.SetValue<string>(Storyboard.TargetPropertyProperty, "X1");
		anim1.From = 10;
		anim1.To = 300;
		anim1.By = 25;
		anim1.Duration = new TimeSpan(0, 0, 10);
		anim1.AutoReverse = true;
		//anim1.RepeatBehavior = RepeatBehavior.Forever;
				
		anim2.SetValue<string>(Storyboard.TargetNameProperty, "TheAmazingMovingLine");
		anim2.SetValue<string>(Storyboard.TargetPropertyProperty, "X2");
		anim2.From = 10;
		anim2.To = 300;
		anim2.By = 25;
		anim2.Duration = new TimeSpan(0, 0, 30);
		anim2.AutoReverse = true;
		//anim2.RepeatBehavior = RepeatBehavior.Forever;
	}
}


/*
    <Line
	x:Name="TheAmazingMovingLine"
	X1="10" Y1="10"
	X2="10" Y2="200" Stroke="Black">

    <Line.Triggers>
      <EventTrigger RoutedEvent="Line.Loaded">
        <BeginStoryboard>
          <Storyboard x:Name="animation">
            <DoubleAnimation
              Storyboard.TargetName="TheAmazingMovingLine"
              Storyboard.TargetProperty="X1"
              From="10" To="300" By="25" Duration="0:0:10"
	      AutoReverse="True" RepeatBehavior="Forever" />

	    <DoubleAnimation
              Storyboard.TargetName="TheAmazingMovingLine"
              Storyboard.TargetProperty="X2"
              From="10" To="300" By="25" Duration="0:0:30"
              AutoReverse="True" RepeatBehavior="Forever" />
	      
          </Storyboard>
        </BeginStoryboard>
      </EventTrigger>
    </Line.Triggers>

  </Line>
</Canvas>

*/
