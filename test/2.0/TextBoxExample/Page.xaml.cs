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
			
			txtTextBox.Text = "hello\rworld\nand\r\nstuff";
			
			txtTextBox.SelectionChanged += OnSelectionChanged;
			txtTextBox.TextChanged += OnTextChanged;
			
			//cbAcceptReturn.IsChecked = txtTextBox.AcceptsReturn;
			//cbAcceptReturn.Unchecked += OnUnchecked;
			//cbAcceptReturn.Checked += OnChecked;
		}
		
		void OnSelectionChanged (object sender, EventArgs args)
		{
			string text = txtTextBox.SelectedText;
			
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
			
			txtSelectionState.Text = "OnSelectionChanged(): SelectionStart = " + txtTextBox.SelectionStart +
				"; SelectionLength = " + txtTextBox.SelectionLength;
		}
		
		void OnTextChanged (object sender, EventArgs args)
		{
			txtTextBlock.Text = txtTextBox.Text.Replace ("\n", "\\n").Replace ("\r", "\\r");
			
			txtSelectionState.Text = "OnTextChanged(): SelectionStart = " + txtTextBox.SelectionStart +
				"; SelectionLength = " + txtTextBox.SelectionLength;
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
