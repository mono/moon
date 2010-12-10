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
		class Wrapper {
			public object Value { get; set; }
		}

		void RunTest ()
		{
			// Take the default style and apply it as a local
			// style too.
			WeakControl = ApplyTemplate (new ScrollBar ());
			WeakControl.Template = WeakControl.Template;
			
			GCAndInvoke (() => {
				if (WeakControl != null)
					Fail ("ScrollBar should be GC'ed");
				else
					Succeed ();
			});
		}
	}
}
