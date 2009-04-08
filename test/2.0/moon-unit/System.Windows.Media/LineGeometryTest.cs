/*
 * LineGeometryTest.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Collections.ObjectModel;
using System.Collections.Generic;
using System.Collections.Specialized;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;


namespace MoonTest.System.Windows.Media
{
	[TestClass]
	public partial class LineGeometryTest
	{
		[TestMethod]
		public void Defaults ()
		{
			LineGeometry lg = new LineGeometry ();
			Assert.AreEqual (new Point (0, 0), lg.StartPoint, "StartPoint");
			Assert.AreEqual (new Point (0, 0), lg.EndPoint, "EndPoint");
			GeometryTest.CheckDefaults (lg);
		}

		[TestMethod]
		public void CustomPoints ()
		{
			LineGeometry lg = new LineGeometry ();
			lg.StartPoint = new Point (1, 2);
			lg.EndPoint = new Point (3, 4);
			Assert.AreEqual (new Rect (1, 2, 2, 2), lg.Bounds, "Bounds");
			Assert.IsNull (lg.Transform, "Transform");
		}

		[TestMethod]
		public void FillRuleTest ()
		{
			string xaml =
@"<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"">
	<Path Fill=""#FF0000"" Stroke=""Green"" StrokeThickness=""2"">
		<Path.Data>
			<LineGeometry FillRule=""EvenOdd"" StartPoint=""10,60"" EndPoint=""110,60"" />
		</Path.Data>
	</Path>
</Canvas>";
			Assert.Throws (delegate {
				XamlReader.Load (xaml);
			}, typeof (XamlParseException));
		}
	}
}
