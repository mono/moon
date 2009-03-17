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


namespace FindName
{
	public partial class Page : UserControl
	{
		public Page()
		{
			InitializeComponent();

//			Console.WriteLine ("finding the canvas:  {0}", FindName ("_canvas"));

			debug_text1.Text = "my_canvas: " + (FindName ("my_canvas"));
			debug_text2.Text = "sub_element: " + (FindName ("sub_element"));

			FrameworkElement se = (FrameworkElement) my_canvas.FindName ("sub_element");
			debug_text3.Text = "my_canvas.sub_element: " + se;
			debug_text4.Text = "my_canvas.sub_element.sub_element: " + se.FindName ("sub_element");
		}
	}
}
