using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Markup;
using System.Windows.Shapes;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows
{
	[TestClass]
	public partial class SetterTest {

		[TestMethod]
		public void Defaults ()
		{
			Setter s = new Setter ();
			Assert.IsNull (s.Property, "Property");
			Assert.IsNull (s.Value, "Value");
			// SetterBase (can't be tested indivisually since the type has no visible ctor)
			Assert.IsFalse (s.IsSealed, "IsSealed");
		}

		[TestMethod]
		public void CreateTest ()
		{
			Setter s = new Setter (UIElement.OpacityProperty, 2.0);

			Assert.AreEqual (UIElement.OpacityProperty, s.Property);
			Assert.AreEqual (2.0, s.Value);
		}

		[TestMethod]
		public void NullProperty ()
		{
			Assert.Throws<NullReferenceException> (delegate { 
				new Setter (null, 2.0); 
			}, "ctor");

			Setter s = new Setter (UIElement.OpacityProperty, 2.0);
			Assert.Throws<NullReferenceException> (delegate {
				s.Property = null;
			}, "Property");
		}

		[TestMethod]
		public void TypeMismatch ()
		{
			Setter s = new Setter (UIElement.OpacityProperty, "does this work?");
			Assert.AreEqual (UIElement.OpacityProperty, s.Property, "Property");
			Assert.AreEqual ("does this work?", s.Value, "Value");
		}

		[TestMethod]
		public void StyleOfDifferentType ()
		{
			Setter s = new Setter (Line.X1Property, 10.0);
			Style style = new Style (typeof (Rectangle));
			style.Setters.Add (s);
		}

		[TestMethod]
		[Ignore ("Throws ExecutionEngineException and abort the process")]
		public void Parse ()
		{
			Setter s = (Setter)XamlReader.Load ("<Setter xmlns=\"http://schemas.microsoft.com/client/2007\" Property=\"IsEnabled\" Value=\"hi\" />");
			Assert.IsNull (s.Property);
			Assert.AreEqual ("hi", s.Value);
		}

		[TestMethod]
		[MoonlightBug ("unsure why SL2 Property is null")]
		public void ParseWithinStyle ()
		{
			Style style = (Style) XamlReader.Load (@"
<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Control"">
	<Setter Property=""IsEnabled"" Value=""hi"" />
</Style>");
			Assert.IsFalse (style.IsSealed, "Style.IsSealed");
			Assert.AreEqual (1, style.Setters.Count, "Style.Setters");
			Assert.AreEqual (typeof (Control), style.TargetType, "Style.TargetType");

			Setter s = (Setter) style.Setters [0];
			Assert.IsNull (s.Property, "Property");
			Assert.AreEqual ("hi", s.Value, "Value");
			Assert.IsTrue (s.IsSealed, "IsSealed");
		}

		[TestMethod]
		[MoonlightBug ("unsure why SL2 Property is null")]
		public void ParseWithinResourceDictionary ()
		{
			ResourceDictionary rd = (ResourceDictionary) XamlReader.Load (@"
<ResourceDictionary xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<Style x:Key=""mystyle"" TargetType=""Control"">
		<Setter Property=""IsEnabled"" Value=""hi"" />
	</Style>
</ResourceDictionary>");
			Assert.AreEqual (1, rd.Count, "Count");
			Assert.IsFalse (rd.IsReadOnly, "IsReadOnly");

			Style style = (Style) rd ["mystyle"];
			Assert.IsFalse (style.IsSealed, "Style.IsSealed");
			Assert.AreEqual (1, style.Setters.Count, "Style.Setters");
			Assert.AreEqual (typeof (Control), style.TargetType, "Style.TargetType");

			Setter s = (Setter) style.Setters [0];
			Assert.IsNull (s.Property, "Property");
			Assert.AreEqual ("hi", s.Value, "Value");
			Assert.IsTrue (s.IsSealed, "IsSealed");
		}

		[TestMethod]
		public void ModifyIsSealed ()
		{
			Style style = (Style) XamlReader.Load (@"
<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Control"">
	<Setter Property=""IsEnabled"" Value=""hi"" />
</Style>");
			Setter s = (Setter) style.Setters [0];
			Assert.IsTrue (s.IsSealed, "IsSealed");

			Assert.Throws<UnauthorizedAccessException> (delegate {
				s.Property = UIElement.OpacityProperty;
			}, "UIElement.OpacityProperty");

			Assert.Throws<UnauthorizedAccessException> (delegate {
				s.Value = s.Value;
			}, "self");
		}

		[TestMethod]
		[Ignore ("Throws ExecutionEngineException and abort the process")]
		public void ParseAndAddToStyle ()
		{
			Setter s = (Setter)XamlReader.Load ("<Setter xmlns=\"http://schemas.microsoft.com/client/2007\" Property=\"Width\" Value=\"5.0\" />");

			Assert.IsNull (s.Property);
			Assert.AreEqual ("5.0", s.Value);

			Style style = new Style(typeof (Rectangle));
			style.Setters.Add (s);

			Assert.IsNull (s.Property);
			Assert.AreEqual ("5.0", s.Value);

			Rectangle r = new Rectangle ();
			r.Style = style;

			Assert.IsNull (s.Property);
			Assert.AreEqual ("5.0", s.Value);
			Assert.IsTrue (Double.IsNaN(r.Width));
		}

		[TestMethod]
		[MoonlightBug ("unsure why SL2 Property is null")]
		public void ParseAndAddToStyle_WithinStyle ()
		{
			// Fails in Silverlight 3
			Style style = (Style) XamlReader.Load (@"
<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Control"">
	<Setter Property=""Width"" Value=""5.0"" />
</Style>");
			Assert.IsFalse (style.IsSealed, "Style.IsSealed");
			Assert.AreEqual (1, style.Setters.Count, "Style.Setters");
			Assert.AreEqual (typeof (Control), style.TargetType, "Style.TargetType");

			Setter s = (Setter) style.Setters [0];
			Assert.IsNull (s.Property);
			Assert.AreEqual ("5.0", s.Value);
		}

		[TestMethod]
		public void ReuseSetter ()
		{
			Style style = (Style) XamlReader.Load (@"
<Style xmlns=""http://schemas.microsoft.com/client/2007"" TargetType=""Control"">
	<Setter Property=""Width"" Value=""5.0"" />
</Style>");
			Assert.IsFalse (style.IsSealed, "Style.IsSealed");
			Assert.AreEqual (1, style.Setters.Count, "Style.Setters");
			Assert.AreEqual (typeof (Control), style.TargetType, "Style.TargetType");

			Setter s = (Setter) style.Setters [0];

			style = new Style (typeof (Rectangle));
			Assert.Throws<InvalidOperationException> (delegate {
				style.Setters.Add (s);
			}, "Setter already a child");
		}
	}
}
