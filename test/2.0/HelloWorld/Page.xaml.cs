using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;

namespace HelloWorld {
	public partial class Page : UserControl
	{
		public Page()
		{
			InitializeComponent();

			button.Click += delegate {
				rtb.Selection.Insert (new InlineUIContainer { Child = new Button { Content = "Yo bitch" } } );
			};
		}
	}
}
