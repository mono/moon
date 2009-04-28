using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace NameTortureTest
{
	public partial class UserControl3 : UserControl
	{
		public UserControl3 ()
		{
			InitializeComponent ();
		}

		public Brush Brush {
			get { return (Brush)GetValue (BrushProperty); }
			set { SetValue (BrushProperty, value); }
		}

		public static readonly DependencyProperty BrushProperty = DependencyProperty.Register ("Brush", typeof (Brush), typeof (UserControl3), null);

	}
}