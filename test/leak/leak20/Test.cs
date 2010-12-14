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
		public class MyContentControl : ContentControl
		{
			public static readonly DependencyProperty MyProperty = DependencyProperty.Register("MyProperty", typeof(object), typeof(MyContentControl), null);
		}

		class Wrapper
        {
            public object Value { get; set; }
        }
		
		void RunTest ()
		{
			var c = new MyContentControl ();
            c.SetValue(MyContentControl.MyProperty, null);

            Style s1 = new Style(typeof(MyContentControl));
            s1.Setters.Add(new Setter(MyContentControl.MyProperty, new Wrapper { Value = c }));
            c.Style = s1;

			WeakControl = c;
			GCAndInvoke (() => {
				if (WeakControl != null)
					Fail ("TextBox should be collected");
				else
					Succeed ();
			});
		}
	}
}
