using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;


namespace MoonTest.System.Windows.Controls
{
	[TestClass]
	public class StackPanelTest {
		[TestMethod]
		public void MeasureTest ()
		{
			Border b1 = new Border ();
			b1.Background = new SolidColorBrush (Colors.Red);
			b1.Width = 50;
			b1.Height = 25;
			
			Border b2 = new Border ();
			b2.Background = new SolidColorBrush (Colors.Blue);
			b2.Width = 25;
			b2.Height = 30;
			
			var stack = new StackPanel ();
			stack.Children.Add (b1);
			stack.Children.Add (b2);
			
			stack.Measure (new Size (Double.PositiveInfinity,Double.PositiveInfinity));
			
			Assert.AreEqual (new Size (50,55), stack.DesiredSize, "stack desired");
		}

		[TestMethod]
		public void ArrangeTest ()
		{
			Border b1 = new Border ();
			b1.Background = new SolidColorBrush (Colors.Red);
			b1.Width = 50;
			b1.Height = 25;
			
			Border b2 = new Border ();
			b2.Background = new SolidColorBrush (Colors.Blue);
			b2.Width = 25;
			b2.Height = 30;
			
			var stack = new StackPanel ();
			stack.Children.Add (b1);
			stack.Children.Add (b2);
			
			stack.Measure (new Size (Double.PositiveInfinity,Double.PositiveInfinity));
			
			Assert.AreEqual (new Size (50,55), stack.DesiredSize, "stack desired");
			
			stack.Arrange (new Rect (10, 10, stack.DesiredSize.Width, stack.DesiredSize.Height));
			
			Assert.AreEqual (new Size (50,55), stack.DesiredSize, "stack desired1");
			Assert.AreEqual (new Size (50,55), stack.RenderSize, "stack render1");
			Assert.AreEqual (new Rect (10,10,50,55), LayoutInformation.GetLayoutSlot (stack), "stack slot");
		}
	}
}
