using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.Windows.Input {

	[TestClass]
	public class InputMethodTest {

		[TestMethod]
		[MoonlightBug]
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
	}
}
