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
using System.Linq;

namespace MoonTest.System.Windows.Media
{
	[TestClass]
	public class ___VisualTreeHelperTest : Microsoft.Silverlight.Testing.SilverlightTest
	{
		Panel Root;
	
		[TestInitialize]
		public void Init ()
		{
            Root = new Canvas { HorizontalAlignment = HorizontalAlignment.Left,
                                VerticalAlignment = VerticalAlignment.Top,
                                Width = 1000,
                                Height = 1000,
                                Name = "Root"
            };
		}
			
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
		public void HitTest1()
		{
			Root.Dispatcher.BeginInvoke(delegate {
                List<UIElement> hits = new List<UIElement>(VisualTreeHelper.FindElementsInHostCoordinates(new Point(0, 0), Root));
				Assert.AreEqual(0, hits.Count, "#1");
				this.TestComplete ();
			});
		}

		[TestMethod]
		[Asynchronous]
		public void HitTest2()
		{
            Root.Background = new SolidColorBrush(Colors.Black);
            CreateAsyncTest(Root, delegate {
                List<UIElement> hits = new List<UIElement>(VisualTreeHelper.FindElementsInHostCoordinates(new Point(0, 0), Root));
                Assert.AreEqual(1, hits.Count, "#1");
            });
		}

		[TestMethod]
		[Asynchronous]
		public void HitTest3()
		{
            Root.Children.Add(new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush(Colors.Black) });
            CreateAsyncTest(Root, delegate {
                List<UIElement> hits = new List<UIElement>(VisualTreeHelper.FindElementsInHostCoordinates(new Point(5, 5), Root));
			    Assert.AreEqual(2, hits.Count, "#1");
			    Assert.IsTrue(hits[0] is Rectangle, "#2");
                Assert.IsTrue(hits[1] == Root, "#3");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void HitTest4()
		{
            Root.Children.Add((Path)XamlReader.Load(
@"<Path xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" x:Name=""ShipShell"" Stretch=""Fill"" Stroke=""#FF000000"" Width=""50"" Height=""44"" Canvas.Left=""1"" Canvas.Top=""1"" Data=""M256,82 L241,99 239,129 231,150 206,162 206,176 257,180 311,174 299,155 280,148 273,127 271,98 z"">
	<Path.Fill>
		<RadialGradientBrush>
			<GradientStop Color=""#FFE1D0D0"" Offset=""0""/>
			<GradientStop Color=""#FF673434"" Offset=""1""/>
			<GradientStop Color=""#FFE3D8D8"" Offset=""0.024""/>
		</RadialGradientBrush>
	</Path.Fill>
</Path>"));

            CreateAsyncTest(Root, delegate {
                List<UIElement> hits = new List<UIElement>(VisualTreeHelper.FindElementsInHostCoordinates(new Point(30, 30), Root));
			    Assert.AreEqual(2, hits.Count, "#1");
			    Assert.IsTrue(hits[0] is Path, "#2");
                Assert.IsTrue(hits[1] == Root, "#3");
		    });
		}
		
		[TestMethod]
		[Asynchronous]
		public void HitTest5()
		{
            Root.Children.Add(new Ellipse { Width = 100, Height = 100 });
            Root.Children.Add(new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush(Colors.Blue) });
            Root.Children.Add((Path)XamlReader.Load(
@"<Path xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" x:Name=""ShipShell"" Stretch=""Fill"" Stroke=""#FF000000"" Width=""50"" Height=""44"" Canvas.Left=""1"" Canvas.Top=""1"" Data=""M256,82 L241,99 239,129 231,150 206,162 206,176 257,180 311,174 299,155 280,148 273,127 271,98 z"">
	<Path.Fill>
		<RadialGradientBrush>
			<GradientStop Color=""#FFE1D0D0"" Offset=""0""/>
			<GradientStop Color=""#FF673434"" Offset=""1""/>
			<GradientStop Color=""#FFE3D8D8"" Offset=""0.024""/>
		</RadialGradientBrush>
	</Path.Fill>
</Path>"));

			CreateAsyncTest(Root, delegate {
                List<UIElement> hits = new List<UIElement>(VisualTreeHelper.FindElementsInHostCoordinates(new Point(30, 30), Root));
			    Assert.AreEqual(3, hits.Count, "#1");
			    Assert.IsTrue(hits[0] is Path, "Should be Path, was " + hits[0].GetType ().Name);
			    Assert.IsTrue(hits[1] is Rectangle, "Should be Rectangle, was " + hits[1].GetType ().Name);
			    Assert.IsTrue(hits[2] is Canvas, "Should be Canvas, was " + hits[2].GetType ().Name);
            });
		}
		
		[TestMethod]
		[Asynchronous]
		public void HitTest6()
		{
			Canvas panel = new Canvas { Width = 1000, Height = 1000, Name = "A" };
			panel.Children.Add(new Ellipse { Width = 100, Height = 100, Name = "A1" });
			panel.Children.Add(new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush(Colors.Blue), Name = "A2" });
			panel.Children.Add(new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush(Colors.Blue), Name = "A3" });
            Root.Children.Add(panel);

			panel = new Canvas { Width = 1000, Height = 1000, Name = "B" };
			panel.Children.Add(new Ellipse { Width = 100, Height = 100, Name = "B1" });
			panel.Children.Add(new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush(Colors.Blue), Name = "B2" });
			panel.Children.Add(new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush(Colors.Blue), Name = "B3" });
            Root.Children.Add(panel);

			CreateAsyncTest(Root, delegate {
                List<FrameworkElement> hits = VisualTreeHelper.FindElementsInHostCoordinates(new Point(30, 30), Root).Cast<FrameworkElement>().ToList();
			    Assert.AreEqual(7, hits.Count, "#1");
			    Assert.AreEqual("B3", hits[0].Name, "#2");
			    Assert.AreEqual("B2", hits[1].Name, "#3");
			    Assert.AreEqual("B", hits[2].Name, "#4");
			    Assert.AreEqual("A3", hits[3].Name, "#2");
			    Assert.AreEqual("A2", hits[4].Name, "#3");
			    Assert.AreEqual("A", hits[5].Name, "#4");
			    Assert.AreEqual("Root", hits[6].Name, "#4");
            });
		}
		
		[TestMethod]
		[Asynchronous]
		public void HitTest7()
		{
			Canvas panel = new Canvas { Width = 1000, Height = 1000, Name = "A" };
			panel.Children.Add(new Ellipse { Width = 100, Height = 100, Name = "A1" });
			panel.Children.Add(new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush(Colors.Blue), Name = "A2" });
			panel.Children.Add(new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush(Colors.Blue), Name = "A3" });
            Root.Children.Add(panel);

