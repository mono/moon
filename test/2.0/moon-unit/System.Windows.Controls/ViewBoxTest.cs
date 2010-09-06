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
using Microsoft.Silverlight.Testing;
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public partial class ViewboxTest : SilverlightTest {

		LayoutPoker Child {
			get; set;
		}

		Border ChildBorder {
			get { return (Border)VisualTreeHelper.GetParent(Child); }
		}

		Size Infinity {
			get { return new Size(double.PositiveInfinity, double.PositiveInfinity); }
		}

		Viewbox Viewbox {
			get;  set;
		}

		[TestInitialize]
		public void Setup()
		{
			TestPanel.Width = 100;
			TestPanel.Height = 100;
			Child = new LayoutPoker ();
			Viewbox = new Viewbox();

			// The default template is applied when the item is added to the tree.
			TestPanel.Children.Add(Viewbox);
			TestPanel.Children.Clear();

			Viewbox.Child = Child;
		}

		[TestMethod]
		[Asynchronous]
		public void VisualTree_RenderTransformSet()
		{
			var box = new Viewbox();
			var child = new LayoutPoker();
			box.Child = child;

			CreateAsyncTest(box,
				() => {
					/* Ensure that the box has been measured at least once */
				}, () => {
					var border = box.FindFirstChild<Border>();
					Assert.IsNotNull(border, "#1");
					Assert.AreNotEqual(DependencyProperty.UnsetValue, border.ReadLocalValue(Border.RenderTransformProperty), "#2");
					Assert.IsInstanceOfType<ScaleTransform> (border.RenderTransform, "#3");
				}
			);
		}

		[TestMethod]
		public void ArrangeTest()
		{
			Child.MeasureFunc = () => new Size(50, 50);
			Viewbox.Measure(Infinity);
			Viewbox.Arrange(new Rect(0, 0, 50, 50));
			Assert.AreEqual(new Size (50, 50), Child.ArrangeArg, "#1");

			Viewbox.Measure(new Size(50, 50));
			Viewbox.Arrange(new Rect(0, 0, 30, 70));
			Assert.AreEqual(new Size (50, 50), Child.ArrangeArg, "#2");
		}

		[TestMethod]
		public void ArrangeTest_CheckScaling_ViewBoxSmaller()
		{
			Viewbox.Width = 25;
			Viewbox.Height = 25;
			Child.MeasureFunc = () => new Size(50, 50);
			Child.ArrangeFunc = () => new Size(50, 50);

			Viewbox.Measure(new Size(50, 50));
			Viewbox.Arrange(new Rect(0, 0, 30, 70));

			Assert.AreEqual(new Size(50, 50), Child.DesiredSize, "#1");
			Assert.AreEqual(new Size(50, 50), Child.RenderSize, "#2");

			Assert.AreEqual(new Size(50, 50), ChildBorder.DesiredSize, "#3");
			Assert.AreEqual(new Size(50, 50), ChildBorder.RenderSize, "#4");

			var scale = (ScaleTransform)((Border)VisualTreeHelper.GetParent(Child)).RenderTransform;
			Assert.AreEqual(scale.CenterX, 0, "#5");
			Assert.AreEqual(scale.CenterY, 0, "#6");
			Assert.AreEqual(scale.ScaleX, 0.5, "#7");
			Assert.AreEqual(scale.ScaleY, 0.5, "#8");
		}

		[TestMethod]
		public void ArrangeTest_CheckFinalSize_ViewBoxSmaller()
		{
			Viewbox.Width = 25;
			Viewbox.Height = 25;
			Child.MeasureFunc = () => new Size(50, 50);

			Viewbox.Measure(new Size(50, 50));
			Viewbox.Arrange(new Rect(0, 0, 30, 70));

			Assert.AreEqual(50, Child.ActualWidth, "#1");
			Assert.AreEqual(50, Child.ActualHeight, "#2");

			Assert.AreEqual(50, ChildBorder.ActualWidth, "#1");
			Assert.AreEqual(50, ChildBorder.ActualHeight, "#2");
		}

		[TestMethod]
		public void ArrangeTest_CheckFinalSize_ViewBoxSmaller2()
		{
			Viewbox.Width = 25;
			Viewbox.Height = 25;
			Child.MeasureFunc = () => new Size(50, 50);
			Child.ArrangeFunc = () => new Size(25, 25);

			Viewbox.Measure(new Size(50, 50));
			Viewbox.Arrange(new Rect(0, 0, 30, 70));

			Assert.AreEqual(25, Child.ActualWidth, "#1");
			Assert.AreEqual(25, Child.ActualHeight, "#2");
			Assert.AreEqual(new Size(50, 50), Child.DesiredSize, "#3");
			Assert.AreEqual(new Size(25, 25), Child.RenderSize, "#4");

			Assert.AreEqual(50, ChildBorder.ActualWidth, "#5");
			Assert.AreEqual(50, ChildBorder.ActualHeight, "#6");
			Assert.AreEqual(new Size(50, 50), ChildBorder.RenderSize, "#7");
			Assert.AreEqual(new Size(50, 50), ChildBorder.DesiredSize, "#8");
		}

		[TestMethod]
		public void ArrangeTest_CheckScaling()
		{
			Child.MeasureFunc = () => new Size(50, 50);

			Viewbox.Measure(new Size(50, 50));
			Viewbox.Arrange(new Rect(0, 0, 30, 70));

			Assert.AreEqual (new Size (50, 50), Child.ArrangeArg, "#a");
			Assert.AreEqual (new Size (50, 50), Child.ArrangeResult, "#b");

			Assert.AreEqual (new Size (50, 50), ChildBorder.DesiredSize, "#c");
			Assert.AreEqual (new Size (50, 50), Child.DesiredSize, "#d");

			var scale = (ScaleTransform)((Border)VisualTreeHelper.GetParent(Child)).RenderTransform;
			Assert.AreEqual(scale.CenterX, 0, "#3");
			Assert.AreEqual(scale.CenterY, 0, "#4");
			Assert.AreEqual(scale.ScaleX, 1, "#5");
			Assert.AreEqual(scale.ScaleY, 1, "#6");
		}

		[TestMethod]
		public void MeasureTest()
		{
			Viewbox.Measure(Infinity);
			Assert.AreEqual(Infinity, Child.MeasureArg, "#1");

			Viewbox.Measure(new Size (50, 50));
			Assert.AreEqual(Infinity, Child.MeasureArg, "#2");
		}

		[TestMethod]
		[MoonlightBug ("The border tries to set the logical parent too and blows up")]
		public void ParentTest()
		{
			Viewbox.Measure(Infinity);
			Assert.AreSame(Viewbox, Child.Parent, "#1");
			Assert.IsInstanceOfType<Border>(VisualTreeHelper.GetParent(Child), "#2");
		}

		[TestMethod]
		[Asynchronous]
		public void VisualTree_NoChild()
		{
			Viewbox box = new Viewbox();
			CreateAsyncTest(box,
				() => {
					/* Ensure that the box has been measured at least once */
				}, () => {
					Assert.VisualChildren(box, "#1",
						 new VisualNode<Border>("#a")
					);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void VisualTree_WithChild()
		{
			var box = new Viewbox();
			var child = new LayoutPoker();
			box.Child = child;

			CreateAsyncTest(box,
				() => {
					/* Ensure that the box has been measured at least once */
				}, () =>  {
					Assert.VisualChildren(box, "#1",
						new VisualNode<Border>("#a",
							new VisualNode<LayoutPoker>("#b")
						)
					);
				}
			);
		}
	}
}
