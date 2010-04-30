//
// PropertyGroupDescription Unit Tests
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
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Data;
using MoonTest.System.ComponentModel;
using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.Windows.Data {

	[TestClass]
	public class PropertyGroupDescriptionTest {

		[TestMethod]
		public void Constructor_1 ()
		{
			var p = new PropertyGroupDescription ();
			ConstructorCore (p, null, null, StringComparison.Ordinal);
		}

		[TestMethod]
		public void Constructor_2 ()
		{
			var p = new PropertyGroupDescription ("");
			ConstructorCore (p, "", null, StringComparison.Ordinal);
		}

		[TestMethod]
		public void Constructor_3 ()
		{
			var p = new PropertyGroupDescription (null);
			ConstructorCore (p, null, null, StringComparison.Ordinal);
		}

		[TestMethod]
		public void Constructor_4 ()
		{
			var p = new PropertyGroupDescription (null, null);
			ConstructorCore (p, null, null, StringComparison.Ordinal);
		}

		[TestMethod]
		public void Constructor_5 ()
		{
			var p = new PropertyGroupDescription (null, null, StringComparison.OrdinalIgnoreCase);
			ConstructorCore (p, null, null, StringComparison.OrdinalIgnoreCase);
		}

		[TestMethod]
		public void ConstructorDoesNotRaisePropertyChanged ()
		{
			var p = new ConcretePropertyGroupDescription ();
			Assert.AreEqual (0, p.OnPropertyChangedCalled.Count, "#1");
		}

		[TestMethod]
		public void GroupNameFromItem_Converter ()
		{
			// An invalid name means return null
			var ob = new Rectangle { Width = 100 };
			var converter = new ValueConverter {
				Converter = (value, targetType, parameter, culture) => {
					return Convert.ToInt32 (value) - 50;
				}
			};

			var p = new PropertyGroupDescription ("Width", converter);
			var result = p.GroupNameFromItem (ob, 0, null);
			Assert.IsInstanceOfType<int> (result, "#1");
			Assert.AreEqual (50, (int) result, "#2");
		}

		[TestMethod]
		public void GroupNameFromItem_Converter_CheckParameters()
		{
			// An invalid name means return null
			var ob = new Rectangle { Width = 100 };
			var converter = new ValueConverter {
				Converter = (value, targetType, parameter, culture) => {
					Assert.IsInstanceOfType<double> (value, "#1");
					Assert.AreEqual (100.0, (double) value, "#2");

					Assert.AreSame (typeof (object), targetType, "#3");

					Assert.IsInstanceOfType<int> (parameter, "#4");
					Assert.AreEqual (77, (int) parameter, "#5");

					Assert.IsNull (culture, "#6");
					return 50;
				}
			};

			var p = new PropertyGroupDescription ("Width", converter);
			p.GroupNameFromItem (ob, 77, null);
		}

		[TestMethod]
		public void GroupNameFromItem_NullPropertyName ()
		{
			// A null name means 'use the object'
			var ob = new object ();
			var p = new ConcretePropertyGroupDescription (null);
			Assert.AreSame (ob, p.GroupNameFromItem (ob, 0, null));
		}

		[TestMethod]
		public void GroupNameFromItem_EmptyPropertyName ()
		{
			// An empty name means 'use the object'
			var ob = new object ();
			var p = new ConcretePropertyGroupDescription ("");
			Assert.AreSame (ob, p.GroupNameFromItem (ob, 0, null));
		}

		[TestMethod]
		public void GroupNameFromItem_InvalidName ()
		{
			// An invalid name means return null
			var ob = new object ();
			var p = new ConcretePropertyGroupDescription ("invalid");
			Assert.IsNull (p.GroupNameFromItem (ob, 0, null));
		}

		[TestMethod]
		public void GroupNameFromItem_ValidName ()
		{
			// An invalid name means return null
			var ob = new Rectangle { Width = 100 };
			var p = new ConcretePropertyGroupDescription ("Width");
			var result = p.GroupNameFromItem (ob, 0, null);
			Assert.IsInstanceOfType<double> (result, "#1");
			Assert.AreEqual (100.0, (double) result, "#2");
		}

		[TestMethod]
		public void GroupNameFromItem_Indexer ()
		{
			// An invalid name means return null
			var o = new Dictionary<string, string> ();
			o.Add ("test", "result");
			var p = new ConcretePropertyGroupDescription ("[test]");
			Assert.AreEqual ("result", p.GroupNameFromItem (o, 0, null));
		}

		[TestMethod]
		public void NamesMatch_DifferentStrings ()
		{
			var p = new PropertyGroupDescription (null, null, StringComparison.OrdinalIgnoreCase);
			Assert.IsFalse (p.NamesMatch ("a", "B"), "#1");
		}

		[TestMethod]
		public void GroupsAreRecreated ()
		{
			Func<object, object, bool> nameMatcher = (groupName, itemName) => (string) groupName == (string) itemName;
			Func<object, int, object> nameCreator = (item, level) => ((int) item <= 2 ? "Lower" : "Upper") + level.ToString ();

			var desc = new ConcretePropertyGroupDescription () {
				GroupNameFromItemFunc = nameCreator,
				NamesMatchFunc = nameMatcher
			};

			var source = new CollectionViewSource { Source = new [] { 0, 1, 2, 3, 4, 5 } };
			source.GroupDescriptions.Add (desc);

			var groups = source.View.Groups;
			var lowerGroup = (CollectionViewGroup) source.View.Groups [0];
			var upperGroup = (CollectionViewGroup) source.View.Groups [1];


			using (source.DeferRefresh ())
			using (source.View.DeferRefresh ()) {
				source.GroupDescriptions.Clear ();
				source.GroupDescriptions.Add (desc);
			}

			Assert.AreSame (groups, source.View.Groups, "#1");
			Assert.AreNotSame (lowerGroup, source.View.Groups [0], "#2");
			Assert.AreNotSame (upperGroup, source.View.Groups [1], "#3");
		}

		[TestMethod]
		public void OneGroupDesciption ()
		{
			Func<object, object, bool> nameMatcher = (groupName, itemName) => (string) groupName == (string) itemName;
			Func<object, int, object> nameCreator = (item, level) => ((int) item <= 2 ? "Lower" : "Upper") + level.ToString ();

			var desc = new ConcretePropertyGroupDescription () {
				GroupNameFromItemFunc = nameCreator,
				NamesMatchFunc = nameMatcher
			};

			var source = new CollectionViewSource { Source = new [] { 0, 1, 2, 3, 4, 5 } };
			using (source.View.DeferRefresh ()) {
				source.GroupDescriptions.Add (desc);
			}
			Assert.AreEqual (2, source.View.Groups.Count, "#1");
			var lowerGroup = (CollectionViewGroup) source.View.Groups [0];
			var upperGroup = (CollectionViewGroup) source.View.Groups [1];

			Assert.AreEqual ("Lower0", lowerGroup.Name, "#2");
			Assert.AreEqual ("Upper0", upperGroup.Name, "#3");

			Assert.IsTrue (lowerGroup.IsBottomLevel, "#4");
			Assert.IsTrue (upperGroup.IsBottomLevel, "#5");

			Assert.AreEqual (3, lowerGroup.ItemCount, "#6");
			Assert.AreEqual (3, upperGroup.ItemCount, "#7");

			for (int i = 0; i < 3; i++)
				Assert.AreEqual (i, (int) lowerGroup.Items [i], "#8." + i);
			for (int i = 0; i < 3; i++)
				Assert.AreEqual (i + 3, (int) upperGroup.Items [i], "#9." + i);
		}

		[TestMethod]
		[MoonlightBug]
		[Ignore ("Test not completed yet")]
		public void TwoGroupDesciptions ()
		{
			Func<object, object, bool> nameMatcher = (groupName, itemName) => (string) groupName == (string) itemName;
			Func<object, int, object> nameCreator = (item, level) => ((int) item <= 2 ? "Lower" : "Upper") + level.ToString ();

			var lowerDesc = new ConcretePropertyGroupDescription () {
				GroupNameFromItemFunc = nameCreator,
				NamesMatchFunc = nameMatcher
			};
			var upperDesc = new ConcretePropertyGroupDescription () {
				GroupNameFromItemFunc = nameCreator,
				NamesMatchFunc = nameMatcher
			};

			var source = new CollectionViewSource { Source = new [] { 0, 1, 2, 3, 4, 5 } };
			using (source.View.DeferRefresh ()) {
				source.GroupDescriptions.Add (lowerDesc);
				source.GroupDescriptions.Add (upperDesc);
			}
			Assert.AreEqual (2, source.View.Groups.Count, "#1");
			var lowerGroup = (CollectionViewGroup) source.View.Groups [0];
			var upperGroup = (CollectionViewGroup) source.View.Groups [1];

			Assert.AreEqual ("Lower0", lowerGroup.Name, "#2");
			Assert.AreEqual ("Upper0", upperGroup.Name, "#3");

			Assert.IsFalse (lowerGroup.IsBottomLevel, "#4");
			Assert.IsFalse (upperGroup.IsBottomLevel, "#5");

			Assert.AreEqual (3, lowerGroup.ItemCount, "#6");
			Assert.AreEqual (3, upperGroup.ItemCount, "#7");

			for (int i = 0; i < 3; i++)
				Assert.AreEqual (i, (int) lowerGroup.Items [i], "#8." + i);
			for (int i = 0; i < 3; i++)
				Assert.AreEqual (i + 3, (int) upperGroup.Items [i], "#9." + i);
		}

		[TestMethod]
		public void NamesMatch_IgnoreCase ()
		{
			var p = new PropertyGroupDescription (null, null, StringComparison.OrdinalIgnoreCase);
			Assert.IsTrue (p.NamesMatch ("a", "A"), "#1");
		}

		[TestMethod]
		public void NamesMatch_NotConverterToString ()
		{
			var p = new PropertyGroupDescription (null, null, StringComparison.OrdinalIgnoreCase);
			Assert.IsFalse (p.NamesMatch ('a', 'A'), "#1");
		}

		void ConstructorCore (PropertyGroupDescription p, string name, IValueConverter converter, StringComparison comparison)
		{
			Assert.AreEqual (p.PropertyName, name, "#1");
			Assert.AreEqual (p.Converter, converter, "#2");
			Assert.AreEqual (p.StringComparison, comparison, "#3");
		}
	}

	class ValueConverter : IValueConverter {

		public Func<object, Type, object, CultureInfo, object> Converter { get; set; }
		public Func<object, Type, object, CultureInfo, object> ConverterBack { get; set; }

		public object Convert (object value, Type targetType, object parameter, CultureInfo culture)
		{
			return Converter (value, targetType, parameter, culture);
		}

		public object ConvertBack (object value, Type targetType, object parameter, CultureInfo culture)
		{
			return ConverterBack (value, targetType, parameter, culture);
		}
	}

	class ConcretePropertyGroupDescription : PropertyGroupDescription {
		public List<string> OnPropertyChangedCalled = new List<string> ();
		public List<string> PropertyChangedFired = new List<string> ();
		public Func<object, object, bool> NamesMatchFunc { get; set; }
		public Func<object, int, object> GroupNameFromItemFunc { get; set; }
		public string Name { get; set; }

		public ConcretePropertyGroupDescription ()
			: this (null)
		{

		}
		
		public ConcretePropertyGroupDescription (string name)
			: base (name)
		{
			PropertyChanged += (o, e) => {
				PropertyChangedFired.Add (e.PropertyName);
			};
		}

		public override object GroupNameFromItem (object item, int level, CultureInfo culture)
		{
			if (GroupNameFromItemFunc != null)
				return GroupNameFromItemFunc (item, level);
			return base.GroupNameFromItem (item, level, culture);
		}

		public override bool NamesMatch (object groupName, object itemName)
		{
			if (NamesMatchFunc != null)
				return NamesMatchFunc (groupName, itemName);
			return base.NamesMatch (groupName, itemName);
		}

		protected override void OnPropertyChanged (PropertyChangedEventArgs e)
		{
			OnPropertyChangedCalled.Add (e.PropertyName);
			base.OnPropertyChanged (e);
		}
	}
}
