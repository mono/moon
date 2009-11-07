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
using System.Windows.Controls;
using System.Windows.Automation.Peers;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Controls.Primitives;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing.UnitTesting.UI;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata.VisualStudio;
using Microsoft.Silverlight.Testing;
using System.Windows.Shapes;
using Microsoft.VisualStudio.TestTools.UnitTesting;

using MoonTest.System.Windows.Media;

namespace MoonTest.System.Windows {

	[TestClass]
	public class UIElementTest : Microsoft.Silverlight.Testing.SilverlightTest {

		// we can't directly inherit from UIElement (no ctor)
		class ConcreteUIElement : FrameworkElement {

			public AutomationPeer OnCreateAutomationPeer_ ()
			{
				return base.OnCreateAutomationPeer ();
			}
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
		public void TransformToVisual ()
		{
			ConcreteUIElement ui = new ConcreteUIElement ();
			Assert.Throws<ArgumentException> (delegate {
				ui.TransformToVisual (ui);
			}, "TransformToVisual(self)");
			Assert.Throws<ArgumentException> (delegate {
				ui.TransformToVisual (new ConcreteUIElement ());
			}, "TransformToVisual(new)");
		}

		[TestMethod]
		[Asynchronous]
		public void TransformToVisual_InVisualTree ()
		{
			ConcreteUIElement ui = new ConcreteUIElement ();
			Canvas c = new Canvas ();

			c.Children.Add (ui);
			Assert.Throws<ArgumentException> (delegate { 
					ui.TransformToVisual (c);
			}, "TransformToVisual (parent)");

			bool loaded_reached = false;

			Exception loaded_exception = null;

			ui.Loaded += delegate {
				loaded_reached = true;

				try {
					Assert.IsNotNull (ui.TransformToVisual (c), "1");
					Assert.IsNotNull (ui.TransformToVisual (null), "2");
				}
				catch (Exception e) {
					loaded_exception = e;
				}
			};

			TestPanel.Children.Add (c);

			EnqueueConditional ( () => loaded_reached );

			if (loaded_exception != null)
				throw loaded_exception;

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void TransformToVisual_InVisualTree2 ()
		{
			ConcreteUIElement ui = new ConcreteUIElement ();
			Canvas c = new Canvas ();

			Canvas.SetLeft (ui, 10);
			Canvas.SetTop (ui, 15);

			c.Children.Add (ui);

			bool loaded_reached = false;

			Exception loaded_exception = null;

			ui.Loaded += delegate {
				loaded_reached = true;
				try {
					MatrixTransform mt = (MatrixTransform)ui.TransformToVisual (null);

					Assert.AreEqual (10, mt.Matrix.OffsetX, "3");
					Assert.AreEqual (15, mt.Matrix.OffsetY, "4");
				}
				catch (Exception e) {
					loaded_exception = e;
				}
			};

			TestPanel.Children.Add (c);

			EnqueueConditional ( () => loaded_reached );

			if (loaded_exception != null)
				throw loaded_exception;

			EnqueueTestComplete ();
		}
		
		[TestMethod]
		[Asynchronous]
		public void TransformToVisual_InVisualTree3 ()
		{
			Rectangle a = new Rectangle { Width = 10, Height = 10, Fill = new SolidColorBrush (Colors.Red) };
			Rectangle b = new Rectangle { Width = 10, Height = 10, Fill = new SolidColorBrush (Colors.Blue ) };
			StackPanel panel = new StackPanel ();
			panel.Children.Add (a);
			panel.Children.Add (b);
			CreateAsyncTest (panel, () => {
				GeneralTransform m = a.TransformToVisual (b);
				Assert.IsTrue (m is MatrixTransform, "#1");
				Assert.Matrix (((MatrixTransform) m).Matrix, 1, 0, 0, 1, 0, -10, "#2");
				
				m = b.TransformToVisual (a);
				Assert.IsTrue (m is MatrixTransform, "#3");
				Assert.Matrix (((MatrixTransform) m).Matrix, 1, 0, 0, 1, 0, 10, "#4");
			});
		}
		
		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void TransformToVisual_InVisualTree4 ()
		{
			Rectangle a = new Rectangle { Width = 10, Height = 10, Fill = new SolidColorBrush (Colors.Red) };
			StackPanel panel = new StackPanel ();
			panel.Children.Add (a);
			CreateAsyncTest (panel, () => {
				GeneralTransform m = a.TransformToVisual (TestPanel);
				Assert.IsTrue (m is MatrixTransform, "#1");
				Assert.Matrix (((MatrixTransform) m).Matrix, 1, 0, 0, 1, 431, 0, "#2"); // Fails in Silverlight 3

				m = TestPanel.TransformToVisual (a);
				Assert.IsTrue (m is MatrixTransform, "#3");
				Assert.Matrix (((MatrixTransform) m).Matrix, 1, 0, 0, 1, -431, 0, "#4");
			});
		}

		[TestMethod]
		public void TransformToVisual_Top () {
			ConcreteUIElement ui = new ConcreteUIElement ();
			this.TestPage.TestPanel.Children.Add (ui);
			ui.TransformToVisual (this.TestPage);
		}

		[TestMethod]
		public void RenderTransform_SemiNonNullable ()
		{
			ConcreteUIElement ui = new ConcreteUIElement ();
			Assert.IsNotNull (ui.RenderTransform, "RenderTransform");

			ui.RenderTransform = new ScaleTransform ();
			Assert.IsTrue ((ui.RenderTransform is ScaleTransform), "RenderTransform/ScaleTransform");

			// when set to null, it reverts to the default value
			ui.RenderTransform = null;
			Assert.IsNotNull (ui.RenderTransform, "RenderTransform/NeverNull");
			Assert.IsTrue ((ui.RenderTransform as MatrixTransform).Matrix.IsIdentity, "RenderTransform/Null/Identity");

			// same behavior with the DP
			ui.SetValue (UIElement.RenderTransformProperty, null);
			Assert.IsNotNull (ui.RenderTransform, "RenderTransform/NeverNullDP");
			Assert.IsTrue ((ui.RenderTransform as MatrixTransform).Matrix.IsIdentity, "RenderTransform/Null/IdentityDP");
		}

		[TestMethod]
		public void RenderTransform_Unfrozen ()
		{
			ConcreteUIElement ui = new ConcreteUIElement ();
			Assert.IsTrue (MatrixTransformTest.CheckFreezer (ui.RenderTransform as MatrixTransform), "RelativeTransform");
		}

		[TestMethod]
		public void NotDestructive ()
		{
			ConcreteUIElement ui = new ConcreteUIElement ();
			// unlike the Brush transforms this one cannot be used to change the (shared) default value

			MatrixTransform mt = (ui.RenderTransform as MatrixTransform);
			Assert.IsTrue (mt.Matrix.IsIdentity, "Original/Identity");

			mt.Matrix = new Matrix (1, 2, 3, 4, 5, 6);
			Assert.IsFalse (mt.Matrix.IsIdentity, "New/NonIdentity");

			Assert.IsTrue ((new ConcreteUIElement ().RenderTransform as MatrixTransform).Matrix.IsIdentity, "ConcreteUIElement");
			Assert.IsTrue ((new Canvas ().RenderTransform as MatrixTransform).Matrix.IsIdentity, "Canvas");
			Assert.IsTrue ((new Slider ().RenderTransform as MatrixTransform).Matrix.IsIdentity, "Slider");
		}

		class ConcreteFrameworkElement : FrameworkElement { }
		
		[TestMethod]
		[MoonlightBug ("SL throws an exception when setting Tag in this way.")]
		public void TagProperty ()
		{
		        Assert.Throws <XamlParseException> (delegate { XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" x:Name=""Canvas"">
  <Canvas.Tag>
    <Button x:Name=""Button"" />
  </Canvas.Tag>
</Canvas>"); });
		}

		[TestMethod]
		public void TagPropertyNamescope2 ()
		{
		        Canvas c;
			Canvas c2;
			Button b;

			c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" x:Name=""Canvas"" />");

			b = new Button ();
			b.Name = "Button";


			Assert.IsNotNull (c.FindName ("Canvas"), "1");

			c.Tag = b;

			Assert.IsNull (c.FindName ("Button"), "2");
			Assert.IsNull (b.FindName ("Canvas"), "2.5");

			c.Children.Add (b);

			Assert.IsNotNull (c.FindName ("Button"), "3");
			Assert.IsNotNull (b.FindName ("Canvas"), "2.5");

			c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" x:Name=""Canvas"" />");
			c2 = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" x:Name=""Canvas2"" />");

			b = new Button ();
			b.Name = "Button";

			c.Tag = b;

			c2.Children.Add (b);
			Assert.IsNull (c.FindName ("Button"), "4");
			Assert.IsNotNull (c2.FindName ("Button"), "5");
			Assert.IsNull (b.FindName ("Canvas"), "5.5");
			Assert.IsNotNull (b.FindName ("Canvas2"), "5.75");
		}
	}
}
