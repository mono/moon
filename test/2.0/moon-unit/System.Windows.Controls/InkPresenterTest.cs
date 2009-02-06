using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Collections.Generic;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Controls
{
	[TestClass]
	public class InkPresenterTest
	{
		[TestMethod]
		public void ClearValueTest()
		{
			StrokeCollection strokes, new_strokes, rlv_strokes;
			InkPresenter ink = new InkPresenter();
			
			// check initial value
			strokes = ink.ReadLocalValue(InkPresenter.StrokesProperty) as StrokeCollection;
			Assert.IsNull(strokes, "initial strokes is null");
			
			// now try ClearValue
			ink.ClearValue(InkPresenter.StrokesProperty);
			
			// check that ReadLocalValue returns null
			rlv_strokes = ink.ReadLocalValue(InkPresenter.StrokesProperty) as StrokeCollection;
			Assert.IsNull(rlv_strokes, "ReadLocalValue after ClearValue returns null");
			
			// check that GetValue returns a StrokeCollection
			new_strokes = ink.GetValue(InkPresenter.StrokesProperty) as StrokeCollection;
			Assert.IsNotNull(new_strokes, "GetValue after ClearValue returns a StrokeCollection");
			
			// check that the GetValue set a local StrokeCollection value on the InkPresenter
			rlv_strokes = ink.ReadLocalValue(InkPresenter.StrokesProperty) as StrokeCollection;
			Assert.IsNotNull(rlv_strokes, "ReadLocalValue after GetValue returns a StrokeCollection");
			
#if false
			// FIXME: current causes us to crash because
			// InkPresenter::OnPropertyChanged()'s
			// new_value->AsStrokeCollection() returning
			// null is not handled.
			
			// now try setting it to null instead of using ClearValue
			ink.Strokes = null;
			
			// check that ReadLocalValue returns null
			rlv_strokes = ink.ReadLocalValue(InkPresenter.StrokesProperty) as StrokeCollection;
			Assert.IsNull(rlv_strokes, "ReadLocalValue after setting to null returns null");
			
			// check that GetValue restores an empty StrokeCollection
			new_strokes = ink.GetValue(InkPresenter.StrokesProperty) as StrokeCollection;
			Assert.IsNotNull(new_strokes, "GetValue after setting to null returns a StrokeCollection");
#endif
		}
	}
}