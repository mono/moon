using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Collections.Generic;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;


namespace MoonTest.System.Windows.Controls
{
	[TestClass]
	public class ControlTemplateTest
	{

		[TestMethod]
		public void LoadTemplateOnlyUsingXamlReader ()
		{
			Console.WriteLine ("LoadTemplateOnlyUsingXamlReader");
			UserControl c = new UserControl ();

			ControlTemplate t = (ControlTemplate)XamlReader.Load (@"
<ControlTemplate TargetType=""UserControl"" xmlns=""http://schemas.microsoft.com/client/2007"">
  <TextBlock Text=""hi"" />
</ControlTemplate>");

			Assert.Throws<InvalidOperationException> (delegate {
				c.Template = t;
			});
			
			Assert.IsFalse (c.ApplyTemplate (), "0");
		}

		[TestMethod]
		public void SetTemplateInXamlOnUserControl ()
		{
			// "Invalid Property: UserControl.Template"
			//
			Console.WriteLine ("SetTemplateInXamlOnUserControl");
			Assert.Throws ( delegate { XamlReader.Load (@"
<UserControl xmlns=""http://schemas.microsoft.com/client/2007"">
  <UserControl.Template>
    <ControlTemplate TargetType=""Button"">
      <TextBlock Text=""hi"" />
    </ControlTemplate>
  </UserControl.Template>
</UserControl>"); },
				typeof (XamlParseException));
		}

		[TestMethod]
		public void SetTemplateInXamlOnButton ()
		{
			Button b;
			try {
			b = (Button)XamlReader.Load (@"
<Button xmlns=""http://schemas.microsoft.com/client/2007"">
  <Button.Template>
    <ControlTemplate TargetType=""Button"">
      <TextBlock Text=""hi"" />
    </ControlTemplate>
  </Button.Template>
</Button>");
			} catch (Exception e) {
				Tester.WriteLine (e.ToString());
				throw e;
			}
			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (b), "0");

			Assert.IsTrue (b.ApplyTemplate (), "1");

			Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (b), "2");

			TextBlock tb = (TextBlock)VisualTreeHelper.GetChild (b, 0);

			Assert.AreEqual ("hi", tb.Text, "3");
		}

		[TestMethod]
		public void TemplateInStaticResource ()
		{
			Console.WriteLine ("TemplateInStaticResource");
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
<Canvas.Resources>
  <ControlTemplate x:Key=""ButtonTemplate"" TargetType=""Button"">
      <TextBlock Text=""hi"" />
  </ControlTemplate> 
</Canvas.Resources>
<Button x:Name=""button"" Template=""{StaticResource ButtonTemplate}"" />
</Canvas>");

			Button b = (Button)c.FindName ("button");

			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (b), "0");

			Assert.IsTrue (b.ApplyTemplate (), "1");

			Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (b), "2");

			TextBlock tb = (TextBlock)VisualTreeHelper.GetChild (b, 0);

			Assert.AreEqual ("hi", tb.Text, "3");
		}


		[TestMethod]
		public void TemplateBindingTest ()
		{
			Console.WriteLine ("TemplateBindingTest");
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
<Canvas.Resources>
  <ControlTemplate x:Key=""ButtonTemplate"" TargetType=""Button"">
      <TextBlock Text=""hi"" Width=""{TemplateBinding Width}"" />
  </ControlTemplate> 
</Canvas.Resources>
<Button x:Name=""button"" Template=""{StaticResource ButtonTemplate}"" />
</Canvas>");

			Button b = (Button)c.FindName ("button");

			Assert.IsTrue (b.ApplyTemplate (), "1");

			Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (b), "2");

			TextBlock tb = (TextBlock)VisualTreeHelper.GetChild (b, 0);

			b.Width = 100;

			Assert.AreEqual (100, b.Width, "3");
			Assert.AreEqual (100, tb.Width, "4");
		}

		[TestMethod]
		public void TemplateBindingWithStaticResourceTest ()
		{
			Console.WriteLine ("TemplateBindingWithStaticResourceTest");
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
    <Canvas.Resources>
        <SolidColorBrush Color=""Red"" x:Key=""RedBrush"" />
        <ControlTemplate x:Key=""ButtonTemplate"" TargetType=""Button"">
            <TextBlock Foreground=""{StaticResource RedBrush}"" Text=""hi"" Width=""{TemplateBinding Width}"" />
        </ControlTemplate>
    </Canvas.Resources>
    <Button x:Name=""button"" Template=""{StaticResource ButtonTemplate}"" />
</Canvas>");

			Button b = (Button)c.FindName ("button");

			Assert.IsTrue (b.ApplyTemplate (), "1");

			Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (b), "2");

			TextBlock tb = (TextBlock)VisualTreeHelper.GetChild (b, 0);

			b.Width = 100;

			Assert.AreEqual (100, b.Width, "3");
			Assert.AreEqual (100, tb.Width, "4");

			Brush brush = tb.Foreground;
			Assert.AreEqual (typeof (SolidColorBrush), brush.GetType (), "5");

			SolidColorBrush scb = brush as SolidColorBrush;
			Assert.AreEqual (Colors.Red, scb.Color, "6");
		}
	}

}
