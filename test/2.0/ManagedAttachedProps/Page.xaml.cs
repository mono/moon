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

namespace ManagedAttachedProps
{
	public partial class Page : UserControl
	{
		public Page()
		{
			InitializeComponent();
			Console.WriteLine("everything initialized!");

			// Stretcher s = (Stretcher) FindName("the_stretcher");
			// string amount = Stretcher.GetStretchAmount(the_rect);
		}

	}
}
