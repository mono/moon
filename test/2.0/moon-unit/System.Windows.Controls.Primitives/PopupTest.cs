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
    [TestClass]
	public class ___PopupTest : Microsoft.Silverlight.Testing.SilverlightTest
    {
        [TestMethod]
        public void Defaults()
        {
            Popup p = new Popup();
            Assert.IsNull(p.Child, "#1");
            Assert.AreEqual(p.HorizontalAlignment, HorizontalAlignment.Stretch, "#2");
            Assert.IsFalse(p.IsOpen, "#3");
            Assert.AreEqual(p.VerticalAlignment, VerticalAlignment.Stretch, "#4");
        }

        [TestMethod]
		[MoonlightBug]
        public void NotInVisualTree()
        {
            Rectangle r = new Rectangle{ Fill = new SolidColorBrush (Colors.Blue)};
            TestPanel.Children.Add (r);
            Popup pop = new Popup();
            Assert.Throws<ArgumentException>(delegate {
                pop.Child = r;
            });
        }

        [TestMethod]
        [Asynchronous]
        public void OpenCloseEventTest()
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
    }
}
