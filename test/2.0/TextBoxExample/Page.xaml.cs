using System;
using System.Windows;
using System.Windows.Controls;

namespace TextBoxExample {
	public partial class Page : Canvas {
		public Page ()
		{
			InitializeComponent ();
			
			txtTextBox.TextChanged += OnTextChanged;
			
			//cbAcceptReturn.IsChecked = txtTextBox.AcceptsReturn;
			//cbAcceptReturn.Unchecked += OnUnchecked;
			//cbAcceptReturn.Checked += OnChecked;
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
