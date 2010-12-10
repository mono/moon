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
		class SelfReferencingControl : ContentControl
		{
			public void AttachToLayoutUpdated ()
			{
				LayoutUpdated += delegate { Console.WriteLine (this); };
			}
		}
		
		void RunTest ()
		{
			var c = new SelfReferencingControl ();
			WeakControl = c;
			c.AttachToLayoutUpdated ();
			
			GCAndInvoke (() => {
				if (WeakControl != null)
					Fail ("Control should be GC'ed");
				else
					Succeed ();
			});
		}
	}
}
