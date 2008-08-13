using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Collections.Generic;

using dependency_properties;

namespace MoonTest.System.Windows
{
	[TestClass]
	public class RoutedEventArgsTest
	{
		[TestMethod]
		public void SourceTest ()
		{
			RoutedEventArgs args = new RoutedEventArgs ();
			Assert.IsNull (args.Source);

			// try object
			Assert.Throws (delegate () { args.Source = new object(); }, typeof (ArgumentException));

			object o;

			// try a DependencyObject subclass
			o = new SolidColorBrush();
			args.Source = o;
			Assert.AreEqual (o, args.Source);

			// try a UIElement subclass
			o = new Ellipse();
			args.Source = o;
			Assert.AreEqual (o, args.Source);

#if notyet
			// try null.  B2 crashes on this line (no
			// exception, firefox goes away with a crash
			// in coreclr.dll)
			args.Source = null;
#endif
		}
	}
}