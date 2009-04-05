using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Net;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace EasingFunctions
{
	public partial class EasingView : UserControl
	{
		public static DependencyProperty EasingFunctionProperty = DependencyProperty.Register ("EasingFunction", typeof (EasingFunctionBase), typeof (EasingView), null);

		public EasingView ()
		{
			InitializeComponent();

			Loaded += delegate {
				Polyline polyline = new Polyline ();

				for (int px = 0; px <= rect.Width; px++) {
					double x = (double) px / rect.Width;

					double y = EasingFunction.Ease (x);

					polyline.Points.Add (new Point (px,rect.Height - y * rect.Height));
				}

				polyline.Stroke = new SolidColorBrush (Colors.Blue);
				polyline.Fill = null;

				Canvas.SetTop (polyline, 5);
				Canvas.SetLeft (polyline, 5);

				LayoutRoot.Children.Add (polyline);
			};
		}

		public EasingFunctionBase EasingFunction {
			get { return (EasingFunctionBase)GetValue(EasingFunctionProperty); }
			set { SetValue (EasingFunctionProperty, value); }
		}
	}
}
