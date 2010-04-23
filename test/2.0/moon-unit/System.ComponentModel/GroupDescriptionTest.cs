//
// GroupDescription Unit Tests
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
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.ComponentModel;
using System.Collections.Generic;
using System.Globalization;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.ComponentModel {

	[TestClass]
	public class ____GroupDescriptionTest {

		[TestMethod]
		public void NamesMatch_BoxedValueTypes ()
		{
			Assert.IsTrue (new ConcreteGroupDescription ().NamesMatch (1, 1), "#1");
		}

		[TestMethod]
		public void NamesMatch_Strings ()
		{
			Assert.IsTrue (new ConcreteGroupDescription ().NamesMatch (new string ('c', 1), new string ('c', 1)), "#1");
		}

		[TestMethod]
		public void GroupNames_PropertyChanged ()
		{
			var gd = new ConcreteGroupDescription ();

			Assert.AreEqual (0, gd.PropertyChangedFired.Count, "#1");
			gd.GroupNames.Add ("test");
			Assert.AreEqual (1, gd.PropertyChangedFired.Count, "#2");
			Assert.AreEqual ("GroupNames", gd.PropertyChangedFired[0], "#3");
		}

		[TestMethod]
		public void ShouldSerializeGroupNames ()
		{
			var g = new ConcreteGroupDescription ();
			g.GroupNames.Add ("name");
			Assert.IsTrue (g.ShouldSerializeGroupNames (), "#1");
		}

		[TestMethod]
		public void ShouldSerializeGroupNames_Empty ()
		{
			var g = new ConcreteGroupDescription ();
			Assert.IsFalse (g.ShouldSerializeGroupNames (), "#1");
		}
	}

	class ConcreteGroupDescription : GroupDescription {

		public List<string> OnPropertyChangedCalled = new List<string> ();
		public List<string> PropertyChangedFired = new List<string> ();

		public ConcreteGroupDescription ()
		{
			PropertyChanged += (o, e) => {
				PropertyChangedFired.Add (e.PropertyName);
			};
		}

		public override object GroupNameFromItem (object item, int level, CultureInfo culture)
		{
			return "SomeName";
		}

		public override bool NamesMatch (object groupName, object itemName)
		{
			return base.NamesMatch (groupName, itemName);
		}

		protected override void OnPropertyChanged (PropertyChangedEventArgs e)
		{
			OnPropertyChangedCalled.Add (e.PropertyName);
			base.OnPropertyChanged (e);
		}
	}
}
