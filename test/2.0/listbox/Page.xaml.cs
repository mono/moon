using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Net;
using System.Text;
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
		ObservableCollection<string> stringCollection;

		public Page()
		{
			InitializeComponent();

 			stringCollection = new ObservableCollection<string>();

			stringCollection.Add ("string 0");

			databoundItemsControl.ItemsSource = stringCollection;

#if DESKTOP
			MouseWheel += delegate (object sender, MouseWheelEventArgs args) {

				if (args.Delta > 0) {
					scale.ScaleX /= 1.1;
					scale.ScaleY /= 1.1;
				}
				else if (args.Delta < 0) {
					scale.ScaleX *= 1.1;
					scale.ScaleY *= 1.1;
				}
			};
#endif
		}

		public void button_Click (object sender, RoutedEventArgs e)
		{
			Button b = new Button ();
			b.Content = string.Format ("added button {0}", button_number++);
			itemsControl.Items.Add (b);
		}

		public void databoundButton_Click (object sender, RoutedEventArgs e)
		{
			stringCollection.Add (string.Format ("string {0}", stringCollection.Count));
		}

		public void horizButtonClicked (object sender, RoutedEventArgs e)
		{
			StringBuilder builder = new StringBuilder ();
			DumpTree (horizontalItemsControl, builder);
			log.Text = builder.ToString();
		}

		void DumpTree (DependencyObject root, StringBuilder builder)
		{
			builder.AppendFormat (string.Format ("{0}", root.ToString()));
			if (VisualTreeHelper.GetChildrenCount (root) > 0) {
				builder.Append (" -> { \n");
				for (int i = 0; i < VisualTreeHelper.GetChildrenCount (root); i ++) {
					DumpTree (VisualTreeHelper.GetChild (root, i), builder);
					if (i < VisualTreeHelper.GetChildrenCount (root) - 1)
						builder.Append (",\n");
				}
				builder.Append (" }");
			}
		}

		int button_number;
	}
}
