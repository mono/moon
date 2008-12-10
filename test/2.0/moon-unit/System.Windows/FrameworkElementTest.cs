//
// FrameworkElement Unit Tests
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

using Mono.Moonlight.UnitTesting;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows {

	[TestClass]
	public class FrameworkElementTest {

		class ConcreteFrameworkElement : FrameworkElement {
		}

		[TestMethod]
		public void DefaultProperties ()
		{
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement ();
			CheckDefaultProperties (fe);
		}

		[TestMethod]
		[MoonlightBug]
		public void BrokenProperties ()
		{
			ConcreteFrameworkElement fe = new ConcreteFrameworkElement ();
			Assert.IsNull (fe.Cursor, "Cursor");
			Assert.IsTrue (Double.IsNaN (fe.Height), "Height");
			Assert.AreEqual (String.Empty, fe.Name, "Name");
			Assert.IsTrue (Double.IsNaN (fe.Width), "Width");
		}

		static public void CheckDefaultProperties (FrameworkElement fe)
		{
			CheckDefaultProperties (fe, null);
		}

		static public void CheckDefaultProperties (FrameworkElement fe, DependencyObject parent)
		{
			// default properties on FrameworkElement
			Assert.AreEqual (0.0, fe.ActualHeight, "ActualHeight");
			Assert.AreEqual (0.0, fe.ActualWidth, "ActualWidth");
//			Assert.IsNull (fe.Cursor, "Cursor");
			Assert.IsNull (fe.DataContext, "DataContext");
//			Assert.IsTrue (Double.IsNaN (fe.Height), "Height");
			Assert.AreEqual (HorizontalAlignment.Stretch, fe.HorizontalAlignment, "HorizontalAlignment");
			Assert.IsNotNull (fe.Language, "Language");
			Assert.AreEqual (new Thickness (0, 0, 0, 0), fe.Margin, "Margin");
			Assert.IsTrue (Double.IsInfinity (fe.MaxHeight), "MaxHeight");
			Assert.IsTrue (Double.IsInfinity (fe.MaxWidth), "MaxWidth");
			Assert.AreEqual (0.0, fe.MinHeight, "MinHeight");
			Assert.AreEqual (0.0, fe.MinWidth, "MinWidth");
//			Assert.AreEqual (String.Empty, fe.Name, "Name");
			Assert.AreEqual (parent, fe.Parent, "Parent");
			Assert.IsNotNull (fe.Resources, "Resources");
			Assert.IsNull (fe.Style, "Style");
			Assert.IsNull (fe.Tag, "Tag");
			Assert.IsNotNull (fe.Triggers, "Triggers");
			Assert.AreEqual (VerticalAlignment.Stretch, fe.VerticalAlignment, "VerticalAlignment");
//			Assert.IsTrue (Double.IsNaN (fe.Width), "Width");

			UIElementTest.CheckDefaultProperties (fe);
		}
	}
}
