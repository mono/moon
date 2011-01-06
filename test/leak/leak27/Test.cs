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
		void RunTest ()
		{
			WeakControl = new ContentControl ();
			ToolTipService.SetToolTip (WeakControl, "Tooltiped!");

			GCAndInvoke (() => {
				if (WeakControl != null)
					Fail ("The control should be collected");
				else
					Succeed ();
			});
		}
	}
}
