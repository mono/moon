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

namespace NS
{
    public partial class PopupTestClass
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
    public class PopupTest
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
        [MoonlightBug]
        public void CreateVisiblePopup()
        {
            Popup p = new Popup();
            Border border = new Border { BorderBrush = new SolidColorBrush(Colors.Blue), BorderThickness = new Thickness(1) };

            Canvas c = new Canvas();

            Button b = new Button();
            b.Click += delegate { p.IsOpen = false; };
            b.Content = "Close";
            c.Children.Add(b);
            border.Child = c;

            p.Child = border;
            p.IsOpen = true;
        }
    }
}
