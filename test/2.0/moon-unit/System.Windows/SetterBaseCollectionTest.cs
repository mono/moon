using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Shapes;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows
{
	[TestClass]
	public class SetterBaseCollectionTest
	{
		[TestMethod]
		public void Sealed ()
		{
			Style style = new Style (typeof (UIElement));
			SetterBaseCollection c = style.Setters;
			Setter s = new Setter (Canvas.LeftProperty, 0);

			c.Add (s);

			style.Seal ();

			Assert.Throws (delegate { c.Add (new Setter (Canvas.TopProperty, 0)); }, typeof (Exception));
			Assert.Throws (delegate { c.Insert (0, new Setter (Canvas.TopProperty, 0)); }, typeof (Exception));

			/*Assert.Throws (delegate {*/ c.Remove (s);/* }, typeof (Exception));*/

			Assert.AreEqual (0, c.Count);

			// need to reinitialize things here since the
			// Remove above actually succeeded.
			style = new Style (typeof (UIElement));
			c = style.Setters;
			s = new Setter (Canvas.LeftProperty, 0);

			c.Add (s);

			style.Seal ();

			// lame, this should raise an exception too
			/*Assert.Throws (delegate {*/ c.RemoveAt (0);/* }, typeof (Exception));*/

			Assert.AreEqual (0, c.Count);

			// need to reinitialize things here since the
			// RemoveAt above actually succeeded.
			style = new Style (typeof (UIElement));
			c = style.Setters;
			s = new Setter (Canvas.LeftProperty, 0);

			c.Add (s);

			style.Seal ();

			Assert.Throws (delegate { c[0] = new Setter (Canvas.TopProperty, 0); }, typeof (Exception));
		}

		[TestMethod]
		public void SealedChildren ()
		{
			Style style = new Style (typeof (UIElement));
			SetterBaseCollection c = style.Setters;
			Setter s = new Setter (Canvas.LeftProperty, 0);

			c.Add (s);

			style.Seal ();

			Assert.Throws (delegate { s.Property = Canvas.TopProperty; }, typeof (UnauthorizedAccessException));
			Assert.Throws (delegate { s.Value = 10; }, typeof (UnauthorizedAccessException));
		}

		[TestMethod]
		public void SetEmptyStyle ()
		{
			Style s = new Style ();
			Rectangle r = new Rectangle ();
			Assert.Throws<NullReferenceException>(delegate {
				r.Style = s;
			}, "Empty Style");
		}

		[TestMethod]
		public void SetStyleToElement()
		{
			Rectangle r = new Rectangle();
			Style s = new Style(typeof(Rectangle));
			s.Seal();
			r.Style = s;

			s = new Style(typeof(Rectangle));
			r = new Rectangle();
			Setter setter = new Setter(Rectangle.HeightProperty, 50);
			Assert.IsFalse(setter.IsSealed, "#2");

			s.Seal();
			Assert.IsTrue(s.IsSealed, "#3");
			Assert.IsFalse(s.Setters.IsSealed, "#4");
			s.Setters.Add(setter);
			Assert.IsTrue(s.IsSealed, "#5");
			Assert.IsTrue(setter.IsSealed, "#6");
			Assert.IsFalse(s.Setters.IsSealed, "#7");
			s.Seal();
			Assert.IsTrue(s.Setters.IsSealed, "#8");

			s = new Style(typeof(Rectangle));
			r = new Rectangle();
			setter = new Setter(Rectangle.HeightProperty, 50);

			s.Setters.Add(setter);
			s.Seal();
			Assert.IsTrue(setter.IsSealed, "#9");
			Assert.IsTrue(s.Setters.IsSealed, "#10");
		}

		[TestMethod]
		[MoonlightBug ("this case is not working, throwing a Windows.Markup.XamlParseException")]
		public void SetStyleToElement_NotWorking ()
		{
			Style s = new Style (typeof (Rectangle));
			Rectangle r = new Rectangle ();
			Setter setter = new Setter (Rectangle.HeightProperty, 50);

			s.Setters.Add (setter);
			r.Style = s;
			Assert.IsTrue (s.IsSealed, "#11");
		}

		[TestMethod]
		public void Add ()
		{
			SetterBaseCollection sbc = new SetterBaseCollection ();
			Assert.Throws<ArgumentNullException> (delegate {
				sbc.Add (null);
			}, "Add(null)");
			Assert.Throws<Exception> (delegate {
				sbc.Add (new Setter ());
			}, "Add(Empty)");
		}

		[TestMethod]
		public void SetterSealedOnAdd ()
		{
			Setter s = new Setter (UIElement.OpacityProperty, 2.0);
			SetterBaseCollection sbc = new SetterBaseCollection ();
			Assert.IsFalse (sbc.IsSealed, "SetterBaseCollection.IsSealed-1");
			Assert.IsFalse (s.IsSealed, "Setter.IsSealed-1");
			sbc.Add (s);
			Assert.IsFalse (sbc.IsSealed, "SetterBaseCollection.IsSealed-2");
			Assert.IsTrue (s.IsSealed, "Setter.IsSealed-2");
		}

		[TestMethod]
		public void Remove ()
		{
			SetterBaseCollection sbc = new SetterBaseCollection ();
			Assert.IsFalse (sbc.Remove (null), "null");
			Assert.IsFalse (sbc.Remove (new Setter ()), "Empty");
		}

		[TestMethod]
		public void SetterStillSealedAfterRemove ()
		{
			Setter s = new Setter (UIElement.OpacityProperty, 2.0);
			SetterBaseCollection sbc = new SetterBaseCollection ();
			Assert.IsFalse (sbc.IsSealed, "SetterBaseCollection.IsSealed-1");
			Assert.IsFalse (s.IsSealed, "Setter.IsSealed-1");
			sbc.Add (s);
			Assert.IsFalse (sbc.IsSealed, "SetterBaseCollection.IsSealed-2");
			Assert.IsTrue (s.IsSealed, "Setter.IsSealed-2");
			sbc.Remove (s);
			Assert.IsFalse (sbc.IsSealed, "SetterBaseCollection.IsSealed-3");
			Assert.IsTrue (s.IsSealed, "Setter.IsSealed-3");
		}
	}
}
