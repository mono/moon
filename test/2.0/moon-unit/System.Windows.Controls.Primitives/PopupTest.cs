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
using NS;
using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using System.Threading;

namespace NS
{
    public partial class PopupTestClass : global::System.Windows.Controls.Canvas
    {
        public PopupTestClass()
            : this(new Uri("/moon-unit;component/System.Windows/BasicCanvas.xaml", UriKind.Relative))
        {
        }

        public PopupTestClass(Uri uri)
        {
            Application.LoadComponent(this, uri);
            InitializeComponent();
            LayoutRoot = (Canvas) FindName("LayoutRoot");
        }
    }
}
namespace MoonTest.System.Windows.Controls.Primitives
{
    [TestClass]
    public class PopupTest : Microsoft.Silverlight.Testing.SilverlightTest
    {
        public static readonly DependencyProperty ChildProperty;
        public static readonly DependencyProperty HorizontalOffsetProperty;
        public static readonly DependencyProperty IsOpenProperty;
        public static readonly DependencyProperty VerticalOffsetProperty;

        [TestMethod]
        [MoonlightBug]
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
            PopupTestClass c = new PopupTestClass();
            Rectangle r = new Rectangle{ Fill = new SolidColorBrush (Colors.Blue)};
            c.LayoutRoot.Children.Add(r);
            Popup pop = new Popup();
            Assert.Throws<ArgumentException>(delegate {
                pop.Child = r;
            });
        }

        [TestMethod]
        [Asynchronous]
        [Ignore ("This causes mono to crash horribly")]
        public void OpenCloseEventTest()
        {
            Button b = new Button { Content = "Close" };
            Canvas canvas = new Canvas { Width = 100, Height = 100 };
            canvas.Children.Add(b);
            Border border = new Border
            {
                BorderBrush = new SolidColorBrush(Colors.Blue),
                BorderThickness = new Thickness(1),
                Child = canvas
            };
            Popup p = new Popup { Child = border };

            ManualResetEvent handle = new ManualResetEvent(false);
            p.Opened += delegate {
                if (handle.WaitOne(10))
                    throw new Exception("Already open");
                handle.Set();
            };
            p.Closed += delegate {
                if (handle.WaitOne(10))
                    throw new Exception("Not already open");
                handle.Set();
            };
            global::System.Threading.ThreadPool.QueueUserWorkItem(delegate {
                p.Dispatcher.BeginInvoke(delegate {
                    p.IsOpen = true;
                    object o = p.Parent;
                    o = null;
                });
                if (!handle.WaitOne(500))
                    throw new Exception("Popup wasn't opened");
                handle.Reset();
                global::System.Threading.Thread.Sleep(10000);
                p.Dispatcher.BeginInvoke(delegate { p.IsOpen = false; });
                if (!handle.WaitOne(500))
                    throw new Exception("Popup wasn't closed");
                handle.Reset();
                EnqueueTestComplete();
            });
        }
    }
}
