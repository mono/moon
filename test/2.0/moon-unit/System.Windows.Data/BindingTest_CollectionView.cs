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
using System.ComponentModel;
using System.Windows.Data;
using System.Collections.Generic;

namespace MoonTest.System.Windows.Data {

	[TestClass]
	public class BindingTest_CollectionView {

		List<object> Data {
			get; set;
		}

		CollectionViewSource Source {
			get; set;
		}

		Rectangle Target {
			get; set;
		}

		ICollectionView View {
			get { return Source.View; }
		}

		[TestInitialize]
		public void Setup ()
		{
			Data = new List<object> {
				new Human { Name="1", Age = 1 },
				new Human { Name="2", Age = 2 },
				new Human { Name="3", Age = 3 },
				new Human { Name="4", Age = 4 },
				new Human { Name="5", Age = 5 },
			};

			Source = new CollectionViewSource {
				Source = Data,
			};

			Target = new Rectangle ();
		}

		[TestMethod]
		public void StandardBinding ()
		{
			// We bind to the CurrentItem of the CollectionView
			Target.SetBinding (Rectangle.WidthProperty, new Binding ("Age") {
				Source = Source,
			});
			Assert.AreEqual (1, Target.Width, "#1");
		}

		[TestMethod]
		public void ChangeCurrentItem ()
		{
			// We bind to the CurrentItem of the CollectionView
			Target.SetBinding (Rectangle.WidthProperty, new Binding ("Age") {
				Source = Source,
			});

			Assert.AreEqual (1, Target.Width, "#1");

			View.MoveCurrentToNext ();
			Assert.AreEqual (2, Target.Width, "#2");
		}

		[TestMethod]
		public void StandardBinding_BindsDirectlyToSource ()
		{
			// We bind directly to the CollectionViewSource instead of the CurrentItem
			// Therefore 'age' cannot be found
			Target.SetBinding (Rectangle.WidthProperty, new Binding ("Age") {
				BindsDirectlyToSource = true,
				Source = Source,
			});
			Assert.IsTrue (double.IsNaN (Target.Width), "#1");
		}

		[TestMethod]
		public void StandardBinding_BindsDirectlyToSource_PropertyOnSource ()
		{
			// We bind directly to the CollectionViewSource instead of the CurrentItem
			// Therefore 'age' cannot be found
			Target.SetBinding (Rectangle.WidthProperty, new Binding ("SortDescriptions.Count") {
				BindsDirectlyToSource = true,
				Source = Source,
			});
			Assert.AreEqual (0, Target.Width, "#1");
		}
	}

	public class Human {
		public double Age { get; set; }
		public string Name { get; set; }

		public override string ToString ()
		{
			return Name;
		}
	}
}
