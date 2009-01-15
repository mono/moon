using System;
using System.Text;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;

namespace TextBoxExample {
	public partial class Page : Canvas {
		public Page ()
		{
			InitializeComponent ();
			
			txtTextBox.SelectionChanged += OnSelectionChanged;
			txtTextBox.TextChanged += OnTextChanged;
			
			//cbAcceptReturn.IsChecked = txtTextBox.AcceptsReturn;
			//cbAcceptReturn.Unchecked += OnUnchecked;
			//cbAcceptReturn.Checked += OnChecked;
		}
		
		void OnSelectionChanged (object sender, EventArgs args)
		{
			RoutedEventArgs routed = args as RoutedEventArgs;
			string text = txtTextBox.SelectedText;
			StringBuilder sb;
			
			if (text != "") {
				txtSelection.Foreground = new SolidColorBrush (Colors.White);
				rectSelection.Fill = new SolidColorBrush (Colors.Blue);
				txtSelection.Text = text;
			} else {
				txtSelection.Foreground = new SolidColorBrush (Colors.Black);
				rectSelection.Fill = new SolidColorBrush (Colors.White);
				txtSelection.Text = "No text selected";
			}
			
			rectSelection.Height = txtSelection.ActualHeight;
			rectSelection.Width = txtSelection.ActualWidth;
			
			sb = new StringBuilder ();
			sb.Append ("SelectionChangedEvent source: ");
			if (routed != null) {
				if (routed.OriginalSource != null) {
					//DependencyObject obj = routed.OriginalSource as DependencyObject;
					
					//if (obj != null && obj.Name != null && obj.Name != "") {
					//	sb.Append (obj.Name);
					//	sb.Append (", which is of type: ");
					//}
					
					sb.Append (routed.OriginalSource.GetType ().ToString ());
				} else {
					sb.Append ("null");
				}
			} else if (args != null) {
				sb.Append ("args was of type ");
				sb.Append (args.GetType ().ToString ());
			} else {
				sb.Append ("args was null");
			}
			txtSelectionSource.Text = sb.ToString ();
		}
		
		void OnTextChanged (object sender, EventArgs args)
		{
			txtTextBlock.Text = txtTextBox.Text;
		}
		
		//void OnUnchecked (object sender, EventArgs args)
		//{
		//	txtTextBox.AcceptsReturn = false;
		//}
		
		//void OnChecked (object sender, EventArgs args)
		//{
		//	txtTextBox.AcceptsReturn = true;
		//}
	}
}
