//
// ToggleButton Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Threading;
using System.Threading;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Controls.Primitives {

	[TestClass]
	public partial class ToggleButtonTest {

		class ToggleButtonPoker : ToggleButton {

			public void OnToggle_ ()
			{
				base.OnToggle ();
			}
		}

		[TestMethod]
		public void Checked ()
		{
			ToggleButtonPoker bb = new ToggleButtonPoker ();
			bb.Checked += delegate (object sender, RoutedEventArgs e) {
				Assert.AreSame (bb, sender, "sender");
				Assert.AreSame (bb, e.OriginalSource, "OriginalSource");
			};
			Assert.IsFalse (bb.IsChecked.Value, "!Checked");
			bb.OnToggle_ ();
			Assert.IsTrue (bb.IsChecked.Value, "Checked");
		}

		[TestMethod]
		public void Unchecked ()
		{
			ToggleButtonPoker bb = new ToggleButtonPoker ();
			bb.Unchecked += delegate (object sender, RoutedEventArgs e) {
				Assert.AreSame (bb, sender, "sender");
				Assert.AreSame (bb, e.OriginalSource, "OriginalSource");
			};
			bb.OnToggle_ ();
			Assert.IsTrue (bb.IsChecked.Value, "Checked");
			bb.OnToggle_ ();
			Assert.IsFalse (bb.IsChecked.Value, "Unchecked");
		}
		
		[TestMethod]
		public void IsCheckedTest ()
		{
			ToggleButton button = new ToggleButton ();
			button.IsChecked = null;
			Assert.IsNull (button.IsChecked, "#1");
			button.SetValue (ToggleButton.IsCheckedProperty, (bool) true);
			Assert.IsTrue (button.IsChecked.HasValue, "#2");
			Assert.IsTrue (button.IsChecked.Value, "#3");
			button.SetValue (ToggleButton.IsCheckedProperty, (bool?) true);
			Assert.IsTrue (button.IsChecked.HasValue, "#4");
			Assert.IsTrue (button.IsChecked.Value, "#5");
			button.SetValue (ToggleButton.IsCheckedProperty, (bool) false);
			Assert.IsTrue (button.IsChecked.HasValue, "#6");
			Assert.IsFalse(button.IsChecked.Value, "#7");
			button.SetValue (ToggleButton.IsCheckedProperty, (bool?) false);
			Assert.IsTrue (button.IsChecked.HasValue, "#8");
			Assert.IsFalse (button.IsChecked.Value, "#9");
			button.SetValue (ToggleButton.IsCheckedProperty, (bool?) null);
			Assert.IsFalse (button.IsChecked.HasValue, "#10");
		}
		
		[TestMethod]
		public void ToStringTest ()
		{
			ToggleButton button = new ToggleButton ();
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:False", button.ToString (), "#A1");
			button.IsChecked = true;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:True", button.ToString (), "#A2");
			button.IsChecked = null;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:null", button.ToString (), "#A3");
			button.IsChecked = false;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:False", button.ToString (), "#A4");

			button.IsThreeState = false;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:False", button.ToString (), "#B1");
			button.IsChecked = true;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:True", button.ToString (), "#B2");
			button.IsChecked = null;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:null", button.ToString (), "#B3");
			button.IsChecked = false;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:False", button.ToString (), "#B4");

			button.IsThreeState = true;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:False", button.ToString (), "#C1");
			button.IsChecked = true;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:True", button.ToString (), "#C2");
			button.IsChecked = null;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:null", button.ToString (), "#C3");
			button.IsChecked = false;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:False", button.ToString (), "#C4");

			button.Content = "Hello";
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content:Hello IsChecked:False", button.ToString (), "#D1");
			button.IsChecked = true;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content:Hello IsChecked:True", button.ToString (), "#D2");
			button.IsChecked = null;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content:Hello IsChecked:null", button.ToString (), "#D3");
			button.IsChecked = false;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content:Hello IsChecked:False", button.ToString (), "#D4");


			button.Content = null;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:False", button.ToString (), "#E1");
			button.IsChecked = true;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:True", button.ToString (), "#E2");
			button.IsChecked = null;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:null", button.ToString (), "#E3");
			button.IsChecked = false;
			Assert.AreEqual ("System.Windows.Controls.Primitives.ToggleButton Content: IsChecked:False", button.ToString (), "#E4");
		}
	}
}
