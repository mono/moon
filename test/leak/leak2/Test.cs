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
			var grid = new Grid ();
			WeakSubtree = new Grid ();
			grid.Children.Add (WeakSubtree);
			grid.Children.RemoveAt (0);

			GCAndInvoke (() => {
				if (WeakSubtree == null)
					Succeed ();
				else
					Fail ("Subtree was not collected");
			});
		}
	}
}
