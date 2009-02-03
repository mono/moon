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
using System.Collections.Generic;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing.UnitTesting.UI;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata.VisualStudio;
using Microsoft.Silverlight.Testing;
using System.Windows.Markup;

namespace MoonTest.System.Windows.Media
{
	[TestClass]
	public class VisualTreeHelperTest : Microsoft.Silverlight.Testing.SilverlightTest
	{
		UIElementCollection Root;
		[TestMethod]
		public void NullTests ()
		{
			Assert.Throws (delegate { VisualTreeHelper.GetChild (null, 1); }, typeof (InvalidOperationException));
			Assert.Throws (delegate { VisualTreeHelper.GetChildrenCount (null); }, typeof (InvalidOperationException));
			Assert.Throws (delegate { VisualTreeHelper.GetParent (null); }, typeof (InvalidOperationException));
		}

		[TestMethod]
		public void GetChild ()
		{
			Canvas p = new Canvas();
			Canvas p2 = new Canvas ();
			p.Children.Add (p2);

			Assert.AreEqual (p2, VisualTreeHelper.GetChild (p, 0));
			Assert.Throws (delegate { VisualTreeHelper.GetChild (p, 1); }, typeof (ArgumentOutOfRangeException));

			Assert.Throws (delegate { VisualTreeHelper.GetChild (new Ellipse(), 1); }, typeof (ArgumentOutOfRangeException));
		}

		[TestMethod]
		public void GetChildrenCount ()
		{
			Canvas p = new Canvas ();

			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (p));

			p.Children.Add (new Canvas());

			Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (p));

			//			p.Resources.Add ("foo", new Canvas());

			Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (p));

			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (new Ellipse()));
		}

		[TestMethod]
		public void GetParent ()
		{
			Canvas p = new Canvas ();
			Canvas p2 = new Canvas ();
			Canvas p3 = new Canvas ();
			SolidColorBrush scb = new SolidColorBrush ();

			p.Children.Add (p2);
			//			p.Resources.Add ("foo", p3);

			p.Background = scb;

			Assert.AreEqual (p, VisualTreeHelper.GetParent (p2));
			//Assert.AreEqual (p, VisualTreeHelper.GetParent (p3));
			Assert.AreEqual (null, VisualTreeHelper.GetParent (p));

			Assert.Throws (delegate { VisualTreeHelper.GetParent (scb); }, typeof (InvalidOperationException));
		}
		
		[TestMethod]
		[Asynchronous]
		[Ignore]
		public void HitTest1()
		{
			Panel panel = ((TestPage)Application.Current.RootVisual).TestPanel;
			panel.Width = 1000;
			panel.Height = 1000;
			panel.Dispatcher.BeginInvoke(delegate {
				List<UIElement> hits = new List<UIElement>(VisualTreeHelper.FindElementsInHostCoordinates(new Point (0, 0), panel));
				Assert.AreEqual(0, hits.Count, "#1");
				this.TestComplete ();
			});
		}

		[TestMethod]
		[Asynchronous]
		[Ignore]
		public void HitTest2()
		{
			Panel panel = ((TestPage)Application.Current.RootVisual).TestPanel;
			panel.Background = new SolidColorBrush(Colors.Black);
			panel.Width = 1000;
			panel.Height = 1000;
			panel.Dispatcher.BeginInvoke(delegate {
				List<UIElement> hits = new List<UIElement>(VisualTreeHelper.FindElementsInHostCoordinates(new Point(0, 0), panel));
				Assert.AreEqual(1, hits.Count, "#1");
				this.TestComplete();
			});
		}
		[TestMethod]
		[Asynchronous]
		[Ignore]
		public void HitTest3()
		{
			Root = ((TestPage)Application.Current.RootVisual).TestPanel.Children;
			Canvas panel = new Canvas {Width =1000, Height = 1000 };
			panel.Children.Add(new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush(Colors.Black) });

			Root.Add(panel);
			panel.Dispatcher.BeginInvoke(delegate {
				List<UIElement> hits = new List<UIElement>(VisualTreeHelper.FindElementsInHostCoordinates(new Rect(0, 0, 5, 5), panel));
				Assert.AreEqual(2, hits.Count, "#1");
				Assert.IsTrue(hits[0] is Rectangle, "#2");
				Assert.IsTrue(hits[1] == panel, "#3");
				this.TestComplete();
			});
		}

		[TestMethod]
		[Asynchronous]
		[Ignore]
		public void HitTest4()
		{
			Root = ((TestPage)Application.Current.RootVisual).TestPanel.Children;
			Canvas panel = new Canvas { Width = 1000, Height = 1000 };
			panel.Children.Add((Path)XamlReader.Load(
@"<Path xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" x:Name=""ShipShell"" Stretch=""Fill"" Stroke=""#FF000000"" Width=""50"" Height=""44"" Canvas.Left=""1"" Canvas.Top=""1"" Data=""M256,82 L241,99 239,129 231,150 206,162 206,176 257,180 311,174 299,155 280,148 273,127 271,98 z"">
	<Path.Fill>
		<RadialGradientBrush>
			<GradientStop Color=""#FFE1D0D0"" Offset=""0""/>
			<GradientStop Color=""#FF673434"" Offset=""1""/>
			<GradientStop Color=""#FFE3D8D8"" Offset=""0.024""/>
		</RadialGradientBrush>
	</Path.Fill>
</Path>"));

			Root.Add(panel);
			panel.Dispatcher.BeginInvoke(delegate
			{
				List<UIElement> hits = new List<UIElement>(VisualTreeHelper.FindElementsInHostCoordinates(new Point (30, 30), panel));
				Assert.AreEqual(2, hits.Count, "#1");
				Assert.IsTrue(hits[0] is Path, "#2");
				Assert.IsTrue(hits[1] == panel, "#3");
				this.TestComplete();
			});
		}
	}
}