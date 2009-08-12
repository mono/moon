using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
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
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows
{
	public class StyledControl : Control
	{
		public static readonly DependencyProperty PropProperty = DependencyProperty.Register("Prop", typeof(int), typeof(StyledControl), null);
		public int Prop {
			get { throw new InvalidOperationException(); }
			set { throw new InvalidOperationException(); }
		}
	}

	public class StyledPanel : Panel
	{
		public static readonly DependencyProperty AttachedPropProperty = DependencyProperty.RegisterAttached("AttachedProp", typeof(int), typeof(StyledPanel), null);
		public int AttachedProp {
			get { throw new InvalidOperationException(); }
			set { throw new InvalidOperationException(); }
		}

		public static void SetAttachedProp (DependencyObject obj, int value)
		{
			obj.SetValue (AttachedPropProperty, value);
		}

		public static int GetAttachedProp (DependencyObject obj)
		{
			return (int)obj.GetValue (AttachedPropProperty);
		}
	}
		
	[TestClass]
	public partial class StyleTest : SilverlightTest
	{
		// NOTE: This test must be run first for it to work - do not remove the 'aaa' from the start of its name
		[TestMethod]
		[MoonlightBug ("Setter.Property should be null until you create a Setter in code using the same DP")]
		public void aaaConstructStyleProgrammatically ()
		{
			Style s = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""Width"" Value=""10""/></Style>");
			Setter setter = (Setter) s.Setters [0];
			Assert.IsNull (setter.Property, "#1"); // Fails in Silverlight 3
			
			new Setter (Button.WidthProperty, 10);
			
			s = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""Width"" Value=""10""/></Style>");
			setter = (Setter) s.Setters [0];
			Assert.IsNotNull (setter.Property);
		}

		[TestMethod]
		public void ApplyDefaultStyle ()
		{
			// Default style is applied when the element is added to the visual tree
			Slider s = new Slider ();
			Assert.AreEqual (1, s.Maximum, "#1");
			TestPanel.Children.Add (s);
			Assert.AreEqual (10, s.Maximum, "#2");
		}

		[TestMethod]
		[Asynchronous]
		public void ApplyDefaultStyle2 ()
		{
			// Default style is applied even if a regular style is applied
			Style style = new Style { TargetType = typeof (Slider) };
			style.Setters.Add (new Setter (Slider.BorderThicknessProperty, new Thickness (2, 2, 2, 2)));

			Slider s = new Slider { Style = style };
			Assert.AreEqual (1, s.Maximum, "#1");
			CreateAsyncTest (s, () => {
				Assert.AreEqual (10, s.Maximum, "#2");
			});
		}

		[TestMethod]
		public void ApplyDefaultStyle3 ()
		{
			//  Default style is not applied during the measure phase
			Slider s = new Slider ();
			s.Measure (new Size (100, 100));
			Assert.AreEqual (1, s.Maximum, "#1");
			TestPanel.Children.Add (s);
			Assert.AreEqual (10, s.Maximum, "#2");
		}

		[TestMethod]
		public void Sealed ()
		{
			Style style = new Style (typeof (UIElement));
			Assert.IsFalse (style.IsSealed, "Style.IsSealed-1");
			Assert.IsFalse (style.Setters.IsSealed, "Style.Setters.IsSealed-1");

			style.Seal ();
			Assert.IsTrue (style.IsSealed, "Style.IsSealed-2");
			Assert.IsTrue (style.Setters.IsSealed, "Style.Setters.IsSealed-2");

			// TargetType is not "sealed" and can be modified
			style.TargetType = typeof (FrameworkElement);
			style.TargetType = typeof (SolidColorBrush);

			Setter setter = new Setter (Rectangle.HeightProperty, "50");
			Assert.IsFalse (setter.IsSealed, "Setter.IsSealed-2");

			Assert.Throws<Exception> (delegate {
				style.Setters.Add (setter);
			}, "can't add to sealed style");
		}

		[TestMethod]
		[MoonlightBug ("SL2 gives different results if we check IsSealed before sealing a style")]
		public void Sealed_CacheIssue ()
		{
			Style style = new Style (typeof (UIElement));

			style.Seal ();

			// TargetType is not "sealed" and can be modified
			style.TargetType = typeof (FrameworkElement);
			style.TargetType = typeof (SolidColorBrush);

			Assert.IsTrue (style.IsSealed, "Style.IsSealed-1");
			Assert.AreEqual (0, style.Setters.Count, "Setters.Count-1");

			Setter setter = new Setter (Rectangle.HeightProperty, "50");
			Assert.IsFalse (setter.IsSealed, "Setter.IsSealed-1");

			// Since we *never* checked that Setters.IsSealed is true then it's not !?!
			// and we can add a new setter to the sealed style
			style.Setters.Add (setter);
			Assert.AreEqual (1, style.Setters.Count, "Setters.Count-2");
			Assert.IsTrue (setter.IsSealed, "Setter.IsSealed-2");
		}

		[TestMethod]
		public void ApplyStyleToManagedDP()
		{
			StyledControl t = new StyledControl();
			Style s = new Style(typeof(StyledControl));
			s.Setters.Add(new Setter(StyledControl.PropProperty, 100));
			t.Style = s;
			Assert.AreEqual(100, t.GetValue(StyledControl.PropProperty));
		}
		
		[TestMethod]
		public void ColorAsString ()
		{
			Style s = new Style (typeof (Rectangle));
			s.Setters.Add (new Setter { Property = Rectangle.FillProperty, Value = "#FFEEDD55" });
			Rectangle r = new Rectangle { Style = s };
			Assert.IsInstanceOfType (r.Fill, typeof (SolidColorBrush), "#1");
			Assert.AreEqual (r.Fill.GetValue (SolidColorBrush.ColorProperty).ToString(), "#FFEEDD55", "#2");
		}

		
		[TestMethod]
		public void SetTwiceOnElement ()
		{
			Style style = new Style (typeof (Rectangle));
			Rectangle r = new Rectangle ();

			// FIXME: This should pass, but commenting it out so i can test the setting an element twice
			Assert.IsTrue (double.IsNaN (r.Width));

			r.Style = style;
			Assert.Throws (delegate { r.Style = style; }, typeof (Exception)); // Fails in Silverlight 3
		}

		[TestMethod]
		[MoonlightBug ("The XamlLoader should not call the managed properties when setting the value of 'Setter.Value'")]
		public void ManagedAccessAfterParsing ()
		{
			Style s = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""Width"" Value=""10""/></Style>");
			Button b = new Button ();

			b.Style = s;
			Assert.AreEqual(typeof(Button), s.TargetType, "#0");
			Setter setter = (Setter)s.Setters[0];
			Assert.IsNotNull (setter.Property, "#1");
			Assert.IsNull (setter.Value, "#2");

			Assert.AreEqual (10, b.Width);
		}

		[TestMethod]
		public void ModifyAfterBinding()
		{
			Button b = new Button();
			Assert.AreEqual(ClickMode.Release, b.ClickMode, "#a");

			Style style = new Style(typeof(Button));

			Assert.IsFalse (style.IsSealed, "Style-IsSealed-1");
			Assert.IsFalse (style.Setters.IsSealed, "Setters-IsSealed-1");
			b.Style = style;
			Assert.IsTrue (style.IsSealed, "Style-IsSealed-2");
			Assert.IsTrue (style.Setters.IsSealed, "Setters-IsSealed-2");

			Assert.Throws<Exception> (delegate {
				style.Setters.Add(new Setter());
			}, "#b");

			Assert.AreEqual(0, style.Setters.Count, "#c");
			Assert.AreEqual(ClickMode.Release, b.ClickMode, "#d");

			// This always fails on SL2 ***because*** we checked the IsSealed properties
			// before getting here - otherwise it would execute fine. There's likely a
			// caching issue inside SL2 code
			Assert.Throws<Exception> (delegate {
				style.Setters.Add (new Setter (Button.ClickModeProperty, ClickMode.Press));
			}, "caching issue");
			Assert.AreEqual (0, style.Setters.Count, "#e");
		}

		[TestMethod]
		[MoonlightBug ("Exception should be thrown because Width needs a double, not string")]
		public void InvalidValueProgrammatically()
		{
			Button b = new Button();
			Style style = new Style(typeof(Button));
			style.Setters.Add (new Setter(Button.WidthProperty, "this is a string"));
			Assert.Throws<Exception> (delegate {
				b.Style = style;
			});
		}

		[TestMethod]
		public void ProgrammaticTypeConverterFromStringTest()
		{
			Button b = new Button();
			Style style = new Style(typeof(Button));
			style.Setters.Add (new Setter(Button.WidthProperty, "10"));
			b.Style = style;
			Assert.AreEqual (10, b.Width, "1");
		}

		[TestMethod]
		[MoonlightBug ("TypeConverter stuff only works in one direction: from string -> type")]
		public void ProgrammaticTypeConverterToStringTest()
		{
			Button b = new Button();
			Style style = new Style(typeof(Button));
			style.Setters.Add (new Setter(FrameworkElement.TagProperty, 10));

			Assert.Throws<ArgumentException>(delegate {
				b.Style = style;
			});

			b = new Button ();
			style = new Style (typeof (Button));
			style.Setters.Add (new Setter(FrameworkElement.TagProperty, new Button()));

			Assert.Throws<ArgumentException>(delegate {
				b.Style = style;
			});
			
			b = new Button ();
			style = new Style (typeof (Button));
			style.Setters.Add (new Setter (FrameworkElement.TagProperty, "str"));
			b.Style = style;
		}

		[TestMethod]
		[MoonlightBug ("we don't have type converter handling yet")]
		public void ParsedTypeConverterToStringTest()
		{
			Style style = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""Tag""><Setter.Value><Button /></Setter.Value></Setter></Style>");

			Button b = new Button();

			Assert.Throws<ArgumentException>(delegate {
				b.Style = style;
			});
		}

		[TestMethod]
		[MoonlightBug ("DP lookup isn't working")]
		public void InvalidValueParsed ()
		{
			Style style = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""Width"" Value=""this is a string""/></Style>");

			Button b = new Button();

			Assert.Throws<Exception>(delegate {
				b.Style = style;
			});
		}

		[TestMethod]
		public void NullLocalValue ()
		{
			Brush blue = new SolidColorBrush(Colors.Blue);
			Brush red = new SolidColorBrush(Colors.Red);
			Style style = new Style(typeof(Canvas));
			style.Setters.Add(new Setter(Canvas.BackgroundProperty, red));
			Canvas c = new Canvas();
			c.Background = blue;
			c.Style = style;
			Assert.AreEqual(blue, c.Background, "#1");
			c.Background = null;
			Assert.AreEqual(null, c.Background, "#2");
			c.ClearValue(Canvas.BackgroundProperty);
			Assert.AreEqual(red, c.Background, "#3");
		}
		
		[TestMethod]
		public void MismatchTargetType ()
		{
			Button b = new Button ();
			Style s = new Style (typeof (CheckBox));
			Assert.Throws<XamlParseException> (delegate { b.Style = s; }, "#1");
			
			s = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""CheckBox""><Setter Property=""Width"" Value=""10""/></Style>");
			b = new Button ();
			Assert.Throws<XamlParseException> (delegate { b.Style = s; }, "#2");
		}

		[TestMethod]
		public void InvalidPropertyNameInSetter ()
		{
			Assert.Throws<XamlParseException> (delegate { XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""WidthOrHeight"" Value=""10""/></Style>"); });

			Assert.Throws<XamlParseException> (delegate { XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""TargetType"" Value=""10""/></Style>"); });
		}

		[TestMethod]
		[Ignore("On silverlight this seems to throw an uncatchable exception")]
		public void ParsedMissingTargetType ()
		{
			Assert.Throws<ExecutionEngineException>(delegate {
				XamlReader.Load(@"<Style xmlns=""http://schemas.microsoft.com/client/2007""><Setter Property=""Width"" Value=""10""/></Style>");
			});

			Assert.Throws<ExecutionEngineException>(delegate {
				XamlReader.Load(@"<Style xmlns=""http://schemas.microsoft.com/client/2007""><Setter Property=""WidthOrHeight"" Value=""10""/></Style>");
			});
		}

		[TestMethod]
		public void ProgramMissingTargetType ()
		{
			Style s = new Style ();
			Button b = new Button ();

			Assert.Throws<NullReferenceException>(delegate {
				b.Style = s;
			}); // Fails in Silverlight 3 (got InvalidOperationException)
		}

		[TestMethod]
		public void UseSetterTwice()
		{
			Style s1 = new Style(typeof(Rectangle));
			Style s2 = new Style(typeof(Rectangle));
			Setter setter = new Setter(Rectangle.WidthProperty, 5);
			s1.Setters.Add(setter);
			Assert.Throws<InvalidOperationException>(delegate {
				s2.Setters.Add(setter);
			});

			Assert.IsFalse (s1.Setters.IsSealed, "Setters.IsSealed-before-clear");
			Assert.IsTrue (setter.IsSealed, "Setter.IsSealed-before-clear");
			s1.Setters.Clear();
			Assert.IsFalse (s1.Setters.IsSealed, "Setters.IsSealed-after-clear");
			Assert.IsTrue (setter.IsSealed, "Setter.IsSealed-after-clear");
			// i.e. a Setter can be reused even if sealed
			s2.Setters.Add (setter);
		}

		public void LoadFromXaml ()
		{
			Button b = (Button)XamlReader.Load(@"
<Button xmlns=""http://schemas.microsoft.com/client/2007"" >
	<Button.Style>
		<Style TargetType=""Button"">
			<Setter Property=""Width"" Value=""10"" />
		</Style>
	</Button.Style>
</Button>");

			Assert.IsTrue(b.Style.IsSealed);
		}

		[TestMethod]
		public void StyleXaml ()
		{
			Thumb t = (Thumb) XamlReader.Load (@"
<Thumb xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<Thumb.Style>
		<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Control"">
			<Setter Property=""Tag"" Value=""Test""/>
		</Style>
	</Thumb.Style>
</Thumb>");
			Assert.IsNotNull (t, "Thumb");
			Assert.IsTrue (t.Style.IsSealed, "IsSealed");
			Assert.AreEqual (1, t.Style.Setters.Count, "Setters");
			Assert.AreEqual (typeof (Control), t.Style.TargetType, "TargetType");
		}

		[TestMethod]
		public void StyleCustomProperty ()
		{
			// a valid value
			XamlReader.Load(@"
		<Style xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:T=""clr-namespace:MoonTest.System.Windows;assembly=moon-unit"" TargetType=""T:StyledControl"">
			<Setter Property=""Prop"" Value=""5""/>
		</Style>");

			// an invalid value
			XamlReader.Load(@"
		<Style xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:T=""clr-namespace:MoonTest.System.Windows;assembly=moon-unit"" TargetType=""T:StyledControl"">
			<Setter Property=""Prop"" Value=""this is a string""/>
		</Style>");
		}

		[TestMethod]
		public void StyleAttachedProperty()
		{
			Assert.Throws<XamlParseException> (delegate { XamlReader.Load(@"
<Rectangle xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<Rectangle.Style>
		<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Rectangle"">
			<Setter Property=""Left"" Value=""5""/>
		</Style>
	</Rectangle.Style>
</Rectangle>"); });

			// try using the (property.path) syntax
			Assert.Throws<XamlParseException> (delegate { XamlReader.Load(@"
<Rectangle xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<Rectangle.Style>
		<Style xmlns=""http://schemas.microsoft.com/client/2007"" x:Key=""Foo"" TargetType=""Rectangle"">
			<Setter Property=""(Canvas.Left)"" Value=""5""/>
		</Style>
	</Rectangle.Style>
</Rectangle>"); });

			// try using the Type.Property syntax. this one works.
			XamlReader.Load(@"
<Rectangle xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<Rectangle.Style>
		<Style xmlns=""http://schemas.microsoft.com/client/2007"" x:Key=""Foo"" TargetType=""Rectangle"">
			<Setter Property=""Canvas.Left"" Value=""5""/>
		</Style>
	</Rectangle.Style>
</Rectangle>");


			// add a couple of tests for custom attached properties which apparently can't be styled
			Assert.Throws<XamlParseException> (delegate { XamlReader.Load(@"
<Rectangle xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<Rectangle.Style>
		<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Rectangle"">
			<Setter Property=""AttachedProp"" Value=""5""/>
		</Style>
	</Rectangle.Style>
</Rectangle>"); }, "Custom Attached Property #1");

			Assert.Throws<XamlParseException> (delegate { XamlReader.Load(@"
<Rectangle xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" xmlns:T=""clr-namespace:MoonTest.System.Windows;assembly=moon-unit"">
	<Rectangle.Style>
		<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Rectangle"">
			<Setter Property=""(T:StyledPanel.AttachedProp)"" Value=""5""/>
		</Style>
	</Rectangle.Style>
</Rectangle>"); }, "Custom Attached Property #2");

			// this one works, though
			XamlReader.Load(@"
<Rectangle xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" xmlns:T=""clr-namespace:MoonTest.System.Windows;assembly=moon-unit"">
	<Rectangle.Style>
		<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Rectangle"">
			<Setter Property=""T:StyledPanel.AttachedProp"" Value=""5""/>
		</Style>
	</Rectangle.Style>
</Rectangle>");

			Assert.Throws<XamlParseException> (delegate { XamlReader.Load(@"
<Rectangle xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" xmlns:T=""clr-namespace:MoonTest.System.Windows;assembly=moon-unit"">
	<Rectangle.Style>
		<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Rectangle"">
			<Setter Property=""T:AttachedProp"" Value=""5""/>
		</Style>
	</Rectangle.Style>
</Rectangle>"); }, "Custom Attached Property #3");
		}

		[TestMethod]
		public void Seal ()
		{
			Setter setter = new Setter (UIElement.OpacityProperty, 2.0);
			Style s = new Style ();
			Assert.IsFalse (s.IsSealed, "Style.IsSealed-1");
			Assert.IsFalse (s.Setters.IsSealed, "Setters.IsSealed-1");
			Assert.IsFalse (setter.IsSealed, "Setter.IsSealed-1");
			s.Setters.Add (setter);
			Assert.IsFalse (s.IsSealed, "Style.IsSealed-2");
			Assert.IsFalse (s.Setters.IsSealed, "Setters.IsSealed-2");
			Assert.IsTrue (setter.IsSealed, "Setter.IsSealed-2");
			s.Seal ();
			Assert.IsTrue (s.IsSealed, "Style.IsSealed-3");
			Assert.IsTrue (s.Setters.IsSealed, "Setters.IsSealed-3");
			Assert.IsTrue (setter.IsSealed, "Setter.IsSealed-3");
		}

		void SealEmptySetters (Style s)
		{
			Assert.IsFalse (s.IsSealed, "Style.IsSealed-1");
			Assert.IsFalse (s.Setters.IsSealed, "Setters.IsSealed-1");
			Assert.AreEqual (0, s.Setters.Count, "Setters.Count");
			s.Seal ();
			Assert.IsTrue (s.IsSealed, "Style.IsSealed-2");
			Assert.IsTrue (s.Setters.IsSealed, "Setters.IsSealed-2");
		}

		[TestMethod]
		public void SealEmptySetters_NoType ()
		{
			SealEmptySetters (new Style ());
		}

		[TestMethod]
		public void SealEmptySetters_WithType ()
		{
			SealEmptySetters (new Style (typeof (Rectangle)));
		}
	}
}
