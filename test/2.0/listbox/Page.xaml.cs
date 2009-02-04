using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace listbox
{
	public partial class Page : UserControl
	{
		public Page()
		{
			InitializeComponent();
		}

		public void button_Click (object sender, RoutedEventArgs e)
		{
			Button b = new Button ();
			b.Content = string.Format ("added button {0}", button_number++);
			itemsControl.Items.Add (b);
		}

		int button_number;
	}
}
