//
// ComboBox Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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
using System.Collections.Generic;
using System.Windows;
using System.Linq;
using System.Windows.Controls;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Shapes;
using Mono.Moonlight.UnitTesting;
using System.Collections.Specialized;

namespace MoonTest.System.Windows.Controls {

	public struct Value {
		public string MethodName;
		public object [] MethodParams;
		public object ReturnValue;
	}

	public class FakeComboBox : ComboBox {
		public List<Value> methods = new List<Value> ();

		public FakeComboBox ()
		{
			this.SelectionChanged += delegate { methods.Add (new Value { MethodName = "SelectionChangedEvent" }); };
			this.DropDownClosed += delegate { methods.Add (new Value { MethodName = "DropDownClosedEvent" }); };
			this.DropDownOpened += delegate { methods.Add (new Value { MethodName = "DropDownOpenedEvent" }); };
		}

		protected override Size ArrangeOverride (Size arrangeBounds)
		{
			methods.Add (new Value { MethodParams = new object [] { arrangeBounds }, ReturnValue = base.ArrangeOverride (arrangeBounds) });
			return (Size) methods.Last ().ReturnValue;
		}

		protected override void ClearContainerForItemOverride (global::System.Windows.DependencyObject element, object item)
		{
			methods.Add (new Value { MethodParams = new object [] { element, item } });
			base.ClearContainerForItemOverride (element, item);
		}

		protected override global::System.Windows.DependencyObject GetContainerForItemOverride ()
		{
			methods.Add (new Value { ReturnValue = base.GetContainerForItemOverride () });
			return (DependencyObject) methods.Last ().ReturnValue;
		}

		protected override bool IsItemItsOwnContainerOverride (object item)
		{
			return base.IsItemItsOwnContainerOverride (item);
		}

		protected override global::System.Windows.Size MeasureOverride (global::System.Windows.Size availableSize)
		{
			return base.MeasureOverride (availableSize);
		}

		public override void OnApplyTemplate ()
		{
			base.OnApplyTemplate ();
		}

		protected override void OnDropDownClosed (EventArgs e)
		{
			methods.Add (new Value { MethodName = "OnDropDownClosed", MethodParams = new object [] { e } });
			base.OnDropDownClosed (e);
		}

		protected override void OnDropDownOpened (EventArgs e)
		{
			methods.Add (new Value { MethodName = "OnDropDownOpened", MethodParams = new object [] { e } });
			base.OnDropDownOpened (e);
		}

		protected override void OnItemsChanged (global::System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
		{
			methods.Add (new Value { MethodName = "OnItemsChanged", MethodParams = new object [] { e } });
			base.OnItemsChanged (e);
		}

		protected override void PrepareContainerForItemOverride (global::System.Windows.DependencyObject element, object item)
		{
			base.PrepareContainerForItemOverride (element, item);
		}
	}

	[TestClass]
	public partial class ComboBoxTest {

		[TestMethod]
		public void DefaultValues ()
		{
			ComboBox b = new ComboBox ();
			Assert.IsFalse (b.IsDropDownOpen, "#1");
			Assert.IsFalse (b.IsEditable, "#2");
			Assert.IsFalse (b.IsSelectionBoxHighlighted, "#3");
			Assert.IsNull (b.ItemContainerStyle, "#4");
			Assert.AreEqual (double.PositiveInfinity, b.MaxDropDownHeight, "#5");
			Assert.IsNull (b.SelectionBoxItem, "#6");
			Assert.IsNull (b.SelectionBoxItemTemplate, "#7");
			Assert.AreEqual (-1, b.SelectedIndex, "#8");
		}

		[TestMethod]
		public void InvalidValues ()
		{
			ComboBox b = new ComboBox ();
			b.MaxDropDownHeight = -1;
			Assert.AreEqual (-1.0, b.MaxDropDownHeight, "#1");
			b.MaxDropDownHeight = -11000.0;
			Assert.AreEqual (-11000.0, b.MaxDropDownHeight, "#2");
			b.MaxDropDownHeight = 0;
			Assert.AreEqual (0, b.MaxDropDownHeight, "#3");
			Assert.Throws<ArgumentException> (delegate {
				b.Items.Add (null);
			});
			b.Items.Add (new Rectangle ());
			Assert.IsNull (b.SelectedItem);
			b.Items.Add ("This is a string");
			b.Items.Add (new ComboBoxItem { Content = "Yeah" });

			Assert.AreEqual (-1, b.SelectedIndex, "#4");
			b.SelectedIndex = 0;
			Assert.IsTrue (b.SelectedItem is Rectangle, "#5");
			b.SelectedItem = new object ();
			Assert.IsTrue (b.SelectedItem is Rectangle, "#6");
			b.SelectedItem = b.Items [1];
			Assert.AreEqual (1, b.SelectedIndex, "#7");
		}

