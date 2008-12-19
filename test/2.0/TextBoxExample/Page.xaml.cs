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
			SetVisualTreeText (txtTextBox);
			
			txtTextBox.SelectionChanged += OnSelectionChanged;
			txtTextBox.TextChanged += OnTextChanged;
			
			//cbAcceptReturn.IsChecked = txtTextBox.AcceptsReturn;
			//cbAcceptReturn.Unchecked += OnUnchecked;
			//cbAcceptReturn.Checked += OnChecked;
		}
		
		void DumpVisualTree (StringBuilder sb, DependencyObject obj, int depth)
		{
			DependencyObject child;
			
			for (int i = 0; i < VisualTreeHelper.GetChildrenCount (obj); i++) {
				child = VisualTreeHelper.GetChild (obj, i);
				sb.Append (' ', depth * 4);
				sb.Append (child.ToString ());
				sb.Append ('\n');
				
				if (VisualTreeHelper.GetChildrenCount (child) > 0)
					DumpVisualTree (sb, child, depth + 1);
			}
		}
		
		void SetVisualTreeText (TextBox textbox)
		{
			StringBuilder sb = new StringBuilder ();
			
			sb.Append ("The current Visual Tree for the TextBox content is:\n");
			DumpVisualTree (sb, textbox, 0);
			
			txtVisualTree.Text = sb.ToString ();
		}
		
		void OnSelectionChanged (object sender, EventArgs args)
		{
			SetVisualTreeText (txtTextBox);
		}
		
		void OnTextChanged (object sender, EventArgs args)
		{
			txtTextBlock.Text = txtTextBox.Text;
			SetVisualTreeText (txtTextBox);
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
