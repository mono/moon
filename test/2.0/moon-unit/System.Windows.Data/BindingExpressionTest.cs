//
// BindingExpression Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
using System;
using System.Windows.Data;
using System.Windows.Shapes;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Data {

	[TestClass]
	public partial class BindingExpressionTest {
		object Data {
			get; set;
		}

		Rectangle Element {
			get; set;
		}

		[TestInitialize]
		public void Initialize ()
		{
			Data = new object ();
			Element = new Rectangle ();
		}

		[TestMethod]
		public void BindingExpressionType ()
		{
			var expression = Element.SetBinding (Rectangle.WidthProperty, new Binding ());
			Assert.AreEqual (typeof (BindingExpression), expression.GetType (), "#1");
		}

		[TestMethod]
		public void DataItem_DataContext_Null ()
		{
			Binding binding = new Binding { };

			var expression = (BindingExpression) Element.SetBinding (Rectangle.WidthProperty, binding);
			Assert.IsNull (expression.DataItem, "#1");
		}

		[TestMethod]
		public void DataItem_DataContext_SetAfter ()
		{
			Binding binding = new Binding { };

			var expression = (BindingExpression) Element.SetBinding (Rectangle.WidthProperty, binding);
			Assert.IsNull (expression.DataItem, "#1");
			Element.DataContext = Data;
			Assert.AreSame (Data, expression.DataItem, "#2");
		}

		[TestMethod]
		public void DataItem_DataContext_SetBefore ()
		{
			Binding binding = new Binding { };

			Element.DataContext = Data;
			var expression = (BindingExpression) Element.SetBinding (Rectangle.WidthProperty, binding);
			Assert.AreSame (Data, expression.DataItem, "#1");
		}

		[TestMethod]
		public void DataItem_Fixed ()
		{
			Binding binding = new Binding { Source = Data };

			var expression = (BindingExpression) Element.SetBinding (Rectangle.WidthProperty, binding);
			Assert.AreSame (Data, expression.DataItem, "#2");
		}

		[TestMethod]
		public void ParentBindingTest ()
		{
			Binding binding = new Binding { Source = Data };

			var expression = (BindingExpression) Element.SetBinding (Rectangle.WidthProperty, binding);
			Assert.AreSame (binding, expression.ParentBinding, "#2");
		}
	}
}