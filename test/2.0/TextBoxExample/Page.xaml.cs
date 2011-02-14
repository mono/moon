using System;
using System.Text;
using System.Windows;
using System.Windows.Media;
using System.Windows.Documents;
using System.Windows.Controls;

namespace TextBoxExample {
	public partial class Page : Grid {
		Run run1, run2, run3, run4, run5;

		public Page ()
		{
			InitializeComponent ();
		
			txtTextBox.ContentChanged += OnContentChanged;	
			txtTextBox.SelectionChanged += OnSelectionChanged;

			boldButton.Click += delegate { txtTextBox.Selection.ApplyPropertyValue (TextElement.FontWeightProperty, FontWeights.Bold); };
			italicsButton.Click += delegate { txtTextBox.Selection.ApplyPropertyValue (TextElement.FontStyleProperty, FontStyles.Italic); };
			//selectionButton.Click += delegate { Console.WriteLine ("HI HI HI HI HI"); txtSelection.Text = txtTextBox.Selection.Xaml.Replace (">", ">\n"); };

			Paragraph p1 = new Paragraph () { FontSize = 12.0 };
			Paragraph p2 = new Paragraph () { FontSize = 15.0 };
			Paragraph p3 = new Paragraph () { FontSize = 15.0 };

			run1 = new Run () { Text = "thisisatest" };
			run2 = new Run () { Text = "thisisonlyatest", FontSize = 16.0 };
			run3 = new Run () { Text = "thisisatest" };

			run4 = new Run () { Text = "thisisalsoatest" };
			run5 = new Run () { Text = "andonemore" };

			p1.Inlines.Add (run1);
			p1.Inlines.Add (run2);
			p1.Inlines.Add (run3);

			p2.Inlines.Add (run4);

			p3.Inlines.Add (run5);

			txtTextBox.Blocks.Add (p1);
			txtTextBox.Blocks.Add (p2);
			txtTextBox.Blocks.Add (p3);

			OnContentChanged (null, EventArgs.Empty);
		}
		
		void OnContentChanged (object sender, EventArgs args)
		{
			txtTextBlock.Text = txtTextBox.Xaml.Replace (">", ">\n");
			if (run4.ReadLocalValue (TextElement.FontSizeProperty) == DependencyProperty.UnsetValue)
				txtSelection.Text = "run4 has inherited FontSizeProperty";
			else
				txtSelection.Text = "run4 has local FontSizeProperty";

		}

		void OnSelectionChanged (object sender, RoutedEventArgs args)
		{
			if (run4.ReadLocalValue (TextElement.FontSizeProperty) == DependencyProperty.UnsetValue)
				txtSelection.Text = "run4 has inherited FontSizeProperty\n";
			else
				txtSelection.Text = "run4 has local FontSizeProperty\n";

			if (txtTextBox.Selection.Xaml == null)
				txtSelection.Text += "selection changed, null xaml == null";
			txtSelection.Text += txtTextBox.Selection.Xaml.Replace (">", ">\n");

			//txtSelection.Text += "\n" + (txtTextBox.Selection.Start.CompareTo (txtTextBox.Selection.End)).ToString();
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
