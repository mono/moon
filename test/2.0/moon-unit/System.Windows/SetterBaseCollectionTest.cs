using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Shapes;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows
{
	[TestClass]
	public partial class SetterBaseCollectionTest
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

		private void SealedChildren (bool seal)
		{
			Style style = new Style (typeof (UIElement));
			SetterBaseCollection c = style.Setters;
			Setter s = new Setter (Canvas.LeftProperty, 0);

			c.Add (s);

			if (seal)
				style.Seal ();

			// the setter IsSealed status has nothing to do with sealing the style
			Assert.Throws (delegate { s.Property = Canvas.TopProperty; }, typeof (UnauthorizedAccessException));
			Assert.Throws (delegate { s.Value = 10; }, typeof (UnauthorizedAccessException));
		}

		[TestMethod]
		public void SealedChildren_SealedStyle ()
		{
			SealedChildren (true);
		}

		[TestMethod]
		public void SealedChildren_UnsealedStyle ()
		{
			SealedChildren (false);
		}

		[TestMethod]
		public void SetEmptyStyle ()
		{
			Style s = new Style ();
			Rectangle r = new Rectangle ();
			Assert.Throws<NullReferenceException>(delegate {
				r.Style = s;
			}, "Empty Style"); // Fails in Silverlight 3
		}

		[TestMethod]
		public void SetSealedStyleWithoutSetterToElement ()
		{
			Style s = new Style (typeof (Rectangle));
			Assert.IsFalse (s.IsSealed, "IsSealed-1");
			Assert.IsFalse (s.Setters.IsSealed, "Setters.IsSealed-1");

			s.Seal ();
			Assert.IsTrue (s.IsSealed, "IsSealed-2");
			Assert.IsTrue (s.Setters.IsSealed, "Setters.IsSealed-2");

			Rectangle r = new Rectangle ();
			r.Style = s;
			Assert.IsTrue (s.IsSealed, "IsSealed-3");
			Assert.IsTrue (s.Setters.IsSealed, "Setters.IsSealed-3");
		}

		[TestMethod]
		[MoonlightBug ("SL2 gives different results if we check IsSealed before sealing a style")]
		public void SetSealedStyleWithoutSetterToElement_CacheIssue ()
		{
			Style s = new Style (typeof (Rectangle));
			// since we don't check IsSealed before sealing the result is different!

			s.Seal ();
			Assert.IsTrue (s.IsSealed, "IsSealed-2");
			Assert.IsFalse (s.Setters.IsSealed, "Setters.IsSealed-2");

			Rectangle r = new Rectangle ();
			r.Style = s;
			Assert.IsTrue (s.IsSealed, "IsSealed-3");
			Assert.IsTrue (s.Setters.IsSealed, "Setters.IsSealed-3"); // Fails in Silverlight 3
		}

		[TestMethod]
		public void SetSealedStyleWithSetterToElement ()
		{
			Setter setter = new Setter (Rectangle.HeightProperty, "50");
			Assert.IsFalse (setter.IsSealed, "Setter.IsSealed-1");

			Style s = new Style (typeof (Rectangle));
			Assert.IsFalse (s.IsSealed, "IsSealed-1");
			Assert.IsFalse (s.Setters.IsSealed, "Setters.IsSealed-1");

			s.Setters.Add (setter);
			Assert.IsTrue (setter.IsSealed, "Setter.IsSealed-2");
			Assert.IsFalse (s.IsSealed, "IsSealed-2");
			Assert.IsFalse (s.Setters.IsSealed, "Setters.IsSealed-2");

			s.Seal ();
			Assert.IsTrue (s.IsSealed, "IsSealed-3");
			Assert.IsTrue (s.Setters.IsSealed, "Setters.IsSealed-3");

			Rectangle r = new Rectangle ();
			r.Style = s;
			Assert.IsTrue (s.IsSealed, "IsSealed-4");
			Assert.IsTrue (s.Setters.IsSealed, "Setters.IsSealed-4");
		}

		[TestMethod]
		public void SetUnsealedStyleWithoutSetterToElement ()
		{
			Style s = new Style (typeof (Rectangle));
			Assert.IsFalse (s.IsSealed, "IsSealed-1");
			Assert.IsFalse (s.Setters.IsSealed, "Setters.IsSealed-1");

			Rectangle r = new Rectangle ();
			r.Style = s;
			Assert.IsTrue (s.IsSealed, "IsSealed-2");
			Assert.IsTrue (s.Setters.IsSealed, "Setters.IsSealed-2");
		}

		[TestMethod]
		public void SetUnsealedStyleWithSetterToElement ()
		{
			Setter setter = new Setter (Rectangle.HeightProperty, "50");
			Assert.IsFalse (setter.IsSealed, "Setter.IsSealed-1");

			Style s = new Style (typeof (Rectangle));
			Assert.IsFalse (s.IsSealed, "IsSealed-1");
			Assert.IsFalse (s.Setters.IsSealed, "Setters.IsSealed-1");

			s.Setters.Add (setter);
			Assert.IsTrue (setter.IsSealed, "Setter.IsSealed-2");
			Assert.IsFalse (s.IsSealed, "IsSealed-2");
			Assert.IsFalse (s.Setters.IsSealed, "Setters.IsSealed-2");

			Rectangle r = new Rectangle ();
			r.Style = s;
			Assert.IsTrue (s.IsSealed, "IsSealed-3");
			Assert.IsTrue (s.Setters.IsSealed, "Setters.IsSealed-3");
			Assert.IsTrue (setter.IsSealed, "Setter.IsSealed-3");
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
