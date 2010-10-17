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
		public void SameTooltipObjectForMultipleObjects_TooltipObject ()
		{
			// We can set multiple tooltips with the same name without getting
			// a name collision as the Name is not registered.
			string name = "TooltipNameNotRegistered";
			var target1 = new Grid ();
			var target2 = new Grid ();
			TestPanel.Children.Add (target1);
			TestPanel.Children.Add (target2);

			ToolTipService.SetToolTip (target1, new ToolTip { Name = name });
			ToolTipService.SetToolTip (target2, new ToolTip { Name = name });
			ToolTipService.SetToolTip (TestPanel, new ToolTip { Name = name });

			Assert.IsNull (TestPanel.FindName (name), "#1");
		}

		[TestMethod]
		public void SamePlacementTarget_MultipleTooltips ()
		{
			// We can use the same object and name multiple times for PlacementTarget
			// as the Name is not registered.
			var placementTarget = new ToolTip { Name = "TooltipNameNotRegistered" };
			var target1 = new Grid ();
			var target2 = new Grid ();

			TestPanel.Children.Add (target1);
			TestPanel.Children.Add (target2);

			ToolTipService.SetPlacementTarget (target1, placementTarget);
			ToolTipService.SetPlacementTarget (target2, placementTarget);
			ToolTipService.SetPlacementTarget (TestPanel, new ToolTip { Name = "TooltipNameNotRegistered" });

			Assert.IsNull (TestPanel.FindName (placementTarget.Name), "#1");
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
