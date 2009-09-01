
using System;
using System.Net;
using System.Resources;
using System.Windows;
using System.Windows.Markup;
using System.Windows.Controls;
using System.Windows.Media.Animation;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;


namespace MoonTest.Misc.Parsing
{
	public class MiscParsingTestPage : Canvas {

		public MiscParsingTestPage (string url)
		{
			Application.LoadComponent (this, new Uri (url, UriKind.Relative));
		}
	}

	public class MiscParsingTestAnimation : Canvas {

		public MiscParsingTestAnimation (string url)
		{
			Application.LoadComponent (this, new Uri (url, UriKind.Relative));
		}
	}

	public class MiscParsingCanvas : Canvas {

		public MiscParsingCanvas ()
		{
			Application.LoadComponent (this, new Uri ("/moon-unit;component/misc/Parsing/MiscParsingCanvas.xaml", UriKind.Relative));
		}
	}

	public class MiscParsingEventBase : Canvas {

		public MiscParsingEventBase ()
		{
			Application.LoadComponent (this, new Uri ("/moon-unit;component/misc/Parsing/MiscParsingEvent.xaml", UriKind.Relative));
		}

		void OnFoo (object sender, EventArgs e)
		{
		}

		public void FireFoo ()
		{
			Foo (this, EventArgs.Empty);
		}

		public event EventHandler Foo;
	}

	public class MiscParsingEventImpl1 : MiscParsingEventBase {

		public bool foo_called = false;

		// A private function with the same name is in the base class
		// make sure this one gets used
		void OnFoo (object sender, EventArgs e)
		{
			foo_called = true;
		}
	}

	public class MiscParsingManagedAttachedProp : Canvas {

		public MiscParsingManagedAttachedProp ()
		{
			Application.LoadComponent (this, new Uri ("/moon-unit;component/misc/Parsing/MiscParsingManagedAttachedProp.xaml", UriKind.Relative));
		}
	}

	public class MiscParsingAttachedPropCanvas : Canvas {

		public static readonly DependencyProperty PropValueProperty = DependencyProperty.RegisterAttached ("PropValue", typeof (double), typeof (MiscParsingAttachedPropCanvas), null);

		private static double value;

		public static void SetPropValue (DependencyObject dob, double d)
		{
			value = d;
		}

		public static double GetPropValue (DependencyObject dob)
		{
			return value;
		}
	}

	public class MiscParsingEventImpl2 : MiscParsingEventBase {
	}

	public class ThingWithEvent : UserControl {

		private string the_prop;
		private string the_other_prop;
		private string the_other_prop_during_event;

		public string TheProp {
			get { return the_prop; }
			set {
				the_prop = value;

				if (ThePropChanged != null)
					ThePropChanged (this, EventArgs.Empty);
			}
		}

		public string TheOtherProp {
			get { return the_other_prop; }
			set { the_other_prop = value; }
		}

		public string TheOtherPropDuringEvent {
			get { return the_other_prop_during_event; }
		}

		public void PropChangedHandler (object sender, EventArgs e)
		{
			the_other_prop_during_event = the_other_prop;
		}

		public event EventHandler ThePropChanged;
	}

	public class HalfDimensionsControl : UserControl {

		public static readonly DependencyProperty HalfHeightProperty = DependencyProperty.RegisterAttached ("HalfHeight", typeof (double), typeof (HalfDimensionsControl), new PropertyMetadata (OnHalfHeightChanged));
		public static readonly DependencyProperty HalfWidthProperty = DependencyProperty.RegisterAttached ("HalfWidth", typeof (double), typeof (HalfDimensionsControl), new PropertyMetadata (OnHalfWidthChanged));

		public double HalfHeight {
			get { return (double) GetValue (HalfHeightProperty); }
			set { SetValue (HalfHeightProperty, value); }
		}

		public double HalfWidth {
			get { return (double) GetValue (HalfWidthProperty); }
			set { SetValue (HalfWidthProperty, value); }
		}

