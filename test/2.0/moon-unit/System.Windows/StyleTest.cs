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

namespace MoonTest.System.Windows
{
	[TestClass]
	public class StyleTest
	{
		public class T : Control
		{
			public static readonly DependencyProperty PropProperty = DependencyProperty.Register("Prop", typeof(int), typeof(T), null);
			public int Prop {
				get { throw new InvalidOperationException(); }
				set { throw new InvalidOperationException(); }
			}
		}
		
		[TestMethod]
		public void Sealed ()
		{
			Style style = new Style (typeof (UIElement));

			style.Seal ();
			
			// This should throw, no?
			/*Assert.Throws (delegate {*/ style.TargetType = typeof (FrameworkElement);/* }, typeof (Exception));*/

			// This too?
			/*Assert.Throws (delegate {*/ style.TargetType = typeof (SolidColorBrush);/* }, typeof (Exception));*/
		}



		[TestMethod]
		public void ApplyStyleToManagedDP()
		{
			T t = new T();
			Style s = new Style(typeof(T));
			s.Setters.Add(new Setter(T.PropProperty, 100));
			t.Style = s;
			Assert.AreEqual(100, t.GetValue(T.PropProperty));
		}
		
		[TestMethod]
		public void SetTwiceOnElement ()
		{
			Style style = new Style (typeof (Rectangle));
			Rectangle r = new Rectangle ();

			// FIXME: This should pass, but commenting it out so i can test the setting an element twice
			//Assert.IsTrue (double.IsNaN (r.Width));

			r.Style = style;
			Assert.Throws (delegate { r.Style = style; }, typeof (Exception));
		}

		[TestMethod]
		[MoonlightBug ("The XamlLoader should not call the managed properties when setting the value of 'Setter.Value'")]
		public void AvailableBeforeLoaded ()
		{
			Style s = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""Width"" Value=""10""/></Style>");
			Button b = new Button ();

			b.Style = s;
			Assert.AreEqual(typeof(Button), s.TargetType, "#0");
			Setter setter = (Setter)s.Setters[0];
			Assert.IsNull(setter.Property, "#1");
			Assert.AreEqual(null, setter.Value, "#2");

			Assert.AreEqual (10, b.Width);
		}

		[TestMethod]
		[MoonlightBug ("Managed properties cannot be found via native code, so styles can't be applied to them")]
		public void ModifyAfterBinding()
		{
			Button b = new Button();
			Assert.AreEqual(ClickMode.Release, b.ClickMode, "#a");

			Style style = new Style(typeof(Button));

			b.Style = style;
			Assert.Throws<Exception>(delegate {
				style.Setters.Add(new Setter());
			}, "#b");

			Assert.AreEqual(0, style.Setters.Count, "#c");
			Assert.AreEqual(ClickMode.Release, b.ClickMode, "#d");

			style.Setters.Add(new Setter(Button.ClickModeProperty, ClickMode.Press));
			Assert.AreEqual(1, style.Setters.Count, "#e");
			Assert.AreEqual(ClickMode.Release, b.ClickMode, "#f");

			b.ClearValue(Button.ClickModeProperty);
			Assert.AreEqual(ClickMode.Release, b.ClickMode, "#f2");

			b.ClickMode = ClickMode.Hover;
			b.ClearValue(Button.ClickModeProperty);
			Assert.AreEqual(ClickMode.Press, b.ClickMode, "#g");

			style.Setters.Clear();
			Assert.AreEqual(0, style.Setters.Count, "#h");
			Assert.AreEqual(ClickMode.Press, b.ClickMode, "#i");

			style = new Style(typeof(Button));
			style.Setters.Add(new Setter(Button.ClickModeProperty, ClickMode.Press));
			b = new Button();
			b.Style = style;
			b.ClearValue(Button.ClickModeProperty);
			Assert.AreEqual(b.ClickMode, ClickMode.Press, "#j");

			style.Setters.Clear();
			Assert.AreEqual(0, style.Setters.Count, "#k");
			Assert.AreEqual(b.ClickMode, ClickMode.Press, "#l");

			b.ClearValue(Button.ClickModeProperty);
			Assert.AreEqual(b.ClickMode, ClickMode.Press, "#m");
		}

		[TestMethod]
		public void InvalidValue()
		{
			Button b = new Button();
			Style style = new Style(typeof(Button));
			Setter setter = new Setter(Button.WidthProperty, "this is a string");
			b.Style = style;
			Assert.IsTrue(double.IsNaN(b.Width));
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
		[MoonlightBug]
		public void MismatchTargetType ()
		{
			Style s = (Style)XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""CheckBox""><Setter Property=""Width"" Value=""10""/></Style>");
			Button b = new Button ();

			Assert.Throws (delegate { b.Style = s; }, typeof (XamlParseException));
		}

		[TestMethod]
		[Ignore("On silverlight this seems to throw an uncatchable exception")]
		public void MissingTargetType ()
		{
			Assert.Throws<ExecutionEngineException>(delegate {
				XamlReader.Load(@"<Style xmlns=""http://schemas.microsoft.com/client/2007""><Setter Property=""Width"" Value=""10""/></Style>");
			});
		}

		[TestMethod]
		public void InvalidPropertyNameInSetter ()
		{
			Assert.Throws (delegate { XamlReader.Load (@"<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Button""><Setter Property=""WidthOrHeight"" Value=""10""/></Style>"); }, typeof (XamlParseException));
		}

		[TestMethod]
		[Ignore("On silverlight this seems to throw an uncatchable exception")]
		public void InvalidPropertyNameInSetterMissingTargetType()
		{
			Assert.Throws<ExecutionEngineException>(delegate {
				XamlReader.Load(@"<Style xmlns=""http://schemas.microsoft.com/client/2007""><Setter Property=""WidthOrHeight"" Value=""10""/></Style>");
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
			s1.Setters.Clear();
			s2.Setters.Add(setter);
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
	}
}