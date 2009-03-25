using System;
using System.Text;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;

namespace PasswordBoxView {
	public partial class Page : Canvas {
		public Page ()
		{
			InitializeComponent ();
			
			txtPasswordBox.PasswordChanged += OnPasswordChanged;
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
			
			sb.Append ("The current Visual Tree for the PasswordBox is:\n\n");
			DumpVisualTree (sb, obj, 0);
			
			txtVisualTree.Text = sb.ToString ();
		}
		
		void OnPasswordChanged (object sender, EventArgs args)
		{
			SetVisualTreeText (txtPasswordBox);
		}
	}
}