			Canvas.SetZIndex(panel, 1);
			Canvas.SetZIndex(panel.Children[0], 1);
			Canvas.SetZIndex(panel.Children[1], 2);
			Canvas.SetZIndex(panel.Children[2], 0);

			panel = new Canvas { Width = 1000, Height = 1000, Name = "B" };
			panel.Children.Add(new Ellipse { Width = 100, Height = 100, Name = "B1" });
			panel.Children.Add(new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush(Colors.Blue), Name = "B2" });
			panel.Children.Add(new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush(Colors.Blue), Name = "B3" });
            Root.Children.Add(panel);

			Canvas.SetZIndex(panel, 0);
			Canvas.SetZIndex(panel.Children[0], 20);
			Canvas.SetZIndex(panel.Children[1], 30);
			Canvas.SetZIndex(panel.Children[2], 10);

            CreateAsyncTest(Root, delegate {
                List<FrameworkElement> hits = VisualTreeHelper.FindElementsInHostCoordinates(new Point(30, 30), Root).Cast<FrameworkElement>().ToList();
			    Assert.AreEqual(7, hits.Count, "#1");
			    Assert.AreEqual("A2", hits[0].Name, "#2");
			    Assert.AreEqual("A3", hits[1].Name, "#3");
			    Assert.AreEqual("A", hits[2].Name, "#4");
			    Assert.AreEqual("B2", hits[3].Name, "#2");
			    Assert.AreEqual("B3", hits[4].Name, "#3");
			    Assert.AreEqual("B", hits[5].Name, "#4");
			    Assert.AreEqual("Root", hits[6].Name, "#4");
            });
		}
	}
}