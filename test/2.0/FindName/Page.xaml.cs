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

			Console.WriteLine ("finding the canvas:  {0}", FindName ("my_canvas"));

			debug_text1.Text = "Find name is null?  " + (FindName ("my_canvas") == null);
		}
	}
}
