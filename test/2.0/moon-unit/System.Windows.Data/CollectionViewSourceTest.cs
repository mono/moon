//
// CollectionViewSource Unit Tests
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
using System.Collections.Generic;
using System.ComponentModel;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;
using System.Collections.ObjectModel;

namespace MoonTest.System.Windows.Data
{

	[TestClass]
	public partial class CollectionViewSourceTest
	{
		List<int> Source = new List<int>() { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

		[TestMethod]
		public void BaseSourceOnOtherSource()
		{
			var cvs1 = new CollectionViewSource();
			var cvs2 = new CollectionViewSource();
			var cvs3 = new CollectionViewSource();
			var data = new Leagues();

			BindingOperations.SetBinding(cvs1, CollectionViewSource.SourceProperty, new Binding() { Source = data });
			BindingOperations.SetBinding(cvs2, CollectionViewSource.SourceProperty, new Binding("Divisions") { Source = cvs1 });
			BindingOperations.SetBinding(cvs3, CollectionViewSource.SourceProperty, new Binding("Teams") { Source = cvs2 });

			Assert.AreSame(data[0].Divisions[0].Teams[0], cvs3.View.CurrentItem, "#1");
		}

		[TestMethod]
		public void BaseSourceOnOtherSource_MoveSelection()
		{
			var cvs1 = new CollectionViewSource();
			var cvs2 = new CollectionViewSource();
			var cvs3 = new CollectionViewSource();
			var data = new Leagues();

			BindingOperations.SetBinding(cvs1, CollectionViewSource.SourceProperty, new Binding() { Source = data });
			BindingOperations.SetBinding(cvs2, CollectionViewSource.SourceProperty, new Binding("Divisions") { Source = cvs1 });
			BindingOperations.SetBinding(cvs3, CollectionViewSource.SourceProperty, new Binding("Teams") { Source = cvs2 });

			cvs1.View.MoveCurrentToPosition(1);
			Assert.AreSame(data[1].Divisions[0].Teams[0], cvs3.View.CurrentItem, "#1");
		}

		[TestMethod]
		public void BaseSourceOnOtherSource_ListBoxUpdates()
		{
			var cvs1 = new CollectionViewSource();
			var cvs2 = new CollectionViewSource();
			var cvs3 = new CollectionViewSource();
			var data = new Leagues();

			BindingOperations.SetBinding(cvs1, CollectionViewSource.SourceProperty, new Binding() { Source = data });
			BindingOperations.SetBinding(cvs2, CollectionViewSource.SourceProperty, new Binding("Divisions") { Source = cvs1 });
			BindingOperations.SetBinding(cvs3, CollectionViewSource.SourceProperty, new Binding("Teams") { Source = cvs2 });

			var lb1 = new ListBox { ItemsSource = cvs1.View };
			var lb2 = new ListBox { ItemsSource = cvs2.View };
			var lb3 = new ListBox { ItemsSource = cvs3.View };
			lb3.UpdateLayout();

			Assert.AreSame(data[0].Divisions[0].Teams[0], cvs3.View.CurrentItem, "#1");
			Assert.AreSame(data[0].Divisions[0].Teams[0], lb3.SelectedItem, "#2");
		}

		[TestMethod]
		public void BaseSourceOnOtherSource_MoveSelection_ListBoxUpdates()
		{
			var cvs1 = new CollectionViewSource();
			var cvs2 = new CollectionViewSource();
			var cvs3 = new CollectionViewSource();
			var data = new Leagues();

			BindingOperations.SetBinding(cvs1, CollectionViewSource.SourceProperty, new Binding() { Source = data });
			BindingOperations.SetBinding(cvs2, CollectionViewSource.SourceProperty, new Binding("Divisions") { Source = cvs1 });
			BindingOperations.SetBinding(cvs3, CollectionViewSource.SourceProperty, new Binding("Teams") { Source = cvs2 });

			var lb1 = new ListBox();
			var lb2 = new ListBox();
			var lb3 = new ListBox();

			lb1.SetBinding(ListBox.ItemsSourceProperty, new Binding { Source = cvs1 });
			lb2.SetBinding(ListBox.ItemsSourceProperty, new Binding { Source = cvs2 });
			lb3.SetBinding(ListBox.ItemsSourceProperty, new Binding { Source = cvs3 });

			cvs1.View.MoveCurrentToPosition(1);
			Assert.AreSame(data[1].Divisions[0].Teams[0], cvs3.View.CurrentItem, "#1");
			Assert.AreSame(data[1].Divisions[0].Teams[0], lb3.SelectedItem, "#2");
		}

		[TestMethod]
		public void BaseSourceOnOtherSource_BindToDataContext()
		{
			var cvs1 = new CollectionViewSource();
			var data = new Leagues();

			BindingOperations.SetBinding(cvs1, CollectionViewSource.SourceProperty, new Binding() { Source = data });

			var lb1 = new ListBox();

			lb1.SetBinding(ListBox.DataContextProperty, new Binding { Source = cvs1 });

			Assert.AreSame(cvs1.View, lb1.DataContext, "#1");
		}

		[TestMethod]
		public void BaseSourceOnOtherSource_BindToEnumerable()
		{
			var cvs1 = new CollectionViewSource();
			var cvs2 = new CollectionViewSource();
			var cvs3 = new CollectionViewSource();
			var data = new Leagues();

			BindingOperations.SetBinding(cvs1, CollectionViewSource.SourceProperty, new Binding() { Source = data });
			BindingOperations.SetBinding(cvs2, CollectionViewSource.SourceProperty, new Binding("Divisions") { Source = cvs1 });
			BindingOperations.SetBinding(cvs3, CollectionViewSource.SourceProperty, new Binding("Teams") { Source = cvs2 });

			var lb1 = new ListBox();
			var lb2 = new ListBox();
			var lb3 = new ListBox();

			lb1.SetBinding(ListBox.ItemsSourceProperty, new Binding { Source = cvs1 });
			lb2.SetBinding(ListBox.ItemsSourceProperty, new Binding { Source = cvs2 });
			lb3.SetBinding(ListBox.ItemsSourceProperty, new Binding { Source = cvs3 });

			Assert.AreSame(cvs1.View, lb1.ItemsSource, "#1");
			Assert.AreSame(cvs2.View, lb2.ItemsSource, "#2");
			Assert.AreSame(cvs3.View, lb3.ItemsSource, "#3");

			lb2.SelectedIndex = 1;
			Assert.AreSame(cvs1.View, lb1.ItemsSource, "#4");
			Assert.AreSame(cvs2.View, lb2.ItemsSource, "#5");
			Assert.AreSame(cvs3.View, lb3.ItemsSource, "#6");
		}

		[TestMethod]
		public void BaseSourceOnOtherSource_MoveSelection_ListBoxCausesUpdates()
		{
			var cvs1 = new CollectionViewSource();
			var cvs2 = new CollectionViewSource();
			var cvs3 = new CollectionViewSource();
			var data = new Leagues();

			BindingOperations.SetBinding(cvs1, CollectionViewSource.SourceProperty, new Binding() { Source = data });
			BindingOperations.SetBinding(cvs2, CollectionViewSource.SourceProperty, new Binding("Divisions") { Source = cvs1 });
			BindingOperations.SetBinding(cvs3, CollectionViewSource.SourceProperty, new Binding("Teams") { Source = cvs2 });

			var lb1 = new ListBox { Name = "lb1" };
			var lb2 = new ListBox { Name = "lb2" };
			var lb3 = new ListBox { Name = "lb3" };

			lb1.SetBinding(ListBox.ItemsSourceProperty, new Binding { Source = cvs1 });
			lb2.SetBinding(ListBox.ItemsSourceProperty, new Binding { Source = cvs2 });
			lb3.SetBinding(ListBox.ItemsSourceProperty, new Binding { Source = cvs3 });

			lb3.SelectedIndex = 1;
			Assert.AreSame(data[0], cvs1.View.CurrentItem, "#1");
			Assert.AreSame(data[0].Divisions[0], cvs2.View.CurrentItem, "#2");
			Assert.AreSame(data[0].Divisions[0].Teams[1], cvs3.View.CurrentItem, "#3");

			lb2.SelectedIndex = 1;
			Assert.AreEqual(data[0], cvs1.View.CurrentItem, "#4");
			Assert.AreEqual(data[0].Divisions[1], cvs2.View.CurrentItem, "#5");
			Assert.AreEqual(data[0].Divisions[1].Teams[0], cvs3.View.CurrentItem, "#6");

			lb1.SelectedIndex = 1;
			Assert.AreSame(data[1], cvs1.View.CurrentItem, "#7");
			Assert.AreSame(data[1].Divisions[0], cvs2.View.CurrentItem, "#8");
			Assert.AreSame(data[1].Divisions[0].Teams[0], cvs3.View.CurrentItem, "#9");
		}

		[TestMethod]
		public void ChangeSourceRecreatesView()
		{
			// If we change the source, we create a new View
			var source = new CollectionViewSource { Source = this.Source };
			var view = source.View;
			source.Source = new List<double>() { 1.5, 2.5, 3.5, 4.5 };
			Assert.AreNotSame(view, source.View, "#1");
		}

		[TestMethod]
		public void ISupportInitializeTest_DontCreateView ()
		{
			var source = new CollectionViewSource();
			((ISupportInitialize)source).BeginInit();
			source.Source = new string[5];
			Assert.IsNull(source.View, "#1");
			((ISupportInitialize)source).EndInit();
			Assert.IsNotNull(source.View, "#1");
		}

		[TestMethod]
		public void SourceFilterIsPropagated()
		{
			FilterEventHandler h = (o, e) => { };
			var source = new CollectionViewSource { Source = this.Source };
			Assert.IsNull(source.View.Filter, "#1");

			source.Filter += h;
			Assert.IsNotNull(source.View.Filter, "#2");

			source.Filter -= h;
			Assert.IsNull(source.View.Filter, "#3");
		}

		[TestMethod]
		public void FilterUsedImmediately()
		{
			bool called = false;
			FilterEventHandler h = (o, e) => { called = true; };

			var source = new CollectionViewSource { Source = this.Source };
			source.Filter += h;

			Assert.IsTrue(called, "#1");
		}

		[TestMethod]
		public void ViewIsReadOnly()
		{
			var source = new CollectionViewSource { Source = this.Source };
			var view = source.View;
			Assert.IsNotNull(view, "#1");

			// We can clear the View even though it has no setter.
			source.ClearValue(CollectionViewSource.ViewProperty);
			Assert.IsNull(source.View, "#2");
		}

		[TestMethod]
		public void ViewsAreReused()
		{
			// If you change the Source collection and then change it back, you
			// end up re-using the View you had the first time.
			var cvs = new CollectionViewSource { Source = Source };
			var view = cvs.View;
			cvs.Source = new List<object>();
			cvs.Source = Source;

			Assert.AreSame(view, cvs.View, "#1");
		}

		[TestMethod]
		public void ViewsAreReusedBetweenCVSs()
		{
			// If you use the same collection in multiple CollectionViewSources
			// they end up with unique View objects
			var cvs1 = new CollectionViewSource { Source = Source };
			var cvs2 = new CollectionViewSource { Source = Source };

			Assert.AreNotSame(cvs1.View, cvs2.View, "#1");
		}
	}


