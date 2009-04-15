//
// Unit tests for AutomationControlType
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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
using System.Windows.Automation.Peers;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class AutomationControlTypeTest {

		[TestMethod]
		public void EnumerationValuesTest ()
		{
			Assert.AreEqual (0,  (int) AutomationControlType.Button, "AutomationControlType.Button");
			Assert.AreEqual (1,  (int) AutomationControlType.Calendar, "AutomationControlType.Calendar");
			Assert.AreEqual (2,  (int) AutomationControlType.CheckBox, "AutomationControlType.CheckBox");
			Assert.AreEqual (3,  (int) AutomationControlType.ComboBox, "AutomationControlType.ComboBox");
			Assert.AreEqual (4,  (int) AutomationControlType.Edit, "AutomationControlType.Edit");
			Assert.AreEqual (5,  (int) AutomationControlType.Hyperlink, "AutomationControlType.Hyperlink");
			Assert.AreEqual (6,  (int) AutomationControlType.Image, "AutomationControlType.Image");
			Assert.AreEqual (8,  (int) AutomationControlType.List, "AutomationControlType.List");
			Assert.AreEqual (7,  (int) AutomationControlType.ListItem, "AutomationControlType.ListItem");
			Assert.AreEqual (9,  (int) AutomationControlType.Menu, "AutomationControlType.Menu");
			Assert.AreEqual (10, (int) AutomationControlType.MenuBar, "AutomationControlType.MenuBar");
			Assert.AreEqual (11, (int) AutomationControlType.MenuItem, "AutomationControlType.MenuItem");
			Assert.AreEqual (12, (int) AutomationControlType.ProgressBar, "AutomationControlType.ProgressBar");
			Assert.AreEqual (13, (int) AutomationControlType.RadioButton, "AutomationControlType.RadioButton");
			Assert.AreEqual (14, (int) AutomationControlType.ScrollBar, "AutomationControlType.ScrollBar");
			Assert.AreEqual (15, (int) AutomationControlType.Slider, "AutomationControlType.Slider");
			Assert.AreEqual (16, (int) AutomationControlType.Spinner, "AutomationControlType.Spinner");
			Assert.AreEqual (17, (int) AutomationControlType.StatusBar, "AutomationControlType.StatusBar");
			Assert.AreEqual (18, (int) AutomationControlType.Tab, "AutomationControlType.Tab");
			Assert.AreEqual (19, (int) AutomationControlType.TabItem, "AutomationControlType.TabItem");
			Assert.AreEqual (20, (int) AutomationControlType.Text, "AutomationControlType.Text");
			Assert.AreEqual (21, (int) AutomationControlType.ToolBar, "AutomationControlType.ToolBar");
			Assert.AreEqual (22, (int) AutomationControlType.ToolTip, "AutomationControlType.ToolTip");
			Assert.AreEqual (23, (int) AutomationControlType.Tree, "AutomationControlType.Tree");
			Assert.AreEqual (24, (int) AutomationControlType.TreeItem, "AutomationControlType.TreeItem");
			Assert.AreEqual (25, (int) AutomationControlType.Custom, "AutomationControlType.Custom");
			Assert.AreEqual (26, (int) AutomationControlType.Group, "AutomationControlType.Group");
			Assert.AreEqual (27, (int) AutomationControlType.Thumb, "AutomationControlType.Thumb");
			Assert.AreEqual (28, (int) AutomationControlType.DataGrid, "AutomationControlType.DataGrid");
			Assert.AreEqual (29, (int) AutomationControlType.DataItem, "AutomationControlType.DataItem");
			Assert.AreEqual (30, (int) AutomationControlType.Document, "AutomationControlType.Document");
			Assert.AreEqual (31, (int) AutomationControlType.SplitButton, "AutomationControlType.SplitButton");
			Assert.AreEqual (32, (int) AutomationControlType.Window, "AutomationControlType.Window");
			Assert.AreEqual (33, (int) AutomationControlType.Pane, "AutomationControlType.Pane");
			Assert.AreEqual (34, (int) AutomationControlType.Header, "AutomationControlType.Header");
			Assert.AreEqual (35, (int) AutomationControlType.HeaderItem, "AutomationControlType.HeaderItem");
			Assert.AreEqual (36, (int) AutomationControlType.Table, "AutomationControlType.Table");
			Assert.AreEqual (37, (int) AutomationControlType.TitleBar, "AutomationControlType.TitleBar");
			Assert.AreEqual (38, (int) AutomationControlType.Separator, "AutomationControlType.Separator");
		}

		[TestMethod]
		public void LiteralValues ()
		{
			TestLiteralValue (AutomationControlType.Button, "button");
			TestLiteralValue (AutomationControlType.Calendar, "calendar");
			TestLiteralValue (AutomationControlType.CheckBox, "checkbox");
			TestLiteralValue (AutomationControlType.ComboBox, "combobox");
			TestLiteralValue (AutomationControlType.Edit, "edit");
			TestLiteralValue (AutomationControlType.Hyperlink, "hyperlink");
			TestLiteralValue (AutomationControlType.Image, "image");
			TestLiteralValue (AutomationControlType.ListItem, "listitem");
			TestLiteralValue (AutomationControlType.List, "list");
			TestLiteralValue (AutomationControlType.Menu, "menu");
			TestLiteralValue (AutomationControlType.MenuBar, "menubar");
			TestLiteralValue (AutomationControlType.MenuItem, "menuitem");
			TestLiteralValue (AutomationControlType.ProgressBar, "progressbar");
			TestLiteralValue (AutomationControlType.RadioButton, "radiobutton");
			TestLiteralValue (AutomationControlType.ScrollBar, "scrollbar");
			TestLiteralValue (AutomationControlType.Slider, "slider");
			TestLiteralValue (AutomationControlType.Spinner, "spinner");
			TestLiteralValue (AutomationControlType.StatusBar, "statusbar");
			TestLiteralValue (AutomationControlType.Tab, "tab");
			TestLiteralValue (AutomationControlType.Text, "text");
			TestLiteralValue (AutomationControlType.ToolBar, "toolbar");
			TestLiteralValue (AutomationControlType.ToolTip, "tooltip");
			TestLiteralValue (AutomationControlType.Tree, "tree");
			TestLiteralValue (AutomationControlType.TreeItem, "treeitem");
			TestLiteralValue (AutomationControlType.Custom, "custom");
			TestLiteralValue (AutomationControlType.Group, "group");
			TestLiteralValue (AutomationControlType.Thumb, "thumb");
			TestLiteralValue (AutomationControlType.DataGrid, "datagrid");
			TestLiteralValue (AutomationControlType.DataItem, "dataitem");
			TestLiteralValue (AutomationControlType.SplitButton, "splitbutton");
			TestLiteralValue (AutomationControlType.Window, "window");
			TestLiteralValue (AutomationControlType.Pane, "pane");
			TestLiteralValue (AutomationControlType.Header, "header");
			TestLiteralValue (AutomationControlType.HeaderItem, "headeritem");
			TestLiteralValue (AutomationControlType.Table, "table");
			TestLiteralValue (AutomationControlType.TitleBar, "titlebar");
			TestLiteralValue (AutomationControlType.Separator, "separator");
		}

		public class ConcreteFrameworkElement : FrameworkElement {
			
			protected override AutomationPeer OnCreateAutomationPeer ()
			{
				return new ConcreteFrameworkElementAutomationPeer (this);
			}
		}

		public class ConcreteFrameworkElementAutomationPeer : FrameworkElementAutomationPeer {

			public ConcreteFrameworkElementAutomationPeer (ConcreteFrameworkElement fe)
				: base (fe)
			{
			}

			protected override AutomationControlType GetAutomationControlTypeCore ()
			{
				return ControlType;
			}

			public AutomationControlType ControlType {
				private get;
				set;
			}
		}

		private void TestLiteralValue (AutomationControlType controlType, string localizedValue)
		{

			ConcreteFrameworkElement fe = new ConcreteFrameworkElement ();
			ConcreteFrameworkElementAutomationPeer peer 
				= FrameworkElementAutomationPeer.CreatePeerForElement (fe) as ConcreteFrameworkElementAutomationPeer;
			peer.ControlType = controlType;
			Assert.AreEqual (localizedValue, peer.GetLocalizedControlType (), string.Format ("Literal value: '{0}'", localizedValue));
		}

	}
}
