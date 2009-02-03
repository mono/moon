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
using DebugLog;
using DebugLog.Extensions;

namespace GridTest
{
	public class UserBackgroundTest : UserControl 
	{
		public UserBackgroundTest()
		{
			Background = new SolidColorBrush (Colors.Purple);
			Width = 20;
			Height = 5;
		}
	}
}
