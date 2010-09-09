//
// Unit tests for CalendarButtonAutomationPeerTest
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
using System.Linq;
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
	public class CalendarButtonAutomationPeerTest : ButtonAutomationPeerTest {

		[TestInitialize]
		public void TestInitialize ()
		{
			calendar = new Calendar ();
			calendar.Height = 200;
			calendar.Width = 200;
			calendar.DisplayMode = CalendarMode.Year;
			DateTime date = new DateTime (2000, 2, 2);
			calendar.DisplayDate = date;
			calendar.SelectedDate = date;
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
			List<AutomationPeer> buttonChildren = null;

			CreateAsyncTest (calendar,
			() => {
				buttonChildren = GetButtonChildren ();
				control = ((FrameworkElementAutomationPeer) buttonChildren [0]).Owner as ContentControl;
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (control);
				oldContent = control.Content;

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


		[TestCleanup]
		public void TestCleanup ()
		{
			TestPanel.Children.Remove (calendar);
		}

		[TestMethod]
		[Asynchronous]
		public override void IsKeyboardFocusable ()
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
			TestIsNotKeyboardFocusableEvent (new CalendarButton ());
		}

		[TestMethod]
		[Asynchronous]
		public override void TestHasKeyboardFocusAfterPattern ()
		{
			IInvokeProvider provider = null;
			AutomationPeer peer = null;

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

		[TestMethod]
		[Asynchronous]
		public override void GetPattern ()
		{
			CreateAsyncTest (calendar,
			() => {
				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				CalendarButtonAutomationPeer cbap = buttonChildren [0] as CalendarButtonAutomationPeer;
				Assert.IsNotNull (cbap, "#0");

				Assert.IsNull (cbap.GetPattern (PatternInterface.Dock), "Dock");
				Assert.IsNull (cbap.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
				Assert.IsNull (cbap.GetPattern (PatternInterface.Grid), "Grid");
				Assert.IsNull (cbap.GetPattern (PatternInterface.MultipleView), "MultipleView");
				Assert.IsNull (cbap.GetPattern (PatternInterface.RangeValue), "RangeValue");
				Assert.IsNull (cbap.GetPattern (PatternInterface.Scroll), "Scroll");
				Assert.IsNull (cbap.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
				Assert.IsNull (cbap.GetPattern (PatternInterface.Selection), "Selection");
				Assert.IsNull (cbap.GetPattern (PatternInterface.Table), "Table");
				Assert.IsNull (cbap.GetPattern (PatternInterface.TableItem), "TableItem");
				Assert.IsNull (cbap.GetPattern (PatternInterface.Toggle), "Toggle");
				Assert.IsNull (cbap.GetPattern (PatternInterface.Transform), "Transform");
				Assert.IsNull (cbap.GetPattern (PatternInterface.Value), "Value");
				Assert.IsNull (cbap.GetPattern (PatternInterface.Window), "Window");

				Assert.IsNotNull (cbap.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
				Assert.IsNotNull (cbap.GetPattern (PatternInterface.GridItem), "GridItem");
				Assert.IsNotNull (cbap.GetPattern (PatternInterface.Invoke), "Invoke");
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
				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				// StackPanel and two TextBlocks
				stackPanel = new StackPanel ();
				stackPanel.Children.Add (new TextBlock () { Text = "Text0" });
				stackPanel.Children.Add (new TextBlock () { Text = "Text1" });
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

				
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetClassName ()
		{
			CreateAsyncTest (calendar,
			() => {
				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				AutomationPeer peer = buttonChildren[0];
				Assert.AreEqual ("CalendarButton", peer.GetClassName (), "GetClassName");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetLocalizedControlType ()
		{
			CreateAsyncTest (calendar,
			() => {
				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				AutomationPeer peer = buttonChildren[0];
				Assert.AreEqual ("calendar button", peer.GetLocalizedControlType (), "GetLocalizedControlType");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetName ()
		{
			CreateAsyncTest (calendar,
			() => {
				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				AutomationPeer peer = buttonChildren[0];
				Assert.AreEqual (CURRENT_MONTH, peer.GetName (), "GetName");
			});
		}

		[TestMethod]
		[Asynchronous]
		public override void GetName_AttachedProperty0 ()
		{
			CreateAsyncTest (calendar,
			() => {
				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				FrameworkElement fe = ((FrameworkElementAutomationPeer) buttonChildren[0]).Owner as FrameworkElement;
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

				Assert.AreEqual (CURRENT_MONTH, peer.GetName (), "GetName");

				string name = "Attached Name";

				fe.SetValue (AutomationProperties.NameProperty, name);
				Assert.AreEqual (name, peer.GetName (), "GetName #1");

				fe.SetValue (AutomationProperties.NameProperty, null);
				Assert.AreEqual (CURRENT_MONTH, peer.GetName (), "GetName #2");
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
				Assert.AreEqual (CURRENT_MONTH, tuple.OldValue, "#3");

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
				Assert.AreEqual (CURRENT_MONTH, (string) tuple.NewValue, "#8");
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
				FrameworkElement element = ((FrameworkElementAutomationPeer) buttonChildren[0]).Owner as FrameworkElement;
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
				Assert.AreEqual (CURRENT_MONTH, peer.GetName (), "GetName #5");
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
				Assert.AreEqual (CURRENT_MONTH, (string) tuple.OldValue, "#3");

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
				List<AutomationPeer> buttonChildren = GetButtonChildren ();
				AutomationPeer parent = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
				AutomationPeer button = buttonChildren [0];
				Assert.AreEqual (parent, button.GetParent (), "GetParent");

				TestPanel.Children.Remove (calendar);
			});
		}

		[TestMethod]
		public override void Null ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				new CalendarButtonAutomationPeer (null);
			});
		}

		#region ISelectionItemProvider Tests

		// NOTE: No SelectionItemProvider UIA events raised by CalendarButtonAutomationPeer

		[TestMethod]
		[Asynchronous]
		public void ISelectionItemProvider_Methods ()
		{
			AutomationPeer calendarAutomationPeer = null;
			AutomationPeer peer = null;
			ISelectionItemProvider selectionItem = null;
			ISelectionItemProvider selectionItem2 = null;
			DateTime date = new DateTime (2000, 2, 1);

			calendar.DisplayMode = CalendarMode.Year;
			calendar.DisplayDate = date;
			calendar.SelectedDate = null;
			List<AutomationPeer> buttonChildren = null;

			CreateAsyncTest (calendar,
			() => {
				calendarAutomationPeer = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
				Assert.IsNotNull (calendarAutomationPeer, "#0");

				buttonChildren = GetButtonChildren ();

				Assert.IsNotNull (buttonChildren.Count, "#1");
				Assert.AreEqual (12, buttonChildren.Count, "#2");
			},
			() => {
				peer = (from c in buttonChildren
				        where c.GetName () == "Jan" // DateTime (2000, 2, 1);
				        select c).FirstOrDefault ();
				Assert.IsNotNull (peer, "#3");
			},
			() => {
				selectionItem = (ISelectionItemProvider) peer.GetPattern (PatternInterface.SelectionItem);
				Assert.IsNotNull (selectionItem, "#4");

				Assert.IsFalse (selectionItem.IsSelected, "#5");
				Assert.AreEqual (calendarAutomationPeer,
					new PeerFromProvider ().GetPeerFromProvider (selectionItem.SelectionContainer), "#6");
			},
			() => selectionItem.AddToSelection (),
			() => {
				Assert.IsFalse (selectionItem.IsSelected, "#6");
				selectionItem.Select ();
			},
			() => {
				Assert.IsTrue (selectionItem.IsSelected, "#7");
				AutomationPeer nextPeer = buttonChildren[buttonChildren.IndexOf (peer) + 1];
				Assert.IsNotNull (nextPeer, "#8");
				selectionItem2 = (ISelectionItemProvider) nextPeer.GetPattern (PatternInterface.SelectionItem);
				Assert.IsNotNull (selectionItem2, "#9");
				Assert.IsFalse (selectionItem2.IsSelected, "#10");
			},
			() => selectionItem2.AddToSelection (),
			() => {
				Assert.IsFalse (selectionItem2.IsSelected, "#11");
				Assert.IsTrue (selectionItem.IsSelected, "#12");
			},
			() => selectionItem2.Select (),
			() => {
				Assert.IsTrue (selectionItem2.IsSelected, "#13");
				Assert.IsFalse (selectionItem.IsSelected, "#14");
			},
			// no changes
			() => selectionItem2.RemoveFromSelection (),
			() => {
				Assert.IsTrue (selectionItem2.IsSelected, "#13");
				Assert.IsFalse (selectionItem.IsSelected, "#14");
			},
			() => TestPanel.Children.Remove (calendar));
		}

		#endregion

		#region IInvokeProvider Tests

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

		#region IGridItemProvider Tests

		// NOTE: No GridItemProvider UIA events raised by CalendarButtonAutomationPeer

		[TestMethod]
		[Asynchronous]
		public void IGridItemProvider_Methods ()
		{
		    AutomationPeer calendarAutomationPeer = null;
		    DateTime date = new DateTime (2000, 2, 2);
			List<AutomationPeer> buttonChildren = null;

		    CreateAsyncTest (calendar,
		    () => {
		        calendarAutomationPeer = FrameworkElementAutomationPeer.CreatePeerForElement (calendar);
		        Assert.IsNotNull (calendarAutomationPeer, "#0");

				buttonChildren = GetButtonChildren ();

		        Assert.IsNotNull (buttonChildren.Count, "#1");
		        Assert.AreEqual (12, buttonChildren.Count, "#2");
		    },
		    () => { calendar.SelectedDate = date; },
		    () => {
		        int count = 0;
		        foreach (AutomationPeer peer in (from c in calendarAutomationPeer.GetChildren ()
		                                         where c.GetType () == typeof (CalendarButtonAutomationPeer)
		                                         select c)) {
		            FrameworkElementAutomationPeer feap = peer as FrameworkElementAutomationPeer;
		            Assert.IsNotNull ("#3");

		            IGridItemProvider gridItem = (IGridItemProvider) peer.GetPattern (PatternInterface.GridItem);
		            Assert.IsNotNull (gridItem, "#4");

		            Assert.AreEqual (feap.Owner.GetValue (Grid.ColumnProperty), gridItem.Column, "#5");
		            Assert.AreEqual ((int) feap.Owner.GetValue (Grid.RowProperty), gridItem.Row, "#6");
		            Assert.AreEqual (1, gridItem.ColumnSpan, "#7");
		            Assert.AreEqual (1, gridItem.RowSpan, "#8");
		            Assert.AreEqual (calendarAutomationPeer, new PeerFromProvider ().GetPeerFromProvider (gridItem.ContainingGrid), "#9");

		            count++;
		        }
		        Assert.AreEqual (12, count, "#10");
		    });
		}

		#endregion

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new CalendarButton ();
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
			        where peer.GetType () == typeof (CalendarButtonAutomationPeer)
			        select peer).ToList<AutomationPeer> ();
		}

		private const string CURRENT_MONTH = "Jan";
		private Calendar calendar;
	}
}
