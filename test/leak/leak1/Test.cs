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
			WeakControl = new Button ();
			Children.Add (WeakControl);
			WeakControl.UpdateLayout ();
			Children.Remove (WeakControl);
			
			for (int i = 0; i < 10; i ++) {
				var b = new Button ();
				Children.Add (b);
				b.UpdateLayout ();
				Children.Remove (b);
			}
			
			GCAndInvoke (() => {
				if (WeakControl == null)
					Fail ("The subtree wasn't GC'ed");
				else
					Succeed ();
			});
		}
	}
}
