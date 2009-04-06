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
	public class CustomEaseIn : IEasingFunction
	{
		public double Ease (double normalizedTime)
		{
			return normalizedTime < 0.5 ? 0 : 1;
		}
	}

	public class CustomEaseOut : IEasingFunction
	{
		public double Ease (double normalizedTime)
		{
			return normalizedTime < 0.5 ? 1 : 0;
		}
	}

	public class CustomEaseInOut : IEasingFunction
	{
		public double Ease (double normalizedTime)
		{
			return normalizedTime < 0.25 ? 1
					 : normalizedTime < 0.5 ? 0
						: normalizedTime < 0.75 ? 1
							: 0;
		}
	}
}
