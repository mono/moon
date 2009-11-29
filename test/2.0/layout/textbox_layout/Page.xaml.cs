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
using System.Text;
using System.Threading;
using System.Windows.Controls.Primitives;
using System.Collections.Generic;
using System.Windows.Markup;
using System.Linq;

namespace textbox_layout
{
    public partial class Page : Canvas
    {
        static TextBox Logger = new TextBox { Text = "A", AcceptsReturn = true, Height = 500, MaxHeight = 500 };

        static StringBuilder sb = new StringBuilder();

	public Page ()
	{
		InitializeComponent ();

		Row2.Children.Add (Logger);

		PanelFactory panelFactory = new ManagedStackPanelFactory ();
		Grid grid1 = CreateGrid(Orientation.Vertical, panelFactory);

		Canvas.SetLeft (grid1, 600);
		Canvas.SetTop (grid1, 10);
		Children.Add (grid1);

		Queue (()=> {
		           DumpLayoutTree (this);
		       });
	}

        public static void Log(string message, params object[] formatting)
        {
            try
            {
                message = string.Format(message, formatting);
                Console.WriteLine(message);
                sb.AppendFormat(message, formatting);
                sb.AppendLine();
                Logger.Text += Environment.NewLine + message;
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
            string s = string.Format("<{0} Actual='{2}/{3}' Desired='{4}/{5}' Rendered='{6}/{7}' MinHeight='{8}' MaxHeight='{9}' MinWidth='{10}' MaxWidth='{11}' Margin='{12}' LayoutSlot='{13}' LayoutClip='{14}'{15}>",
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


        private static Grid CreateGrid(Orientation innerPanelOrientation, PanelFactory panelFactory)
        {
            Grid grid1 = new Grid();
            grid1.Background = new SolidColorBrush(Colors.Orange);
            grid1.Margin = new Thickness(3);
            grid1.Width = 230;
            grid1.Height = 260;
            grid1.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(80) });
            grid1.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Auto) });
            grid1.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
            grid1.RowDefinitions.Add(new RowDefinition { Height = new GridLength(80) });
            grid1.RowDefinitions.Add(new RowDefinition { Height = new GridLength(1, GridUnitType.Auto) });
            grid1.RowDefinitions.Add(new RowDefinition { Height = new GridLength(1, GridUnitType.Star) });

            for (int i = 0; i < 3; i++)
            {
                for (int j = 0; j < 3; j++)
                {
                    Panel panel;
                    if (innerPanelOrientation == Orientation.Horizontal)
                    {
                        panel = CreateInnerHorizontalPanel(panelFactory);
                    }
                    else
                    {
                        panel = CreateInnerVerticalPanel(panelFactory);
                    }
                    panel.Width = Double.NaN;
                    panel.Height = Double.NaN;
                    panel.VerticalAlignment = VerticalAlignment.Stretch;
                    panel.HorizontalAlignment = HorizontalAlignment.Stretch;
                    panel.SetValue(Grid.ColumnProperty, i);
                    panel.SetValue(Grid.RowProperty, j);
                    grid1.Children.Add(panel);
                }
            }
            return grid1;
        }

        private static Panel CreateInnerHorizontalPanel(PanelFactory panelFactory)
        {
            Panel panel1 = panelFactory.CreatePanel(Orientation.Horizontal);
            panel1.Background = new SolidColorBrush(Colors.Green);
            panel1.Margin = new Thickness(5);
            panel1.Height = 50;
            panel1.Width = 100;

            AddRectanglesToPanel(panel1, 20, 20);

            return panel1;
        }

        private static Panel CreateInnerVerticalPanel(PanelFactory panelFactory)
        {
            Panel panel1 = panelFactory.CreatePanel(Orientation.Vertical);
            panel1.Background = new SolidColorBrush(Colors.Green);
            panel1.Margin = new Thickness(5);
            panel1.Height = 100;
            panel1.Width = 50;

            AddRectanglesToPanel(panel1, 20, 20);

            return panel1;
        }

        private static Rectangle CreateRectangle()
        {
            Rectangle rectangle = new Rectangle();
            rectangle.Fill = new SolidColorBrush(Colors.Red);
            rectangle.Stroke = new SolidColorBrush(Colors.Black);
            rectangle.StrokeThickness = 3;
            rectangle.Margin = new Thickness(2);
            return rectangle;
        }

        private static void AddRectanglesToPanel(Panel panel, double width, double height)
        {
            Rectangle topLeftRect = CreateRectangle();
            topLeftRect.Width = width;
            topLeftRect.Height = height;
            topLeftRect.HorizontalAlignment = HorizontalAlignment.Left;
            topLeftRect.VerticalAlignment = VerticalAlignment.Top;
            panel.Children.Add(topLeftRect);

            Rectangle collapsedRect = CreateRectangle();
            collapsedRect.Visibility = Visibility.Collapsed;
            panel.Children.Add(collapsedRect);

            Rectangle clippedRect = CreateRectangle();
            clippedRect.Width = width;
            clippedRect.Height = height;
            clippedRect.HorizontalAlignment = HorizontalAlignment.Center;
            clippedRect.VerticalAlignment = VerticalAlignment.Center;
            EllipseGeometry clipGeometry = new EllipseGeometry();
            clipGeometry.Center = new Point(0.5 * width, 0.5 * height);
            clipGeometry.RadiusX = 0.25 * width;
            clipGeometry.RadiusY = 0.4 * height;
            clippedRect.Clip = clipGeometry;
            panel.Children.Add(clippedRect);

            Rectangle bottomRightRect = CreateRectangle();
            bottomRightRect.Width = width;
            bottomRightRect.Height = height;
            bottomRightRect.HorizontalAlignment = HorizontalAlignment.Right;
            bottomRightRect.VerticalAlignment = VerticalAlignment.Bottom;
            panel.Children.Add(bottomRightRect);

            Rectangle stretchedRect = CreateRectangle();
            stretchedRect.Width = Double.NaN;
            stretchedRect.Height = Double.NaN;
            stretchedRect.MinWidth = width;
            stretchedRect.MinHeight = height;
            stretchedRect.HorizontalAlignment = HorizontalAlignment.Stretch;
            stretchedRect.VerticalAlignment = VerticalAlignment.Stretch;
            panel.Children.Add(stretchedRect);
        }

    }

    public abstract class PanelFactory
    {
        public abstract Panel CreatePanel();
        public abstract Panel CreatePanel(Orientation orientation);
    }

    public class ManagedStackPanelFactory : PanelFactory
    {
        public override Panel CreatePanel()
        {
            return new LayoutExtnCustomPanel.ManagedStackPanel();
        }

        public override Panel CreatePanel(Orientation orientation)
        {
            LayoutExtnCustomPanel.ManagedStackPanel stackPanel = new LayoutExtnCustomPanel.ManagedStackPanel();
            stackPanel.Orientation = orientation;
            return stackPanel;
        }
    }

}
