//
// UIElement Unit Tests
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
using System.Windows.Automation.Peers;
using System.Windows.Media;
using System.Windows.Controls.Primitives;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows {

	[TestClass]
	public class UIElementTest {

		// we can't directly inherit from UIElement (no ctor)
		class ConcreteUIElement : FrameworkElement {

			public AutomationPeer OnCreateAutomationPeer_ ()
			{
				return base.OnCreateAutomationPeer ();
			}
		}

		[TestMethod]
		public void DefaultProperties ()
		{
			ConcreteUIElement ui = new ConcreteUIElement ();

			// default properties on UIElement...
			CheckDefaultProperties (ui);
		}

		static public void CheckDefaultProperties (UIElement ui)
		{
			// default properties on UIElement
			Assert.IsNull (ui.Clip, "Clip");
			Assert.AreEqual (new Size (0, 0), ui.DesiredSize, "DesiredSize");
			Assert.IsTrue (ui.IsHitTestVisible, "IsHitTestVisible");
			Assert.AreEqual (1.0d, ui.Opacity, "Opacity");
			Assert.IsNull (ui.OpacityMask, "OpacityMask");
			Assert.AreEqual (new Size (0, 0), ui.RenderSize, "RenderSize");
//			Assert.IsTrue (ui.RenderTransform is MatrixTransform, "RenderTransform");
//			Assert.IsTrue ((ui.RenderTransform as MatrixTransform).Matrix.IsIdentity, "RenderTransform/Identity");
			Assert.AreEqual (new Point (0, 0), ui.RenderTransformOrigin, "RenderTransformOrigin");
			Assert.IsTrue (ui.UseLayoutRounding, "UseLayoutRounding");
			Assert.AreEqual (Visibility.Visible, ui.Visibility, "Visibility");

			// default properties on DependencyObject
			DependencyObjectTest.CheckDefaultProperties (ui);
		}

		[TestMethod]
		[MoonlightBug]
		public void BadDefaultProperties ()
		{
			ConcreteUIElement ui = new ConcreteUIElement ();
			Assert.IsTrue (ui.RenderTransform is MatrixTransform, "RenderTransform");
			Assert.IsTrue ((ui.RenderTransform as MatrixTransform).Matrix.IsIdentity, "RenderTransform/Identity");
		}

		[TestMethod]
		public void DefaultMethods ()
		{
			ConcreteUIElement ui = new ConcreteUIElement ();

			Assert.Throws<InvalidOperationException> (delegate {
				ui.Arrange (Rect.Empty);
			}, "Arrange(Empty)");

			ui.Arrange (new Rect (0,0,0,0));
			
			Assert.Throws<InvalidOperationException> (delegate {
					ui.Arrange (new Rect (10, 10, Double.PositiveInfinity, Double.PositiveInfinity));
				}, "Arrange(Infinite)");

			Assert.Throws<InvalidOperationException> (delegate {
					ui.Arrange (new Rect (10, 10, Double.NaN, Double.NaN));
				}, "Arrange(NaN)");

			Assert.IsFalse (ui.CaptureMouse (), "CaptureMouse");

			ui.InvalidateArrange ();
			ui.InvalidateMeasure ();

			Assert.IsNull (ui.OnCreateAutomationPeer_ (), "OnCreateAutomationPeer_");

			// no exception, unlike Arrange
			ui.Measure (Size.Empty);

			ui.ReleaseMouseCapture ();

			Assert.Throws<ArgumentException> (delegate {
				ui.TransformToVisual (null);
			}, "TransformToVisual(null)");
		}

		[TestMethod]
		[MoonlightBug ("no exception is thrown")]
		public void TransformToVisual ()
		{
			ConcreteUIElement ui = new ConcreteUIElement ();
			Assert.Throws<ArgumentException> (delegate {
				ui.TransformToVisual (ui);
			}, "TransformToVisual(self)");
			Assert.Throws<ArgumentException> (delegate {
				ui.TransformToVisual (new ConcreteUIElement ());
			}, "TransformToVisual(new)");
			// an UIElement is probably not complete enough to complete the call
		}
	}
}
