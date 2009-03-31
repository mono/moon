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
using System.Windows.Controls.Primitives;
using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using System.Threading;

namespace MoonTest.System.Windows.Controls.Primitives
{
//	[TestClass]
	public class PopupTest : Microsoft.Silverlight.Testing.SilverlightTest
	{
		[TestMethod]
		public void Defaults ()
		{
			Popup p = new Popup ();
			Assert.IsNull (p.Child, "#1");
			Assert.AreEqual (p.HorizontalAlignment, HorizontalAlignment.Stretch, "#2");
			Assert.IsFalse (p.IsOpen, "#3");
			Assert.AreEqual (p.VerticalAlignment, VerticalAlignment.Stretch, "#4");
		}

		[TestMethod]
		[Asynchronous]
		public void OpenCloseEventTest ()
		{
			bool opened = false;
			Button b = new Button { Content = "Close" };
			Popup p = new Popup { Child = b };

			p.Opened += delegate { Assert.IsFalse (opened, "#1"); opened = true; };
			p.Closed += delegate { Assert.IsTrue (opened, "#2"); opened = false; };

			Enqueue (() => p.IsOpen = true);
			Enqueue (() => {
				Assert.IsTrue (p.IsOpen);
				Assert.IsNull (p.Parent);
				p.IsOpen = false;
			});
			Enqueue (() => {
				Assert.IsFalse (p.IsOpen);
				Assert.IsNull (p.Parent);
			});

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void OpenCloseEventTest2 ()
		{
			bool opened = false;
			Button b = new Button { Content = "Close" };
			Popup p = new Popup { Child = b };
			Canvas.SetLeft (p, 200);
			Canvas.SetTop (p, 200);
			p.Opened += delegate { Assert.IsFalse (opened, "#1"); opened = true; };
			p.Closed += delegate { Assert.IsTrue (opened, "#2"); opened = false; };

			Enqueue (() => p.IsOpen = true);
			Enqueue (() => {
				Assert.IsTrue (p.IsOpen);
				Assert.IsNull (p.Parent);
				Assert.AreEqual (0, Canvas.GetLeft (b));
				Assert.AreEqual (0, Canvas.GetTop (b));
				p.IsOpen = false;
			});
			Enqueue (() => {
				Assert.IsFalse (p.IsOpen);
				Assert.IsNull (p.Parent);
			});

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void OpenCloseEventTest3 ()
		{
			bool opened = false;
			Button b = new Button { Content = "Close" };
			Popup p = new Popup { Child = b };
			p.VerticalOffset = 100;
			p.HorizontalOffset = 100;
			Canvas.SetLeft (p, 100);
			Canvas.SetTop (p, 100);
			Canvas.SetLeft (b, 100);
			Canvas.SetTop (b, 100);
			p.Opened += delegate { Assert.IsFalse (opened, "#1"); opened = true; };
			p.Closed += delegate { Assert.IsTrue (opened, "#2"); opened = false; };

			Enqueue (() => p.IsOpen = true);
			Enqueue (() => {
				Assert.IsTrue (p.IsOpen);
				Assert.IsNull (p.Parent);
				Assert.AreEqual (100, Canvas.GetLeft (b));
				Assert.AreEqual (100, Canvas.GetTop (b));
				p.IsOpen = false;
			});
			Enqueue (() => {
				Assert.IsFalse (p.IsOpen);
				Assert.IsNull (p.Parent);
			});

			EnqueueTestComplete ();
		}

		[TestMethod]
		[MoonlightBug]
		public void PopupParent ()
		{
			Rectangle r = new Rectangle { Fill = new SolidColorBrush (Colors.Blue) };
			Popup pop = new Popup ();
			pop.Child = r;
			Assert.AreEqual (pop, r.Parent);
		}

		[TestMethod]
		public void VisualChildren ()
		{
			Rectangle r = new Rectangle ();
			Popup p = new Popup { Child = r };

			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (r), "#1");
			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (p), "#2");
			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (TestPanel), "#3");

			TestPanel.Children.Add (p);
			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (r), "#4");
			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (p), "#5");
			Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (TestPanel), "#6");

			p.IsOpen = true;
			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (r), "#7");
			Assert.AreEqual (0, VisualTreeHelper.GetChildrenCount (p), "#8");
			Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (TestPanel), "#9");

			Assert.AreEqual (1, VisualTreeHelper.GetChildrenCount (TestPanel.Parent), "#10");
		}

		[TestMethod]
		[MoonlightBug]
		public void VisualParent ()
		{
			Rectangle r = new Rectangle ();
			Popup p = new Popup { Child = r };

			Assert.AreEqual (null, VisualTreeHelper.GetParent (r), "#1");
			Assert.AreEqual (null, VisualTreeHelper.GetParent (p), "#2");
			TestPanel.Children.Add (p);
			Assert.AreEqual (null, VisualTreeHelper.GetParent (r), "#3");
			Assert.AreEqual (TestPanel, VisualTreeHelper.GetParent (p), "#4");
			p.IsOpen = true;
			Assert.AreEqual (null, VisualTreeHelper.GetParent (r), "#5");
			Assert.AreEqual (TestPanel, VisualTreeHelper.GetParent (p), "#6");
		}

		[TestMethod]
		[MoonlightBug]
		public void VisualTree ()
		{
			Rectangle r = new Rectangle { Fill = new SolidColorBrush (Colors.Blue) };
			TestPanel.Children.Add (r);
			Assert.Throws<ArgumentException> (() => new Popup { Child = r });
		}

		[TestMethod]
		[MoonlightBug]
		public void VisualTree2 ()
		{
			Rectangle r = new Rectangle ();
			Popup p = new Popup { Child = r };
			Assert.AreEqual (p, r.Parent, "#1");
			TestPanel.Children.Add (r);
			Assert.AreEqual (p, r.Parent, "#2");
			p.Child = null;
			Assert.AreEqual (TestPanel, r.Parent, "#3");
			Assert.Throws<ArgumentException> (() => p.Child = r, "#4");
		}

		[TestMethod]
		public void VisualTree3 ()
		{
			Popup pop = new Popup ();
			TestPanel.Children.Add (pop);
			Assert.Throws<InvalidOperationException> (() => TestPanel.Children.Add (pop));
		}

		[TestMethod]
		[MoonlightBug]
		public void VisualTree4 ()
		{
			Rectangle r = new Rectangle ();
			Popup p1 = new Popup { Child = r };
			Assert.Throws<InvalidOperationException> (() => new Popup { Child = r }); 
		}
	}
}