		public static void OnHalfHeightChanged (DependencyObject sender, DependencyPropertyChangedEventArgs args)
		{
			(sender as FrameworkElement).Height = (double) args.NewValue * 2;
		}

		public static void OnHalfWidthChanged (DependencyObject sender, DependencyPropertyChangedEventArgs args)
		{
			(sender as FrameworkElement).Width = (double) args.NewValue * 2;
		}
	}

	internal class ParsingPrivateControl : UserControl {
		
	}

	internal class ParsingPrivateControlWxClass : UserControl {

		public ParsingPrivateControlWxClass ()
		{
			Application.LoadComponent (this, new Uri ("/moon-unit;component/misc/Parsing/MiscParsingPrivateControl.xaml", UriKind.Relative));
		}

		public UIElement TheContent {
			get { return Content; }
		}
	}

	[TestClass]
	public class MiscParsingTest : SilverlightTest
	{
		[TestMethod]
		public void FindNameOfManagedControl ()
		{
			MiscParsingTestPage page = new MiscParsingTestPage ("/moon-unit;component/misc/Parsing/MiscParsingTestPage.xaml");
			

			Assert.IsNotNull (page, "1");

			MiscParsingCanvas canvas = (MiscParsingCanvas) page.FindName ("the_canvas");
			Assert.IsNotNull (canvas, "2");
			Assert.IsNotNull (canvas.FindName ("the_block"), "3");
			Assert.IsNull (page.FindName ("the_block"), "4");

			//
			// This shows that it actually isn't a parsing issue, setting the
			// name in code has the same problems.
			//
			
			TextBlock the_block = (TextBlock) canvas.FindName ("the_block");
			the_block.Name = "some_name";
			Assert.IsNotNull (canvas.FindName ("some_name"), "5");
			Assert.IsNull (canvas.FindName ("the_block"), "6");
			Assert.IsNull (page.FindName ("some_name"), "7");

			//
			// Make sure we are picking up the sub element and not ourself
			//
			Canvas sub_canvas = canvas.FindName ("the_canvas") as Canvas;
			Assert.IsNotNull (sub_canvas, "8");
			Assert.AreEqual (0, sub_canvas.Children.Count, "9");
		}

