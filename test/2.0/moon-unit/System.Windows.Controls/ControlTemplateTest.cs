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
	
	internal class ControlTemplateTestPage : Canvas {

		public ControlTemplateTestPage ()
		{
			Application.LoadComponent (this, new Uri ("/moon-unit;component/System.Windows.Controls/ControlTemplateTest.xaml", UriKind.Relative));
		}

		public void MouseHandler (object sender, MouseButtonEventArgs e)
		{
		}
	}

	[TestClass]
	public partial class ControlTemplateTest
	{
		[TestMethod]
		public void TargetTypeWithNamespace ()
		{
			object o = XamlReader.Load (@"
<ResourceDictionary
	xmlns=""http://schemas.microsoft.com/client/2007""
	xmlns:T=""clr-namespace:System.Windows.Shapes"">
	<ControlTemplate Name=""Name"" TargetType=""T:Rectangle""> 
		<Grid />
	</ControlTemplate>
</ResourceDictionary>");

			Assert.IsNotNull (o, "#1");
			Assert.IsTrue (o is ResourceDictionary, "#2");
			ResourceDictionary d = (ResourceDictionary) o;
			Assert.AreEqual(1, d.Count, "#3");

			ControlTemplate template = (ControlTemplate) d["Name"];
			Assert.IsNotNull (template, "#4");
			Assert.AreEqual(typeof (global::System.Windows.Shapes.Rectangle), template.TargetType, "#5");
		}
		
		[TestMethod]
		public void LoadTemplateOnlyUsingXamlReader ()
		{
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
			Assert.Throws<XamlParseException> ( delegate { XamlReader.Load (@"
<UserControl xmlns=""http://schemas.microsoft.com/client/2007"">
  <UserControl.Template>
    <ControlTemplate TargetType=""Button"">
      <TextBlock Text=""hi"" />
    </ControlTemplate>
  </UserControl.Template>
</UserControl>"); } );
		}

		[TestMethod]
		public void SetTemplateInXamlOnButton ()
		{
			Button b;
			try {
			b = (Button)XamlReader.Load (@"
<Button xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Button.Template>
    <ControlTemplate TargetType=""Button"">
      <TextBlock x:Name=""text"" Text=""hi"" />
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
		public void TemplatePropertyValuePrecedence ()
		{
			Button b;
			try {
			b = (Button)XamlReader.Load (@"
<Button xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Button.Template>
    <ControlTemplate TargetType=""Button"">
      <TextBlock x:Name=""text"" Text=""hi"" Foreground=""Blue""/>
    </ControlTemplate>
  </Button.Template>
</Button>");
			} catch (Exception e) {
				Tester.WriteLine (e.ToString());
				throw e;
			}

			b.ApplyTemplate ();

			TextBlock tb = (TextBlock)VisualTreeHelper.GetChild (b, 0);

			Assert.IsTrue (tb.ReadLocalValue (TextBlock.ForegroundProperty) is SolidColorBrush, "1");
			Assert.AreEqual (Colors.Blue, ((SolidColorBrush)tb.ReadLocalValue (TextBlock.ForegroundProperty)).Color, "1.1");

			tb.ClearValue (TextBlock.ForegroundProperty);

			Assert.AreEqual (DependencyProperty.UnsetValue, tb.ReadLocalValue (TextBlock.ForegroundProperty), "2");
			Assert.IsNotNull (tb.GetValue (TextBlock.ForegroundProperty), "2.1");
			Assert.IsTrue (tb.GetValue (TextBlock.ForegroundProperty) is SolidColorBrush, "2.2");
			Assert.AreEqual (Colors.Black, ((SolidColorBrush)tb.GetValue (TextBlock.ForegroundProperty)).Color, "2.3");
		}

		[TestMethod]
		public void TemplateInStaticResource ()
		{
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
		public void ParentOfContent ()
		{
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
<Canvas.Resources>
  <ControlTemplate x:Key=""ButtonTemplate"" TargetType=""Button"">
      <ContentPresenter Content=""{TemplateBinding Content}"" />
  </ControlTemplate>
</Canvas.Resources>
<Button x:Name=""button"" Template=""{StaticResource ButtonTemplate}"">
  <Button.Content>
    <TextBlock Text=""hi there"" />
  </Button.Content>
</Button>
</Canvas>");

			Button b = (Button)c.FindName ("button");

			Assert.IsTrue (b.ApplyTemplate (), "1");

			Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (b), "2");
			Assert.IsTrue (VisualTreeHelper.GetChild (b, 0) is ContentPresenter, "3");

			ContentPresenter cp = (ContentPresenter)VisualTreeHelper.GetChild (b, 0);

			Assert.IsTrue (object.ReferenceEquals (cp.Content, b.Content), "4");

			Assert.AreEqual (b, ((TextBlock)cp.Content).Parent, "5");
		}


		[TestMethod]
		public void TemplateBindingTest1 ()
		{
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
		public void TemplateBindingTest2 ()
		{
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
<Canvas.Resources>
  <ControlTemplate x:Key=""ButtonTemplate"" TargetType=""Button"">
      <TextBlock Text=""hi"" Width=""{TemplateBinding Width}"" />
  </ControlTemplate> 
</Canvas.Resources>
<Button x:Name=""button"" Width=""100"" Template=""{StaticResource ButtonTemplate}"" />
</Canvas>");

			Button b = (Button)c.FindName ("button");

			Assert.IsTrue (b.ApplyTemplate (), "1");

			Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (b), "2");

			TextBlock tb = (TextBlock)VisualTreeHelper.GetChild (b, 0);

			Assert.AreEqual (100, tb.Width, "3");
		}



		[TestMethod]
		public void TemplateBindingWithStaticResourceTest ()
		{
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

		[TestMethod]
		public void TemplateBindingInsideTemplateTest ()
		{
			// For now just make sure it parses
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
<Canvas.Resources>
  <ControlTemplate x:Key=""ControlTemplate"" TargetType=""Button"">
      <ControlTemplate x:Key=""ButtonTemplate"" TargetType=""Button"">
          <TextBlock Text=""hi"" Width=""{TemplateBinding Width}"" />
      </ControlTemplate>
  </ControlTemplate> 
</Canvas.Resources>
</Canvas>");

		}

		class TestButton : Button {
			public TestButton ()
			{
				this.ApplyTemplate ();
			}

			public override void OnApplyTemplate ()
			{
				// XXX note this depends on the
				// generic.xaml name for Button's
				// border element.
				base.OnApplyTemplate();
				NameFound = base.GetTemplateChild("Background") != null;
			}

			public bool NameFound;
		}

		[TestMethod]
		public void ApplyTemplateTest ()
		{
			Button b = new Button ();

			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (b), "1");

			b.ApplyTemplate ();

			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (b), "2");

			TestButton tb = new TestButton ();

			Assert.IsFalse (tb.NameFound, "3");
		}

		[TestMethod]
		public void HandlerInPageTest ()
		{
			ControlTemplateTestPage page = new ControlTemplateTestPage ();

			Assert.IsNotNull (page, "1");
			
			Button button = (Button) page.FindName ("button");
			Assert.IsNotNull (button, "2");

			button.ApplyTemplate ();
		}
	}
}
