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

namespace Leak
{
	public partial class Page
	{
		void RunTest ()
		{
			Control = ApplyTemplate (new INPC ());

			WeakSubtree = (FrameworkElement) VisualTreeHelper.GetChild (Control, 0);
			WeakSubtree.SetBinding (FrameworkElement.WidthProperty, new Binding ("Value") { Mode = BindingMode.TwoWay, RelativeSource = new RelativeSource (RelativeSourceMode.TemplatedParent) });

			Control.Template = null;

			GCAndInvoke (() => {
				if (WeakSubtree != null)
					Fail ("The subtree should be collected");
				else
					Succeed ();
			});
		}
	}

	public class INPC : ContentControl, INotifyPropertyChanged {
		public event PropertyChangedEventHandler PropertyChanged;

		public double Value {
			get; set;
		}
	}
}
