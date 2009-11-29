using System;
using System.Windows;
using System.Windows.Controls;

namespace TextBoxWidth {
	public class MyTextBox : TextBox {
		public MyTextBox () : base ()
		{
			
		}
		
		public DependencyObject ContentElement {
			get; internal set;
		}

		public FrameworkElement ViewElement {
			get; internal set;
		}
		
		public override void OnApplyTemplate ()
		{
			ContentElement = GetTemplateChild ("ContentElement");
			
			base.OnApplyTemplate ();
			
			if (ContentElement is ContentControl)
				ViewElement = (ContentElement as ContentControl).Content as FrameworkElement;
			else if (ContentElement is Border)
				ViewElement = (ContentElement as Border).Child as FrameworkElement;
			else
				ViewElement = (ContentElement as FrameworkElement);
		}
	}
}
