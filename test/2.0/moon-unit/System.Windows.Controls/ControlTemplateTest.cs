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
		[Ignore ("this doesn't seem to work on windows.. 'c.Template = t' raises an exception")]
		public void LoadTemplateOnlyUsingXamlReader ()
		{
			Console.WriteLine ("LoadTemplateOnlyUsingXamlReader");
			UserControl c = new UserControl ();

			ControlTemplate t = (ControlTemplate)XamlReader.Load (@"
<ControlTemplate TargetType=""UserControl"" xmlns=""http://schemas.microsoft.com/client/2007"">
  <TextBlock Text=""hi"" />
</ControlTemplate>");

			c.Template = t;

			Assert.IsTrue (c.ApplyTemplate (), "0");

			Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (c), "1");

			TextBlock tb = (TextBlock)VisualTreeHelper.GetChild (c, 0);

			Assert.AreEqual ("hi", tb.Text, "2");
		}

		[TestMethod]
		[MoonlightBug]
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
		[MoonlightBug]
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
		[MoonlightBug]
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
		[MoonlightBug]
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

	}

}
