//
// Unit tests for AutomationElementIdentifiers
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
using System.Windows.Automation;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class AutomationElementIdentifiersTest {

		[TestMethod]
		public void FieldsTest ()
		{
			Assert.IsNotNull (AutomationElementIdentifiers.AcceleratorKeyProperty, "#0");
			Assert.IsNotNull (AutomationElementIdentifiers.AccessKeyProperty, "#1");
			Assert.IsNotNull (AutomationElementIdentifiers.AutomationIdProperty, "#2");
			Assert.IsNotNull (AutomationElementIdentifiers.BoundingRectangleProperty, "#3");
			Assert.IsNotNull (AutomationElementIdentifiers.ClassNameProperty, "#4");
			Assert.IsNotNull (AutomationElementIdentifiers.ClickablePointProperty, "#5");
			Assert.IsNotNull (AutomationElementIdentifiers.ControlTypeProperty, "#6");
			Assert.IsNotNull (AutomationElementIdentifiers.HasKeyboardFocusProperty, "#7");
			Assert.IsNotNull (AutomationElementIdentifiers.HelpTextProperty, "#8");
			Assert.IsNotNull (AutomationElementIdentifiers.IsContentElementProperty, "#9");
			Assert.IsNotNull (AutomationElementIdentifiers.IsControlElementProperty, "#10");
			Assert.IsNotNull (AutomationElementIdentifiers.IsEnabledProperty, "#11");
			Assert.IsNotNull (AutomationElementIdentifiers.IsKeyboardFocusableProperty, "#12");
			Assert.IsNotNull (AutomationElementIdentifiers.IsOffscreenProperty, "#13");
			Assert.IsNotNull (AutomationElementIdentifiers.IsPasswordProperty, "#14");
			Assert.IsNotNull (AutomationElementIdentifiers.IsRequiredForFormProperty, "#15");
			Assert.IsNotNull (AutomationElementIdentifiers.ItemStatusProperty, "#16");
			Assert.IsNotNull (AutomationElementIdentifiers.ItemTypeProperty, "#17");
			Assert.IsNotNull (AutomationElementIdentifiers.LabeledByProperty, "#18");
			Assert.IsNotNull (AutomationElementIdentifiers.LocalizedControlTypeProperty, "#19");
			Assert.IsNotNull (AutomationElementIdentifiers.NameProperty, "#20");
			Assert.IsNotNull (AutomationElementIdentifiers.OrientationProperty, "#21");
		}
	}
}
