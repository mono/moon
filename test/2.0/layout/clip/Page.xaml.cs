using System;
using System.Collections.Generic;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Threading;
using System.Xml;
using System.IO;
using System.Text;

namespace clip
{

    public partial class Page : UserControl
    {
        static StringBuilder sb = new StringBuilder();

		
        public Page()
        {
		InitializeComponent();
		Queue (()=> {
				DumpLayoutTree (this);
			});
        }

        public void Log(string message, params object[] formatting)
        {
            try
            {
                message = string.Format(message, formatting);
                Console.WriteLine(message);
                sb.AppendFormat(message, formatting);
                sb.AppendLine();
                this.Logger.Text += Environment.NewLine + message;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                throw;
            }
        }

        void DumpTree(DependencyObject d)
        {
            StringBuilder sb = new StringBuilder();

            while (d != null && VisualTreeHelper.GetParent(d) != null)
                d = VisualTreeHelper.GetParent(d);

            if (d != null)
                DumpTree(d, sb, 0);

            Console.WriteLine();
            Console.WriteLine();
            Console.WriteLine();
            Console.WriteLine(sb.ToString());
        }

        void DumpLayoutTree(DependencyObject d)
        {
            StringBuilder sb = new StringBuilder();

            while (d != null && VisualTreeHelper.GetParent(d) != null)
                d = VisualTreeHelper.GetParent(d);

            if (d != null)
                DumpLayoutTree(d, sb, 0);

            Console.WriteLine();
            Console.WriteLine();
            Console.WriteLine();
            Console.WriteLine(sb.ToString());
			Log (sb.ToString ());
        }

        void DumpTree(DependencyObject d, StringBuilder sb, int depth)
        {
            string s = string.Format("{0} - {1}", d.GetType(), d.GetValue(FrameworkElement.NameProperty));
            DependencyObject parent = d is FrameworkElement ? ((FrameworkElement)d).Parent : null;
            object name = parent == null ? null : parent.GetValue(FrameworkElement.NameProperty);
            sb.AppendLine(string.Format("{1}<{0} Parent='{2}' - '{3}'>", s, new string('	', depth), parent, name));
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(d); i++)
                DumpTree(VisualTreeHelper.GetChild(d, i), sb, depth + 1);

            sb.AppendLine(string.Format("{1}</{0}>", s, new string('	', depth)));
        }

        void DumpLayoutTree(DependencyObject d, StringBuilder sb, int depth)
        {
            string s = string.Format("<{0} Actual='{2}/{3}' Desired='{4}/{5}' Rendered='{6}/{7}' MinHeight='{8}' MaxHeight='{9}' MinWidth='{10}' MaxWidth='{11}' Width='{12}' Height='{13}' Margin='{14}' LayoutSlot='{15}' LayoutClip='{16}'{17}>",
                		d.GetType().Name,
				null,
                		((FrameworkElement)d).ActualWidth,
                		((FrameworkElement)d).ActualHeight,
                		((FrameworkElement)d).DesiredSize.Width,
                		((FrameworkElement)d).DesiredSize.Height,
                		((FrameworkElement)d).RenderSize.Width,
                		((FrameworkElement)d).RenderSize.Height,
				((FrameworkElement)d).MinHeight,
                		((FrameworkElement)d).MaxHeight,
				((FrameworkElement)d).MinWidth,
                		((FrameworkElement)d).MaxWidth,
				     ((FrameworkElement)d).Width,
				     ((FrameworkElement)d).Height,
				((FrameworkElement)d).Margin,
				LayoutInformation.GetLayoutSlot (((FrameworkElement)d)),
				FormatLayoutClip ((FrameworkElement)d),
				VisualTreeHelper.GetChildrenCount(d) == 0 ? "/" : "" );

            sb.AppendLine(string.Format("{0}{1}", new string('	', depth), s));
	    if (VisualTreeHelper.GetChildrenCount (d) > 0) {

            	for (int i = 0; i < VisualTreeHelper.GetChildrenCount(d); i++)
                	DumpLayoutTree(VisualTreeHelper.GetChild(d, i), sb, depth + 1);

            	sb.AppendLine(string.Format("{0}</{1}>", new string('	', depth), d.GetType().Name));
	    }
        }

	string FormatLayoutClip (FrameworkElement f)
	{
		Geometry g = LayoutInformation.GetLayoutClip (f);
		if (g == null)
			return "(null)";
		if (g.GetType() != typeof (RectangleGeometry)) {
			return string.Format ("Complex({0})", g.GetType());
		}

		RectangleGeometry rg = (RectangleGeometry)g;

		return string.Format ("Rectangle({0})", rg.Rect);
	}

        Queue<Action> actions = new Queue<Action>();
        void Queue(Action action)
        {
            Queue(action, 2000);
        }


        void Queue(Action action, int sleep)
        {
            System.Windows.Threading.Dispatcher d = Dispatcher;
            ThreadPool.QueueUserWorkItem((a) =>
            {
                System.Threading.Thread.Sleep(sleep);
                d.BeginInvoke(action);
            });
        }

    }
}