	public class Leagues : ObservableCollection<League>
	{
		public Leagues()
		{
			Division division;
			League league;

			league = new League { Name = "League 1" };
			division = new Division { Name = "Divison 1" };
			division.Teams.Add(new Team { Name = "League 1 Div 1 Team 1" });
			division.Teams.Add(new Team { Name = "League 1 Div 1 Team 2" });
			division.Teams.Add(new Team { Name = "League 1 Div 1 Team 3" });
			league.Divisions.Add(division);

			division = new Division { Name = "Divison 2" };
			division.Teams.Add(new Team { Name = "League 1 Div 2 Team 1" });
			division.Teams.Add(new Team { Name = "League 1 Div 2 Team 2" });
			division.Teams.Add(new Team { Name = "League 1 Div 2 Team 3" });
			league.Divisions.Add(division);

			division = new Division { Name = "Divison 3" };
			division.Teams.Add(new Team { Name = "League 1 Div 3 Team 1" });
			division.Teams.Add(new Team { Name = "League 1 Div 3 Team 2" });
			division.Teams.Add(new Team { Name = "League 1 Div 3 Team 3" });
			league.Divisions.Add(division);
			Add(league);

			league = new League { Name = "League 2" };
			division = new Division { Name = "Divison 1" };
			division.Teams.Add(new Team { Name = "League 2 Div 1 Team 1" });
			division.Teams.Add(new Team { Name = "League 2 Div 1 Team 2" });
			division.Teams.Add(new Team { Name = "League 2 Div 1 Team 3" });
			league.Divisions.Add(division);

			division = new Division { Name = "Divison 2" };
			division.Teams.Add(new Team { Name = "League 2 Div 2 Team 1" });
			division.Teams.Add(new Team { Name = "League 2 Div 2 Team 2" });
			division.Teams.Add(new Team { Name = "League 2 Div 2 Team 3" });
			league.Divisions.Add(division);

			division = new Division { Name = "Divison 3" };
			division.Teams.Add(new Team { Name = "League 2 Div 3 Team 1" });
			division.Teams.Add(new Team { Name = "League 2 Div 3 Team 2" });
			division.Teams.Add(new Team { Name = "League 2 Div 3 Team 3" });
			league.Divisions.Add(division);
			Add(league);

			league = new League { Name = "League 3" };
			division = new Division { Name = "Divison 1" };
			division.Teams.Add(new Team { Name = "League 3 Div 1 Team 1" });
			division.Teams.Add(new Team { Name = "League 3 Div 1 Team 2" });
			division.Teams.Add(new Team { Name = "League 3 Div 1 Team 3" });
			league.Divisions.Add(division);

			division = new Division { Name = "Divison 2" };
			division.Teams.Add(new Team { Name = "League 3 Div 2 Team 1" });
			division.Teams.Add(new Team { Name = "League 3 Div 2 Team 2" });
			division.Teams.Add(new Team { Name = "League 3 Div 2 Team 3" });
			league.Divisions.Add(division);

			division = new Division { Name = "Divison 3" };
			division.Teams.Add(new Team { Name = "League 3 Div 3 Team 1" });
			division.Teams.Add(new Team { Name = "League 3 Div 3 Team 2" });
			division.Teams.Add(new Team { Name = "League 3 Div 3 Team 3" });
			league.Divisions.Add(division);
			Add(league);
		}
	}

	public class Divisions : ObservableCollection<Division>
	{

	}

	public class Teams : ObservableCollection<Team>
	{

	}

	public class League  {

		public string Name {
			get; set;
		}
		public Divisions Divisions {
			get; private set;
		}

		public League()
		{
			Divisions = new Divisions();
		}
	}

	public class Division {

		public string Name {
			get; set;
		}

		public Teams Teams {
			get; private set;
		}

		public Division ()
		{
			Teams = new Teams();
		}
	}

	public class Team {

		public string Name {
			get; set;
		}
	}
}
