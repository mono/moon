using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace NameTortureTest
{
	public partial class Page : UserControl
	{
		public Page()
		{
			InitializeComponent ();

			Assert.SetLog (log);

			RunTests ();
		}

		private void RunTests ()
		{
			RunTests ("BasicXamlReaderTests");
			RunTests ("AddXamlReaderOutputToExistingTree");
			RunTests ("UserControlNamescope1");
			RunTests ("NonVisualTypes");
			RunTests ("VisualTypes");
			RunTests ("StaticResources");
			RunTests ("UserControlEmbeddedInXaml");
		}

		private void RunTests (string testName)
		{
			bool exception_thrown = false;

			Assert.Reset ();

			log.Text += string.Format ("{0}\n", testName);

			try {
				MethodInfo mi = typeof (Page).GetMethod (testName);
				mi.Invoke (this, new object[] { });
			}
			catch (Exception e) {
				log.Text += e.ToString() + "\n";
				exception_thrown = true;
			}

			log.Text += string.Format ("    Assertions: {0}, failures: {1}\n\n", Assert.Count, Assert.Failures);
			if (Assert.Failures > 0 || exception_thrown)
				log.Background = new SolidColorBrush(Colors.Red);
		}

		public void BasicXamlReaderTests ()
		{
			// root of loaded tree is a FWE
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
  <Canvas.Background>
    <SolidColorBrush x:Name=""brush"" />
  </Canvas.Background>
  <Border x:Name=""border"" />
  <TextBlock Name=""text"">
    <Run Name=""run1"" />
    <Run x:Name=""run2"" />
  </TextBlock>
</Canvas>
");
			// we can use FindName on the toplevel FWE to find all objects created with x:Names in the xaml
			Assert.IsNotNull (c.FindName ("brush"), "1");
			Assert.IsNotNull (c.FindName ("border"), "2");
			Assert.IsNotNull (c.FindName ("run2"), "3");

			// and also those created with Name where appropriate
			Assert.IsNotNull (c.FindName ("text"), "4");
			Assert.IsNotNull (c.FindName ("run1"), "5");

			// root of loaded tree is not a FWE
			ResourceDictionary rd = (ResourceDictionary)XamlReader.Load (@"
<ResourceDictionary
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
  <Border x:Name=""border"" />
  <Storyboard x:Name=""hi"" />
</ResourceDictionary>
");

			Assert.IsNotNull (rd["border"], "6");
			Assert.IsNotNull (((Border)rd["border"]).FindName("hi"), "2");
		}

		public void AddXamlReaderOutputToExistingTree ()
		{
			// first create an empty canvas
			Canvas c1 = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
    x:Name=""parentCanvas"">
</Canvas>
");

			// now we create a named canvas and add it to the first canvas's children.
			Canvas c2 = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
    x:Name=""nestedCanvas"">
</Canvas>
");

			c1.Children.Add (c2);

			// the names are visible in the originating canvas' namescopes
			Assert.IsNotNull (c1.FindName("parentCanvas"), "1");
			Assert.IsNotNull (c2.FindName("nestedCanvas"), "2");

			// but neither is visible in the other scopes
			Assert.IsNull (c1.FindName ("nestedCanvas"), "3");
			Assert.IsNull (c2.FindName ("parentCanvas"), "4");
		}

		public void UserControlNamescope1 ()
		{
			UserControl1Container ucc = new UserControl1Container();

			Assert.IsNotNull (ucc.FindName("userControl1"), "1");

			/* now with the reference to userControl1,
			   look up the name inside the UserControl1's
			   definition namescope (i.e. the one used to
			   parse it) */
			UserControl1 uc1 = (UserControl1)ucc.FindName("userControl1");

			Assert.IsNotNull (uc1.FindName ("userControl1"), "2");
			

			/* now see if the two controls are sharing the
			   same namescope (i.e. it's been merged) by
			   checking to see if we can look up the
			   container's name in the embedded control's
			   namescope */

			Assert.IsNull (uc1.FindName ("userControl1Container"), "3");

			/* and an element defined in the UserControl's
			   xaml is not locatable from outside that
			   scope */

			Assert.IsNull (ucc.FindName ("border"), "4");
			Assert.IsNotNull (uc1.FindName ("border"), "5");
		}

		public void NonVisualTypes ()
		{
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
  <Canvas.Background>
    <SolidColorBrush x:Name=""brush"" Color=""Green""/>
  </Canvas.Background>
</Canvas>
");

			// just for sanity's sake
			Assert.IsNotNull (c.FindName("brush"), "0");

			// Clear the property and check if that clears the name
			c.ClearValue (Panel.BackgroundProperty);
			Assert.IsNull (c.FindName("brush"), "1");

			c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
  <Canvas.Background>
    <SolidColorBrush x:Name=""brush"" Color=""Green""/>
  </Canvas.Background>
</Canvas>
");
			// setting to null?
			c.Background = null;
			Assert.IsNull (c.FindName("brush"), "2");


			// what happens when we "reparent" a nonvisual type?  does it's name move?
			c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
  <Canvas.Background>
    <SolidColorBrush x:Name=""brush"" Color=""Green""/>
  </Canvas.Background>
</Canvas>
");

			Canvas c2 = new Canvas();

			c2.Background = (Brush)c.FindName("brush");

			// it's still visible in the original canvas
			Assert.IsNotNull (c.FindName ("brush"), "3");

			// but not in the newly created one.
			Assert.IsNull (c2.FindName ("brush"), "4");

			// let's try a XamlReader.Load'ed canvas
			c2 = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" />
");

			c2.Background = (Brush)c.FindName ("brush");

			// it's still visible in the original canvas
			Assert.IsNotNull (c.FindName ("brush"), "5");

			// and it's still not visible in the newly created one.
			Assert.IsNull (c2.FindName ("brush"), "6");
		}

		public void VisualTypes ()
		{
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >

    <Border x:Name=""border"" />
</Canvas>
");
			// just for sanity's sake
			Assert.IsNotNull (c.FindName("border"), "0");

			// Clear the children list and check if that clears the name
			c.Children.Clear();
			Assert.IsNull (c.FindName("border"), "1");


			// try again, this time remove the border and then add it back again
			c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >

    <Border x:Name=""border"" />
</Canvas>
");

			Border b = (Border)c.FindName("border");
			c.Children.Remove (b);

			c.Children.Add (b);

			Assert.IsNotNull (c.FindName("border"), "2");

			// try moving it to a newly created canvas

			c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >

    <Border x:Name=""border"" />
</Canvas>
");

			Canvas c2 = new Canvas ();

			b = (Border)c.FindName("border");
			c.Children.Remove (b);

			c2.Children.Add (b);

			Assert.IsNull (c.FindName("border"), "3");

			Assert.IsNull (c2.FindName("border"), "4");

			// try moving it to a XamlReader.Load'ed canvas

			c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >

    <Border x:Name=""border"" />
</Canvas>
");

			c2 = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" />
");

			b = (Border)c.FindName("border");
			c.Children.Remove (b);

			c2.Children.Add (b);

			Assert.IsNull (c.FindName("border"), "5");

			Assert.IsNotNull (c2.FindName("border"), "6");
		}

		public void StaticResources ()
		{
			UserControl3Container ucc = new UserControl3Container ();

			/* make sure the brush is in the resources */

			Assert.IsNotNull (ucc.Resources["backgroundBrush"], "1");

			/* it should also be available through FindName */
			Assert.IsNotNull (ucc.FindName ("backgroundBrush"), "2");

			UserControl3 uc3 = (UserControl3)ucc.FindName ("userControl3");

			// the brush's name is not available through the control's namescope.
			Assert.IsNull (uc3.FindName ("backgroundBrush"), "4");
		}

		public void UserControlEmbeddedInXaml ()
		{
			Assert.Throws <XamlParseException> (() => new UserControl2Container(), "1");
		}
	}
}
