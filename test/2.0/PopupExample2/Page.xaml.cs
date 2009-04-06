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
using System.Windows.Data;
using System.Windows.Controls.Primitives;
using System.Threading;
using System.Windows.Threading;

namespace PopupExample2
{
	public partial class Page : UserControl
	{
		Rectangle expected = new Rectangle { Opacity = 0.5, Width = 100, Height = 100, Fill = new SolidColorBrush (Colors.Black) };
		Popup p;
		Action Delay (Action action)
		{
			return () => {
				action ();
				Thread.Sleep (1000);
			};
		}
		public Page ()
		{
			InitializeComponent ();
			LayoutRoot.Children.Add (expected);
			LayoutRoot.Width = 500;
			LayoutRoot.Height = 500;
			Queue<Action> test = new Queue<Action> (new List<Action> {
				Test1,
				Test2,
				Test3,
				Test4,
				Test5,
				Test6,
				Test7,
				Test8,
				Test9
			});
			DispatcherTimer timer = new DispatcherTimer ();
			timer.Interval = TimeSpan.FromSeconds (1);
			timer.Tick += delegate {
				if (p != null)
					p.IsOpen = false;

				if(test.Count > 0)
					test.Dequeue ().Invoke ();
			};
			timer.Start ();
		}

		public void Test1 ()
		{
			p = new Popup { Child = new Button { Width=30, Height=30, Background = new SolidColorBrush (Colors.Brown), Content = "Ted" } };
			SetExpected (30, 30, 0, 0);
			p.IsOpen = true;
		}

		public void Test2 ()
		{
			Rectangle r = new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush (Colors.Brown) };
			r.RenderTransform = new TranslateTransform { X = 100, Y = 100 };
			SetExpected (100, 100, 100, 100);
			p = new Popup { Child = r };
			p.IsOpen = true;
		}

		public void Test3 ()
		{
			Rectangle r = new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush (Colors.Brown) };
			p = new Popup { Child = r };
			p.RenderTransform = new TranslateTransform { X = 100, Y = 100 };
			SetExpected (100, 100, 100, 100);
			p.IsOpen = true;
		}

		public void Test4 ()
		{
			Rectangle r = new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush (Colors.Brown) };
			p = new Popup { Child = r };
			RenderTransform = new TranslateTransform { X = 100, Y = 100 };
			SetExpected (100, 100, -100, -100);
			p.IsOpen = true;
		}

		public void Test5 ()
		{
			Rectangle r = new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush (Colors.Brown) };
			p = new Popup { Child = r };
			LayoutRoot.Children.Add (p);
			RenderTransform = new TranslateTransform { X = 100, Y = 100 };
			SetExpected (100, 100, 0, 0);
			p.IsOpen = true;
		}

		public void Test6 ()
		{
			Rectangle r = new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush (Colors.Brown) };
			p = new Popup { Child = r };
			LayoutRoot.Children.Add (p);
			RenderTransform = new TranslateTransform { X = 100, Y = 100 };
			p.RenderTransform = new TranslateTransform { X = 100, Y = 100 };
			r.RenderTransform = new TranslateTransform { X = 100, Y = 100 };
			SetExpected (100, 100, 200, 200);
			p.IsOpen = true;
		}

		public void Test7 ()
		{
			Rectangle r = new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush (Colors.Brown) };
			p = new Popup { Child = r };
			Canvas.SetLeft (p, 100);
			SetExpected (100, 100, 0, -100);
			p.IsOpen = true;
		}

		public void Test8 ()
		{
			Rectangle r = new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush (Colors.Brown) };
			p = new Popup { Child = r };
			p.HorizontalOffset = 100;
			p.VerticalOffset = 100;
			Canvas.SetLeft (p, -100);
			SetExpected (100, 100, -100, 0);
			p.IsOpen = true;
		}

		public void Test9 ()
		{
			Rectangle r = new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush (Colors.Orange) };
			p = new Popup { Child = r };
			p.HorizontalOffset = 100;
			p.VerticalOffset = 100;
			Canvas.SetLeft (p, -100);
			SetExpected (100, 100, -100, 0);
			p.IsOpen = true;
			Dispatcher.BeginInvoke (() => p.Child = new Rectangle { Width = 100, Height = 100, Fill = new SolidColorBrush (Colors.Green) }); 
		}

		void SetExpected (int width, int height, int left, int top)
		{
			expected.Width = width;
			expected.Height = height;
			Canvas.SetLeft (expected, left);
			Canvas.SetTop (expected, top);
		}
	}
}

