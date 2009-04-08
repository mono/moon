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
	public partial class InkPresenterTest
	{
		[TestMethod]
		public void ClearValueTest()
		{
			object strokes, new_strokes, rlv_strokes;
			InkPresenter ink = new InkPresenter();
			
			// check initial value
			strokes = ink.ReadLocalValue(InkPresenter.StrokesProperty);
			Assert.AreEqual(DependencyProperty.UnsetValue, strokes, "initial strokes is not set");
			
			// now try ClearValue
			ink.ClearValue(InkPresenter.StrokesProperty);
			
			// check that ReadLocalValue returns unset
			rlv_strokes = ink.ReadLocalValue(InkPresenter.StrokesProperty);
			Assert.AreEqual(DependencyProperty.UnsetValue, rlv_strokes, "ReadLocalValue after ClearValue is unset");
			
			// check that GetValue returns a StrokeCollection
			new_strokes = ink.GetValue(InkPresenter.StrokesProperty);
			Assert.AreNotEqual(DependencyProperty.UnsetValue, new_strokes, "GetValue after a ClearValue is set");
			Assert.IsNotNull(new_strokes as StrokeCollection, "GetValue after a ClearValue does not return null");
			
			// check that ReadLocalValue still returns unset
			rlv_strokes = ink.ReadLocalValue(InkPresenter.StrokesProperty);
			Assert.AreEqual(DependencyProperty.UnsetValue, rlv_strokes, "ReadLocalValue after a GetValue still returns unset");
			
			// add a stroke
			strokes = new_strokes;
			((StrokeCollection) strokes).Add(new Stroke());
			
			// check that ReadLocalValue still returns unset
			rlv_strokes = ink.ReadLocalValue(InkPresenter.StrokesProperty);
			Assert.AreEqual(DependencyProperty.UnsetValue, rlv_strokes, "ReadLocalValue after adding a stroke still returns unset");
			
			// check that GetValue still returns the same StrokeCollection
			new_strokes = ink.GetValue(InkPresenter.StrokesProperty);
			Assert.AreEqual(strokes, new_strokes, "strokes are the same");
			
			// set the strokes to something
			strokes = ink.Strokes = new StrokeCollection();
			
			// check that ReadLocalValue doesn't return unset anymore
			rlv_strokes = ink.ReadLocalValue(InkPresenter.StrokesProperty);
			Assert.AreEqual(strokes, rlv_strokes, "ReadLocalValue returned the strokes we just set on it");
		}
		
		[TestMethod]
		[MoonlightBug ("Property value semantics still not right for SetValue (Strokes, null)")]
		public void SetStrokesToNull ()
		{
			InkPresenter ink = new InkPresenter();
			object strokes;
			
			// check that ReadLocalValue returns a collection after setting it to null via the property accessor
			ink.Strokes = null;
			strokes = ink.ReadLocalValue(InkPresenter.StrokesProperty);
			Assert.IsTrue(strokes is StrokeCollection, "ReadLocalValue after setting to null returns a collection");
			
			// check that ReadLocalValue returns a collection after SetValue to null
			ink.SetValue(InkPresenter.StrokesProperty, null);
			strokes = ink.ReadLocalValue(InkPresenter.StrokesProperty);
			Assert.IsTrue(strokes is StrokeCollection, "ReadLocalValue after SetValue(null) returns a collection");
		}
		
		[TestMethod]
		public void ResetValueTest()
		{
			InkPresenter ink = new InkPresenter();
			StrokeCollection strokes;
			
			strokes = ink.GetValue(InkPresenter.StrokesProperty) as StrokeCollection;
			
			// does setting the value to null reset the collection?
			strokes.Add(new Stroke());
			ink.Strokes = null;
			strokes = ink.GetValue(InkPresenter.StrokesProperty) as StrokeCollection;
			Assert.AreEqual(0, strokes.Count, "Nulled strokes not empty as expected");
			
			// does clearing the value reset the collection?
			strokes.Add(new Stroke());
			ink.ClearValue(InkPresenter.StrokesProperty);
			strokes = ink.GetValue(InkPresenter.StrokesProperty) as StrokeCollection;
			Assert.AreEqual(0, strokes.Count, "Cleared strokes not empty as expected");
		}
	}
}