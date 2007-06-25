using System;
using System.Threading;
using IO=System.IO;

using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Golem.Desklet
{
  public class Clock : Canvas 
  {
    RotateTransform secondsHand;
    RotateTransform minuteHand;
    RotateTransform hourHand;
    Image background;
    Timer t;
    Uri moon1;
    Uri moon2;
    
    void UpdateTime (object status)
    {
      Console.WriteLine ("UpdateTime");
      DateTime now = DateTime.Now;

      if (now.Second < 30)
	background.Source = moon1;
      else
	background.Source = moon2;

      secondsHand.Angle = now.Second * 6;
      minuteHand.Angle = now.Minute * 6;
      hourHand.Angle = now.Hour * 30;
    }

    public void PageLoaded (object o, EventArgs e)
    {
      Console.WriteLine ("page loaded");
      secondsHand = FindName ("secondsHand") as RotateTransform;
      minuteHand = FindName ("minuteHand") as RotateTransform;
      hourHand = FindName ("hourHand") as RotateTransform;
      background = FindName ("backImage") as Image;

      string imagesDir = IO.Path.Combine (Environment.CurrentDirectory, "images");
      moon1 = new Uri (IO.Path.Combine (imagesDir, "moon1.jpg"));
      moon2 = new Uri (IO.Path.Combine (imagesDir, "moon2.jpg"));
      
      if (secondsHand == null || minuteHand == null || hourHand == null || background == null) {
	// TODO: Should communicate the error to the desklet host
	Console.WriteLine ("Missing elements in the desklet xaml file");
	return;
      }

      Console.WriteLine ("Starting timer");
      AutoResetEvent autoEvent = new AutoResetEvent(false);
      t = new Timer (new TimerCallback (UpdateTime), autoEvent, 5, 1000);
    }
  }
}
