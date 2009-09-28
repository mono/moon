using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Text;
using System.Windows.Controls.Primitives;
using System.Threading;
using System.Windows.Threading;

namespace LayoutTest5
{
    public partial class Page : UserControl
    {
        public Page()
        {
            InitializeComponent();
            MyContentControl c = new MyContentControl ();
            LayoutRoot.Children.Add (c);
            DispatcherTimer timer = new DispatcherTimer { Interval = TimeSpan.Zero };
            timer.Tick += delegate {
                LayoutRoot.Children.Add (new TextBlock  { Text = c.Measured ? "Success" : "Failure - The control should've been measured" });
                timer.Stop ();
            };
            timer.Start ();
        }

    }
    
    class MyContentControl : ContentControl
    {
        public bool Measured;

        public MyContentControl ()
        {
        }

        protected override Size MeasureOverride (Size availableSize)
        {
            Measured = true;
            return base.MeasureOverride (availableSize);
        }
    }
}
