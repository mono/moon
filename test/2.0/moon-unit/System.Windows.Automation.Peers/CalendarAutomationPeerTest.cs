//
// Unit tests for CalendarAutomationPeerTest
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

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class CalendarAutomationPeerTest : FrameworkElementAutomationPeerTest {

		[TestMethod]
		[Asynchronous]
		public override void IsKeyboardFocusable ()
		{
			TestIsNotKeyboardFocusable ();
		}

		[TestMethod]
		[Asynchronous]
		public override void IsKeyboardFocusable_Event ()
		{
			TestIsNotKeyboardFocusableEvent ();
		}

		[TestMethod]
		public override void Null ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				new CalendarAutomationPeer (null);
			});
		}

		[TestMethod]
		public override void GetAutomationControlType ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (new Calendar ());
			Assert.AreEqual (AutomationControlType.Calendar, peer.GetAutomationControlType (), "GetAutomationControlType");
		}

		[TestMethod]
		public override void GetName ()
		{
			Calendar fe = CreateConcreteFrameworkElement () as Calendar;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			Assert.AreEqual (peer.GetName (), fe.DisplayDate.ToString (), "GetName");
		}

		[TestMethod]
		public override void GetClassName ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual ("Calendar", peer.GetClassName (), "GetClassNameCore");
		}

		[TestMethod]
		public override void GetName_AttachedProperty0 ()
		{
			Calendar fe = CreateConcreteFrameworkElement () as Calendar;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

			Assert.AreEqual (fe.DisplayDate.ToString (), peer.GetName (), "GetName");
			string name = "Attached Name";

			fe.SetValue (AutomationProperties.NameProperty, name);
			Assert.AreEqual (name, peer.GetName (), "GetName #1");

			fe.SetValue (AutomationProperties.NameProperty, null);
			Assert.AreEqual (fe.DisplayDate.ToString (), peer.GetName (), "GetName #2");
		}

		[TestMethod]
		public override void GetName_AttachedProperty0Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			Calendar fe = CreateConcreteFrameworkElement () as Calendar;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNull (tuple, "#0");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.NameProperty, "Attached Name");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#1");
			Assert.AreEqual ("Attached Name", (string) tuple.NewValue, "#2");
			Assert.AreEqual (fe.DisplayDate.ToString (), tuple.OldValue, "#3");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.NameProperty, "Name");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#4");
			Assert.AreEqual ("Name", (string) tuple.NewValue, "#5");
			Assert.AreEqual ("Attached Name", (string) tuple.OldValue, "#6");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.NameProperty, null);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#7");
			Assert.AreEqual (fe.DisplayDate.ToString (), (string) tuple.NewValue, "#8");
			Assert.AreEqual ("Name", (string) tuple.OldValue, "#9");
		}

		[TestMethod]
		public override void GetName_AttachedProperty1 ()
		{
			Calendar element = CreateConcreteFrameworkElement () as Calendar;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (element);

			string textblockName = "Hello world:";
			string nameProperty = "TextBox name";

			TextBlock textblock = new TextBlock ();
			textblock.Text = textblockName;

			element.SetValue (AutomationProperties.NameProperty, nameProperty);
			Assert.AreEqual (nameProperty, peer.GetName (), "GetName #0");

			element.SetValue (AutomationProperties.LabeledByProperty, textblock);
			Assert.AreEqual (textblockName, peer.GetName (), "GetName #1");

			textblock.Text = null;
			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #2");

			textblock.Text = string.Empty;
			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #3");

			element.SetValue (AutomationProperties.NameProperty, null);

			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #4");

			element.SetValue (AutomationProperties.LabeledByProperty, null);

			Assert.AreEqual (element.DisplayDate.ToString (), peer.GetName (), "GetName #5");
		}

		[TestMethod]
		public override void GetName_AttachedProperty1Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			Calendar fe = CreateConcreteFrameworkElement () as Calendar;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			TextBlock textblock = new TextBlock () { Text = "Hello world:" };
			AutomationPeer textblockPeer = FrameworkElementAutomationPeer.CreatePeerForElement (textblock);

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNull (tuple, "#0");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.NameProperty, "My name");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#1");
			Assert.AreEqual ("My name", (string) tuple.NewValue, "#2");
			Assert.AreEqual (fe.DisplayDate.ToString (), (string) tuple.OldValue, "#3");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.LabeledByProperty, textblock);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#4");
			Assert.AreEqual ("Hello world:", (string) tuple.NewValue, "#5");
			Assert.AreEqual ("My name", (string) tuple.OldValue, "#6");

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
			fe.SetValue (AutomationProperties.LabeledByProperty, null);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#16");
			Assert.AreEqual ("My name", (string) tuple.NewValue, "#17");
			Assert.AreEqual (string.Empty, (string) tuple.OldValue, "#18");

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.LabeledByProperty);
			Assert.IsNotNull (tuple, "#19");
			Assert.AreEqual (null, tuple.NewValue, "#20");
			Assert.AreEqual (textblock, tuple.OldValue, "#21");
		}

		[TestMethod]
		[Asynchronous]
		public override void GetChildren ()
		{
			Calendar calendar = new Calendar ();
			AutomationPeer peer = null;

			CreateAsyncTest (calendar,
			() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
				Assert.IsNotNull (peer, "#0");
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #0");
			},
			() => calendar.DisplayMode = CalendarMode.Month,
			() => {
				List<AutomationPeer> children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #1");
				// 3 buttons: previous, month-year, next
				// 7 labels: each day of the week
				// 42 buttons: 7 days x 6 rows
				// 12 years: used by decade (are offscreen)
				Assert.AreEqual (64, children.Count, "GetChildren #2");
			},
			() => calendar.DisplayMode = CalendarMode.Decade,
			() => {
				List<AutomationPeer> children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #3");
				// 3 buttons: previous, month-year, next
				// 7 labels: each day of the week
				// 42 buttons: 7 days x 6 rows
				// 12 years: used by decade (are offscreen)
				Assert.AreEqual (64, children.Count, "GetChildren #4");
			},
			() => calendar.DisplayMode = CalendarMode.Year,
			() => {
				List<AutomationPeer> children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #5");
				// 3 buttons: previous, month-year, next
				// 7 labels: each day of the week (offscreen)
				// 42 buttons: 7 days x 6 rows (offscreen)
				// 12 years: used by decade
				Assert.AreEqual (64, children.Count, "GetChildren #6");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetPattern ()
		{
			Calendar calendar = new Calendar ();
			AutomationPeer peer = null;
			CreateAsyncTest (calendar,
			() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
				Assert.IsNotNull (peer, "#0");
			},
			() => {
				Assert.IsNull (peer.GetPattern (PatternInterface.Dock), "Dock");
				Assert.IsNull (peer.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
				Assert.IsNull (peer.GetPattern (PatternInterface.GridItem), "GridItem");
				Assert.IsNull (peer.GetPattern (PatternInterface.Invoke), "Invoke");
				Assert.IsNull (peer.GetPattern (PatternInterface.RangeValue), "RangeValue");
				Assert.IsNull (peer.GetPattern (PatternInterface.Scroll), "Scroll");
				Assert.IsNull (peer.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
				Assert.IsNull (peer.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
				Assert.IsNull (peer.GetPattern (PatternInterface.TableItem), "TableItem");
				Assert.IsNull (peer.GetPattern (PatternInterface.Toggle), "Toggle");
				Assert.IsNull (peer.GetPattern (PatternInterface.Transform), "Transform");
				Assert.IsNull (peer.GetPattern (PatternInterface.Value), "Value");
				Assert.IsNull (peer.GetPattern (PatternInterface.Window), "Window");

				Assert.IsNotNull (peer.GetPattern (PatternInterface.Table), "Table");
				Assert.IsNotNull (peer.GetPattern (PatternInterface.MultipleView), "MultipleView");
				Assert.IsNotNull (peer.GetPattern (PatternInterface.Grid), "Grid");
				Assert.IsNotNull (peer.GetPattern (PatternInterface.Selection), "Selection");
			});
		}

		#region IGridProvider Tests

		// NOTE: No GridProvider UIA events raised by CalendarAutomationPeer

		[TestMethod]
		[Asynchronous]
		public void IGridProvider_Methods ()
		{
			Calendar calendar = CreateConcreteFrameworkElement () as Calendar;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
			IGridProvider gridProvider = null;
			IRawElementProviderSimple cell = null;
			AutomationPeer cellPeer = null;

			CreateAsyncTest (calendar,
			() => {
				Assert.IsNotNull (peer, "#0");
				gridProvider = peer.GetPattern (PatternInterface.Grid) as IGridProvider;
				Assert.IsNotNull (gridProvider, "#1");
			},
			// IGridProvider.RowCount
			() => calendar.DisplayMode = CalendarMode.Month,
			() => Assert.AreEqual (6, gridProvider.RowCount, "RowCount #0"), // First row displays day titles
			() => calendar.DisplayMode = CalendarMode.Decade,
			() => Assert.AreEqual (3, gridProvider.RowCount, "RowCount #1"),
			() => calendar.DisplayMode = CalendarMode.Year,
			() => Assert.AreEqual (3, gridProvider.RowCount, "RowCount #2"),
			// IGridProvider.ColumnCount
			() => calendar.DisplayMode = CalendarMode.Month,
			() => Assert.AreEqual (7, gridProvider.ColumnCount, "ColumnCount #0"),
			() => calendar.DisplayMode = CalendarMode.Decade,
			() => Assert.AreEqual (4, gridProvider.ColumnCount, "ColumnCount #1"),
			() => calendar.DisplayMode = CalendarMode.Year,
			() => Assert.AreEqual (4, gridProvider.ColumnCount, "ColumnCount #2"),
			// IGridProvider.GetItem
			() => calendar.DisplayMode = CalendarMode.Month,
			() => {
				cell = gridProvider.GetItem (0, 3);
				Assert.IsNotNull (cell, "GetItem #0");
				cellPeer = new PeerFromProvider ().GetPeerFromProvider (cell);
				Assert.AreEqual (typeof (CalendarDayButton).Name, cellPeer.GetClassName (), "GetItem.ClassName #0");
			},
			() => calendar.DisplayMode = CalendarMode.Year,
			() => {
				cell = gridProvider.GetItem (2, 3);
				Assert.IsNotNull (cell, "GetItem #1");
				cellPeer = new PeerFromProvider ().GetPeerFromProvider (cell);
				Assert.AreEqual (typeof (CalendarButton).Name, cellPeer.GetClassName (), "GetItem.ClassName #1");
			},
			() => calendar.DisplayMode = CalendarMode.Decade,
			() => {
				cell = gridProvider.GetItem (2, 3);
				Assert.IsNotNull (cell, "GetItem #2");
				cellPeer = new PeerFromProvider ().GetPeerFromProvider (cell);
				Assert.AreEqual (typeof (CalendarButton).Name, cellPeer.GetClassName (), "GetItem.ClassName #2");

				cell = gridProvider.GetItem (10, 10);
				Assert.IsNull (cell, "GetItem #3");
			});
		}

		#endregion

		#region IMultipleViewProvider Tests

		// NOTE: No MultipleViewProvider UIA events raised by CalendarAutomationPeer

		[TestMethod]
		[Asynchronous]
		public void IMultipleViewProvider_Methods ()
		{
			Calendar calendar = CreateConcreteFrameworkElement () as Calendar;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
			IMultipleViewProvider multiViewProvider = null;

			CreateAsyncTest (calendar,
			() => {
				Assert.IsNotNull (peer, "#0");
				multiViewProvider = peer.GetPattern (PatternInterface.MultipleView) as IMultipleViewProvider;
				Assert.IsNotNull (multiViewProvider, "#1");
			},
			// IMultipleViewProvider.GetSupportedViews
			() => {
				int[] views = multiViewProvider.GetSupportedViews ();
				Assert.IsNotNull (views, "GetSupportedViews #0");
				Assert.AreEqual ((int) CalendarMode.Month, views[0], "GetSupportedViews #1");
				Assert.AreEqual ((int) CalendarMode.Year, views[1], "GetSupportedViews #2");
				Assert.AreEqual ((int) CalendarMode.Decade, views[2], "GetSupportedViews #3");
			},
			// IMultipleViewProvider.GetViewName
			() => {
				Assert.AreEqual (CalendarMode.Month.ToString (), multiViewProvider.GetViewName (0), "GetViewName #0");
				Assert.AreEqual (CalendarMode.Year.ToString (), multiViewProvider.GetViewName (1), "GetViewName #1");
				Assert.AreEqual (CalendarMode.Decade.ToString (), multiViewProvider.GetViewName (2), "GetViewName #2");
			},
			// IMultipleViewProvider.CurrentView
			() => calendar.DisplayMode = CalendarMode.Month,
			() => Assert.AreEqual ((int) CalendarMode.Month, multiViewProvider.CurrentView, "CurrentView #0"),
			() => calendar.DisplayMode = CalendarMode.Year,
			() => Assert.AreEqual ((int) CalendarMode.Year, multiViewProvider.CurrentView, "CurrentView #1"),
			() => calendar.DisplayMode = CalendarMode.Decade,
			() => Assert.AreEqual ((int) CalendarMode.Decade, multiViewProvider.CurrentView, "CurrentView #2"),
			// IMultipleViewProvider.SetCurrentView
			() => multiViewProvider.SetCurrentView (0),
			() => {
				Assert.AreEqual (CalendarMode.Month, calendar.DisplayMode, "SetCurrentView #0");
				Assert.AreEqual ((int) CalendarMode.Month, multiViewProvider.CurrentView, "SetCurrentView #1");
			},
			() => multiViewProvider.SetCurrentView (1),
			() => {
				Assert.AreEqual (CalendarMode.Year, calendar.DisplayMode, "SetCurrentView #2");
				Assert.AreEqual ((int) CalendarMode.Year, multiViewProvider.CurrentView, "SetCurrentView #3");
			},
			() => multiViewProvider.SetCurrentView (2),
			() => {
				Assert.AreEqual (CalendarMode.Decade, calendar.DisplayMode, "SetCurrentView #4");
				Assert.AreEqual ((int) CalendarMode.Decade, multiViewProvider.CurrentView, "SetCurrentView #5");
			},
			() => Assert.Throws<ArgumentOutOfRangeException> (() => multiViewProvider.SetCurrentView (3)));
		}

		#endregion

		#region ISelectionProvider Tests

		// NOTE: No SelectionProvider UIA events raised by CalendarAutomationPeer

		[TestMethod]
		[Asynchronous]
		public void ISelectionProvider_Methods ()
		{
			Calendar calendar = CreateConcreteFrameworkElement () as Calendar;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
			ISelectionProvider selectionProvider = null;

			CreateAsyncTest (calendar,
			() => {
				Assert.IsNotNull (peer, "#0");
				selectionProvider = peer.GetPattern (PatternInterface.Selection) as ISelectionProvider;
				Assert.IsNotNull (selectionProvider, "#1");
			},
			// ISelectionProvider.IsSelectionRequired
			() => calendar.SelectionMode = CalendarSelectionMode.None,
			() => Assert.IsFalse (selectionProvider.IsSelectionRequired, "IsSelectionRequired #0"),
			() => calendar.SelectionMode = CalendarSelectionMode.SingleDate,
			() => Assert.IsFalse (selectionProvider.IsSelectionRequired, "IsSelectionRequired #1"),
			() => calendar.SelectionMode = CalendarSelectionMode.SingleRange,
			() => Assert.IsFalse (selectionProvider.IsSelectionRequired, "IsSelectionRequired #2"),
			() => calendar.SelectionMode = CalendarSelectionMode.MultipleRange,
			() => Assert.IsFalse (selectionProvider.IsSelectionRequired, "IsSelectionRequired #3"),
			// ISelectionProvider.CanSelectMultiple
			() => calendar.SelectionMode = CalendarSelectionMode.None,
			() => Assert.IsFalse (selectionProvider.CanSelectMultiple, "CanSelectMultiple #0"),
			() => calendar.SelectionMode = CalendarSelectionMode.SingleDate,
			() => Assert.IsFalse (selectionProvider.CanSelectMultiple, "CanSelectMultiple #1"),
			() => calendar.SelectionMode = CalendarSelectionMode.SingleRange,
			() => Assert.IsTrue (selectionProvider.CanSelectMultiple, "CanSelectMultiple #2"),
			() => calendar.SelectionMode = CalendarSelectionMode.MultipleRange,
			() => Assert.IsTrue (selectionProvider.CanSelectMultiple, "CanSelectMultiple #3"),
			// ISelectionProvider.GetSelection
			() => {
				Assert.IsNull (calendar.SelectedDate);
				Assert.IsTrue (selectionProvider.CanSelectMultiple, "GetSelection #0");
			},
			() => calendar.DisplayMode = CalendarMode.Year,
			() => calendar.SelectedDates.AddRange (new DateTime(2000, 2, 10), new DateTime (2000, 3, 30)),
			() => {
				IRawElementProviderSimple[] selection = selectionProvider.GetSelection ();
				Assert.IsNull (selection, "GetSelection #1");
			},
			() => calendar.DisplayMode = CalendarMode.Month,
			() => {
				IRawElementProviderSimple[] selection = selectionProvider.GetSelection ();
				Assert.IsNotNull (selection, "GetSelection #4");
				Assert.AreEqual (selection.Length, 31, "GetSelection #5");
				AutomationPeer cellPeer = new PeerFromProvider ().GetPeerFromProvider (selection [0]);
				Assert.AreEqual (cellPeer.GetClassName (), typeof(CalendarDayButton).Name, "GetSelection #6");
			});
		}

		#endregion

		#region ITableProvider Tests

		// NOTE: No TableProvider UIA events raised by CalendarAutomationPeer

		[TestMethod]
		[Asynchronous]
		public void ITableProvider_Methods ()
		{
			Calendar calendar = CreateConcreteFrameworkElement () as Calendar;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
			ITableProvider tableProvider = null;

			CreateAsyncTest (calendar,
			() => {
				Assert.IsNotNull (peer, "#0");
				tableProvider = peer.GetPattern (PatternInterface.Selection) as ITableProvider;
				Assert.IsNotNull (tableProvider, "#1");
			},
			// ITableProvider.RowOrColumnMajor
			() => Assert.AreEqual (tableProvider.RowOrColumnMajor, RowOrColumnMajor.RowMajor, "RowOrColumnMajor #0"),
			// ITableProvider.GetRowHeaders
			() => {
				IRawElementProviderSimple[] headers = tableProvider.GetRowHeaders ();
				Assert.IsNull (headers, "GetRowHeaders #1");
			},
			// ITableProvider.GetColumnHeaders
			() => calendar.DisplayMode = CalendarMode.Month,
			() => {
				IRawElementProviderSimple[] headers = tableProvider.GetColumnHeaders ();
				Assert.IsNotNull (headers, "GetColumnHeaders #2");
				Assert.AreEqual (headers.Length, 7, "GetColumnHeader #3");
			},
			() => calendar.DisplayMode = CalendarMode.Decade,
			() => {
				IRawElementProviderSimple[] headers = tableProvider.GetColumnHeaders ();
				Assert.IsNull (headers, "GetColumnHeaders #4");
			},
			() => calendar.DisplayMode = CalendarMode.Year,
			() => {
				IRawElementProviderSimple[] headers = tableProvider.GetColumnHeaders ();
				Assert.IsNull (headers, "GetColumnHeaders #5");;
			});
		}

		#endregion

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			Calendar calendar = new Calendar ();
			calendar.Height = 200;
			calendar.Width = 200;
			DateTime date = new DateTime (2000, 2, 2);
			calendar.DisplayDate = date;
			calendar.SelectedDate = date;
			return calendar;
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			// Can't subclass CalendarAutomationPeer
			return null;
		}

	}

}
