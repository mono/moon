//
// Unit tests for System.Windows.WindowsSettings
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows {

	[TestClass]
	public partial class WindowsSettingsTest {

		[TestMethod]
		public void SetValue ()
		{
			// WindowsSettings only provides getters - trying SetValue throws
			WindowSettings ws = new WindowSettings ();
			Assert.Throws<ArgumentException> (delegate {
				ws.SetValue (WindowSettings.HeightProperty, 0.0d);
			}, "HeightProperty");
			Assert.Throws<ArgumentException> (delegate {
				ws.SetValue (WindowSettings.LeftProperty, 0.0d);
			}, "LeftProperty");
			Assert.Throws<ArgumentException> (delegate {
				ws.SetValue (WindowSettings.TitleProperty, null);
			}, "TitleProperty");
			Assert.Throws<ArgumentException> (delegate {
				ws.SetValue (WindowSettings.TopProperty, 0.0d);
			}, "TopProperty");
			Assert.Throws<ArgumentException> (delegate {
				ws.SetValue (WindowSettings.WidthProperty, 0.0d);
			}, "WidthProperty");
			Assert.Throws<ArgumentException> (delegate {
				ws.SetValue (WindowSettings.WindowStartupLocationProperty, WindowStartupLocation.CenterScreen);
			}, "WindowStartupLocationProperty");
			Assert.Throws<ArgumentException> (delegate {
				ws.SetValue (WindowSettings.WindowStyleProperty, WindowStyle.BorderlessRoundCornersWindow);
			}, "WindowStyleProperty");
		}
	}
}