		[TestMethod]
		public void TestOverrides ()
		{
			FakeComboBox b = new FakeComboBox ();
			object o = new object ();
			b.Items.Add (o);
			Assert.AreEqual (0, b.Items.IndexOf (b.Items [0]), "#0");
			Assert.AreEqual (1, b.methods.Count, "#1");
			Assert.AreEqual ("OnItemsChanged", b.methods [0].MethodName, "#2");
			b.IsDropDownOpen = true;
			Assert.AreEqual ("OnDropDownOpened", b.methods [1].MethodName, "#3");
			Assert.AreEqual ("DropDownOpenedEvent", b.methods [2].MethodName, "#4");
			b.IsDropDownOpen = false;
			Assert.AreEqual ("OnDropDownClosed", b.methods [3].MethodName, "#5");
			Assert.AreEqual ("DropDownClosedEvent", b.methods [4].MethodName, "#6");
			b.SelectedItem = new object ();
			Assert.AreEqual (5, b.methods.Count, "#7");
			b.SelectedItem = b.Items [0];
			Assert.AreEqual (6, b.methods.Count, "#8");
			Assert.AreEqual ("SelectionChangedEvent", b.methods [5].MethodName);
			b.SelectedIndex = -1;
			Assert.AreEqual (null, b.SelectedItem, "#9");
			Assert.AreEqual (7, b.methods.Count, "#10");
			Assert.AreEqual ("SelectionChangedEvent", b.methods [6].MethodName, "#11");
			b.SelectedItem = b.Items [0];
			b.methods.Clear ();
			b.Items.RemoveAt (0);
			Assert.AreEqual (1, b.methods.Count, "#10");
			Assert.AreEqual ("OnItemsChanged", b.methods [0].MethodName, "#11");
			Assert.AreEqual (o, b.SelectedItem, "#12");
			Assert.AreEqual (0, b.SelectedIndex, "#13");
		}

		[TestMethod]
		public void AddTest ()
		{
			FakeComboBox box = new FakeComboBox ();
			box.Items.Add ("blah");
			Assert.AreEqual (1, box.methods.Count, "#1");
			Assert.AreEqual ("OnItemsChanged", box.methods [0].MethodName, "#2");
			NotifyCollectionChangedEventArgs e = (NotifyCollectionChangedEventArgs) box.methods [0].MethodParams[0];
			Assert.AreEqual (null, e.OldItems, "#3");
			Assert.AreEqual (-1, e.OldStartingIndex, "#4");
			Assert.IsNotNull (e.NewItems, "#5");
			Assert.AreEqual (1, e.NewItems.Count, "#6");
			Assert.AreEqual ("blah", e.NewItems [0], "#7");
			Assert.AreEqual (0, e.NewStartingIndex, "#8");
		}
		
		[TestMethod]
		public void SelectedItemTest ()
		{
			FakeComboBox box = new FakeComboBox ();
			Assert.AreEqual (-1, box.SelectedIndex, "#1");
			Assert.AreEqual (null, box.SelectedItem, "#2");

			box.SelectedItem = new object ();
			Assert.AreEqual (-1, box.SelectedIndex, "#3");
			Assert.AreEqual (null, box.SelectedItem, "#4");

			object a = new object ();
			object b = new object ();
			object c = new object ();
			box.Items.Add (a);
			box.Items.Add (b);
			box.Items.Add (c);
			box.SelectedItem = new object ();
			Assert.AreEqual (-1, box.SelectedIndex, "#5");
			Assert.AreEqual (null, box.SelectedItem, "#6");

			box.SelectedItem = a;
			Assert.AreEqual (0, box.SelectedIndex, "#7");
			Assert.AreEqual (a, box.SelectedItem, "#8");

			box.SelectedIndex = -1;
			Assert.AreEqual (-1, box.SelectedIndex, "#9");
			Assert.AreEqual (null, box.SelectedItem, "#10");

			box.SelectedItem = a;
			box.SelectedItem = b;
			Assert.AreEqual (1, box.SelectedIndex, "#11");
			Assert.AreEqual (b, box.SelectedItem, "#12");
			Assert.AreEqual (7, box.methods.Count, "#13");
			int i = 0;
			Assert.AreEqual ("OnItemsChanged", box.methods [i++].MethodName, "#14." + i);
			Assert.AreEqual ("OnItemsChanged", box.methods [i++].MethodName, "#14." + i);
			Assert.AreEqual ("OnItemsChanged", box.methods [i++].MethodName, "#14." + i);
			Assert.AreEqual ("SelectionChangedEvent", box.methods [i++].MethodName, "#14." + i);
			Assert.AreEqual ("SelectionChangedEvent", box.methods [i++].MethodName, "#14." + i);
			Assert.AreEqual ("SelectionChangedEvent", box.methods [i++].MethodName, "#14." + i);
			Assert.AreEqual ("SelectionChangedEvent", box.methods [i++].MethodName, "#14." + i);
		}
		
		[TestMethod]
		public void SelectedItemTest2 ()
		{
			FakeComboBox box = new FakeComboBox ();
			object o = new object ();
			box.Items.Add (o);
			box.SelectedItem = o;
			box.methods.Clear ();
			box.SelectedItem = null;
			Assert.AreEqual (1, box.methods.Count, "#1");
			Assert.AreEqual ("SelectionChangedEvent", box.methods [0].MethodName, "#2");
			Assert.AreEqual (null, box.SelectedItem, "#3");
			Assert.AreEqual (-1, box.SelectedIndex, "#4");
		}
	}
}
