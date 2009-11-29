using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.Windows.Input {

	[TestClass]
	public class InputMethodTest {

		[TestMethod]
		public void SetIsInputMethodEnabled ()
		{
			Assert.Throws<ArgumentException> (() => InputMethod.SetIsInputMethodEnabled (new Button (), true), "#1");
			Assert.Throws<ArgumentException> (() => InputMethod.SetIsInputMethodEnabled (new TextBlock (), true), "#2");
			Assert.Throws<ArgumentException> (() => Assert.IsFalse (InputMethod.GetIsInputMethodEnabled (new Button ()), "#3"));

			TextBox box = new TextBox ();
			Assert.IsTrue (InputMethod.GetIsInputMethodEnabled (box), "#4");

			InputMethod.SetIsInputMethodEnabled (box, false);
			Assert.IsFalse (InputMethod.GetIsInputMethodEnabled (box), "#5");
		}

		class ConcreteDependencyObject : DependencyObject {
		}

		class MyTextBox : TextBox {
		}

		[TestMethod]
		public void GetIsInputMethodEnabled_Validations ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				InputMethod.GetIsInputMethodEnabled (null);
			}, "null");

			ConcreteDependencyObject cdo = new ConcreteDependencyObject ();
			Assert.Throws<ArgumentException> (delegate {
				InputMethod.GetIsInputMethodEnabled (cdo);
			}, "not a TextBox");
		}

		[TestMethod]
		public void GetIsInputMethodEnabled_TextBox ()
		{
			TextBox tb = new TextBox ();
			Assert.IsTrue (InputMethod.GetIsInputMethodEnabled (tb), "TextBox");

			MyTextBox mtb = new MyTextBox ();
			Assert.IsTrue (InputMethod.GetIsInputMethodEnabled (mtb), "MyTextBox");
		}

		[TestMethod]
		[MoonlightBug]
		public void GetValue ()
		{
			ConcreteDependencyObject cdo = new ConcreteDependencyObject ();
			Assert.Throws<Exception> (delegate {
				cdo.GetValue (InputMethod.IsInputMethodEnabledProperty);
			}, "GetValue");

			TextBox tb = new TextBox ();
			Assert.IsTrue ((bool) tb.GetValue (InputMethod.IsInputMethodEnabledProperty), "TextBox");

			MyTextBox mtb = new MyTextBox ();
			Assert.IsTrue ((bool) mtb.GetValue (InputMethod.IsInputMethodEnabledProperty), "MyTextBox");
		}

		[TestMethod]
		public void SetIsInputMethodEnabled_Validations ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				InputMethod.SetIsInputMethodEnabled (null, true);
			}, "null,true");
			Assert.Throws<ArgumentNullException> (delegate {
				InputMethod.SetIsInputMethodEnabled (null, false);
			}, "null,false");

			ConcreteDependencyObject cdo = new ConcreteDependencyObject ();
			Assert.Throws<ArgumentException> (delegate {
				InputMethod.SetIsInputMethodEnabled (cdo, true);
			}, "not a TextBox, true");
			Assert.Throws<ArgumentException> (delegate {
				InputMethod.SetIsInputMethodEnabled (cdo, false);
			}, "not a TextBox, false");
		}

		[TestMethod]
		public void SetIsInputMethodEnabled_TextBox ()
		{
			TextBox tb = new TextBox ();
			Assert.IsTrue (InputMethod.GetIsInputMethodEnabled (tb), "TextBox-1");
			InputMethod.SetIsInputMethodEnabled (tb, false);
			Assert.IsFalse (InputMethod.GetIsInputMethodEnabled (tb), "TextBox-2");
			InputMethod.SetIsInputMethodEnabled (tb, true);
			Assert.IsTrue (InputMethod.GetIsInputMethodEnabled (tb), "TextBox-3");

			MyTextBox mtb = new MyTextBox ();
			Assert.IsTrue (InputMethod.GetIsInputMethodEnabled (mtb), "MyTextBox-1");
			InputMethod.SetIsInputMethodEnabled (mtb, false);
			Assert.IsFalse (InputMethod.GetIsInputMethodEnabled (mtb), "MyTextBox-2");
			InputMethod.SetIsInputMethodEnabled (mtb, true);
			Assert.IsTrue (InputMethod.GetIsInputMethodEnabled (mtb), "MyTextBox-3");
		}

		[TestMethod]
		[MoonlightBug]
		public void SetValue ()
		{
			ConcreteDependencyObject cdo = new ConcreteDependencyObject ();
			Assert.Throws<Exception> (delegate {
				cdo.SetValue (InputMethod.IsInputMethodEnabledProperty, true);
			}, "SetValue");

			TextBox tb = new TextBox ();
			tb.SetValue (InputMethod.IsInputMethodEnabledProperty, false);
			Assert.IsFalse (InputMethod.GetIsInputMethodEnabled (tb), "TextBox");

			MyTextBox mtb = new MyTextBox ();
			mtb.SetValue (InputMethod.IsInputMethodEnabledProperty, false);
			Assert.IsFalse (InputMethod.GetIsInputMethodEnabled (mtb), "MyTextBox");
		}
	}
}
