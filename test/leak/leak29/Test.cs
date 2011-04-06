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
using System.Windows.Markup;
using System.ComponentModel;
using System.Windows.Data;
using System.Collections.ObjectModel;

namespace Leak
{
	public partial class Page
	{
		public object Holder {
			get; set;
		}

		void RunTest ()
		{
			// Verify that elements which are focused but never get a chance to emit
			// the focus event (and so are queued on the Surface forever) are still
			// able to be collected on application shutdown.
			for (int i = 0; i < 50; i++) {
				var c = new ContentControl ();
				Children.Add (c);
				c.Focus ();
			}
			Succeed ();
		}
	}
}
