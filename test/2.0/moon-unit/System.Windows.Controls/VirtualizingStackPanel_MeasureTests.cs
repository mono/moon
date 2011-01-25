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
using System.Windows.Controls.Primitives;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Windows.Markup;

namespace MoonTest.System.Windows.Controls
{
	[TestClass]
	public class VirtualizingStackPanel_MeasureTests : SilverlightTest  {
		public ItemsControl Control {
			get; set;
		}
		
		LayoutPoker[] Items {
			get; set;
		}

		[TestInitialize]
		public void Initialize()
		{
			Control = new ListBox();
			Control.ItemsPanel = (ItemsPanelTemplate)XamlReader.Load(@"
<ItemsPanelTemplate xmlns=""http://schemas.microsoft.com/client/2007""
			xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
		<VirtualizingStackPanel x:Name=""Virtual"" />
</ItemsPanelTemplate>");

			Items = new[] {
				new LayoutPoker { MeasureResult = new Size(30, 30), ArrangeResult = new Size(30, 30) },
				new LayoutPoker { MeasureResult = new Size(30, 30), ArrangeResult = new Size(30, 30) },
				new LayoutPoker { MeasureResult = new Size(30, 30), ArrangeResult = new Size(30, 30) },
				new LayoutPoker { MeasureResult = new Size(30, 30), ArrangeResult = new Size(30, 30) },
				new LayoutPoker { MeasureResult = new Size(30, 30), ArrangeResult = new Size(30, 30) },
				new LayoutPoker { MeasureResult = new Size(30, 30), ArrangeResult = new Size(30, 30) },
				new LayoutPoker { MeasureResult = new Size(30, 30), ArrangeResult = new Size(30, 30) },
			};

			foreach (var v in Items)
				Control.Items.Add(v);
		}

		[TestMethod]
		public void ClearChildren ()
		{
			TestPanel.Children.Add (Control);
			Control.UpdateLayout ();
			var vsp = Control.FindFirstChild <VirtualizingStackPanel> ();
			Assert.IsGreater (0, vsp.ExtentWidth, "#1");
			Assert.IsGreater (0, vsp.ExtentHeight, "#2");
			Assert.IsGreater (0, vsp.ViewportWidth, "#3");
			Assert.IsGreater (0, vsp.ViewportHeight, "#4");
			Control.Items.Clear ();
			Assert.IsGreater (0, vsp.ExtentWidth, "#5");
			Assert.IsGreater (0, vsp.ExtentHeight, "#6");
			Assert.IsGreater (0, vsp.ViewportWidth, "#7");
			Assert.IsGreater (0, vsp.ViewportHeight, "#8");
		}

		[TestMethod]
		public void MoreItemsThanHeight_CheckMeasure()
		{
			TestPanel.Children.Add(Control);

			Control.Height = 100;
			Control.Width = 100;

			Control.UpdateLayout();
			for (int i = 0; i < 4; i++)
				Assert.AreEqual(new Size(Double.PositiveInfinity, Double.PositiveInfinity), Items[i].MeasureArg, "#1." + i);
			for (int i = 4; i < Items.Length; i++)
				Assert.AreEqual(new Size(0, 0), Items[i].MeasureArg, "#2." + i);
		}

		[TestMethod]
		[Asynchronous]
		public void MoreItemsThanHeight_CheckArrange()
		{
			Control.Height = 100;
			Control.Width = 100;
			CreateAsyncTest(Control,
				() => Control.ApplyTemplate(),
				() => {
					for (int i = 0; i < 4; i++)
						Assert.AreEqual(new Size(30, 30), Items[i].ArrangeArg, "#1." + i);
					for (int i = 4; i < Items.Length; i++)
						Assert.AreEqual(new Size(0, 0), Items[i].ArrangeArg, "#2." + i);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void MaxHeightOnVSP_Scrolling()
		{
			// Give it a panel witha  tiny max height to verify that it does not
			// affect the children when scrolling.
			Control.ItemsPanel = (ItemsPanelTemplate)XamlReader.Load(@"
<ItemsPanelTemplate xmlns=""http://schemas.microsoft.com/client/2007""
			xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
		<VirtualizingStackPanel x:Name=""Virtual"" MaxHeight=""50"" MaxWidth=""50"" />
</ItemsPanelTemplate>");
			
			CreateAsyncTest(Control, () => {
				for (int i =0 ; i < 2; i++)
					Assert.AreEqual(new Size(double.PositiveInfinity, double.PositiveInfinity), Items[i].MeasureArg, "#1." + i);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void MaxHeightOnVSP_NonScrolling_Vertical()
		{
			// Give it a panel witha  tiny max height to verify that it does not
			// affect the children when scrolling.
			Control.ItemsPanel = (ItemsPanelTemplate)XamlReader.Load(@"
<ItemsPanelTemplate xmlns=""http://schemas.microsoft.com/client/2007""
			xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
		<VirtualizingStackPanel x:Name=""Virtual"" MaxHeight=""50"" MaxWidth=""50"" />
</ItemsPanelTemplate>");
			Control.ApplyTemplate();
			TestPanel.Height = 300;
			TestPanel.Width = 300;
			CreateAsyncTest(Control,
				() => {
					// Disable Scrolling
					var sv = Control.FindFirstChild<ScrollViewer>();
					sv.HorizontalScrollBarVisibility = ScrollBarVisibility.Disabled;
					sv.VerticalScrollBarVisibility = ScrollBarVisibility.Disabled;
				}, () => {
					// Verify we still pass infinity for the vertical
					for (int i = 0; i < 2; i++)
						Assert.AreEqual(new Size(44, double.PositiveInfinity), Items[i].MeasureArg, "#1." + i);
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void MaxHeightOnVSP_NonScrolling_Horizontal()
		{
			Control.ItemsPanel = (ItemsPanelTemplate)XamlReader.Load(@"
<ItemsPanelTemplate xmlns=""http://schemas.microsoft.com/client/2007""
			xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
		<VirtualizingStackPanel x:Name=""Virtual"" MaxHeight=""50"" MaxWidth=""50"" Orientation=""Horizontal"" />
</ItemsPanelTemplate>");
			Control.ApplyTemplate();
			TestPanel.Height = 300;
			TestPanel.Width = 300;
			CreateAsyncTest(Control,
				() => {
					// Disable scrolling
					var sv = Control.FindFirstChild<ScrollViewer>();
					sv.HorizontalScrollBarVisibility = ScrollBarVisibility.Disabled;
					sv.VerticalScrollBarVisibility = ScrollBarVisibility.Disabled;
				}, () => {
					// Verify that we still pass infinity on the horizontal
					for (int i = 0; i < 2; i++)
						Assert.AreEqual(new Size(double.PositiveInfinity, 44), Items[i].MeasureArg, "#1." + i);
				}
			);
		}
	}
}