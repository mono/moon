
using System;
using System.Net;
using System.Resources;
using System.Windows;
using System.Windows.Data;
using System.Windows.Shapes;
using System.Windows.Markup;
using System.Windows.Controls;
using System.Windows.Media.Animation;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using System.Collections.Generic;
using System.Linq.Expressions;


namespace MoonTest.Misc.Parsing
{
	public class AttachedProperties : FrameworkElement
	{
		public static bool InstantiateLists;

		public static readonly DependencyProperty FE_GetProperty =
			DependencyProperty.RegisterAttached ("FE_Get", typeof (FrameworkElement), typeof (AttachedProperties), null);
		public static FrameworkElement GetFE_Get (DependencyObject o)
		{
			return (FrameworkElement) o.GetValue (FE_GetProperty);
		}


		public static readonly DependencyProperty FE_SetProperty =
			DependencyProperty.RegisterAttached ("FE_Set", typeof (FrameworkElement), typeof (AttachedProperties), null);
		public static void SetFE_Set (DependencyObject o, FrameworkElement value)
		{
			o.SetValue (FE_SetProperty, value);
		}


		public static readonly DependencyProperty FE_GetSetProperty =
			DependencyProperty.RegisterAttached ("FE_GetSet", typeof (FrameworkElement), typeof (AttachedProperties), null);
		public static FrameworkElement GetFE_GetSet (DependencyObject o)
		{
			return (FrameworkElement) o.GetValue (FE_GetSetProperty);
		}
		public static void SetFE_GetSet (DependencyObject o, FrameworkElement value)
		{
			o.SetValue (FE_GetSetProperty, value);
		}


		public static readonly DependencyProperty IList_GetProperty =
			DependencyProperty.RegisterAttached ("IList_Get", typeof (List<FrameworkElement>), typeof (AttachedProperties), null);
		public static List<FrameworkElement> GetIList_Get (DependencyObject o)
		{
			var f = (List<FrameworkElement>) o.GetValue (IList_GetProperty);
			if (InstantiateLists) {
				InstantiateLists = false;
				f = f ?? new List<FrameworkElement> ();
				o.SetValue (IList_GetProperty, f);
			}
			return f;
		}


		public static readonly DependencyProperty IList_SetProperty =
			DependencyProperty.RegisterAttached ("IList_Set", typeof (List<FrameworkElement>), typeof (AttachedProperties), null);
		public static void SetIList_Set (DependencyObject o, List<FrameworkElement> value)
		{
			o.SetValue (IList_SetProperty, value);
		}


		public static readonly DependencyProperty IList_GetSetProperty =
			DependencyProperty.RegisterAttached ("IList_GetSet", typeof (List<FrameworkElement>), typeof (AttachedProperties), null);
		public static List<FrameworkElement> GetIList_GetSet (DependencyObject o)
		{
			var f = (List<FrameworkElement>) o.GetValue (IList_GetSetProperty);
			if (InstantiateLists) {
				InstantiateLists = false;
				f = f ?? new List<FrameworkElement>();
				o.SetValue (IList_GetSetProperty, f);
			}
			return f;
		}
		public static void SetIList_GetSet (DependencyObject o, List<FrameworkElement> value)
		{
			o.SetValue (IList_GetSetProperty, value);
		}
	}

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

	public class CustomControl : FrameworkElement
	{
		
	}

	public class Person
	{
		public double Left {
			get { return 100; }
			set { }
		}
	}

