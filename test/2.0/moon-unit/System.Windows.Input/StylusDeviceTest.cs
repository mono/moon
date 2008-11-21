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
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
 
namespace MoonTest.System.Windows.Input
{
	[TestClass]
	public class StylusDeviceTest
	{
// MouseEventArgs ctor is not public anymore
#if false
		[TestMethod]
		[Ignore ()]
		public void TestNRE ()
		{
			MouseEventArgs args = new MouseEventArgs ();
			StylusDevice device;
			Assert.Throws (delegate { device = args.StylusDevice; }, typeof (NullReferenceException));
		}
#endif
	}
}
