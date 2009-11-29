using System;
using System.Text;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;

namespace TextBoxWidth {
	public partial class Page : Canvas {
		public Page ()
		{
			InitializeComponent ();
			
			textbox.TextChanged += OnTextChanged;
			textbox.MouseEnter += OnMouseEnter;
		}
		
		void UpdateTextBoxViewWidth ()
		{
			FrameworkElement tbv = textbox.ViewElement;
			textblock.Text = "TextBoxView width is " + tbv.ActualWidth;
		}
		
		void OnMouseEnter (object sender, EventArgs args)
		{
			UpdateTextBoxViewWidth ();
		}
		
		void OnTextChanged (object sender, EventArgs args)
		{
			UpdateTextBoxViewWidth ();
		}
	}
}
