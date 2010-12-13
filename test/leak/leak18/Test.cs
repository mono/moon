using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Leak
{
	public partial class Page
	{
		void RunTest ()
		{
			// Ensure TextBoxes are collectable
			WeakControl = ApplyTemplate (new TextBox ());
			
			// Work around a FrameworkTemplate issue whereby it
			// keeps one control alive longer than necessary
			for (int i = 0; i < 10; i ++)
				ApplyTemplate (new TextBox ());

			GCAndInvoke (() => {
				if (WeakControl != null)
					Fail ("TextBox should be collected");
				else
					Succeed ();
			});
		}
	}
}
