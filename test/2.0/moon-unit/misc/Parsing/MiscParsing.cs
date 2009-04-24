
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

	public class HalfHeightControl : UserControl {

		public static readonly DependencyProperty HalfHeightProperty = DependencyProperty.RegisterAttached ("HalfHeight", typeof (double), typeof (HalfHeightControl), new PropertyMetadata (OnHalfHeightChanged));

		public double HalfHeight {
			get { return (double) GetValue (HalfHeightProperty); }
			set { SetValue (HalfHeightProperty, value); }
		}

		public static void OnHalfHeightChanged (DependencyObject sender, DependencyPropertyChangedEventArgs args)
		{
			(sender as FrameworkElement).Height = (double) args.NewValue * 2;
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
		[MoonlightBug]
		public void ResolveManagedPropertyPath1 ()
		{
			MiscParsingTestAnimation page = new MiscParsingTestAnimation ("/moon-unit;component/misc/Parsing/MiscParsingAnimation.xaml");

			Storyboard s = (Storyboard) page.FindName ("the_storyboard");
			HalfHeightControl control = (HalfHeightControl) page.FindName ("the_control");
			DoubleAnimation the_animation = (DoubleAnimation) page.FindName ("the_animation");
			
			s.Begin ();
			Enqueue (() => Assert.AreEqual (25, control.HalfHeight, "#1"));

			// Make sure the path isn't magically expanded by the parser
			Enqueue (() => Assert.AreEqual ("(moon:ManagedUserControl.HalfHeight)", Storyboard.GetTargetProperty (the_animation).Path, "#2"));

			// Try setting the path programmatically (reset it first)
			Enqueue (() => s.Stop ());
			Enqueue (() => Storyboard.SetTargetProperty (the_animation, new PropertyPath ("Height")));
			Enqueue (() => s.Begin ());
			Enqueue (() => Assert.AreEqual (25, control.Height, "#3"));

			Enqueue (() => s.Stop ());
			Enqueue (() => Storyboard.SetTargetProperty (the_animation, new PropertyPath ("(moon:ManagedUserControl.HalfHeight)")));
			Enqueue (() => s.Begin ());
			Enqueue (() => Assert.AreEqual (25, control.HalfHeight, "#4"));

			// OK, this shouldn't work and makes little sense, but it does (notice the xmlns)
			Enqueue (() => s.Stop ());
			Enqueue (() => Storyboard.SetTargetProperty (the_animation, new PropertyPath ("(monkey:ManagedUserControl.HalfHeigh)")));
			Enqueue (() => Assert.Throws<InvalidOperationException> (() => s.Begin ()));

			EnqueueTestComplete ();
		}
	}
}

