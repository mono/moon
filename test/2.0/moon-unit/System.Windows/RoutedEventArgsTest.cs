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
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;


namespace MoonTest.System.Windows
{
	[TestClass]
	public class RoutedEventArgsTest
	{
		/*
		 * FIXME: RoutedEventArgs.set_OriginalSource is internal

		[TestMethod]
		public void SourceTest ()
		{
			RoutedEventArgs args = new RoutedEventArgs ();
			Assert.IsNull (args.OriginalSource);

			// try object
			Assert.Throws (delegate () { args.OriginalSource = new object(); }, typeof (ArgumentException));

			object o;

			// try a DependencyObject subclass
			o = new SolidColorBrush();
			args.OriginalSource = o;
			Assert.AreEqual (o, args.OriginalSource);

			// try a UIElement subclass
			o = new Ellipse();
			args.OriginalSource = o;
			Assert.AreEqual (o, args.OriginalSource);

#if notyet
			// try null.  B2 crashes on this line (no
			// exception, firefox goes away with a crash
			// in coreclr.dll)
			args.OriginalSource = null;
#endif
		}
		*/
	}
}