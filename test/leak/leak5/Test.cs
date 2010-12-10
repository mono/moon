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
			// Store a CLR object in a custom DP. Make that CLR object hold
			// a reference to the Control its stored in to try and create a
			// cycle which prevents GC.
			var dp = DependencyProperty.RegisterAttached ("CustomGCCheckDP", typeof (Wrapper), typeof (ContentControl), null);
			WeakControl = new ContentControl ();
			WeakControl.SetValue (dp, new Wrapper { Value = WeakControl });
			
			GCAndInvoke (() => {
				if (WeakControl == null)
					Succeed ();
				else
					Fail ("FailureReason");
			});
		}
	}
}
