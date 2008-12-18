//
// DesignerProperties Unit Tests
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
using System.ComponentModel;
using System.Windows;
using System.Windows.Shapes;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.ComponentModel {

	[TestClass]
	public class DesignerPropertiesTest {

		private Rectangle rect = new Rectangle ();

		[TestMethod]
		public void GetIsInDesignMode ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				DesignerProperties.GetIsInDesignMode (null);
			}, "null");
			Assert.IsFalse (DesignerProperties.GetIsInDesignMode (rect), "Rectangle");
		}

		[TestMethod]
		public void SetIsInDesignMode ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				DesignerProperties.SetIsInDesignMode (null, true);
			}, "null");

			Assert.Throws<NotImplementedException> (delegate {
				DesignerProperties.SetIsInDesignMode (rect, true);
			}, "Rectangle/True");

			rect.SetValue (DesignerProperties.IsInDesignModeProperty, true);
			Assert.IsFalse (DesignerProperties.GetIsInDesignMode (rect), "Rectangle/SetValue/False/1");
			Assert.IsTrue ((bool) rect.GetValue (DesignerProperties.IsInDesignModeProperty), "Rectangle/GetValue/True");

			Assert.Throws<NotImplementedException> (delegate {
				DesignerProperties.SetIsInDesignMode (rect, false);
			}, "Rectangle/False");

			rect.SetValue (DesignerProperties.IsInDesignModeProperty, false);
			Assert.IsFalse (DesignerProperties.GetIsInDesignMode (rect), "Rectangle/SetValue/False/2");
			Assert.IsFalse ((bool) rect.GetValue (DesignerProperties.IsInDesignModeProperty), "Rectangle/GetValue/False");
		}
	}
}