	[TestClass]
	public class MiscParsingTest : SilverlightTest
	{
		T AttachedPropertiesCore <T> (Expression<Func<DependencyProperty>> expression)
			where T : UIElement
		{
			// Load up some xaml where the child of the canvas is of type 'T' and we set
			// the AttachedProperty which is named in the ExpressionTree above.

			// Get the property name and strip out the 'Property' part at the end
			string propName = ((MemberExpression) expression.Body).Member.Name;
			propName = propName.Replace ("Property", "");
			
			// Work out the type name. If it's attached properties we need to prepend clr:
			// otherwise assume it's a regular type like Rectangle which doesn't need any prepending.
			string typeName = typeof (T).Name;
			if (typeof (T) == typeof (AttachedProperties))
				typeName = "clr:" + typeName;

			// Create our xaml and load it
			string s = string.Format (@"	
<Canvas	xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
		xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
		xmlns:clr=""clr-namespace:MoonTest.Misc.Parsing;assembly=moon-unit"">
	<{0}>
		<clr:AttachedProperties.{1}>
			<Rectangle x:Name=""Hidden"" />
		</clr:AttachedProperties.{1}>
	</{0}>
</Canvas>
", typeName, propName);
			
			// Return the child of the canvas so we can poke its values
			Canvas canvas = (Canvas) XamlReader.Load (s);
			return (T) canvas.Children [0];
		}

		[TestMethod]
		public void AttachedProp_FE_GetOnly_OnAttachedProperties ()
		{
			// Attached properties need a getter and setter
			Assert.Throws<XamlParseException> (() =>
				AttachedPropertiesCore<AttachedProperties> (() => AttachedProperties.FE_GetProperty)
			, "#1");
		}

		[TestMethod]
		[MoonlightBug]
		public void AttachedProp_FE_GetOnly_OnRectangle ()
		{
			// Attached properties need a getter and setter
			Assert.Throws<XamlParseException> (() =>
				AttachedPropertiesCore<Rectangle> (() => AttachedProperties.FE_GetProperty)
			, "#1");
		}

		[TestMethod]
		public void AttachedProp_FE_SetOnly_OnAttachedProperties ()
		{
			// Attached properties need a getter and setter
			Assert.Throws<XamlParseException> (() =>
				AttachedPropertiesCore<AttachedProperties> (() => AttachedProperties.FE_SetProperty)
			, "#1");
		}

		[TestMethod]
		[MoonlightBug]
		public void AttachedProp_FE_SetOnly_OnRectangle ()
		{
			var f = AttachedPropertiesCore<Rectangle> (() => AttachedProperties.FE_SetProperty);
			Assert.IsInstanceOfType<Rectangle> (f.GetValue (AttachedProperties.FE_SetProperty), "#1");
			Assert.IsNotNull (((FrameworkElement) f.Parent).FindName ("Hidden"), "#should be findable");
		}

		[TestMethod]
		public void AttachedProp_FE_GetAndSet_OnAttachedProperties ()
		{
			Assert.Throws<XamlParseException> (() =>
				AttachedPropertiesCore<AttachedProperties> (() => AttachedProperties.FE_GetSetProperty)
			, "#1");
		}

		[TestMethod]
		public void AttachedProp_FE_GetAndSet_OnRectangle ()
		{
			var f = AttachedPropertiesCore<Rectangle> (() => AttachedProperties.FE_GetSetProperty);
			Assert.IsInstanceOfType<Rectangle> (f.GetValue (AttachedProperties.FE_GetSetProperty), "#1");

			// The rectangle should not be parented already, so this should succeed
			new Canvas ().Children.Add ((Rectangle) f.GetValue (AttachedProperties.FE_GetSetProperty));
			Assert.IsNotNull (((FrameworkElement) f.Parent).FindName ("Hidden"), "#should be findable");
		}

		[TestMethod]
		public void AttachedProp_IList_GetOnly_OnAttachedProperties ()
		{
			AttachedProperties.InstantiateLists = true;
			// Attached properties need a getter and setter
			Assert.Throws<XamlParseException> (() =>
				AttachedPropertiesCore<AttachedProperties> (() => AttachedProperties.IList_GetProperty)
			, "#1");
		}

		[TestMethod]
		[MoonlightBug ("The rectangle in the IList should not have its name registered")]
		public void AttachedProp_IList_GetOnly_OnRectangle ()
		{
			AttachedProperties.InstantiateLists = true;
			// Attached properties need a getter and setter
			var f = AttachedPropertiesCore<Rectangle> (() => AttachedProperties.IList_GetProperty);
			var list = (List<FrameworkElement>) f.GetValue (AttachedProperties.IList_GetProperty);
			Assert.AreEqual (1, list.Count, "#1");
			Assert.IsInstanceOfType<Rectangle> (list [0], "#1");
			Assert.IsNull (((FrameworkElement) f.Parent).FindName ("Hidden"), "#should not be findable");
		}

		[TestMethod]
		public void AttachedProp_IList_SetOnly_OnAttachedProperties ()
		{
			AttachedProperties.InstantiateLists = true;
			// Attached properties need a getter and setter
			Assert.Throws<XamlParseException> (() =>
				AttachedPropertiesCore<AttachedProperties> (() => AttachedProperties.IList_SetProperty)
			, "#1");
		}

		[TestMethod]
		public void AttachedProp_IList_SetOnly_OnRectangle ()
		{
			AttachedProperties.InstantiateLists = true;
			// Attached properties need a getter and setter
			Assert.Throws<XamlParseException> (() =>
				AttachedPropertiesCore<Rectangle> (() => AttachedProperties.IList_SetProperty)
			, "#1");
		}

		[TestMethod]
		public void AttachedProp_IList_GetAndSet_OnAttachedProperties ()
		{
			AttachedProperties.InstantiateLists = true;
			Assert.Throws<XamlParseException>(() =>
				AttachedPropertiesCore<AttachedProperties> (() => AttachedProperties.IList_GetSetProperty)
			, "#1");
		}

		[TestMethod]
		[MoonlightBug ("The rectangle in the IList should not have its name registered")]
		public void AttachedProp_IList_GetAndSet_OnRectangle ()
		{
			AttachedProperties.InstantiateLists = true;
			var f = AttachedPropertiesCore<Rectangle> (() => AttachedProperties.IList_GetSetProperty);
			var list = (List<FrameworkElement>) f.GetValue (AttachedProperties.IList_GetSetProperty);
			Assert.AreEqual (1, list.Count, "#1");
			Assert.IsInstanceOfType<Rectangle> (list [0], "#2");

			// The rectangle should not be parented already, so this won't throw an exception
			new Canvas ().Children.Add (list[0]);
			Assert.IsNull (((FrameworkElement) f.Parent).FindName ("Hidden"), "#should not be findable");
		}

		[TestMethod]
		public void AttachedPropWithManagedNamespace_NoTemplateOwner()
		{
			string xaml =
@"<DataTemplate 
	xmlns='http://schemas.microsoft.com/winfx/2006/xaml/presentation' 
	xmlns:local='clr-namespace:MoonTest.Misc.Parsing;assembly=moon-unit'>
	<local:CustomControl local:Canvas.Left='{Binding Left}'/>
</DataTemplate>";
			var template = (DataTemplate)XamlReader.Load(xaml);

			var child = (CustomControl)template.LoadContent();
			child.DataContext = new Person();
			Assert.AreEqual(100.0, child.GetValue(Canvas.LeftProperty), "#1");
		}

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
			Enqueue (() => Storyboard.SetTargetProperty (the_animation, new PropertyPath ("(monkey:HalfDimensionsControl.HalfHeight)")));
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

		[TestMethod]
		[MoonlightBug]
		public void StructsToDoubleTest ()
		{
			StackPanel c = (StackPanel) XamlReader.Load (@"
    <StackPanel xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
				xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
        <StackPanel.Resources>
            <Thickness x:Name=""my_thickness"">10,2,3,4</Thickness>
	    <FontStretch x:Name=""your_stretch"">Condensed</FontStretch>
       </StackPanel.Resources>
       <Rectangle x:Name=""my_rect"" Width=""{StaticResource your_stretch}"" Height=""{StaticResource my_thickness}"" />
    </StackPanel>");

			var rect = (Rectangle) c.FindName ("my_rect");
			// var stretch = (FontStretch) c.FindName ("your_stretch");

			Assert.AreEqual (10, rect.Height, "A1");
			// need a good way of checking these values, can't actually cast a struct to double
			// Assert.AreEqual ((double) stretch, rect.Width, "A1");
		}

		[TestMethod]
		public void EscapedExtensions ()
		{
			// Parse a binding which has escaped curly braces with escaped commas inside it
			Canvas canvas = (Canvas) XamlReader.Load(@"	
<Canvas	xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
Width=""100"" Height=""100"">
	<Canvas.Resources>
	</Canvas.Resources>
	<TextBlock x:Name=""text"" Text=""{Binding somedate, Converter={StaticResource DateTimeConverter}, ConverterParameter=\{0:MMMM d\, yyyy\}, Mode=OneWay}"" />
</Canvas>
");
			
			var beb = (BindingExpression) canvas.Children[0].ReadLocalValue (TextBlock.TextProperty);
			Assert.AreEqual ("{0:MMMM d, yyyy}", beb.ParentBinding.ConverterParameter, "#1");
		}

		[TestMethod]
		public void EscapedExtensions2 ()
		{
			// Parse a binding which uses escaped curly braces and has whitespace in it
			Canvas canvas = (Canvas) XamlReader.Load(@"	
<Canvas	xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
Width=""100"" Height=""100"">
	<Canvas.Resources>
	</Canvas.Resources>
	<TextBlock x:Name=""text"" Text=""{Binding somedate, Converter={StaticResource DateTimeConverter}, ConverterParameter=\{0:t\} ET, Mode=OneWay}"" />
</Canvas>
");

			var beb = (BindingExpression) canvas.Children [0].ReadLocalValue (TextBlock.TextProperty);
			Assert.AreEqual ("{0:t} ET", beb.ParentBinding.ConverterParameter, "#1");
		}

		[TestMethod]
		public void EscapedExtensions_CommaTerminated ()
		{
			// Parse a binding with spaces and escaped curly braces
			Canvas canvas = (Canvas) XamlReader.Load(@"	
<Canvas	xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"">
	<TextBlock Text=""{Binding Something, Converter={StaticResource SomeFormatter}, ConverterParameter=Click \{0\}, Mode=OneWay}"" />
</Canvas>
");
			var beb = (BindingExpression) canvas.Children [0].ReadLocalValue (TextBlock.TextProperty);
			Assert.AreEqual ("Click {0}", beb.ParentBinding.ConverterParameter, "#1");
		}

		[TestMethod]
		public void EscapedCharacters ()
		{
			// Parse a binding with escaped backslashes, escaped curly braces and escaped commas
			// with whitespace at the end
			Canvas canvas = (Canvas) XamlReader.Load (@"	
<Canvas	xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"">
	<TextBlock Text=""{Binding Something, Converter={StaticResource SomeFormatter}, ConverterParameter=\\a\{\, test , Mode=OneWay}"" />
</Canvas>
");
			var beb = (BindingExpression) canvas.Children [0].ReadLocalValue (TextBlock.TextProperty);
			Assert.AreEqual (@"\a{, test", beb.ParentBinding.ConverterParameter, "#1");
		}

		[TestMethod]
		public void QuotedString_EscapedQuotes ()
		{
			// Parse a binding with an escaped quote mark and whitespace at the end
			Canvas canvas = (Canvas) XamlReader.Load (@"	
<Canvas	xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"">
	<TextBlock Text=""{Binding Something, Converter={StaticResource SomeFormatter}, ConverterParameter='\'' , Mode=OneWay}"" />
</Canvas>
");
			var beb = (BindingExpression) canvas.Children [0].ReadLocalValue (TextBlock.TextProperty);
			Assert.AreEqual (@"'", beb.ParentBinding.ConverterParameter, "#1");
		}

		[TestMethod]
		public void QuotedString_EscapedBackslashesAndWhitespace ()
		{
			// Parse a binding with an escaped quote mark with whitespace inside it
			Canvas canvas = (Canvas) XamlReader.Load (@"	
<Canvas	xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"">
	<TextBlock Text=""{Binding Something, Converter={StaticResource SomeFormatter}, ConverterParameter='\\ \\ ' , Mode=OneWay}"" />
</Canvas>
");
			var beb = (BindingExpression) canvas.Children [0].ReadLocalValue (TextBlock.TextProperty);
			Assert.AreEqual (@"\ \ ", beb.ParentBinding.ConverterParameter, "#1");
		}

		[TestMethod]
		public void QuotedString_EscapedQuotesAndWhitespace ()
		{
			// Parse a binding with an escaped quote mark with whitespace inside it
			Canvas canvas = (Canvas) XamlReader.Load (@"	
<Canvas	xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"">
	<TextBlock Text=""{Binding Something, Converter={StaticResource SomeFormatter}, ConverterParameter='\' ' , Mode=OneWay}"" />
</Canvas>
");
			var beb = (BindingExpression) canvas.Children [0].ReadLocalValue (TextBlock.TextProperty);
			Assert.AreEqual (@"' ", beb.ParentBinding.ConverterParameter, "#1");
		}

		[TestMethod]
		public void QuotedString_UnescapedComma ()
		{
			// Parse a binding unescaped commas inside a quoted string with whitespace at the end
			Canvas canvas = (Canvas) XamlReader.Load (@"	
<Canvas	xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"">
	<TextBlock Text=""{Binding Something, ConverterParameter='dddd, MMMM, yyyy' , Mode=OneWay}"" />
</Canvas>
");
			var beb = (BindingExpression) canvas.Children [0].ReadLocalValue (TextBlock.TextProperty);
			Assert.AreEqual (@"dddd, MMMM, yyyy", beb.ParentBinding.ConverterParameter, "#1");
		}

		[TestMethod]
		public void QuotedString_UnescapedBraces ()
		{
			// parse a binding with unescaped curly braces and commas
			Canvas canvas = (Canvas) XamlReader.Load (@"	
<Canvas	xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"">
	<TextBlock Text=""{Binding Something, ConverterParameter=',}{ } }{' , Mode=OneWay}"" />
</Canvas>
");
			var beb = (BindingExpression) canvas.Children [0].ReadLocalValue (TextBlock.TextProperty);
			Assert.AreEqual (@",}{ } }{", beb.ParentBinding.ConverterParameter, "#1");
		}
	}
}
