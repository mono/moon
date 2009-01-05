using System;
using System.Text;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;

namespace TextBoxView {
	public partial class Page : Canvas {
		public Page ()
		{
			InitializeComponent ();
			
			txtTextBox.TextChanged += OnTextChanged;
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
		
		void SetVisualTreeText (DependencyObject obj)
		{
			StringBuilder sb = new StringBuilder ();
			
			sb.Append ("The current Visual Tree for the TextBox's ContentElement is:\n\n");
			DumpVisualTree (sb, obj, 0);
			
			txtVisualTree.Text = sb.ToString ();
		}
		
		void OnTextChanged (object sender, EventArgs args)
		{
			txtContentType.Text = "ContentElement's type is " + txtTextBox.ContentElement.GetType ().ToString ();
			if (txtTextBox.ContentElement is ScrollViewer)
				txtContentType.Text += " and the content is a " +
					((ScrollViewer) txtTextBox.ContentElement).Content.GetType ().ToString ();
			SetVisualTreeText (txtTextBox.ContentElement);
		}
	}
}
