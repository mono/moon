//
// ItemContainerGenerator Unit Tests
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
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Controls.Primitives;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using System.Windows.Markup;
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.Windows.Controls
{
	[TestClass]
	public partial class ItemContainerGeneratorTest : SilverlightTest
	{
		ItemsControlPoker Control {
			get;set;
		}

		ItemContainerGenerator Generator {
			get { return Control.ItemContainerGenerator; }
		}

		[TestInitialize]
		public void Initialize ()
		{
			Control = new ItemsControlPoker ();
		}

		[TestMethod]
		public void ContainerFromIndex_Negative ()
		{
			Assert.IsNull (Generator.ContainerFromIndex (-1), "1#");
			Assert.IsNull (Generator.ContainerFromIndex (-10), "1#");
		}

		[TestMethod]
		public void ContainerFromIndex_OutOfRange ()
		{
			Assert.IsNull (Generator.ContainerFromIndex (100), "#1");
		}

		[TestMethod]
		[Asynchronous]
		public void ContainerFromIndex_Realised ()
		{
			object o = new object();
			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => Control.Items.Add (o),
				() => Assert.IsNotNull (Generator.ContainerFromItem (o))
			);
		}

		[TestMethod]
		public void ContainerFromIndex_Unrealised ()
		{
			Control.Items.Add (123);
			Control.Items.Add (123);
			Assert.IsNull (Generator.ContainerFromIndex (0), "#1");
		}

		[TestMethod]
		public void ContainerFromItem_NotInCollection ()
		{
			Assert.IsNull (Generator.ContainerFromItem (new object ()), "#1");
		}

		[TestMethod]
		public void ContainerFromItem_Null ()
		{
			Assert.IsNull (Generator.ContainerFromItem (null), "#1");
		}

		[TestMethod]
		[Asynchronous]
		public void ContainerFromItem_Realised ()
		{
			object o = new object ();
			Control.Items.Add (123);
			Control.Items.Add (o);
			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => Assert.IsNotNull (Generator.ContainerFromItem (o), "#1")
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ContainerFromItem_ValueType()
		{
			int[] items = new[] { 5, 5, 5, 5, 5 };
			Control.ItemsSource = items;
			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => Assert.IsNotNull (Generator.ContainerFromIndex (0), "#1"),
				() => Assert.AreEqual (Generator.ContainerFromIndex (0), Generator.ContainerFromItem (5), "#2")
			);
		}

		[TestMethod]
		public void ContainerFromItem_Unrealised ()
		{
			object o = new object ();
			Control.Items.Add (123);
			Control.Items.Add (o);
			Assert.IsNull (Generator.ContainerFromItem (o), "#1");
		}

		[TestMethod]
		[Asynchronous]
		public void ContainerMatchesItem ()
		{
			// Check that ContainerFromItem and ItemFromContainer match up
			object item = new object ();
			DependencyObject container = null;
			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => Control.Items.Add (item),
				() => container = Generator.ContainerFromItem (item),
				() => Assert.AreSame (item, Generator.ItemFromContainer (container), "#1")
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ContainerMatchesIndex ()
		{
			// Check that ContainerFromIndex and IndexFromContainer match up
			DependencyObject container = null;
			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => {
					Control.Items.Add (0);
					Control.Items.Add (1);
					Control.Items.Add (2);
				},
				() => container = Generator.ContainerFromIndex (1),
				() => Assert.AreEqual (1, Generator.IndexFromContainer (container), "#1")
			);
		}

		[TestMethod]
		public void GeneratorPositionFromIndex_Negative ()
		{
			GeneratorPosition p = Generator.GeneratorPositionFromIndex (-100);
			Assert.AreEqual (-1, p.Index, "#1");
			Assert.AreEqual (0, p.Offset, "#2");

			p = Generator.GeneratorPositionFromIndex (-1);
			Assert.AreEqual (-1, p.Index, "#3");
			Assert.AreEqual (0, p.Offset, "#4");
		}

		[TestMethod]
		public void GeneratorPositionFromIndex_OutOfRange_NoElements ()
		{
			GeneratorPosition p = Generator.GeneratorPositionFromIndex (100);
			Assert.AreEqual (-1, p.Index, "#1");
			Assert.AreEqual (0, p.Offset, "#2");
		}

		[TestMethod]
		public void GeneratorPositionFromIndex_OutOfRange_TwoElements_Unrealised ()
		{
			Control.Items.Add (new object ());
			Control.Items.Add (new object ());

			GeneratorPosition p = Generator.GeneratorPositionFromIndex (100);
			Assert.AreEqual (-1, p.Index, "#1");
			Assert.AreEqual (0, p.Offset, "#2");
		}

		[TestMethod]
		[Asynchronous]
		public void GeneratorPositionFromIndex_OutOfRange_TwoElements_Realised ()
		{
			Control.Items.Add (new object ());
			Control.Items.Add (new object ());

			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => {
					GeneratorPosition p = Generator.GeneratorPositionFromIndex (100);
					Assert.AreEqual (-1, p.Index, "#1");
					Assert.AreEqual (0, p.Offset, "#2");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void GeneratorPositionFromIndex_Realised_First ()
		{
			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => {
					Control.Items.Add (new object ());
					Control.Items.Add (new object ());
					Control.Items.Add (new object ());
					Control.Items.Add (new object ());
					Control.Items.Add (new object ());
				}, () => {
					GeneratorPosition p = Generator.GeneratorPositionFromIndex (0);
					Assert.AreEqual (0, p.Index, "#1");
					Assert.AreEqual (0, p.Offset, "#2");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void GeneratorPositionFromIndex_Realised_Middle ()
		{
			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => {
					Control.Items.Add (new object ());
					Control.Items.Add (new object ());
					Control.Items.Add (new object ());
					Control.Items.Add (new object ());
					Control.Items.Add (new object ());
				}, () => {
					GeneratorPosition p = Generator.GeneratorPositionFromIndex (2);
					Assert.AreEqual (2, p.Index, "#1");
					Assert.AreEqual (0, p.Offset, "#2");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void GeneratorPositionFromIndex_Realised_Last ()
		{
			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => {
					Control.Items.Add (new object ());
					Control.Items.Add (new object ());
					Control.Items.Add (new object ());
					Control.Items.Add (new object ());
					Control.Items.Add (new object ());
				}, () => {
					GeneratorPosition p = Generator.GeneratorPositionFromIndex (4);
					Assert.AreEqual (4, p.Index, "#1");
					Assert.AreEqual (0, p.Offset, "#2");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void GeneratorCallsItemsControlPrepare ()
		{
			Control.Items.Add (new object ());
			CreateAsyncTest (Control, () => {
				Control.LastPreparedContainer = null;
				Control.LastPreparedItem = null;
				
				var container = Control.ItemContainerGenerator.ContainerFromIndex (0);
				((IItemContainerGenerator) Control.ItemContainerGenerator).PrepareItemContainer (container);
				Assert.AreSame (container, Control.ItemContainerGenerator.ContainerFromIndex (0), "#1");
				Assert.AreSame (Control.Items [0], Control.LastPreparedItem, "#2");
			});
		}
	}
}