		[TestMethod]
		[Asynchronous]
		public void ResolveManagedPropertyPath2 ()
		{
			MiscParsingTestAnimation page = new MiscParsingTestAnimation ("/moon-unit;component/misc/Parsing/MiscParsingAnimation.xaml");

			Storyboard s = (Storyboard) page.FindName ("the_storyboard");
			HalfDimensionsControl control = (HalfDimensionsControl) page.FindName ("the_control");
			DoubleAnimation the_animation = (DoubleAnimation) page.FindName ("the_animation");
			
			s.Begin ();
			Enqueue (() => Assert.AreEqual (25, control.HalfHeight, "#1"));

			// Make sure the path isn't magically expanded by the parser
			Enqueue (() => Assert.AreEqual ("(moon:HalfDimensionsControl.HalfHeight)", Storyboard.GetTargetProperty (the_animation).Path, "#2"));

			// Try setting the path programmatically (reset it first)
			Enqueue (() => s.Stop ());
			Enqueue (() => Storyboard.SetTargetProperty (the_animation, new PropertyPath ("Height")));
			Enqueue (() => s.Begin ());
			Enqueue (() => Assert.AreEqual (25, control.Height, "#3"));

			Enqueue (() => s.Stop ());
			Enqueue (() => Storyboard.SetTargetProperty (the_animation, new PropertyPath ("(moon:HalfDimensionsControl.HalfWidth)")));
			Enqueue (() => s.Begin ());
			Enqueue (() => Assert.AreEqual (25, control.HalfWidth, "#4"));

			Enqueue (() => s.Stop ());
			Enqueue (() => Storyboard.SetTargetProperty (the_animation, new PropertyPath ("(monkey:HalfDimensionsControl.HalfHeigh)")));
			Enqueue (() => Assert.Throws<InvalidOperationException> (() => s.Begin ()));

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void ResolveManagedPropertyPathInCode ()
		{
			MiscParsingTestAnimation page = new MiscParsingTestAnimation ("/moon-unit;component/misc/Parsing/MiscParsingAnimation.xaml");

			Storyboard s = (Storyboard) page.FindName ("the_storyboard");
			HalfDimensionsControl control = (HalfDimensionsControl) page.FindName ("the_control");

			DoubleAnimation the_animation = new DoubleAnimation ();

			the_animation.Duration = new Duration (new TimeSpan (0));
			the_animation.To = 25;

			s.Children.Clear ();
			s.Children.Add (the_animation);

			Storyboard.SetTarget (the_animation, control);
			Storyboard.SetTargetProperty (the_animation, new PropertyPath ("(moon:HalfDimensionsControl.HalfHeight)"));
			
			Enqueue (() => s.Begin ());
			Enqueue (() => Assert.AreEqual (25, control.HalfHeight, "#1"));

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void ResolveManagedPropertyPathInCodeFull ()
		{
			Storyboard s = new Storyboard ();
			HalfDimensionsControl control = new HalfDimensionsControl ();

			control.Name = "the_control";

			DoubleAnimation the_animation = new DoubleAnimation ();

			the_animation.Duration = new Duration (new TimeSpan (0));
			the_animation.To = 25;

			s.Children.Clear ();
			s.Children.Add (the_animation);

			Storyboard.SetTarget (the_animation, control);
			Storyboard.SetTargetProperty (the_animation, new PropertyPath ("(monkeylove:HalfDimensionsControl.HalfHeight)"));
			
			Enqueue (() => s.Begin ());
			Enqueue (() => Assert.AreEqual (25, control.HalfHeight, "#1"));

			EnqueueTestComplete ();
		}

		[TestMethod]
		[MoonlightBug]
		public void EscapeMarkup ()
		{
			Canvas c;
			string s;

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   	 xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
							    <Canvas.Resources><TextBlock x:Name=""the_block"" Text=""{}{StaticResource foobar}"" /></Canvas.Resources>
							</Canvas>");

			TextBlock the_block = (TextBlock) c.FindName ("the_block");
			Assert.AreEqual ("{StaticResource foobar}", the_block.Text, "#1");


			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   	 xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
							    <Canvas.Resources><TextBlock x:Name=""the_block"" Text=""{}{Binding foobar}"" /></Canvas.Resources>
							</Canvas>");

			the_block = (TextBlock) c.FindName ("the_block");
			Assert.AreEqual ("{Binding foobar}", the_block.Text, "#2");


			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   	 xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
							    <Canvas.Resources><TextBlock x:Name=""the_block"" Text=""{}{}"" /></Canvas.Resources>
							</Canvas>");

			the_block = (TextBlock) c.FindName ("the_block");
			Assert.AreEqual ("{}", the_block.Text, "#3");


			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   	 xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
							    <Canvas.Resources><TextBlock x:Name=""the_block"" Text=""{}"" /></Canvas.Resources>
							</Canvas>");

			the_block = (TextBlock) c.FindName ("the_block");
			Assert.AreEqual ("", the_block.Text, "#4");


			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   	 xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
							    <Canvas.Resources><TextBlock x:Name=""the_block"" Text=""  {} foo"" /></Canvas.Resources>
							</Canvas>");

			the_block = (TextBlock) c.FindName ("the_block");
			Assert.AreEqual ("  {} foo", the_block.Text, "#5");


			Assert.Throws <XamlParseException> (() => XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   	 xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
							    <Canvas.Resources><TextBlock x:Name=""the_block"" Text=""{ }foo"" /></Canvas.Resources>
							</Canvas>"));


			Assert.Throws <XamlParseException> (() => XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   	 xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
							    <Canvas.Resources><TextBlock x:Name=""the_block"" Text=""{       }foo"" /></Canvas.Resources>
							</Canvas>"));

			c = (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
						   	 xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
							    <Canvas.Resources><TextBlock x:Name=""the_block"" Text=""   {       }  foo"" /></Canvas.Resources>
							</Canvas>");
			the_block = (TextBlock) c.FindName ("the_block");
			Assert.AreEqual ("   {       }  foo", the_block.Text, "#6");

			Style style = (Style) XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007""
						   	 xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
						   	 TargetType=""TextBlock"" >
							    <Setter Property=""Text"" Value=""{}{0:p2}"" />
							</Style>");
			Assert.AreEqual ("{0:p2}", ((Setter) style.Setters [0]).Value);
		}

		[TestMethod]
		public void EventHandlerInBaseAndImplClasses ()
		{

			MiscParsingEventImpl1 impl1 = new MiscParsingEventImpl1 ();

			impl1.FireFoo ();

			Assert.IsTrue (impl1.foo_called, "#a1");
			
		}

		[TestMethod]
		public void EventHandlerInBaseClass ()
		{
			Assert.Throws <XamlParseException> (() => new MiscParsingEventImpl2 ());
		}

		[TestMethod]
		public void MissingXmlnsOnAttachedProp ()
		{
			MiscParsingManagedAttachedProp m = new MiscParsingManagedAttachedProp ();
		}
		
		[TestMethod]
		public void StaticResourceFromStyleTest ()
		{
			StackPanel c = (StackPanel) XamlReader.Load (@"
    <StackPanel xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
				xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
        <StackPanel.Resources>
            <SolidColorBrush x:Key=""FirstColor"" Color=""#486974"" />
            <Style x:Key=""DemoContent"" TargetType=""ContentControl"">
                <Setter Property=""Background"" Value=""{StaticResource FirstColor}"" />
            </Style>
       </StackPanel.Resources>
    </StackPanel>");

			Assert.AreEqual (2, c.Resources.Count);
			Style s = (Style) c.Resources ["DemoContent"];
			Setter setter = (Setter)s.Setters [0];
			Assert.AreEqual (c.Resources ["FirstColor"], setter.Value, "#1");
		}

		[TestMethod]
		public void SetHandlerBeforeProps ()
		{
			var c = (ThingWithEvent) XamlReader.Load (@"<c:ThingWithEvent xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
							   	    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
								    xmlns:c=""clr-namespace:MoonTest.Misc.Parsing;assembly=moon-unit"" ThePropChanged=""PropChangedHandler"" TheProp=""foo"" TheOtherProp=""bar"">
								      
							    </c:ThingWithEvent>");
			Assert.IsNull (c.TheOtherPropDuringEvent, "1");

			c = (ThingWithEvent) XamlReader.Load (@"<c:ThingWithEvent xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
							   	    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
								    xmlns:c=""clr-namespace:MoonTest.Misc.Parsing;assembly=moon-unit"" ThePropChanged=""PropChangedHandler"" TheOtherProp=""bar"" TheProp=""foo"">
								      
							    </c:ThingWithEvent>");
			Assert.AreEqual ("bar", c.TheOtherPropDuringEvent, "2");
		}

		[TestMethod]
		public void AttachedPropWithText ()
		{
			var c = (TextBox) XamlReader.Load (@"<TextBox xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
							   	    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Left>5</Canvas.Left></TextBox>");

			object val = c.GetValue (Canvas.LeftProperty);
			Assert.AreEqual (5.0, c.GetValue (Canvas.LeftProperty), "1");
		}

		[TestMethod]
		public void PrivateType ()
		{
			Assert.Throws<XamlParseException> (() => XamlReader.Load (@"<c:ParsingPrivateControl xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
							   	    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
								    xmlns:c=""clr-namespace:MoonTest.Misc.Parsing;assembly=moon-unit"">"));
		}

		[TestMethod]
		public void PrivateTypeInXClass ()
		{
			var c = new ParsingPrivateControlWxClass ();

			Assert.IsNotNull (c.TheContent, "1");
			Assert.AreEqual (typeof (Border), c.TheContent.GetType (), "2");
		}
	}
}
