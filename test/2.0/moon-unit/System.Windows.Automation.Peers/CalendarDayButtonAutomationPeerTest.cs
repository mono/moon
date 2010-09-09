//
// Unit tests for CalendarDayButtonAutomationPeerTest
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
using SG = System.Globalization;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class CalendarDayButtonAutomationPeerTest : ButtonAutomationPeerTest {

		[TestInitialize]
		public void TestInitialize ()
		{
			calendar = new Calendar ();
			calendar.Height = 200;
			calendar.Width = 200;
			DateTime date = new DateTime (2000, 2, 2);
			calendar.DisplayDate = date;
			calendar.SelectedDate = date;

		}

		[TestCleanup]
		public void TestCleanup ()
		{
			TestPanel.Children.Remove (calendar);
		}

		[TestMethod]
		[Asynchronous]
		public override void GetBoundingRectangle ()
		{
			CreateAsyncTest (calendar,
			() => {
			List<AutomationPeer> buttonChildren = GetButtonChildren ();
			AutomationPeer peer = buttonChildren [0];
			Rect boundingRectangle = peer.GetBoundingRectangle ();
			Assert.AreNotSame (Rect.Empty, boundingRectangle, "GetBoundingRectangleCore Isempty");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void IsKeyboardFocusable()
		{
			CreateAsyncTest (calendar,
			() => {
			List<AutomationPeer> buttonChildren = GetButtonChildren ();
			AutomationPeer peer = buttonChildren [0];
			Assert.IsFalse (peer.IsKeyboardFocusable (), "IsKeyboardFocusable");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void IsKeyboardFocusable_Event ()
		{
			TestIsNotKeyboardFocusableEvent (new CalendarDayButton ());
		}

		[TestMethod]
		[Asynchronous]
		public override void GetPattern ()
		{
			CreateAsyncTest (calendar,
			() => {
				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				CalendarDayButtonAutomationPeer cdbap = buttonChildren[0] as CalendarDayButtonAutomationPeer;
				Assert.IsNotNull (cdbap, "#0");

				Assert.IsNull (cdbap.GetPattern (PatternInterface.Dock), "Dock");
				Assert.IsNull (cdbap.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
				Assert.IsNull (cdbap.GetPattern (PatternInterface.Grid), "Grid");
				Assert.IsNull (cdbap.GetPattern (PatternInterface.MultipleView), "MultipleView");
				Assert.IsNull (cdbap.GetPattern (PatternInterface.RangeValue), "RangeValue");
				Assert.IsNull (cdbap.GetPattern (PatternInterface.Scroll), "Scroll");
				Assert.IsNull (cdbap.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
				Assert.IsNull (cdbap.GetPattern (PatternInterface.Selection), "Selection");
				Assert.IsNull (cdbap.GetPattern (PatternInterface.Table), "Table");
				Assert.IsNull (cdbap.GetPattern (PatternInterface.Toggle), "Toggle");
				Assert.IsNull (cdbap.GetPattern (PatternInterface.Transform), "Transform");
				Assert.IsNull (cdbap.GetPattern (PatternInterface.Value), "Value");
				Assert.IsNull (cdbap.GetPattern (PatternInterface.Window), "Window");

				Assert.IsNotNull (cdbap.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
				Assert.IsNotNull (cdbap.GetPattern (PatternInterface.TableItem), "TableItem");
				Assert.IsNotNull (cdbap.GetPattern (PatternInterface.GridItem), "GridItem");
				Assert.IsNotNull (cdbap.GetPattern (PatternInterface.Invoke), "Invoke");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void ContentTest ()
		{
			Button button = null;
			StackPanel stackPanel = null;
			object oldContent = null;
			CreateAsyncTest (calendar,
			() => {
				// StackPanel and two TextBlocks
				stackPanel = new StackPanel ();
				stackPanel.Children.Add (new TextBlock () { Text = "Text0" });
				stackPanel.Children.Add (new TextBlock () { Text = "Text1" });
				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				button = ((FrameworkElementAutomationPeer) buttonChildren [0]).Owner as Button;
				oldContent = button.Content;
			},
			() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (button);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				List<AutomationPeer> children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #0");
				Assert.AreEqual (1, children.Count, "GetChildren.Count #0");

				button.Content = stackPanel;
			},
			() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (button);
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #1");
				Assert.AreEqual (2, peer.GetChildren ().Count, "GetChildren.Count #1");
				// We add one TextBlock
				stackPanel.Children.Add (new TextBlock () { Text = "Text2" });
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #2");
				Assert.AreEqual (3, peer.GetChildren ().Count, "GetChildren.Count #2");
			},
			() => button.Content = oldContent,
			() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (button);
				List<AutomationPeer> children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #3");
				Assert.AreEqual (1, children.Count, "GetChildren.Count #3");

				TestPanel.Children.Remove (calendar);
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetChildren ()
		{
			CreateAsyncTest (calendar,
			() => {
			List<AutomationPeer> buttonChildren = GetButtonChildren ();
			AutomationPeer peer = buttonChildren[0];
			List<AutomationPeer> children = peer.GetChildren ();
			Assert.IsNotNull (children, "GetChildren");
			Assert.AreEqual (1, children.Count, "GetChildren.Count");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetChildrenChanged ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			ContentControl control = null;

			AutomationPeer peer = null;
			List<AutomationPeer> children = null;
			Button button = null;
			AutomationEventTuple tuple = null;
			object oldContent = null;
			AutomationPeer oldChild = null;

			CreateAsyncTest (calendar,
			() => {
				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				control = ((FrameworkElementAutomationPeer) buttonChildren [0]).Owner as ContentControl;
				oldContent = control.Content;

				peer = FrameworkElementAutomationPeer.CreatePeerForElement (control);
				Assert.IsNotNull (peer, "Peer");
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #0");
				Assert.AreEqual (1, children.Count, "GetChildren #1");
				oldChild = children[0];
			},
			() => {
				button = new Button () { Content = "Button0" };
				control.Content = button;
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #1");
				Assert.AreEqual (1, children.Count, "Children.Count #0");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (button),
					children[0], "GetChildren #2");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #0");
				EventsManager.Instance.Reset ();
			},
			() => control.Content = null,
			() => {
				Assert.IsNull (peer.GetChildren (), "GetChildren #3");
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
		[Asynchronous]
		public override void IsOffScreen ()
		{
			FrameworkElement fe = null;
			AutomationPeer peer = null;
	
			CreateAsyncTest (calendar,
			() => {
				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				fe = ((FrameworkElementAutomationPeer) buttonChildren [0]).Owner as FrameworkElement;
			},
			() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				Assert.IsFalse (peer.IsOffscreen (), "IsOffScreen #1");
			}, 
			() => fe.Visibility = Visibility.Collapsed,
			() => Assert.IsTrue (peer.IsOffscreen (), "IsOffScreen #2"),
			() => fe.Visibility = Visibility.Visible,
			() => Assert.IsFalse (peer.IsOffscreen (), "IsOffScreen #3"),
			// If we are Visible but our Parent is not, we should be offscreen too
			() => calendar.Visibility = Visibility.Collapsed,
			() => Assert.IsTrue (peer.IsOffscreen (), "IsOffScreen #4"),
			() => fe.Visibility = Visibility.Collapsed,
			() => Assert.IsTrue (peer.IsOffscreen (), "IsOffScreen #5"),
			() => calendar.Visibility = Visibility.Visible,
			() => Assert.IsTrue (peer.IsOffscreen (), "IsOffScreen #6"),
			() => fe.Visibility = Visibility.Visible,
			() => Assert.IsFalse (peer.IsOffscreen (), "IsOffScreen #7")
			);
		}

		[TestMethod]
		[Asynchronous]
		public override void IsOffScreen_ScrollViewer ()
		{
			Control control = new CalendarDayButton ();
			ScrollViewer scrollViewer = new ScrollViewer () { Height = 100 };
			StackPanel panel = new StackPanel ();
			scrollViewer.Content = panel;
			AutomationPeer peer = null;

			CreateAsyncTest (scrollViewer,
			() => {
				for (int i = 0; i < 30; i++)
					panel.Children.Add (new TextBlock () { Text = i.ToString () });
				// Our control won't be visible, but still won't be offscreen
				panel.Children.Add (control);
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (control);
				Assert.IsNotNull (peer, "#0");
			},
			() => Assert.IsFalse (peer.IsOffscreen (), "IsOffScreen #1"),
			() => control.Visibility = Visibility.Collapsed,
			() => Assert.IsTrue (peer.IsOffscreen (), "IsOffScreen #2"),
			() => control.Visibility = Visibility.Visible,
			() => Assert.IsFalse (peer.IsOffscreen (), "IsOffScreen #3")
			);
		}

		[TestMethod]
		[Asynchronous]
		public override void GetClassName ()
		{
			CreateAsyncTest (calendar,
			() => {
			List<AutomationPeer> buttonChildren = GetButtonChildren ();
			AutomationPeer peer = buttonChildren [0];
			Assert.AreEqual ("CalendarDayButton", peer.GetClassName (), "GetClassName");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetHelpText ()
		{
			CreateAsyncTest (calendar,
			() => {
			List<AutomationPeer> buttonChildren = GetButtonChildren ();
			AutomationPeer peer = buttonChildren[0];
			Button button = (Button) ((FrameworkElementAutomationPeer) peer).Owner; 
			Assert.AreEqual (GetCurrentDateFormat (button), peer.GetHelpText (), "GetHelpText");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetHelpText_AttachedProperty ()
		{
			CreateAsyncTest (calendar,
			() => {
			List<AutomationPeer> buttonChildren = GetButtonChildren ();
			FrameworkElement fe = ((FrameworkElementAutomationPeer) buttonChildren[0]).Owner as FrameworkElement;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			Button button = (Button) ((FrameworkElementAutomationPeer) peer).Owner; 
			string currentDate = GetCurrentDateFormat (button);

			Assert.AreEqual (currentDate, peer.GetHelpText (), "GetHelpText");

			string helpText = "My Help Text property";

			fe.SetValue (AutomationProperties.HelpTextProperty, helpText);
			Assert.AreEqual (currentDate, peer.GetHelpText (), "GetHelpText #1");

			fe.SetValue (AutomationProperties.HelpTextProperty, null);
			Assert.AreEqual (currentDate, peer.GetHelpText (), "GetHelpText #2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetLocalizedControlType ()
		{
			CreateAsyncTest (calendar,
			() => {
			List<AutomationPeer> buttonChildren = GetButtonChildren ();
			AutomationPeer peer = buttonChildren [0];
			Assert.AreEqual ("day button", peer.GetLocalizedControlType (), "GetLocalizedControlType");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetName ()
		{
			CreateAsyncTest (calendar,
			() => {
			List<AutomationPeer> buttonChildren = GetButtonChildren ();
			AutomationPeer peer = buttonChildren [0];
			Assert.AreEqual (CURRENT_DAY, peer.GetName (), "GetName");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetName_AttachedProperty0 ()
		{
			CreateAsyncTest (calendar,
			() => {
			List<AutomationPeer> buttonChildren = GetButtonChildren ();
			FrameworkElement fe = ((FrameworkElementAutomationPeer) buttonChildren [0]).Owner as FrameworkElement;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

			Assert.AreEqual (CURRENT_DAY, peer.GetName (), "GetName");

			string name = "Attached Name";

			fe.SetValue (AutomationProperties.NameProperty, name);
			Assert.AreEqual (name, peer.GetName (), "GetName #1");

			fe.SetValue (AutomationProperties.NameProperty, null);
			Assert.AreEqual (CURRENT_DAY, peer.GetName (), "GetName #2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetName_AttachedProperty0Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			CreateAsyncTest (calendar,
			() => {
			List<AutomationPeer> buttonChildren = GetButtonChildren ();
			FrameworkElement fe = ((FrameworkElementAutomationPeer) buttonChildren [0]).Owner as FrameworkElement;
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
			Assert.AreEqual (CURRENT_DAY, tuple.OldValue, "#3");

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
			Assert.AreEqual (CURRENT_DAY, (string) tuple.NewValue, "#8");
			Assert.AreEqual ("Name", (string) tuple.OldValue, "#9");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetName_AttachedProperty1 ()
		{
			CreateAsyncTest (calendar,
			() => {
			List<AutomationPeer> buttonChildren = GetButtonChildren ();
			FrameworkElement element = ((FrameworkElementAutomationPeer) buttonChildren [0]).Owner as FrameworkElement;
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
			Assert.AreEqual (CURRENT_DAY, peer.GetName (), "GetName #5");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetName_AttachedProperty1Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			CreateAsyncTest (calendar,
			() => {
			List<AutomationPeer> buttonChildren = GetButtonChildren ();
			FrameworkElement fe = ((FrameworkElementAutomationPeer) buttonChildren [0]).Owner as FrameworkElement;
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
			Assert.AreEqual (CURRENT_DAY, (string) tuple.OldValue, "#3");

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
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetParentTest ()
		{
			CreateAsyncTest (calendar,
			() => {
				AutomationPeer parent = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				AutomationPeer button = buttonChildren [0];
				Assert.AreEqual (parent, button.GetParent (), "GetParent");

				TestPanel.Children.Remove (calendar);
			});
		}

		[TestMethod]
		public override void Null ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				new CalendarDayButtonAutomationPeer (null);
			});
		}

		// Switch months on the calendar and make sure that they day
		// buttons are properly sending property changed events for
		// NameProperty.
		[TestMethod]
		[Asynchronous]
		public void NameProperty_Events ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			DateTime date = new DateTime (2010, 2, 17);
			calendar.DisplayDate = date;
			calendar.SelectedDate = null;

			AutomationPeer prev = null;
			AutomationPeer calendarAutomationPeer = null;
			AutomationPeer month = null;
			AutomationPeer firstSunday = null;

			CreateAsyncTest (calendar,
			() => {
				calendarAutomationPeer
					= FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
				Assert.IsNotNull (calendarAutomationPeer, "calendarAutomationPeer != null");

				var children = calendarAutomationPeer.GetChildren ();
				Assert.IsTrue (children.Count > 10, "Children count");

				prev = children[0];
				month = children[1];
				firstSunday = children[10];

				Assert.IsNotNull (prev, "prev != null");
				Assert.IsNotNull (month, "mnth != null");
				Assert.IsNotNull (firstSunday, "firstSunday != null");
			},
			() => {
				Assert.AreEqual (date.ToString ("y"),
				                 month.GetName (),
				                 "February, 2010 == month");

				Assert.AreEqual (new DateTime (2010, 1, 31).Day.ToString (),
				                 firstSunday.GetName (),
				                 "2010-1-31 == firstSunday");
			},
			() => {
				EventsManager.Instance.Reset ();

				var prevInvoke = prev.GetPattern (
					PatternInterface.Invoke) as IInvokeProvider;
				prevInvoke.Invoke ();

				Assert.AreEqual (new DateTime (2010, 1, 17).ToString ("Y"),
				                 month.GetName (),
				                 "January, 2010 == month");

				Assert.AreEqual (new DateTime (2010, 12, 27).Day.ToString (),
				                 firstSunday.GetName (),
				                 "2009-12-27 == firstSunday");

				var events = EventsManager.Instance.GetAutomationEventFrom (
					firstSunday,
					AutomationElementIdentifiers.NameProperty
				);

				Assert.IsNotNull (events, "events != null");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void TestHasKeyboardFocusAfterPattern ()
		{
			AutomationPeer peer = null;
			IInvokeProvider provider = null;

			CreateAsyncTest (calendar,
			() => {
				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				peer = buttonChildren [0];
				provider = (IInvokeProvider) peer.GetPattern (PatternInterface.Invoke);
				Assert.IsNotNull (provider, "#0");
			},
			() => provider.Invoke (),
			() => Assert.IsFalse (peer.HasKeyboardFocus (), "#1"));
		}

		#region ISelectionItemProvider Tests

		[TestMethod]
		[Asynchronous]
		public void ISelectionItemProvider_Methods ()
		{
			AutomationPeer calendarAutomationPeer = null;
			AutomationPeer peer = null;
			ISelectionItemProvider selectionItem = null;
			ISelectionItemProvider selectionItem2 = null;
			DateTime date = new DateTime (2000, 2, 2);
			List<AutomationPeer> buttonChildren = null;

			calendar.DisplayDate = date;
			calendar.SelectedDate = date;

			CreateAsyncTest (calendar,
			() => {
				calendarAutomationPeer = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
				Assert.IsNotNull (calendarAutomationPeer, "#0");

				buttonChildren = GetButtonChildren ();
				Assert.IsNotNull (buttonChildren.Count, "#1");
				Assert.AreEqual (42, buttonChildren.Count, "#2");
			},
			() => { calendar.SelectedDate = date; },
			() => {
				peer = (from c in buttonChildren
				        where c.GetHelpText () == GetCurrentDateFormat (null, new DateTime (2000, 02, 02)) // "Wednesday, February 02, 2000" 
				        select c).FirstOrDefault ();
				Assert.IsNotNull (peer, "#3");
			},
			() => {
				selectionItem = (ISelectionItemProvider) peer.GetPattern (PatternInterface.SelectionItem);
				Assert.IsNotNull (selectionItem, "#4");

				Assert.IsTrue (selectionItem.IsSelected, "#5");
				Assert.AreEqual (calendarAutomationPeer,
					new PeerFromProvider ().GetPeerFromProvider (selectionItem.SelectionContainer), "#6");
			},
			() => selectionItem.RemoveFromSelection (),
			() => {
				Assert.IsFalse (selectionItem.IsSelected, "#7");
				selectionItem.AddToSelection ();
			},
			() => Assert.IsTrue (selectionItem.IsSelected, "#8"),
			// Check selection in SingleDate mode
			() => calendar.SelectionMode = CalendarSelectionMode.SingleDate,
			() => {
				AutomationPeer nextPeer = buttonChildren [buttonChildren.IndexOf (peer) + 1];
				Assert.IsNotNull (nextPeer, "#9");
				selectionItem2 = (ISelectionItemProvider) nextPeer.GetPattern (PatternInterface.SelectionItem);
				Assert.IsNotNull (selectionItem2, "#10");
				Assert.IsFalse (selectionItem2.IsSelected, "#11");
			},
			() => selectionItem2.AddToSelection (),
			() => {
				Assert.IsTrue (selectionItem2.IsSelected, "#12");
				Assert.IsFalse (selectionItem.IsSelected, "#13");
			},
			// Check blackout day
			() => {
				selectionItem.RemoveFromSelection();
				calendar.BlackoutDates.Add (new CalendarDateRange (date));
			},
			() => {
				selectionItem.AddToSelection ();
				Assert.IsFalse (selectionItem.IsSelected, "#14");
			},
			// Check selection in None mode
			() => calendar.SelectionMode = CalendarSelectionMode.None,
			() => {
				Assert.IsFalse (selectionItem2.IsSelected, "#15");
				selectionItem2.AddToSelection ();
			},
			() => Assert.IsFalse (selectionItem2.IsSelected, "#16"),
			// Check selection in MultiRange mode
			() => calendar.BlackoutDates.Clear (),
			() => {
				calendar.SelectionMode = CalendarSelectionMode.MultipleRange;
				Assert.IsFalse (selectionItem.IsSelected, "#17");
				Assert.IsFalse (selectionItem2.IsSelected, "#18");
			},
			() => selectionItem.AddToSelection (),
			() => selectionItem2.AddToSelection (),
			() => {
				Assert.IsTrue (selectionItem.IsSelected, "#19");
				Assert.IsTrue (selectionItem2.IsSelected, "#20");
				selectionItem2.RemoveFromSelection ();
			},
			() => {
				Assert.IsTrue (selectionItem.IsSelected, "#21");
				Assert.IsFalse (selectionItem2.IsSelected, "#22");
			},
			() => selectionItem.RemoveFromSelection (),
			() => {
				Assert.IsFalse (selectionItem.IsSelected, "#23");
			},
			() => TestPanel.Children.Remove (calendar));
		}

		[TestMethod]
		[Asynchronous]
		public void ISelectionItemProvider_Events ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			AutomationPeer calendarAutomationPeer = null;
			AutomationPeer peer = null;
			AutomationPeer nextPeer = null;
			AutomationEventTuple tuple = null;
			ISelectionItemProvider selectionItem = null;
			ISelectionItemProvider selectionItem2 = null;
			DateTime date = new DateTime (2000, 2, 2);

			calendar.DisplayDate = date;
			calendar.SelectedDate = date;

			List<AutomationPeer> buttonChildren = null;

			CreateAsyncTest (calendar,
			() => {
				calendarAutomationPeer = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
				Assert.IsNotNull (calendarAutomationPeer, "#0");

				buttonChildren = GetButtonChildren ();
				Assert.IsNotNull (buttonChildren.Count, "#1");
				Assert.AreEqual (42, buttonChildren.Count, "#2");
			},
			() => { calendar.SelectedDate = date; },
			() => {
				peer = (from c in buttonChildren
				        where c.GetHelpText () == GetCurrentDateFormat (null, new DateTime (2000, 02, 02)) // "Wednesday, February 02, 2000"
				        select c).FirstOrDefault ();
				Assert.IsNotNull (peer, "#3");
			},
			() => {
				selectionItem = (ISelectionItemProvider) peer.GetPattern (PatternInterface.SelectionItem);
				Assert.IsNotNull (selectionItem, "#4");

				Assert.IsTrue (selectionItem.IsSelected, "#5");
				Assert.AreEqual (calendarAutomationPeer,
					new PeerFromProvider ().GetPeerFromProvider (selectionItem.SelectionContainer), "#6");
			},
			() => {
				EventsManager.Instance.Reset ();
				selectionItem.RemoveFromSelection ();
			},
			() => {
				Assert.IsFalse (selectionItem.IsSelected, "#7");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       AutomationEvents.SelectionItemPatternOnElementRemovedFromSelection);
				Assert.IsNotNull (tuple, "Tuple #0");

				EventsManager.Instance.Reset ();
				selectionItem.AddToSelection ();
			},
			() => {
				Assert.IsTrue (selectionItem.IsSelected, "#8");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNotNull (tuple, "Tuple #1");
			},
			// Check selection in SingleDate mode
			() => calendar.SelectionMode = CalendarSelectionMode.SingleDate,
			() => {
				nextPeer = buttonChildren [buttonChildren.IndexOf (peer) + 1];
				Assert.IsNotNull (nextPeer, "#9");
				selectionItem2 = (ISelectionItemProvider) nextPeer.GetPattern (PatternInterface.SelectionItem);
				Assert.IsNotNull (selectionItem2, "#10");
				Assert.IsFalse (selectionItem2.IsSelected, "#11");

				EventsManager.Instance.Reset ();
				selectionItem2.AddToSelection ();
			},
			() => {
				Assert.IsTrue (selectionItem2.IsSelected, "#12");
				Assert.IsFalse (selectionItem.IsSelected, "#13");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       AutomationEvents.SelectionItemPatternOnElementRemovedFromSelection);
				Assert.IsNull (tuple, "Tuple #2");

				tuple = EventsManager.Instance.GetAutomationEventFrom (nextPeer,
				                                                       AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNotNull (tuple, "Tuple #3");
			},
			// Check blackout day
			() => {
				EventsManager.Instance.Reset ();
				selectionItem.RemoveFromSelection();
				calendar.BlackoutDates.Add (new CalendarDateRange (date));
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       AutomationEvents.SelectionItemPatternOnElementRemovedFromSelection);
				Assert.IsNull (tuple, "Tuple #4");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       AutomationEvents.SelectionItemPatternOnElementAddedToSelection);
				Assert.IsNull (tuple, "Tuple #5");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNull (tuple, "Tuple #6");

				EventsManager.Instance.Reset ();
				selectionItem.AddToSelection ();
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       AutomationEvents.SelectionItemPatternOnElementRemovedFromSelection);
				Assert.IsNull (tuple, "Tuple #7");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       AutomationEvents.SelectionItemPatternOnElementAddedToSelection);
				Assert.IsNull (tuple, "Tuple #8");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNull (tuple, "Tuple #9");
				Assert.IsFalse (selectionItem.IsSelected, "#14");
			},
			// Check selection in None mode
			() => {
				EventsManager.Instance.Reset ();
				calendar.SelectionMode = CalendarSelectionMode.None;
			},
			() => {
				Assert.IsFalse (selectionItem2.IsSelected, "#15");
				EventsManager.Instance.Reset ();
				selectionItem2.AddToSelection ();
			},
			() => {
				Assert.IsFalse (selectionItem2.IsSelected, "#16");

				tuple = EventsManager.Instance.GetAutomationEventFrom (nextPeer,
				                                                       AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNull (tuple, "Tuple #10");
			},
			// Check selection in MultiRange mode
			() => calendar.BlackoutDates.Clear (),
			() => {
				calendar.SelectionMode = CalendarSelectionMode.MultipleRange;
				Assert.IsFalse (selectionItem.IsSelected, "#17");
				Assert.IsFalse (selectionItem2.IsSelected, "#18");
			},
			() => {
				EventsManager.Instance.Reset ();
				selectionItem.AddToSelection ();
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNotNull (tuple, "Tuple #11");
			},
			() => {
				EventsManager.Instance.Reset ();
				selectionItem2.AddToSelection ();
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (nextPeer,
				                                                       AutomationEvents.SelectionItemPatternOnElementAddedToSelection);
				Assert.IsNotNull (tuple, "Tuple #12");
			},
			() => {
				Assert.IsTrue (selectionItem.IsSelected, "#19");
				Assert.IsTrue (selectionItem2.IsSelected, "#20");
				EventsManager.Instance.Reset ();
				selectionItem2.RemoveFromSelection ();
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
				                                                       AutomationEvents.SelectionItemPatternOnElementSelected);
				Assert.IsNotNull (tuple, "Tuple #13");
			},
			() => {
				Assert.IsTrue (selectionItem.IsSelected, "#21");
				Assert.IsFalse (selectionItem2.IsSelected, "#22");
			},
			() => TestPanel.Children.Remove (calendar));
		}

		#endregion

		#region IInvokeProvider Tests

		[TestMethod]
		[Asynchronous]
		public void IInvokeProvider_Methods ()
		{
			AutomationPeer calendarAutomationPeer = null;
			AutomationPeer february = null;
			AutomationPeer january = null;
			DateTime date = new DateTime (2000, 2, 2);

			IInvokeProvider invokeFeb = null;
			IInvokeProvider invokeJan = null;

			CreateAsyncTest (calendar,
			() => calendar.SelectedDate = date,
			() => calendar.DisplayDate = date,
			() => {
				calendarAutomationPeer = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
				Assert.IsNotNull (calendarAutomationPeer, "#0");

				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				Assert.IsNotNull (buttonChildren.Count, "#1");
				Assert.AreEqual (42, buttonChildren.Count, "#2");
			},
			() => Assert.AreEqual (2, calendar.DisplayDate.Month, "#3"),
			() => {
				january = (from c in calendarAutomationPeer.GetChildren ()
				           where c.GetHelpText () == GetCurrentDateFormat (null, new DateTime (2000, 01, 31)) //"Monday, January 31, 2000"
				           select c).FirstOrDefault ();
				Assert.IsNotNull (january, "#4");

				invokeJan = january.GetPattern (PatternInterface.Invoke) as IInvokeProvider;
				Assert.IsNotNull (invokeJan, "#5");
			},
			() => invokeJan.Invoke (),
			() => Assert.AreEqual (1, calendar.SelectedDate.Value.Month, "#6"),
			() => {
				february = (from c in calendarAutomationPeer.GetChildren ()
				            where c.GetHelpText () == GetCurrentDateFormat (null, new DateTime (2000, 02, 02)) // "Wednesday, February 02, 2000"
				            select c).FirstOrDefault ();
				Assert.IsNotNull (february, "#7");

				invokeFeb = february.GetPattern (PatternInterface.Invoke) as IInvokeProvider;
				Assert.IsNotNull (invokeFeb, "#8");
			},
			() => invokeFeb.Invoke (),
			() => Assert.AreEqual (2, calendar.SelectedDate.Value.Month, "#9"),
			() => TestPanel.Children.Remove (calendar));
		}

		[TestMethod]
		[Asynchronous]
		public override void IInvokeProvider_Invoke ()
		{
			CreateAsyncTest (calendar,
			() => {
				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				Button button = ((FrameworkElementAutomationPeer) buttonChildren [0]).Owner as Button;
				Test_InvokeProvider_Invoke (button);
			});
		}

		[TestMethod]
		public override void InvokeProvider_Events ()
		{
			// Calling Invoke in CalendaryDayButton doesn't raise Invoked event.
		}

		#endregion

		#region ITableItemProvider Tests

		// NOTE: No TableItemProvider UIA events raised by CalendarDayButtonAutomationPeer

		[TestMethod]
		[Asynchronous]
		public void ITableItemProvider_Methods ()
		{
			AutomationPeer calendarAutomationPeer = null;
			AutomationPeer peer = null;
			DateTime date = new DateTime (2000, 2, 2);

			CreateAsyncTest (calendar,
			() => {
				calendarAutomationPeer = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
				Assert.IsNotNull (calendarAutomationPeer, "#0");

				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				Assert.IsNotNull (buttonChildren.Count, "#1");
				Assert.AreEqual (42, buttonChildren.Count, "#2");
			},
			() => { calendar.SelectedDate = date; },
			() => {
				peer = (from c in calendarAutomationPeer.GetChildren ()
				        where c.GetHelpText () == GetCurrentDateFormat (null, new DateTime (2000, 02, 02)) // "Wednesday, February 02, 2000"
				        select c).FirstOrDefault ();
				Assert.IsNotNull (peer, "#3");

				ITableItemProvider tableItem = (ITableItemProvider) peer.GetPattern (PatternInterface.TableItem);
				Assert.IsNotNull (tableItem, "#4");

				IRawElementProviderSimple[] headers = tableItem.GetColumnHeaderItems ();
				Assert.AreEqual (1, headers.Length);
				Assert.Equals ((((ITableProvider) calendarAutomationPeer).GetColumnHeaders ())[3], headers[0]);
				Assert.IsNull (tableItem.GetRowHeaderItems ());
			},
			() => TestPanel.Children.Remove (calendar));
		}

		#endregion

		#region IGridItemProvider Tests

		// NOTE: No GridItemProvider UIA events raised by CalendarDayButtonAutomationPeer

		[TestMethod]
		[Asynchronous]
		public void IGridItemProvider_Methods ()
		{
			AutomationPeer calendarAutomationPeer = null;
			DateTime date = new DateTime (2000, 2, 2);

			CreateAsyncTest (calendar,
			() => {
				calendarAutomationPeer = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
				Assert.IsNotNull (calendarAutomationPeer, "#0");

				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				Assert.IsNotNull (buttonChildren.Count, "#1");
				Assert.AreEqual (42, buttonChildren.Count, "#2");
			},
			() => { calendar.SelectedDate = date; },
			() => {
				int count = 0;
				foreach (AutomationPeer peer in (from c in calendarAutomationPeer.GetChildren ()
				                                 where c.GetType () == typeof (CalendarDayButtonAutomationPeer)
				                                 select c)) {
					FrameworkElementAutomationPeer feap = peer as FrameworkElementAutomationPeer;
					Assert.IsNotNull ("#3");

					IGridItemProvider gridItem = (IGridItemProvider) peer.GetPattern (PatternInterface.GridItem);
					Assert.IsNotNull (gridItem, "#4");

					Assert.AreEqual (feap.Owner.GetValue (Grid.ColumnProperty), gridItem.Column);
					Assert.AreEqual ((int) feap.Owner.GetValue (Grid.RowProperty) - 1, gridItem.Row);
					Assert.AreEqual (1, gridItem.ColumnSpan);
					Assert.AreEqual (1, gridItem.RowSpan);
					Assert.AreEqual (calendarAutomationPeer, new PeerFromProvider ().GetPeerFromProvider (gridItem.ContainingGrid));

					count++;
				}
				Assert.AreEqual (42, count, "#5");
			},
			() => TestPanel.Children.Remove (calendar));
		}

		#endregion

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new CalendarDayButton ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			// Can't subclass CalendarAutomationPeer
			return null;
		}

		private List<AutomationPeer> GetButtonChildren ()
		{
			AutomationPeer calendarPeer = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
			return (from peer in calendarPeer.GetChildren ()
			        where peer.GetType () == typeof (CalendarDayButtonAutomationPeer)
			        select peer).ToList<AutomationPeer> ();
		}

		// Based on Calendar/DateTimeHelper.cs
		private string GetCurrentDateFormat (Button button)
		{
			return GetCurrentDateFormat (button, null);
		}
		
		private string GetCurrentDateFormat (Button button, DateTime? dateTime)
		{
			SG.DateTimeFormatInfo dt = null;
			if (SG.CultureInfo.CurrentCulture.Calendar is SG.GregorianCalendar)
				dt = SG.CultureInfo.CurrentCulture.DateTimeFormat;
			else {
				foreach (SG.Calendar cal in SG.CultureInfo.CurrentCulture.OptionalCalendars) {
					if (cal is SG.GregorianCalendar) {
						//if the default calendar is not Gregorian, return the first supported GregorianCalendar dtfi
						dt = new SG.CultureInfo(SG.CultureInfo.CurrentCulture.Name).DateTimeFormat;
						dt.Calendar = cal;
						break;
					}
				}
				//if there are no GregorianCalendars in the OptionalCalendars list, use the invariant dtfi
				dt = new SG.CultureInfo(SG.CultureInfo.InvariantCulture.Name).DateTimeFormat;
				dt.Calendar = new SG.GregorianCalendar();
			}

			DateTime dataContext = dateTime.HasValue ? dateTime.Value : (DateTime) button.DataContext;
			return dataContext.Date.ToString (dt.LongDatePattern, dt);
		}

		//private const string CURRENT_DATE = "Sunday, January 30, 2000";
		private const string CURRENT_DAY = "30";

		private Calendar calendar;
	}
}
