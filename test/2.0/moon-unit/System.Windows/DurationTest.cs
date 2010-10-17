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

namespace MoonTest.System.Windows {

	[TestClass]
	public class DurationTest {

		[TestMethod]
		public void GetNonTimeSpanDurationTimeSpan ()
		{
			Assert.Throws<InvalidOperationException> (() => { var t = Duration.Forever.TimeSpan; GC.KeepAlive (t); } );
		}

		[TestMethod]
		public void DefaultValues ()
		{
			Duration d = new Duration ();
			Assert.AreEqual (false, d.HasTimeSpan, "HasTimeSpan");
			Assert.Throws<InvalidOperationException> (() => { object o = d.TimeSpan; GC.KeepAlive(o); }, "TimeSpan");
			Assert.AreEqual ("Automatic", d.ToString (), "ToString");
		}
	}
}
