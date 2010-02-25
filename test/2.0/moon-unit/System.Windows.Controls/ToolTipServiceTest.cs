using System;
using System.Windows.Controls;
using System.Windows;
using System.Windows.Shapes;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using Mono.Moonlight.UnitTesting;

namespace MoonTest
{

	[TestClass]
	public partial class ToolTipServiceTest : SilverlightTest {

		[TestMethod]
		public void ToolTipDoesSetParent ()
		{
			// ToolTip does not set "ContentControl.ContentSetsParent" to false
			var data = new Rectangle ();
			var target = new Rectangle ();

			TestPanel.Children.Add (data);
			Assert.Throws<ArgumentException> (() =>
				ToolTipService.SetToolTip (TestPanel, data)
			);
		}

		[TestMethod]
		public void CanSetToolTipTwice ()
		{
			// ToolTip does not set "ContentControl.ContentSetsParent" to false
			var data = new Rectangle ();

			ToolTipService.SetToolTip (TestPanel, data);
			ToolTipService.SetToolTip (TestPanel, data);
		}

		[TestMethod]
		public void CanSetTooltipOnMultipleObjects ()
		{
			// ToolTip does not set "ContentControl.ContentSetsParent" to false
			var data = new Rectangle ();
			var target1 = new Grid ();
			var target2 = new Grid ();

			ToolTipService.SetToolTip (target1, data);
			ToolTipService.SetToolTip (target2, data);
			ToolTipService.SetToolTip (TestPanel, data);
		}

		[TestMethod]
		public void SameTooltipObjectForMultipleObjects ()
		{
			// ToolTip does not set "ContentControl.ContentSetsParent" to false
			var data = new Rectangle ();
			var target1 = new Grid ();
			var target2 = new Grid ();

			ToolTipService.SetToolTip (target1, data);
			ToolTipService.SetToolTip (target2, data);
			ToolTipService.SetToolTip (TestPanel, data);

			var a = (FrameworkElement) ToolTipService.GetToolTip (target1);
			var b = (FrameworkElement) ToolTipService.GetToolTip (target2);
			var c = (FrameworkElement) ToolTipService.GetToolTip (TestPanel);

			Assert.IsInstanceOfType<ToolTip> (a.Parent, "#1");
			Assert.AreSame (a.Parent, b.Parent, "#2");
			Assert.AreSame (b.Parent, c.Parent, "#3");
		}

		[TestMethod]
		public void SameTooltipObjectForMultipleObjects_RemoveFromBoth ()
		{
			// ToolTip does not set "ContentControl.ContentSetsParent" to false
			var data = new Rectangle ();
			var target1 = new Grid ();
			var target2 = new Grid ();

			ToolTipService.SetToolTip (target1, data);
			ToolTipService.SetToolTip (target2, data);

			target1.ClearValue (ToolTipService.ToolTipProperty);
			target2.ClearValue (ToolTipService.ToolTipProperty);
			Assert.IsInstanceOfType<ToolTip> (data.Parent, "#1");
		}

		[TestMethod]
		public void SameTooltipObjectForMultipleObjects_RemoveFromOwner ()
		{
			// ToolTip does not set "ContentControl.ContentSetsParent" to false
			var data = new Rectangle ();
			var target1 = new Grid ();
			var target2 = new Grid ();

			ToolTipService.SetToolTip (target1, data);
			ToolTipService.SetToolTip (target2, data);

			target1.ClearValue (ToolTipService.ToolTipProperty);
			Assert.IsInstanceOfType<ToolTip> (data.Parent, "#1");
		}

		[TestMethod]
		public void SameTooltipObjectForMultipleObjects_RemoveFromSecond ()
		{
			// ToolTip does not set "ContentControl.ContentSetsParent" to false
			var data = new Rectangle ();
			var target1 = new Grid ();
			var target2 = new Grid ();

			ToolTipService.SetToolTip (target1, data);
			ToolTipService.SetToolTip (target2, data);

			target2.ClearValue (ToolTipService.ToolTipProperty);
			Assert.IsInstanceOfType<ToolTip> (data.Parent);
		}
	}
}
