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

namespace LayoutTest4
{
    public partial class Page : UserControl
    {
        static StringBuilder sb = new StringBuilder();
        public static void Log(string message, params object[] formatting)
        {
            sb.AppendFormat(message, formatting);
            sb.AppendLine();
        }

        public Page()
        {
            InitializeComponent();
            PrintExpected ();
            LayoutRoot.MouseLeftButtonDown += delegate {
                Popup p = Tester.Popup;
                p.IsOpen = true;
                Queue(() => PrintActual());
            };
LayoutRoot.Children.Add (new TextBlock { Text = "Click on the empty canvas then check the term for the output" });
        }

        void PrintExpected()
        {
            Console.WriteLine();
            Console.WriteLine(@"Expected:
Loaded: A
OnApplyTemplate: A
MeasureOverride: A
OnApplyTemplate: B
MeasureOverride: B
Loaded: B
Loaded: Popup
Loaded: C
OnApplyTemplate: C
MeasureOverride: C
OnApplyTemplate: D
MeasureOverride: D
Loaded: D
}");
        }

        void PrintActual()
        {
            Console.WriteLine();
            Console.WriteLine("Actual:");
            Console.WriteLine(sb.ToString());
        }

        void Tester_Loaded(object sender, RoutedEventArgs e)
        {
            Log("Loaded: {0}", ((FrameworkElement)sender).Name);
        }

        void Queue(Action action)
        {
            System.Windows.Threading.Dispatcher d = Dispatcher;
            ThreadPool.QueueUserWorkItem (delegate {
                System.Threading.Thread.Sleep(1000);
                d.BeginInvoke(action);
            });
        }
    }

    public class Tester : ContentControl
    {
        public static Popup Popup;
        protected override Size MeasureOverride(Size availableSize)
        {
            Page.Log("MeasureOverride: {0}", Name);
            return base.MeasureOverride(availableSize);
        }

        public override void OnApplyTemplate()
        {
            Page.Log("OnApplyTemplate: {0}", Name);
            base.OnApplyTemplate();
            Popup = Popup ?? GetTemplateChild("Popup") as Popup;
        }
    }
}
