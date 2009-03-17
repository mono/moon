using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;


namespace FindName
{
	public partial class MyTextBox : TextBox
	{
		public MyTextBox ()
		{
		}

		public override void OnApplyTemplate ()
		{
			if (TemplateApplied != null)
				TemplateApplied (this, EventArgs.Empty);
		}

		public DependencyObject GetTemplateChild_ (string name)
		{
			return GetTemplateChild (name);
		}

		public event EventHandler TemplateApplied;
	}
}
