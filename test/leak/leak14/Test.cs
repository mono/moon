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
			// This is special because its style defines a ControlTemplate inside
			// a ControlTemplate. This can leak if the XamlContexts are held
			// strongly.
			WeakControl = ApplyTemplate (new ScrollBar ());
			
			GCAndInvoke (() => {
				if (WeakControl != null)
					Fail ("FailureReason");
				else
					Succeed ();
			});
		}
	}
}
