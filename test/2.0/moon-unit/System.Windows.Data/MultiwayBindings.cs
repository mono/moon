using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;

namespace MoonTest.System.Windows.Data
{
	// Add tests for when two objects are databound to the one source.
	[TestClass]
	public partial class BindingTest
	{
		[TestMethod]
		public void InitialValues()
		{
			Person source;
			TextBlock child1;
			TextBlock child2;
			Create(out source, out child1, out child2);

			Assert.AreEqual(string.Format("{0:C}", 10), child1.Text, "#1");
			Assert.AreEqual(string.Format("{0:C}", 10), child2.Text, "#2");
			Assert.AreEqual(10, source.Age, "#3");
		}

		[TestMethod]
		public void AlterSource ()
		{
			Person source;
			TextBlock child1;
			TextBlock child2;
			Create(out source, out child1, out child2);

			// Altering 'Age' updates both listeners
			source.Age = 15;
			Assert.AreEqual(string.Format("{0:C}", 15), child1.Text, "#1");
			Assert.AreEqual(string.Format("{0:C}", 15), child2.Text, "#2");
			Assert.AreEqual(15, source.Age, "#3");
		}

		[TestMethod]
		public void AlterChild1_Integer()
		{
			Person source;
			TextBlock child1;
			TextBlock child2;
			Create(out source, out child1, out child2);

			// Altering a listener updates the source and the other listener
			child1.Text = "15";
			Assert.AreEqual(string.Format("{0:C}", 15), child1.Text, "#1");
			Assert.AreEqual(string.Format("{0:C}", 15), child2.Text, "#2");
			Assert.AreEqual(15, source.Age, "#3");
		}

		[TestMethod]
		public void AlterChild1_Invalid()
		{
			Person source;
			TextBlock child1;
			TextBlock child2;
			Create(out source, out child1, out child2);

			// Updating a listener with a value the datasource can't accept results in
			// nothing happening
			child1.Text = "invalid";
			Assert.AreEqual("invalid", child1.Text, "#1");
			Assert.AreEqual(string.Format("{0:C}", 10), child2.Text, "#2");
			Assert.AreEqual(10, source.Age, "#3");
		}

		[TestMethod]
		public void AlterChild1_EmptyString()
		{
			Person source;
			TextBlock child1;
			TextBlock child2;
			Create(out source, out child1, out child2);

			child1.Text = "";
			Assert.AreEqual("", child1.Text, "#1");
			Assert.AreEqual(string.Format("{0:C}", 10), child2.Text, "#1");
			Assert.AreEqual (10, source.Age, "#3");
		}

		[TestMethod]
		public void ForceSourceToNull ()
		{
			Person source = new Person { Age = 7 };
			TextBlock block = new TextBlock();
			block.SetBinding(TextBlock.TextProperty, new Binding {
				Path=new PropertyPath("Age"),
				Mode = BindingMode.TwoWay,
				TargetNullValue = "",
			});

			block.DataContext = source;
			block.Text = "";
			Assert.IsNull(source.Age, "#1");
		}

		[TestMethod]
		public void AlterSource_Null()
		{
			Person source;
			TextBlock child1;
			TextBlock child2;
			Create(out source, out child1, out child2);

			// Updating a listener with a value the datasource can't accept results in
			// nothing happening
			source.Age = null;
			Assert.AreEqual("66", child1.Text, "#1");
			Assert.AreEqual("77", child2.Text, "#2");
			Assert.IsNull(source.Age, "#3");
		}

		static void Create(out Person source, out TextBlock target1, out TextBlock target2)
		{
			source = new Person { Age = 10 };
			target1 = new TextBlock();
			target2 = new TextBlock();

			target1.SetBinding(TextBlock.TextProperty, new Binding
			{
				FallbackValue = 99,
				Path = new PropertyPath("Age"),
				Mode = BindingMode.TwoWay,
				Source = source,
				StringFormat = "C",
				TargetNullValue = "66",
			});

			target2.SetBinding(TextBlock.TextProperty, new Binding
			{
				FallbackValue = 88,
				Path = new PropertyPath("Age"),
				Mode = BindingMode.TwoWay,
				Source = source,
				StringFormat = "C",
				TargetNullValue = "77",
			});
		}
	}

	public class Person : INotifyPropertyChanged
	{
		public event PropertyChangedEventHandler PropertyChanged;
		
		int? age;
		public int? Age {
			get { return age; }
			set {
				if (age != value) {
					age = value;
					var h = PropertyChanged;
					if (h != null)
						h(this, new PropertyChangedEventArgs("Age"));
				}
			}
		}
	}

	public class BrokenConverter : IValueConverter
	{
		public object Convert(object value, global::System.Type targetType, object parameter, global::System.Globalization.CultureInfo culture)
		{
			return DependencyProperty.UnsetValue;
		}

		public object ConvertBack(object value, global::System.Type targetType, object parameter, global::System.Globalization.CultureInfo culture)
		{
			return DependencyProperty.UnsetValue;
		}
	}
}
