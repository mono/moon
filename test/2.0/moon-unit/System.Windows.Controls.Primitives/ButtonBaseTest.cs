//
// ButtonBase Unit Tests
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

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Controls.Primitives {

	[TestClass]
	public class ButtonBaseTest {

		class ConcreteButtonBase : ButtonBase {

			public bool IsPressedChangedOldValue;
			public bool IsPressedChangedNewValue;

			public void SetIsPressed (bool value)
			{
				IsPressed = value;
			}

			protected override void OnIsPressedChanged (DependencyPropertyChangedEventArgs e)
			{
				IsPressedChangedOldValue = (bool) e.OldValue;
				IsPressedChangedNewValue = (bool) e.NewValue;
				base.OnIsPressedChanged (e);
			}

			public void OnClick_ ()
			{
				base.OnClick ();
			}
		}

		[TestMethod]
		public void DefaultProperties ()
		{
			ConcreteButtonBase bb = new ConcreteButtonBase ();
			// default properties on ButtonBase
			Assert.IsFalse (bb.IsFocused, "IsFocused");
			Assert.IsFalse (bb.IsMouseOver, "IsMouseOver");
			Assert.IsFalse (bb.IsPressed, "IsPressed");
			Assert.AreEqual (ClickMode.Release, bb.ClickMode, "ClickMode");

			// default properties on Control...
			ControlTest.CheckDefaultProperties (bb);
		}

		[TestMethod]
		[MoonlightBug ("are we yet disallowing setting read-only properties via SetValue ?")]
		public void SetValueOnReadOnlyProperties ()
		{
			ConcreteButtonBase bb = new ConcreteButtonBase ();

			Assert.Throws<InvalidOperationException> (delegate {
				bb.SetValue (ButtonBase.IsFocusedProperty, true);
			});
			Assert.IsFalse (bb.IsFocused, "IsFocused");

			Assert.Throws<InvalidOperationException> (delegate {
				bb.SetValue (ButtonBase.IsFocusedProperty, true);
			});
			Assert.IsFalse (bb.IsMouseOver, "IsMouseOver");
		}

		[TestMethod]
		public void Properties ()
		{
			ConcreteButtonBase bb = new ConcreteButtonBase ();

			bb.SetIsPressed (true);
			Assert.IsTrue (bb.IsPressed, "IsPressed-1");
			Assert.IsFalse (bb.IsPressedChangedOldValue, "IsPressed-1/OldValue");
			Assert.IsTrue (bb.IsPressedChangedNewValue, "IsPressed-1/NewValue");
			bb.SetIsPressed (false);
			Assert.IsFalse (bb.IsPressed, "IsPressed-2");
			Assert.IsTrue (bb.IsPressedChangedOldValue, "IsPressed-2/OldValue");
			Assert.IsFalse (bb.IsPressedChangedNewValue, "IsPressed-2/NewValue");

			bb.ClickMode = ClickMode.Hover;
			Assert.AreEqual (ClickMode.Hover, bb.ClickMode, "ClickMode/Hover");
			bb.ClickMode = ClickMode.Press;
			Assert.AreEqual (ClickMode.Press, bb.ClickMode, "ClickMode/Press");
			bb.ClickMode = ClickMode.Release;
			Assert.AreEqual (ClickMode.Release, bb.ClickMode, "ClickMode/Release");

			Assert.Throws<ArgumentException> (delegate {
				bb.ClickMode = (ClickMode) Int32.MinValue;
			});
		}

		[TestMethod]
		public void DefaultMethods ()
		{
			ConcreteButtonBase bb = new ConcreteButtonBase ();
			ControlTest.CheckDefaultMethods (bb);
		}

		[TestMethod]
		public void Events ()
		{
			ConcreteButtonBase bb = new ConcreteButtonBase ();
			bb.Click += delegate (object sender, RoutedEventArgs e) {
				Assert.AreSame (bb, sender, "sender");
				Assert.AreSame (bb, e.OriginalSource, "OriginalSource");
			};
			bb.OnClick_ ();
		}
	}
}
