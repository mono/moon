//
// Unit tests for DataGridColumnHeaderAutomationPeer
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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
using System.Windows.Automation;
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Linq;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class DataGridColumnHeaderAutomationPeerTest : FrameworkElementAutomationPeerTest {

		[TestInitialize]
		public void TestInitialize ()
		{
			canvas = new Canvas ();
			datagrid = CreateDataGrid ();
			canvas.Children.Add (datagrid);
			TestPanel.Children.Add (canvas);
		}

		[TestCleanup]
		public void TestCleanup ()
		{
			TestPanel.Children.Remove (canvas);
		}

		// All these methods are tested in AllTests
		public override void GetAutomationControlType () {}
		public override void GetBoundingRectangle () {}
		public override void GetChildren () {}
		public override void IsKeyboardFocusable () {}
		public override void GetClassName () {}
		public override void IsContentElement () {}
		public override void GetName () {}
		public override void GetClickablePoint () {}
		public override void GetParentTest () {}
		public override void IsOffScreen () {}

		[TestMethod]
		[MinRuntimeVersion (4)] // NRE in SL2, ANE in SL4
		public override void Null ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				new DataGridColumnHeaderAutomationPeer (null);
			});
		}

		[TestMethod]
		public void AllTests ()
		{
			DataGridColumnHeader fe = CreateConcreteFrameworkElement () as DataGridColumnHeader;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

			// GetAutomationControlType
			Assert.AreEqual (AutomationControlType.HeaderItem, peer.GetAutomationControlType (), "GetAutomationControlType");

			// GetBoundingRectangle
			Rect boundingRectangle = peer.GetBoundingRectangle ();
			Assert.AreNotEqual (0, boundingRectangle.X, "GetBoundingRectangle X");
			Assert.AreNotEqual (0, boundingRectangle.Y, "GetBoundingRectangle Y");
			Assert.AreNotEqual (0, boundingRectangle.Width, "GetBoundingRectangle Width");
			Assert.AreNotEqual (0, boundingRectangle.Height, "GetBoundingRectangle Height");

			// GetChildren
			List<AutomationPeer> children = peer.GetChildren ();
			Assert.IsNotNull (children, "#0");
			Assert.AreEqual (1, children.Count, "#1");

			// IsKeyboardFocusable
			Assert.IsFalse (peer.IsKeyboardFocusable (), "IsKeyboardFocusable");

			// GetClassName
			Assert.AreEqual ("DataGridColumnHeader", peer.GetClassName (), "#0");

			// IsContentElement
			Assert.IsFalse (peer.IsContentElement (), "#0");

			// GetName
			Assert.AreEqual (fe.Content, peer.GetName (), "GetName");

			// GetClickablePoint
			Point p = peer.GetClickablePoint ();
			Assert.IsFalse (double.IsNaN (p.X), "#0");
			Assert.IsFalse (double.IsNaN (p.Y), "#1");

			// GetParent
			Assert.IsNotNull (peer.GetParent (), "GetParent");

			// IsOffscreen
			Assert.IsFalse (peer.IsOffscreen (), "IsOffScreen");
		}

		[TestMethod]
		[Asynchronous]
		public override void GetChildrenChanged ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			TestPanel.Children.Clear ();
			datagrid = CreateDataGrid ();

			ContentControl control = null;
			AutomationPeer peer = null;
			List<AutomationPeer> children = null;
			AutomationEventTuple tuple = null;
			object oldContent = null;

			CreateAsyncTest (datagrid,
			() => {
				peer = GetDataGridColumnHeader (datagrid);
				control = ((FrameworkElementAutomationPeer) peer).Owner as ContentControl;
				oldContent = control.Content;
				Assert.IsNotNull (peer, "Peer");
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #0");
				Assert.AreEqual (1, children.Count, "GetChildren #1");
			},
			() => {
				control.Content = "Hello";
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #1");
				Assert.AreEqual (1, children.Count, "Children.Count #0");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #0");
				EventsManager.Instance.Reset ();
			},
			() => control.Content = null,
			() => {
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #3");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #1");
				EventsManager.Instance.Reset ();

				control.Content = oldContent;
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #4");
				Assert.AreEqual (1, children.Count, "Children.Count #1");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #6");
			});
		}

		[TestMethod]
		public override void GetName_AttachedProperty0 ()
		{
			DataGridColumnHeader fe = CreateConcreteFrameworkElement () as DataGridColumnHeader;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			Assert.AreEqual (fe.Content, peer.GetName (), "GetName");

			string name = "Attached Name";

			fe.SetValue (AutomationProperties.NameProperty, name);
			Assert.AreEqual (fe.Content, peer.GetName (), "GetName #1");

			fe.SetValue (AutomationProperties.NameProperty, null);
			Assert.AreEqual (fe.Content, peer.GetName (), "GetName #2");
		}

		[TestMethod]
		public override void GetName_AttachedProperty1 ()
		{
			DataGridColumnHeader element = CreateConcreteFrameworkElement () as DataGridColumnHeader;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (element);

			string textblockName = "Hello world:";
			string nameProperty = "TextBox name";

			TextBlock textblock = new TextBlock ();
			textblock.Text = textblockName;

			element.SetValue (AutomationProperties.NameProperty, nameProperty);
			Assert.AreEqual (element.Content, peer.GetName (), "GetName #0");

			element.SetValue (AutomationProperties.LabeledByProperty, textblock);
			Assert.AreEqual (textblockName, peer.GetName (), "GetName #1");

			textblock.Text = null;
			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #2");

			textblock.Text = string.Empty;
			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #3");

			element.SetValue (AutomationProperties.NameProperty, null);

			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #4");

			element.SetValue (AutomationProperties.LabeledByProperty, null);

			Assert.AreEqual (element.Content, peer.GetName (), "GetName #5");
		}

		[TestMethod]
		public override void GetPattern ()
		{
			AutomationPeer feap = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());

			Assert.IsNull (feap.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (feap.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (feap.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (feap.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (feap.GetPattern (PatternInterface.RangeValue), "RangeValue");
			Assert.IsNull (feap.GetPattern (PatternInterface.Scroll), "Scroll");
			Assert.IsNull (feap.GetPattern (PatternInterface.Selection), "Selection");
			Assert.IsNull (feap.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (feap.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (feap.GetPattern (PatternInterface.Value), "Value");
			Assert.IsNull (feap.GetPattern (PatternInterface.Window), "Window");

			Assert.IsNotNull (feap.GetPattern (PatternInterface.Invoke), "Invoke");
			Assert.IsNotNull (feap.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNotNull (feap.GetPattern (PatternInterface.Transform), "Transform");
		}

		[TestMethod]
		public override void IsOffScreen_ScrollViewer ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.IsNotNull (peer, "IsOffScreen");

			DataGridColumnHeader control = ((FrameworkElementAutomationPeer) peer).Owner as DataGridColumnHeader;

			Assert.IsFalse (peer.IsOffscreen (), "IsOffScreen #1");
			control.Visibility = Visibility.Collapsed;
			Assert.IsTrue (peer.IsOffscreen (), "IsOffScreen #2");
			control.Visibility = Visibility.Visible;
			Assert.IsFalse (peer.IsOffscreen (), "IsOffScreen #3");
		}

		[TestMethod]
		public override void ContentTest ()
		{
			// AutomationPeer is not ContentElement
		}

		[TestMethod]
		public override void IsKeyboardFocusable_Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			DataGridColumnHeader control = CreateConcreteFrameworkElement () as DataGridColumnHeader;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (control);
			AutomationPropertyEventTuple tuple = null;

			Assert.IsFalse (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #1");
			EventsManager.Instance.Reset ();
			control.IsEnabled = false;

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
			                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
			Assert.IsNull (tuple, "#0");

			EventsManager.Instance.Reset ();
			control.IsEnabled = true;

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
			                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
			Assert.IsNull (tuple, "#1");

			EventsManager.Instance.Reset ();
			control.Visibility = Visibility.Collapsed;

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
			                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
			Assert.IsNull (tuple, "#2");

			EventsManager.Instance.Reset ();
			control.Visibility = Visibility.Visible;

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
			                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
			Assert.IsNull (tuple, "#3");

			EventsManager.Instance.Reset ();
			control.IsTabStop = false;

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
			                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
			Assert.IsNull (tuple, "#4");

			EventsManager.Instance.Reset ();
			control.IsTabStop = true;

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
			                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
			Assert.IsNotNull (tuple, "#5");
			Assert.IsFalse ((bool) tuple.OldValue, "OldValue #0");
			Assert.IsTrue ((bool) tuple.NewValue, "NewValue #0");

			EventsManager.Instance.Reset ();
			control.IsTabStop = false;

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
			                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
			Assert.IsNotNull (tuple, "#6");
			Assert.IsTrue ((bool) tuple.OldValue, "OldValue #1");
			Assert.IsFalse ((bool) tuple.NewValue, "NewValue #1");
		}

		[TestMethod]
		public override void GetName_AttachedProperty0Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			DataGridColumnHeader columnHeader = CreateConcreteFrameworkElement () as DataGridColumnHeader;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (columnHeader);
			AutomationPropertyEventTuple tuple = null;

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNull (tuple, "#0");

			EventsManager.Instance.Reset ();
			columnHeader.SetValue (AutomationProperties.NameProperty, "Attached Name");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNull (tuple, "#1");

			EventsManager.Instance.Reset ();
			columnHeader.SetValue (AutomationProperties.NameProperty, "Name");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNull (tuple, "#2");

			EventsManager.Instance.Reset ();
			columnHeader.SetValue (AutomationProperties.NameProperty, null);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNull (tuple, "#3");
		}

		[TestMethod]
		public override void GetName_AttachedProperty1Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			DataGridColumnHeader columnHeader = CreateConcreteFrameworkElement () as DataGridColumnHeader;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (columnHeader);
			AutomationPropertyEventTuple tuple = null;

			TextBlock textblock = new TextBlock () { Text = "Hello world:" };
			AutomationPeer textblockPeer = FrameworkElementAutomationPeer.CreatePeerForElement (textblock);

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNull (tuple, "#0");

			EventsManager.Instance.Reset ();
			columnHeader.SetValue (AutomationProperties.NameProperty, "My name");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNull (tuple, "#1");

			EventsManager.Instance.Reset ();
			columnHeader.SetValue (AutomationProperties.LabeledByProperty, textblock);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#2");
			Assert.AreEqual ("Hello world:", (string) tuple.NewValue, "#5");
			Assert.AreEqual (columnHeader.Content, (string) tuple.OldValue, "#6");

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.LabeledByProperty);
			Assert.IsNotNull (tuple, "#7");
			Assert.AreEqual (textblock, tuple.NewValue, "#8");
			Assert.AreEqual (null, tuple.OldValue, "#9");

			EventsManager.Instance.Reset ();
			textblock.Text = null;
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#10");
			Assert.AreEqual (string.Empty, (string) tuple.NewValue, "#11");
			Assert.AreEqual ("Hello world:", (string) tuple.OldValue, "#12");

			tuple = EventsManager.Instance.GetAutomationEventFrom (textblockPeer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#13");
			Assert.AreEqual (string.Empty, (string) tuple.NewValue, "#14");
			Assert.AreEqual ("Hello world:", (string) tuple.OldValue, "#15");

			EventsManager.Instance.Reset ();
			columnHeader.SetValue (AutomationProperties.LabeledByProperty, null);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#16");
			Assert.AreEqual (columnHeader.Content, (string) tuple.NewValue, "#17");
			Assert.AreEqual (string.Empty, (string) tuple.OldValue, "#18");

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.LabeledByProperty);
			Assert.IsNotNull (tuple, "#19");
			Assert.AreEqual (null, tuple.NewValue, "#20");
			Assert.AreEqual (textblock, tuple.OldValue, "#21");
		}

		[TestMethod]
		public override void IsOffScreen_Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			DataGridColumnHeader fe = CreateConcreteFrameworkElement () as DataGridColumnHeader;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.BoundingRectangleProperty);
			Assert.IsNull (tuple, "#0");

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
			Assert.IsNull (tuple, "IsOffscreen #0");

			EventsManager.Instance.Reset ();
			fe.Visibility = Visibility.Visible;

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.BoundingRectangleProperty);
			Assert.IsNull (tuple, "#1");

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
			Assert.IsNull (tuple, "IsOffscreen #1");

			EventsManager.Instance.Reset ();
			fe.Visibility = Visibility.Collapsed;

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.BoundingRectangleProperty);
			Assert.IsNotNull (tuple, "#2");

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
			Assert.IsNotNull (tuple, "IsOffscreen #2");

			EventsManager.Instance.Reset ();
			fe.Visibility = Visibility.Visible;

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.BoundingRectangleProperty);
			Assert.IsNotNull (tuple, "#8");

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
			Assert.IsNotNull (tuple, "IsOffscreen #5");
		}

		[TestMethod]
		public override void IsOffScreen_Event1 ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			DataGridColumnHeader fe = CreateConcreteFrameworkElement () as DataGridColumnHeader;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

			AutomationPropertyEventTuple tuple = null;

			EventsManager.Instance.Reset ();
			canvas.Visibility = Visibility.Visible;

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
			Assert.IsNull (tuple, "IsOffscreen #4X");

			// Testing when our parent is not Visible
			EventsManager.Instance.Reset ();
			canvas.Visibility = Visibility.Collapsed;

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
			Assert.IsNotNull (tuple, "IsOffscreen #1");
			Assert.IsTrue ((bool) tuple.NewValue, "IsOffscreen #2");
			Assert.IsFalse ((bool) tuple.OldValue, "IsOffscreen #3");
		}

		#region IInvokeProvider Tests

		[TestMethod]
		public void IInvokeProvider_Invoke ()
		{
			DataGridColumnHeader fe = CreateConcreteFrameworkElement () as DataGridColumnHeader;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

			IInvokeProvider invoke = peer.GetPattern (PatternInterface.Invoke) as IInvokeProvider;
			Assert.IsNotNull (invoke, "InvokeProvider is null");

			invoke.Invoke ();
			fe.IsEnabled = false;

			invoke.Invoke ();

			fe.IsEnabled = true;
			invoke.Invoke ();
		}

		[TestMethod]
		[Asynchronous]
		public void InvokeProvider_Events ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			TestPanel.Children.Clear ();
			datagrid = CreateDataGrid ();
			IInvokeProvider provider = null;
			AutomationEventTuple tuple = null;
			AutomationPeer peer = null;

			CreateAsyncTest (datagrid,
			() => {
				peer = GetDataGridColumnHeader (datagrid);
				provider = (IInvokeProvider) peer.GetPattern (PatternInterface.Invoke);
				Assert.IsNotNull (provider, "GetPattern #0");
				EventsManager.Instance.Reset ();
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.InvokePatternOnInvoked);
				Assert.IsNull (tuple, "GetAutomationEventFrom #0");
				provider.Invoke ();
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.InvokePatternOnInvoked);
				Assert.IsNotNull (tuple, "GetAutomationEventFrom #1");
			});
		}

		#endregion

		#region ITransformProvider tests

		// No UIA events are raised

		[TestMethod]
		public void ITransformProvider_Methods ()
		{
			DataGridColumnHeader fe = CreateConcreteFrameworkElement () as DataGridColumnHeader;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

			ITransformProvider transform = peer.GetPattern (PatternInterface.Invoke) as ITransformProvider;
			Assert.IsNotNull (transform, "#0");

			Assert.IsFalse (transform.CanMove, "#1");
			Assert.IsTrue (transform.CanResize, "#2");
			Assert.IsFalse (transform.CanRotate, "#3");

			// No errors or exceptions should happen
			transform.Move (10, 10);
			transform.Resize (100, 1000);
			transform.Rotate (10);
		}

		#endregion

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			FrameworkElementAutomationPeer peer = GetDataGridColumnHeader (datagrid) as FrameworkElementAutomationPeer;
			return peer.Owner as DataGridColumnHeader;
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			// Can't subclass DataGridColumnHeaderAutomationPeer
			return null;
		}

		private AutomationPeer GetDataGridColumnHeader (DataGrid datagrid)
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (datagrid);
			List<AutomationPeer> children = peer.GetChildren ();
			foreach (AutomationPeer child in children) {
				if (child.GetAutomationControlType () == AutomationControlType.Header)
					return child.GetChildren () [0];
			}
			return null;
		}

		private DataGrid CreateDataGrid ()
		{
			DataGrid dg = new DataGrid ();
			dg.Width = 50;
			dg.Height = 100;

			List<Data> source = new List<Data> ();
			int itemsCount = 10;

			for (int i = 0; i < itemsCount; i++) {
				source.Add (new Data () {
					Name = "First/Last",
					Age = i,
					Available = (i % 2 == 0)
				});
			}

			dg.ItemsSource = source;

			return dg;
		}

		public class Data {
			public string Name { get; set; }
			public int Age { get; set; }
			public bool Available { get; set; }
		}

		private Canvas canvas;
		private DataGrid datagrid;
	}
}
