//
// Unit tests for AutomationElementIdentifiers
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008 Novell, Inc (http://www.novell.com)
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
using System.Windows;
using System.Windows.Controls;
using System.Windows.Automation;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class AutomationPropertiesTest {

		[TestMethod]
		public void AcceleratorKeyPropertyTest ()
		{
			DefaultTestString (AutomationProperties.AcceleratorKeyProperty, 
			                   "CTRL+W", 
					   AutomationProperties.GetAcceleratorKey);
		}

		[TestMethod]
		public void AccessKeyPropertyTest ()
		{
			DefaultTestString (AutomationProperties.AccessKeyProperty, 
			                   "ALT+W", 
					   AutomationProperties.GetAccessKey);
		}

		[TestMethod]
		public void HelpTextPropertyTest ()
		{
			DefaultTestString (AutomationProperties.HelpTextProperty, 
			                   "Help text property", 
					   AutomationProperties.GetHelpText);
		}

		[TestMethod]
		public void AutomationIdPropertyTest ()
		{
			DefaultTestString (AutomationProperties.AutomationIdProperty, 
			                   "Id 0", 
					   AutomationProperties.GetAutomationId);
		}

		[TestMethod]
		public void IsRequiredForFormPropertyTest ()
		{
			Assert.IsNotNull (AutomationProperties.IsRequiredForFormProperty, "#0");

			TextBlock block = new TextBlock ();
			block.SetValue (AutomationProperties.IsRequiredForFormProperty, true);
			Assert.AreEqual (AutomationProperties.GetIsRequiredForForm (block),
				block.GetValue (AutomationProperties.IsRequiredForFormProperty), "#1");

			Assert.IsTrue (AutomationProperties.GetIsRequiredForForm (block), "#2");
			Assert.IsTrue ((bool) block.GetValue (AutomationProperties.IsRequiredForFormProperty), "#3");
		}

		[TestMethod]
		public void ItemStatusPropertyTest ()
		{
			DefaultTestString (AutomationProperties.ItemStatusProperty, 
			                   "Item status", 
					   AutomationProperties.GetItemStatus);
		}

		[TestMethod]
		public void ItemTypePropertyTest ()
		{
			DefaultTestString (AutomationProperties.ItemTypeProperty, 
			                   "Item type", 
					   AutomationProperties.GetItemType);
		}

		[TestMethod]
		public void LabeledByPropertyTest ()
		{
			Assert.IsNotNull (AutomationProperties.LabeledByProperty, "#0");

			TextBlock block = new TextBlock ();
			TextBlock labeledBy = new TextBlock();
			block.SetValue (AutomationProperties.LabeledByProperty, labeledBy);
			Assert.AreEqual (AutomationProperties.GetLabeledBy (block),
				block.GetValue(AutomationProperties.LabeledByProperty), "#1");

			Assert.AreSame (labeledBy, AutomationProperties.GetLabeledBy (block), "#2");
			Assert.AreSame (labeledBy, block.GetValue (AutomationProperties.LabeledByProperty), "#3");
		}

		[TestMethod]
		public void NamePropertyTest ()
		{
			DefaultTestString (AutomationProperties.NameProperty, 
			                   "My name is... What!?", 
					   AutomationProperties.GetName);
		}

		private void DefaultTestString (DependencyProperty property, 
		                                object setValue, 
						StringPropertyDelegate stringDelegate)
		{
			Assert.IsNotNull (property, "#0");

			TextBlock block = new TextBlock ();

			block.SetValue (property, setValue);
			Assert.AreEqual (stringDelegate (block), block.GetValue (property), "#1");

			Assert.AreEqual (setValue, stringDelegate (block), "#2");
			Assert.AreEqual (setValue, block.GetValue (property), "#3");
		}

		delegate string StringPropertyDelegate (UIElement element);
	}
}
