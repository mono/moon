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
			var c = new ObservableCollection<int> { 1, 2, 3, 4, 5 };
			WeakControl = new ContentControl ();
			WeakControl.SetBinding (ContentControl.ContentProperty, new Binding ("[0]") { Source = c });
			Holder = c;

			GCAndInvoke (() => {
				if (WeakControl != null)
					Fail ("The control should be collected");
				else
					Succeed ();
			});
		}
	}
}
