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
		[TestMethod]
		public void aaaConstructStyleProgrammatically ()
		{
			Style s = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""Width"" Value=""10""/></Style>");
			Setter setter = (Setter) s.Setters [0];
			Assert.AreSame (Canvas.WidthProperty, setter.Property, "#1");
			
			new Setter (Button.WidthProperty, 10);
			
			s = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""Width"" Value=""10""/></Style>");
			setter = (Setter) s.Setters [0];
			Assert.IsNotNull (setter.Property);
		}

		[TestMethod]
		public void AddIncompleteSetter ()
		{
			Style s = new Style (typeof (Rectangle));
			Assert.Throws<Exception> (() => s.Setters.Add (new Setter ()), "#1");
			Assert.Throws<Exception> (() => s.Setters.Add (new Setter { Value = 5 }), "#2");
			Assert.Throws<Exception> (() => s.Setters.Add (new Setter { Property = Rectangle.WidthProperty }), "#3");

			Assert.Throws<NullReferenceException> (() => s.Setters.Add (new Setter { Property = null, Value = 5 }), "#4");

			// this one should succeed
			s.Setters.Add (new Setter { Property = Rectangle.FillProperty, Value = null });
		}

		[TestMethod]
		[MoonlightBug ("we fail the x:Null as value test")]
		public void ParseIncompleteSetter ()
		{
			// missing Value attribute
			Assert.Throws<XamlParseException> ( () => XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Rectangle""><Setter Property=""Width"" /></Style>"), "#1");

			// missing Property attribute
			Assert.Throws<XamlParseException> ( () => XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Rectangle""><Setter Value=""10"" /></Style>"), "#2");

			// x:Null as value
			Assert.Throws<XamlParseException> ( () => XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Rectangle""><Setter Property=""Fill"" Value=""{x:Null}"" /></Style>"), "#3");

			// x:Null as property
			Assert.Throws<XamlParseException> ( () => XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Rectangle""><Setter Property=""{x:Null}"" Value=""10"" /></Style>"), "#4");
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
		public void BasedOnOverride ()
		{
			// The value in 'first' overrides the one defined in 'second'
			Rectangle rect = new Rectangle ();
			Style first = new Style (typeof (Rectangle));
			first.Setters.Add (new Setter (Rectangle.WidthProperty, 10));

			Style second = new Style (typeof (Rectangle));
			second.Setters.Add (new Setter (Rectangle.WidthProperty, 100));

			first.BasedOn = second;
			rect.Style = first;
			Assert.AreEqual (10, rect.Width, "#1");
		}

		[TestMethod]
		public void BasedOnOverride2 ()
		{
			// The style 'second' is not in the tree so it's ignored
			Rectangle rect = new Rectangle ();
			Style first = new Style (typeof (Rectangle));
			first.Setters.Add (new Setter (Rectangle.WidthProperty, 10));

			Style second = new Style (typeof (Rectangle));
			second.Setters.Add (new Setter (Rectangle.WidthProperty, 100));

			second.BasedOn = first;
			rect.Style = first;
			Assert.AreEqual (10, rect.Width, "#1");
		}

		[TestMethod]
		public void BasedOn_FindName_UseOnce()
		{
			BasedOn_FindName_Core(false);
		}

		[TestMethod]
		[MoonlightBug ("We should not register the name in this case")]
		public void BasedOn_FindName_UseTwice()
		{
			BasedOn_FindName_Core(true);
		}

		void BasedOn_FindName_Core(bool useTwice)
		{
			Style s1 = new Style(typeof(Grid));
			Style s2 = new Style(typeof(Grid));
			Style basedon = new Style(typeof(Grid));

			s1.SetValue(FrameworkElement.NameProperty, "s1");
			s2.SetValue(FrameworkElement.NameProperty, "s2");
			basedon.SetValue(FrameworkElement.NameProperty, "basedon");

			s1.BasedOn = basedon;
			if (useTwice)
				s2.BasedOn = basedon;
			TestPanel.Children.Add(new Grid { Name = "Grid2", Style = s2 });
			TestPanel.Children.Add(new Grid { Name = "Grid1", Style = s1 });

			Assert.IsInstanceOfType<Style>(TestPanel.FindName("s1"), "#1");
			Assert.IsInstanceOfType<Style>(TestPanel.FindName("s2"), "#2");
			if (useTwice)
				Assert.IsNull(TestPanel.FindName("basedon"), "#3");
			else
				Assert.AreSame(s1.BasedOn, TestPanel.FindName("basedon"), "#4");
		}

		[TestMethod]
		public void BasedOnSealed ()
		{
			// The value in 'first' overrides the one defined in 'second'
			Style first = new Style (typeof (Rectangle));
			first.Setters.Add (new Setter (Rectangle.WidthProperty, 10));

			Style second = new Style (typeof (Rectangle));
			second.Setters.Add (new Setter (Rectangle.WidthProperty, 100));

			first.BasedOn = second;

			Assert.IsFalse (first.IsSealed, "#1");
			Assert.IsFalse (first.Setters.IsSealed, "#2");
			Assert.IsFalse (second.IsSealed, "#3");
			Assert.IsFalse (second.Setters.IsSealed, "#4");
			new Rectangle { Style = first };
			Assert.IsTrue (first.IsSealed, "#5");
			Assert.IsTrue (first.Setters.IsSealed, "#6");
			Assert.IsTrue (second.IsSealed, "#7");
			Assert.IsTrue (second.Setters.IsSealed, "#8");
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
			// We can now set the style twice on one element.
			Rectangle r = new Rectangle ();
			r.Style = new Style (typeof (Rectangle));
			r.Style = new Style (typeof (Rectangle));
		}

		[TestMethod]
		public void ReplaceStyleOnElement ()
		{
			Rectangle r = new Rectangle ();

			Style style = new Style (typeof (Rectangle));
			style.Setters.Add (new Setter (FrameworkElement.WidthProperty, 100));

			Style style2 = new Style (typeof (Rectangle));
			style2.Setters.Add (new Setter (FrameworkElement.HeightProperty, 100));

			r.Style = style;
			Assert.AreEqual (100, r.Width, "#1");
			Assert.IsTrue (double.IsNaN (r.Height), "#2");

			// First style is replaced by the new style
			r.Style = style2;
			Assert.IsTrue (double.IsNaN (r.Width), "#3");
			Assert.AreEqual (100, r.Height, "#4");
		}

		[TestMethod]
		[MaxRuntimeVersion(3)]
		[MoonlightBug ("we're failing #2")]
		public void ManagedAccessAfterParsing_sl3 ()
		{
			Style s = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""Width"" Value=""10""/></Style>");
			Button b = new Button ();

			b.Style = s;
			Assert.AreEqual(typeof(Button), s.TargetType, "#0");
			Setter setter = (Setter)s.Setters[0];
			Assert.IsNotNull (setter.Property, "#1");
			Assert.IsNull (setter.Value, "#2");
		}

		[TestMethod]
		[MinRuntimeVersion(4)]
		public void ManagedAccessAfterParsing_sl4 ()
		{
			Style s = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""Width"" Value=""10""/></Style>");
			Button b = new Button ();

			b.Style = s;
			Assert.AreEqual(typeof(Button), s.TargetType, "#0");
			Setter setter = (Setter)s.Setters[0];
			Assert.IsNotNull (setter.Property, "#1");
			Assert.IsInstanceOfType<double> (setter.Value, "#2");
			Assert.AreEqual (10.0, setter.Value, "#3");

			Assert.AreEqual (10.0, b.Width, "#4");
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
		public void InvalidValueProgrammatically()
		{
			Button b = new Button();
			Style style = new Style(typeof(Button));
			style.Setters.Add (new Setter(Button.WidthProperty, "this is a string"));
			b.Style = style;
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

			b.Style = style;
			Assert.IsNull (b.Tag, "#1");

			b = new Button ();
			style = new Style (typeof (Button));
			style.Setters.Add (new Setter(FrameworkElement.TagProperty, new Button()));

			b.Style = style;
			Assert.IsNull(b.Tag, "#2");

			b = new Button ();
			style = new Style (typeof (Button));
			style.Setters.Add (new Setter (FrameworkElement.TagProperty, "str"));

			b.Style = style;
			Assert.AreEqual("str", b.Tag, "#3");
		}

		[TestMethod]
		[MoonlightBug ("we don't have type converter handling yet")]
		public void ParsedTypeConverterToStringTest()
		{
			Style style = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""Tag""><Setter.Value><Button /></Setter.Value></Setter></Style>");

			Button b = new Button();
			b.Style = style;
			Assert.IsNull(b.Tag, "#1");
		}

		[TestMethod]
		public void InvalidValueParsed ()
		{
			Style style = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""Width"" Value=""this is a string""/></Style>");

			Button b = new Button();
			b.Style = style;
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
			Console.WriteLine (1);
			c.Style = style;
			Console.WriteLine (2);
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
		public void ParsedMissingTargetType ()
		{
			Assert.Throws<XamlParseException>(delegate {
				XamlReader.Load(@"<Style xmlns=""http://schemas.microsoft.com/client/2007""><Setter Property=""Width"" Value=""10""/></Style>");
			});

			Assert.Throws<XamlParseException>(delegate {
				XamlReader.Load(@"<Style xmlns=""http://schemas.microsoft.com/client/2007""><Setter Property=""WidthOrHeight"" Value=""10""/></Style>");
			});
		}

		[TestMethod]
		public void ProgramMissingTargetType ()
		{
			Style s = new Style ();
			Button b = new Button ();

			Assert.Throws<InvalidOperationException>(delegate {
				b.Style = s;
			});
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
