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

namespace dependency_properties
{
	public partial class Page : Canvas
	{
		public Page()
		{
			InitializeComponent();
		}
		public void AddLine(string line)
		{
			Output.Text += line + "\n";
		}
		public void Add (string text)
		{
			Output.Text += text;
		}

		private void Output_MouseLeftButtonDown (object sender, MouseButtonEventArgs e)
		{
			System.Windows.Browser.HtmlPage.Window.Eval ("OnCanvasClick ();");
		}
	}
}
