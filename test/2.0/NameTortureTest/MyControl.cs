using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Text;
using System.Windows;
using System.Windows.Browser;
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
	public class MyControl : ContentControl
	{
		public Canvas TemplateCanvas
		{
			get { return (Canvas) GetTemplateChild("Canvas"); }
		}
		public DependencyObject GetTemplateChild (string name)
		{
			return base.GetTemplateChild(name);
		}
		public MyControl()
		{
		}
	}
}
