using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace VSMTest
{
	public partial class VSMControl : Control
	{
		public Grid TemplateGrid
		{
			get { return (Grid) GetTemplateChild ("Grid"); }
		}
		public VSMControl ()
		{
            System.Windows.Application.LoadComponent(this, new System.Uri("/moon-unit;component/System.Windows/VSMControl.xaml", System.UriKind.Relative));
		}
	}
}
