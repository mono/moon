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
using System.Collections.ObjectModel;
using System.Collections.Generic;
using System.Collections.Specialized;
using Mono.Moonlight.UnitTesting;

using Microsoft.VisualStudio.TestTools.UnitTesting;


namespace MoonTest.System.Collections.ObjectModel {

	[TestClass]
	public class ObservableCollectionTest
	{
		[TestMethod]
		public void AddTest ()
		{
			NotifyCollectionChangedEventArgs args = null;
			ObservableCollection<int> col = new ObservableCollection<int>();
			col.CollectionChanged += delegate (object sender, NotifyCollectionChangedEventArgs e) {
				args = e;
			};
			col.Add (5);

			Tester.WriteLine ("1");
			Assert.AreEqual (NotifyCollectionChangedAction.Add, args.Action);
			Tester.WriteLine ("2");
			Assert.AreEqual (1, args.NewItems.Count);
			Assert.AreEqual (5, (int)args.NewItems[0]);
			Tester.WriteLine ("3");
			Assert.IsNull   (args.OldItems);
			Tester.WriteLine ("4");
			Assert.AreEqual (-1, args.OldStartingIndex);
			Tester.WriteLine ("5");
			Assert.AreEqual (0, args.NewStartingIndex);
		}

		[TestMethod]
		public void RemoveTest ()
		{
			NotifyCollectionChangedEventArgs args = null;
			ObservableCollection<int> col = new ObservableCollection<int>();
			col.Add (5);
			col.Add (10);
			col.CollectionChanged += delegate (object sender, NotifyCollectionChangedEventArgs e) {
				args = e;
			};
			col.Remove (10);

			Tester.WriteLine ("1");
			Assert.AreEqual (NotifyCollectionChangedAction.Remove, args.Action);
			Tester.WriteLine ("2");
			Assert.IsNull (args.NewItems);
			Tester.WriteLine ("3");
			Assert.AreEqual (1, args.OldItems.Count);
			Assert.AreEqual (10, (int)args.OldItems[0]);
			Tester.WriteLine ("4");
			Assert.AreEqual (1, args.OldStartingIndex);
			Tester.WriteLine ("5");
			Assert.AreEqual (-1, args.NewStartingIndex);
		}
	}
}
