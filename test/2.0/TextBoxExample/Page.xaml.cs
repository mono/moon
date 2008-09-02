using System;
using System.Windows;
using System.Windows.Controls;

namespace TextBoxExample {
	public partial class Page : UserControl {
		public Page ()
		{
			InitializeComponent ();
		}

		public void OnMouseEnter (object sender, EventArgs args)
		{
			txtTextBox.AcceptsReturn = false;
		}
		
		public void OnMouseLeave (object sender, EventArgs args)
		{
			txtTextBox.AcceptsReturn = true;
		}
	}
}
