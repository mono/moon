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
			// Apply the default template multiple times and ensure the
			// first subtree is collected.
			Control = ApplyTemplate (new Button ());
			WeakSubtree = (FrameworkElement) VisualTreeHelper.GetChild (Control, 0);

			var template = Control.Template;
			Control.Template = null;
			Control.Template = template;

			GCAndInvoke (() => {
				if (WeakSubtree != null)
					Fail ("FailureReason");
				else
					Succeed ();
			});
		}
	}
}
