using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Controls
{
	[TestClass]
	public partial class StackPanelTest : Microsoft.Silverlight.Testing.SilverlightTest {
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
			Assert.AreEqual (new Size (0,0), new Size (stack.ActualWidth, stack.ActualHeight), "stack actual");
			
			stack.Arrange (new Rect (10, 10, stack.DesiredSize.Width, stack.DesiredSize.Height));
			
			Assert.AreEqual (new Size (50,55), stack.DesiredSize, "stack desired1");
			Assert.AreEqual (new Size (50,55), stack.RenderSize, "stack render1");
			Assert.AreEqual (new Rect (10,10,50,55), LayoutInformation.GetLayoutSlot (stack), "stack slot");
		}

		public FrameworkElement CreateSlotItem ()
		{
			Border border = new Border ();
			border.Width = 25;
			border.Height = 33;
			return border;
		}

		[TestMethod]
		public void LayoutSlotTest ()
		{
			var stack = new StackPanel ();
			stack.Children.Add (CreateSlotItem ());
			stack.Children.Add (CreateSlotItem ());
			stack.Children.Add (CreateSlotItem ());
			
			stack.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			stack.Arrange (new Rect (0,0,stack.DesiredSize.Width,stack.DesiredSize.Height));
			
			Assert.AreEqual (new Rect (0,0,25,33), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[0]));
			Assert.AreEqual (new Rect (0,33,25,33), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[1]));
			Assert.AreEqual (new Rect (0,66,25,33), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[2]));
		}

		[TestMethod]
		public void LayoutSlotTest2 ()
		{
			Border b = new Border ();
			var stack = new StackPanel ();
			stack.Children.Add (CreateSlotItem ());
			stack.Children.Add (CreateSlotItem ());
			stack.Children.Add (CreateSlotItem ());
			b.Width = 50;
			b.Height = 300;
			b.Child = stack;

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			b.Arrange (new Rect (0,0,b.DesiredSize.Width,b.DesiredSize.Height));
			
			Assert.AreEqual (new Rect (0,0,50,33), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[0]));
			Assert.AreEqual (new Rect (0,33,50,33), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[1]));
			Assert.AreEqual (new Rect (0,66,50,33), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[2]));
			
			Assert.AreEqual (new Size (50,300),b.DesiredSize);
			Assert.AreEqual (new Size (50,300),b.RenderSize);
			
		}

		[TestMethod]
		public void LayoutSlotTest3 ()
		{
			Border b = new Border ();
			var stack = new StackPanel ();

			var panel = new Canvas ();
			panel.Children.Add (b);

			panel.Background = new SolidColorBrush (Colors.Green);

			stack.Children.Add (CreateSlotItem ());
			stack.Children.Add (CreateSlotItem ());
			stack.Children.Add (CreateSlotItem ());
			stack.HorizontalAlignment = HorizontalAlignment.Right;
			b.Width = 50;
			b.Child = stack;

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (0,0), new Size (stack.ActualWidth, stack.ActualHeight), "stack actual");

			b.Arrange (new Rect (0,0,b.DesiredSize.Width,b.DesiredSize.Height));
			
			Assert.AreEqual (new Rect (0,0,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[0]).ToString (), "slot0");
			Assert.AreEqual (new Rect (0,33,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[1]).ToString (), "slot1");
			Assert.AreEqual (new Rect (0,66,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[2]).ToString (), "slot2");
			Assert.AreEqual (new Rect (0,0,50,99).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack).ToString ());
			Assert.AreEqual (new Rect (0,0,50,99).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)b).ToString ());
			
			Assert.AreEqual (new Size (50,99),b.DesiredSize);
			Assert.AreEqual (new Size (25,99),stack.DesiredSize);
			Assert.AreEqual (new Size (25,99), new Size (stack.ActualWidth, stack.ActualHeight), "stack actual");
			Assert.AreEqual (new Size (25,99), stack.RenderSize, "stack render");
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void LayoutSlotTest3InTree ()
		{
			Border b = new Border ();
			var stack = new StackPanel ();

			var panel = new Canvas ();
			panel.Children.Add (b);

			panel.Background = new SolidColorBrush (Colors.Green);

			stack.Children.Add (CreateSlotItem ());
			stack.Children.Add (CreateSlotItem ());
  			stack.Children.Add (CreateSlotItem ());
			stack.HorizontalAlignment = HorizontalAlignment.Right;
			b.Width = 50;
			b.Child = stack;

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (0,0), new Size (stack.ActualWidth, stack.ActualHeight), "stack actual");

			b.Arrange (new Rect (0,0,b.DesiredSize.Width,b.DesiredSize.Height));
			
			Assert.AreEqual (new Rect (0,0,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[0]).ToString (), "slot0");
			Assert.AreEqual (new Rect (0,33,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[1]).ToString (), "slot1");
			Assert.AreEqual (new Rect (0,66,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[2]).ToString (), "slot2");
			Assert.AreEqual (new Rect (0,0,50,99).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack).ToString ());
			Assert.AreEqual (new Rect (0,0,50,99).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)b).ToString ());
			
			Assert.AreEqual (new Size (50,99),b.DesiredSize);
			Assert.AreEqual (new Size (25,99),stack.DesiredSize);
			Assert.AreEqual (new Size (25,99), new Size (stack.ActualWidth, stack.ActualHeight), "stack actual");

			TestPanel.Children.Add (panel);
			Assert.AreEqual (new Size (0,0),b.DesiredSize);
			Assert.AreEqual (new Size (0,0),stack.DesiredSize);

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (0,0), new Size (stack.ActualWidth, stack.ActualHeight), "stack actual");
			b.Arrange (new Rect (0,0,b.DesiredSize.Width,b.DesiredSize.Height));

			Assert.AreEqual (new Rect (0,0,0,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[0]).ToString (), "slot0 in tree");
			Assert.AreEqual (new Rect (0,0,0,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[1]).ToString (), "slot1 in tree");
			Assert.AreEqual (new Rect (0,0,0,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[2]).ToString (), "slot2 in tree");
			Assert.AreEqual (new Rect (0,0,0,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack).ToString ());
			Assert.AreEqual (new Rect (0,0,0,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)b).ToString ());
			Assert.AreEqual (new Size (50,99),b.DesiredSize);
			Assert.AreEqual (new Size (25,99),stack.DesiredSize);
			TestPanel.Children.Remove (panel);

			CreateAsyncTest (panel, () => {
					Assert.AreEqual (new Rect (0,0,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[0]).ToString (), "slot0");
					Assert.AreEqual (new Rect (0,33,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[1]).ToString (), "slot1");
					Assert.AreEqual (new Rect (0,66,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[2]).ToString (), "slot2");
					Assert.AreEqual (new Rect (0,0,50,99).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack).ToString ());
					Assert.AreEqual (new Rect (0,0,50,99).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)b).ToString ());
					
					Assert.AreEqual (new Size (50,99),b.DesiredSize, "b desired async");
					Assert.AreEqual (new Size (25,99),stack.DesiredSize, "stack desired async");
				});
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void LayoutSlotTest4InTree ()
		{
			Border b = new Border ();
			var stack = new StackPanel ();

			var panel = new Grid ();
			panel.Children.Add (b);

			panel.Background = new SolidColorBrush (Colors.Green);

			stack.Children.Add (CreateSlotItem ());
			stack.Children.Add (CreateSlotItem ());
  			stack.Children.Add (CreateSlotItem ());
			stack.HorizontalAlignment = HorizontalAlignment.Right;
			b.Width = 50;
			b.Child = stack;

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (0,0), new Size (stack.ActualWidth, stack.ActualHeight), "stack actual");

			b.Arrange (new Rect (0,0,b.DesiredSize.Width,b.DesiredSize.Height));
			
			Assert.AreEqual (new Rect (0,0,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[0]).ToString (), "slot0");
			Assert.AreEqual (new Rect (0,33,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[1]).ToString (), "slot1");
			Assert.AreEqual (new Rect (0,66,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[2]).ToString (), "slot2");
			Assert.AreEqual (new Rect (0,0,50,99).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack).ToString ());
			Assert.AreEqual (new Rect (0,0,50,99).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)b).ToString ());
			
			Assert.AreEqual (new Size (50,99),b.DesiredSize);
			Assert.AreEqual (new Size (25,99),stack.DesiredSize);
			Assert.AreEqual (new Size (25,99), new Size (stack.ActualWidth, stack.ActualHeight), "stack actual");

			TestPanel.Children.Add (panel);
			Assert.AreEqual (new Size (0,0),b.DesiredSize);
			Assert.AreEqual (new Size (0,0),stack.DesiredSize);

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			Assert.AreEqual (new Size (0,0), new Size (stack.ActualWidth, stack.ActualHeight), "stack actual");
			b.Arrange (new Rect (0,0,b.DesiredSize.Width,b.DesiredSize.Height));

			Assert.AreEqual (new Rect (0,0,0,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[0]).ToString (), "slot0 in tree");
			Assert.AreEqual (new Rect (0,0,0,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[1]).ToString (), "slot1 in tree");
			Assert.AreEqual (new Rect (0,0,0,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[2]).ToString (), "slot2 in tree");
			Assert.AreEqual (new Rect (0,0,0,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack).ToString ());
			Assert.AreEqual (new Rect (0,0,0,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)b).ToString ());
			Assert.AreEqual (new Size (50,99),b.DesiredSize);
			Assert.AreEqual (new Size (25,99),stack.DesiredSize);
			TestPanel.Children.Remove (panel);

			CreateAsyncTest (panel, () => {
					Assert.AreEqual (new Rect (0,0,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[0]).ToString (), "slot0");
					Assert.AreEqual (new Rect (0,33,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[1]).ToString (), "slot1");
					Assert.AreEqual (new Rect (0,66,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[2]).ToString (), "slot2");
					Assert.AreEqual (new Rect (0,0,50,99).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack).ToString ());
					Assert.AreEqual (new Rect (0,0,50,99).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)b).ToString ());
					
					Assert.AreEqual (new Size (50,99),b.DesiredSize, "b desired async");
					Assert.AreEqual (new Size (25,99),stack.DesiredSize, "stack desired async");
				});
		}

		[TestMethod]
		public void AlignmentTest ()
		{
			Border b = new Border ();
			var stack = new StackPanel ();
			stack.Children.Add (CreateSlotItem ());
			stack.Children.Add (CreateSlotItem ());
			stack.Children.Add (CreateSlotItem ());
			stack.HorizontalAlignment = HorizontalAlignment.Right;
			b.Width = 50;
			b.Child = stack;

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			b.Arrange (new Rect (0,0,b.DesiredSize.Width,b.DesiredSize.Height));
			
			Assert.AreEqual (new Rect (0,0,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[0]).ToString ());
			Assert.AreEqual (new Rect (0,33,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[1]).ToString ());
			Assert.AreEqual (new Rect (0,66,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[2]).ToString ());
			Assert.AreEqual (new Rect (0,0,50,99).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack).ToString ());
			Assert.AreEqual (new Rect (0,0,50,99).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)b).ToString ());
			
			Assert.AreEqual (new Size (50,99),b.DesiredSize);
			Assert.AreEqual (new Size (25,99),stack.DesiredSize);
		}

		[TestMethod]
		public void AlignmentTest2 ()
		{
			Border b = new Border ();
			var stack = new StackPanel ();
			stack.Children.Add (new Border ());
			stack.Children.Add (new Border ());
			stack.Children.Add (new Border ());
			stack.HorizontalAlignment = HorizontalAlignment.Right;
			b.Width = 50;
			b.Child = stack;

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			b.Arrange (new Rect (0,0,b.DesiredSize.Width,b.DesiredSize.Height));
			
			Assert.AreEqual (new Rect (0,0,0,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[0]).ToString (), "child 0");
			Assert.AreEqual (new Rect (0,0,0,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[1]).ToString (), "child 1");
			Assert.AreEqual (new Rect (0,0,0,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[2]).ToString (), "child 2");
			Assert.AreEqual (new Rect (0,0,50,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack).ToString (), "stack slot");
			Assert.AreEqual (new Rect (0,0,50,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)b).ToString (), "b slot");
			
			Assert.AreEqual (new Size (50,0),b.DesiredSize, "b desired");
			Assert.AreEqual (new Size (0,0),stack.DesiredSize, "stack desired");
		}

		[TestMethod]
		public void AlignmentTest3 ()
		{
			Border b = new Border ();
			var stack = new StackPanel ();
			stack.Children.Add (new Border ());
			stack.Children.Add (new Border ());
			stack.Children.Add (new Border ());
			stack.HorizontalAlignment = HorizontalAlignment.Right;
			b.Width = 50;
			stack.Width = 50;
			b.Child = stack;

			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			b.Arrange (new Rect (0,0,b.DesiredSize.Width,b.DesiredSize.Height));
			
			Assert.AreEqual (new Rect (0,0,50,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[0]).ToString (), "child 0");
			Assert.AreEqual (new Rect (0,0,50,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[1]).ToString (), "child 1");
			Assert.AreEqual (new Rect (0,0,50,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[2]).ToString (), "child 2");
			Assert.AreEqual (new Rect (0,0,50,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack).ToString (), "stack slot");
			Assert.AreEqual (new Rect (0,0,50,0).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)b).ToString (), "b slot");
			
			Assert.AreEqual (new Size (0,0), stack.Children[0].DesiredSize, "child 0 desired");
			Assert.AreEqual (new Size (50,0),b.DesiredSize, "b desired");
			Assert.AreEqual (new Size (50,0),stack.DesiredSize, "stack desired");
		}

		[TestMethod]
		public void LayoutMarginTest ()
		{
			Border b = new Border ();
			var stack = new StackPanel ();
			stack.Children.Add (CreateSlotItem ());
			stack.Children.Add (CreateSlotItem ());
			stack.Children.Add (CreateSlotItem ());
			stack.HorizontalAlignment = HorizontalAlignment.Right;
			b.Width = 50;
			b.Child = stack;
			stack.Margin = new Thickness (10,20,0,0);
			
			b.Measure (new Size (Double.PositiveInfinity, Double.PositiveInfinity));
			b.Arrange (new Rect (0,0,b.DesiredSize.Width,b.DesiredSize.Height));
			
			Assert.AreEqual (new Rect (0,0,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[0]).ToString ());
			Assert.AreEqual (new Rect (0,33,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[1]).ToString ());
			Assert.AreEqual (new Rect (0,66,25,33).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack.Children[2]).ToString ());
			Assert.AreEqual (new Rect (0,0,50,119).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)stack).ToString (), "stack");
			Assert.AreEqual (new Rect (0,0,50,119).ToString (), LayoutInformation.GetLayoutSlot ((FrameworkElement)b).ToString (), "border");
			
			Assert.AreEqual (new Size (50,119),b.DesiredSize);
			Assert.AreEqual (new Size (35,119),stack.DesiredSize);
		}
	}
}
