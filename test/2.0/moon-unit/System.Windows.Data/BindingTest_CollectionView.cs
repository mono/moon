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

		List<Human> Data {
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
			Data = new List<Human> {
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
		public void StandardBinding_CVS ()
		{
			// We bind to the CurrentItem of the CollectionView
			Target.SetBinding (Rectangle.WidthProperty, new Binding ("Age") {
				Source = Source,
			});
			Assert.AreEqual (1, Target.Width, "#1");
		}

		[TestMethod]
		public void StandardBinding_ICV ()
		{
			// We bind to the CurrentItem of the CollectionView
			Target.SetBinding (Rectangle.WidthProperty, new Binding ("Age") {
				Source = Source.View,
			});
			Assert.AreEqual (1, Target.Width, "#1");
		}

		[TestMethod]
		public void BindToEnumerable_CVS ()
		{
			var target = new ListBox ();
			target.SetBinding (ListBox.ItemsSourceProperty, new Binding {
				Source = Source
			});
			Assert.AreSame (Source.View, target.ItemsSource, "#1");
		}

		[TestMethod]
		public void BindToEnumerable_ICV ()
		{
			var target = new ListBox ();
			target.SetBinding (ListBox.ItemsSourceProperty, new Binding {
				Source = Source.View
			});
			Assert.AreSame (Source.View, target.ItemsSource, "#1");
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
		public void SourceNotSet ()
		{
			Source.Source = null;
			Target.SetBinding (Rectangle.TagProperty, new Binding ("Age") {
				Source = Source
			});
			Assert.IsNull (Target.Tag, "#1");
		}

		[TestMethod]
		public void SourceSet_AfterBinding ()
		{
			Source.Source = null;
			Target.SetBinding (Rectangle.TagProperty, new Binding ("Age") {
				Source = Source
			});
			Source.Source = Data;
			Assert.IsInstanceOfType<double>(Target.Tag, "#1");
			Assert.AreEqual(1.0, (double) Target.Tag, "#2");
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

		[TestMethod]
		public void PropertyOnCollectionViewTakesPriority_CollectionView ()
		{
			// As 'CurrentPosition' exists on CollectionViewSource, we bind to that directly
			// instead of binding to the current item.
			Data [0].CurrentPosition = 1000;
			Target.SetBinding (Rectangle.WidthProperty, new Binding ("CurrentPosition") {
				Source = Source.View,
			});

			Assert.AreEqual (0, Target.Width, "#1");
		}

		[TestMethod]
		public void PropertyOnCollectionViewTakesPriority_CollectionViewSource ()
		{
			// As 'CurrentPosition' exists on CollectionViewSource, we bind to that directly
			// instead of binding to the current item.
			Data[0].CurrentPosition = 1000;
			Target.SetBinding (Rectangle.WidthProperty, new Binding ("CurrentPosition") {
				Source = Source,
			});

			Assert.AreEqual (0, Target.Width, "#1");
		}
	}

	public class Human {
		public double Age { get; set; }
		public string Name { get; set; }
		public double CurrentPosition { get; set; }

		public override string ToString ()
		{
			return Name;
		}
	}
}
