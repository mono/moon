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

			// debug_text.Text = "Doing lookups";
			DoLookups (this);
			DoLookups (my_canvas);
			DoLookups ((FrameworkElement) my_canvas.FindName ("sub_element"));
			DoLookups ((FrameworkElement) my_canvas.FindName ("component_element"));

		}

		private void DoLookups (FrameworkElement elem)
		{
			debug_panel.Children.Add (new TextBlock () { Text =  String.Format ("Lookups from {0} ({1})", elem.Name, elem) });

			
			debug_panel.Children.Add (new TextBlock () { Text = String.Format ("{0}.my_canvas: {1}", elem, elem.FindName ("my_canvas")) });
			debug_panel.Children.Add (new TextBlock () { Text = String.Format ("{0}.sub_element: {1}", elem, elem.FindName ("sub_element")) });
			debug_panel.Children.Add (new TextBlock () { Text = String.Format ("{0}.component_element: {1}", elem, elem.FindName ("component_element")) });
			debug_panel.Children.Add (new Canvas () { Height = 20 });

		}
	}
}
