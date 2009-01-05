using System;
using System.Windows;
using System.Windows.Controls;

namespace TextBoxView {
	public class TextWidget : TextBox {
		public TextWidget () : base ()
		{
			
		}
		
		public DependencyObject ContentElement {
			get; internal set;
		}
		
		public override void OnApplyTemplate ()
		{
			ContentElement = GetTemplateChild ("ContentElement");
			
			base.OnApplyTemplate ();
		}
	}
}
