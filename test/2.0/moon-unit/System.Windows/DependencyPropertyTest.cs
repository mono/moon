using System;
using System.Net;
using System.Reflection;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Collections.Generic;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Data;
using Microsoft.Silverlight.Testing;

#pragma warning disable 414
#pragma warning disable 219

namespace MoonTest.System.Windows
{
	[TestClass ()]
	public class DependencyPropertyTest : SilverlightTest
	{
		/*
		 * Declaring Type _ Property Name _ [index, not included in property name _] _ Property Type
		 */
		private DependencyPropertyInfo Canvas_Custom_double;
		private DependencyPropertyInfo Canvas_Custom_nullable_double;
		private DependencyPropertyInfo Canvas_Custom_nullable_bool;
		private DependencyPropertyInfo Canvas_Custom_Canvas;
		private DependencyPropertyInfo Canvas_Custom_CustomClass;
		private DependencyPropertyInfo Canvas_Custom_CustomCanvasType;

		private DependencyPropertyInfo Canvas_Height_int;
		private DependencyPropertyInfo Canvas_Height_double;
		private DependencyPropertyInfo Canvas_Height_CustomClass;
		private DependencyPropertyInfo Canvas_Height_CustomCanvasType;

		private DependencyPropertyInfo CustomClass_Height_int;
		private DependencyPropertyInfo CustomClass_Height_double;
		private DependencyPropertyInfo CustomClass_Height_CustomClass;
		private DependencyPropertyInfo CustomClass_Height_CustomCanvasType;

		private DependencyPropertyInfo CustomCanvasType_Height_int;
		private DependencyPropertyInfo CustomCanvasType_Height_double;
		private DependencyPropertyInfo CustomCanvasType_Height_void;
		private DependencyPropertyInfo CustomCanvasType_Height_Canvas;
		private DependencyPropertyInfo CustomCanvasType_Height_CustomClass;
		private DependencyPropertyInfo CustomCanvasType_Height_CustomCanvasType;
		private DependencyPropertyInfo CustomCanvasType_Height_CustomInterface;
		private DependencyPropertyInfo CustomCanvasType_Height_CustomStruct;
		private DependencyPropertyInfo CustomCanvasType_Height_CustomEnum;
		private DependencyPropertyInfo CustomCanvasType_Height_CustomDelegate;
		private DependencyPropertyInfo CustomCanvasType_Height_CustomClassCtorA;
		private DependencyPropertyInfo CustomCanvasType_Height_CustomClassCtorB;

		private DependencyPropertyInfo CustomCanvasType2_Height_double;
		
		// These are interesting because Height is already defined on FrameworkElement
		private DependencyPropertyInfo FrameworkElement_Height_int;
		private DependencyPropertyInfo FrameworkElement_Height_double;
		private DependencyPropertyInfo FrameworkElement_Height_CustomClass;

		[TestMethod ()]
		public void Register_NullParametersTest ()
		{
			Assert.Throws (delegate { DependencyProperty.Register (null, typeof (int), typeof (Canvas), new PropertyMetadata (null)); }, typeof (ArgumentNullException));
			Assert.Throws (delegate { DependencyProperty.Register (string.Empty, typeof (int), typeof (Canvas), new PropertyMetadata (null)); }, typeof (ArgumentException));
			Assert.Throws (delegate { DependencyProperty.Register ("a", null, typeof (Canvas), new PropertyMetadata (null)); }, typeof (ArgumentNullException));
			Assert.Throws (delegate { DependencyProperty.Register ("a", typeof (int), null, new PropertyMetadata (null)); }, typeof (ArgumentNullException));
			// null PropertyMetadata shouldn't throw.
			DependencyProperty.Register ("a", typeof (int), typeof (Canvas), null);
		}

		[TestMethod ()]
		public void Register_HuhTest1 ()
		{
			DependencyPropertyInfo info = new DependencyPropertyInfo ("Custom", typeof (InkPresenter), typeof (int), false);
			DependencyProperty property = info.Property;
			Canvas canvas = new Canvas ();

			canvas.GetValue (property); // This should throw, the property doesn't exist on the canvas.

			Assert.Throws (delegate { canvas.GetValue (InkPresenter.StrokesProperty); }, typeof (Exception)); // And this throws a catastrophic error.
		}

		static DependencyProperty ValidationOrderProperty = DependencyProperty.Register (
			"ValidationOrder", typeof (double), typeof (ScrollViewer),
			new PropertyMetadata (new PropertyChangedCallback (OnReadOnlyDependencyPropertyChanged)));

		private static void OnReadOnlyDependencyPropertyChanged (DependencyObject d, DependencyPropertyChangedEventArgs e)
		{
			throw new InvalidOperationException ();
		}

		[TestMethod]
		public void ValidationOrder ()
		{
			ScrollViewer sc = new ScrollViewer ();
			// we're setting a boolean value to a double DependencyProperty
			// and even with a PropertyChangedCallback we still get the
			// ArgumentException before the InvalidOperationException
			Assert.Throws<ArgumentException> (delegate {
				sc.SetValue (ValidationOrderProperty, true);
			}, "wrong value, callback is called later");
		}

		[TestMethod]
		[MoonlightBug]
		public void SameReferenceTest ()
		{
			object o = 5;
			string text = "Hi";
			
			// 'Core' types do not preserve the object reference
			Assert.AreNotSame (text, new TextBox { Text = text }.GetValue (TextBox.TextProperty), "#2");
			Assert.AreNotSame (text, new TextBox { Name = text }.GetValue (FrameworkElement.NameProperty), "#3");
			Assert.AreNotSame (text, new TimelineMarker { Type = text }.GetValue (TimelineMarker.TypeProperty), "#4");
			Assert.AreNotSame (o, new ContentControl { Content = o }.GetValue (ContentControl.ContentProperty), "#13");
			Assert.AreNotSame (o, new DiscreteObjectKeyFrame { Value = o }.GetValue (ObjectKeyFrame.ValueProperty), "#13");
			
			// DataContext appears to be the odd one out here - it likes to break the 'rule'
			Assert.AreSame (o, new TextBox { DataContext = o }.GetValue (TextBox.DataContextProperty), "#12");
			
			
			// 'User types' do preserve the object reference
			Assert.AreSame (text, new TextBox { Tag = text }.GetValue(FrameworkElement.TagProperty), "#6");
			Assert.AreSame (o, new TextBox { Tag = o }.GetValue (TextBox.TagProperty), "#11");

			ManagedTestClass c = new ManagedTestClass ();
			c.SetValue (ManagedTestClass.A.Property, text);
			Assert.AreSame (text, c._A_, "#8");

			ListBox box = new ListBox ();
			box.Items.Add (text);
			box.SelectedItem = text;
			Assert.AreSame (text, box.GetValue (ListBox.SelectedItemProperty), "#9");
			Assert.AreSame (text, box.GetValue (ListBox.SelectedItemProperty), "#10");


		}

#region Canvas Custom
		[TestMethod ()]
		public void Custom_Property_Parents ()
		{
			bool buttonLoaded = false;
			CustomCanvas canvas = new CustomCanvas ();
			TestPanel.Children.Add (canvas);
			canvas.Child = new Button { Name = "Ted" };
			canvas.Child2 = new Button { Name = "Ted" };
			canvas.Children.Add (new Button { Name= "Ted" });
		}
		
		[TestMethod ()]
		public void Register_Canvas_Custom_double ()
		{
			Canvas canvas = new Canvas ();
			CustomCanvasType custom_canvas = new CustomCanvasType ();
			DependencyProperty property;
			DependencyPropertyInfo info;
			DependencyPropertyInfo.ChangedInfo changed_info;
			InkPresenter ink = new InkPresenter (); // The only builtin type derived from Canvas
			object actual_value;
			object previous_expected_value = (double) 0;
			int iterations = 0;
			int changes = 0;
			
			Canvas_Custom_double = new DependencyPropertyInfo ("Custom", typeof (Canvas), typeof (double), false);
			info = Canvas_Custom_double;

			property = info.Property;

			Assert.AreEqual (0.0, (double) canvas.GetValue (property));

			Assert.AreEqual (0.0, (double) ink.GetValue (property));

			Assert.Throws (delegate { canvas.SetValue (property, 1); }, typeof (ArgumentException));
			Assert.Throws (delegate { canvas.SetValue (property, ""); }, typeof (ArgumentException));
			Assert.Throws (delegate { canvas.SetValue (property, new CustomClass ()); }, typeof (ArgumentException));
			Assert.Throws (delegate { canvas.SetValue (property, null); }, typeof (ArgumentException));
			Assert.Throws (delegate { canvas.SetValue (property, new Canvas ()); }, typeof (ArgumentException));

			foreach (object expected_value in new object [] { 1.1 }) {
				iterations++;

				canvas.SetValue (property, expected_value);
				actual_value = canvas.GetValue (property);

				if ((double) expected_value != (double) previous_expected_value) {
					changes++;
					changed_info = info.Changes [info.Changes.Count - 1];
					Assert.AreEqual ((double) changed_info.args.OldValue, (double) previous_expected_value);
					Assert.AreEqual ((double) changed_info.args.NewValue, (double) expected_value);
					Assert.AreSame (changed_info.obj, canvas);
				}

				previous_expected_value = expected_value;

				Assert.AreEqual ((double) expected_value, (double) actual_value, "Iteration #{0}", iterations);
				Assert.AreEqual (changes, info.Changes.Count, "Iteration #{0} there should be {1} changes, but there were {2} changes", iterations, changes, info.Changes.Count);
			}
		}
		[TestMethod ()]
		public void Register_Canvas_Custom_nullable_double ()
		{
			Canvas canvas = new Canvas ();
			CustomCanvasType custom_canvas = new CustomCanvasType ();
			DependencyProperty property;
			DependencyPropertyInfo info;
			DependencyPropertyInfo.ChangedInfo changed_info;
			InkPresenter ink = new InkPresenter (); // The only builtin type derived from Canvas
			object actual_value;
			object previous_expected_value = null;
			int iterations = 0;
			int changes = 0;
			
			Canvas_Custom_nullable_double = new DependencyPropertyInfo ("Custom", typeof (Canvas), typeof (Nullable<double>), false);
			info = Canvas_Custom_nullable_double;

			property = info.Property;

			Assert.AreEqual (null, (Nullable<double>) canvas.GetValue (property));

			Assert.AreEqual (null, (Nullable<double>) ink.GetValue (property));

			Assert.Throws (delegate { canvas.SetValue (property, 1); }, typeof (ArgumentException));
			Assert.Throws (delegate { canvas.SetValue (property, ""); }, typeof (ArgumentException));
			Assert.Throws (delegate { canvas.SetValue (property, new CustomClass ()); }, typeof (ArgumentException));
			Assert.Throws (delegate { canvas.SetValue (property, new Canvas ()); }, typeof (ArgumentException));

			foreach (object expected_value in new object [] { (Nullable<double>) 1.1, null, 2.2 }) {
				iterations++;
				
				canvas.SetValue (property, expected_value);
				actual_value = canvas.GetValue (property);

				if ((Nullable<double>) expected_value != (Nullable<double>) previous_expected_value) {
					changes++;
					changed_info = info.Changes [info.Changes.Count - 1];
					Assert.AreEqual ((Nullable<double>) changed_info.args.OldValue, (Nullable<double>) previous_expected_value);
					Assert.AreEqual ((Nullable<double>) changed_info.args.NewValue, (Nullable<double>) expected_value);
					Assert.AreSame (changed_info.obj, canvas);
				}

				previous_expected_value = expected_value;

				Assert.AreEqual ((Nullable<double>) expected_value, (Nullable<double>) actual_value, "Iteration #{0}", iterations);
				Assert.AreEqual (changes, info.Changes.Count, "Iteration #{0} there should be {1} changes, but there were {2} changes", iterations, changes, info.Changes.Count);
			}			
		}
		
		[TestMethod ()]
		public void Register_Canvas_Custom_nullable_bool ()
		{
			Canvas canvas = new Canvas ();
			CustomCanvasType custom_canvas = new CustomCanvasType ();
			DependencyProperty property;
			DependencyPropertyInfo info;
			DependencyPropertyInfo.ChangedInfo changed_info;
			InkPresenter ink = new InkPresenter (); // The only builtin type derived from Canvas
			object actual_value;
			object previous_expected_value = null;
			int iterations = 0;
			int changes = 0;
			
			Canvas_Custom_nullable_bool = new DependencyPropertyInfo ("Custom", typeof (Canvas), typeof (Nullable<bool>), false);
			info = Canvas_Custom_nullable_bool;

			property = info.Property;

			Assert.AreEqual (null, (Nullable<bool>) canvas.GetValue (property));

			Assert.AreEqual (null, (Nullable<bool>) ink.GetValue (property));

			Assert.Throws (delegate { canvas.SetValue (property, 1); }, typeof (ArgumentException));
			Assert.Throws (delegate { canvas.SetValue (property, ""); }, typeof (ArgumentException));
			Assert.Throws (delegate { canvas.SetValue (property, new CustomClass ()); }, typeof (ArgumentException));
			Assert.Throws (delegate { canvas.SetValue (property, new Canvas ()); }, typeof (ArgumentException));

			foreach (object expected_value in new object [] { (Nullable<bool>) true, null, false, null, (Nullable<bool>) false, (Nullable<bool>) true}) {
				iterations++;
				
				canvas.SetValue (property, expected_value);
				actual_value = canvas.GetValue (property);

				if ((Nullable<bool>) expected_value != (Nullable<bool>) previous_expected_value) {
					changes++;
					changed_info = info.Changes [info.Changes.Count - 1];
					Assert.AreEqual ((Nullable<bool>) changed_info.args.OldValue, (Nullable<bool>) previous_expected_value);
					Assert.AreEqual ((Nullable<bool>) changed_info.args.NewValue, (Nullable<bool>) expected_value);
					Assert.AreSame (changed_info.obj, canvas);
				}

				previous_expected_value = expected_value;

				Assert.AreEqual ((Nullable<bool>) expected_value, (Nullable<bool>) actual_value, "Iteration #{0}", iterations);
				Assert.AreEqual (changes, info.Changes.Count, "Iteration #{0} there should be {1} changes, but there were {2} changes", iterations, changes, info.Changes.Count);
			}			
		}
		
		[TestMethod ()]
		public void Register_Canvas_Custom_CustomClass ()
		{
			Canvas_Custom_CustomClass = new DependencyPropertyInfo ("Custom", typeof (Canvas), typeof (CustomClass), false);
		}

		[TestMethod ()]
		public void Register_Canvas_Custom_Canvas ()
		{
			Canvas canvas = new Canvas ();
			CustomCanvasType custom_canvas = new CustomCanvasType ();
			DependencyProperty property;
			DependencyPropertyInfo info;
			DependencyPropertyInfo.ChangedInfo changed_info;
			InkPresenter ink = new InkPresenter (); // The only builtin type derived from Canvas
			object actual_value;
			object previous_expected_value = null;
			int iterations = 0;
			int changes = 0;

			Canvas_Custom_Canvas = new DependencyPropertyInfo ("Custom", typeof (Canvas), typeof (Canvas), false);
			info = Canvas_Custom_Canvas;

			property = info.Property;

			Assert.IsNull (canvas.GetValue (property));
			Assert.IsNull (ink.GetValue (property));

			Assert.Throws (delegate { canvas.SetValue (property, 1); }, typeof (ArgumentException));
			Assert.Throws (delegate { canvas.SetValue (property, ""); }, typeof (ArgumentException));
			Assert.Throws (delegate { canvas.SetValue (property, new CustomClass ()); }, typeof (ArgumentException));

			foreach (object expected_value in new object [] { null, new Canvas (), null, canvas, canvas, null, new CustomCanvasType (), custom_canvas, custom_canvas, ink}) {
				iterations++;

				canvas.SetValue (property, expected_value);
				actual_value = canvas.GetValue (property);

				if (expected_value != previous_expected_value) {
					changes++;
					changed_info = info.Changes [info.Changes.Count - 1];
					Assert.AreSame (changed_info.args.OldValue, previous_expected_value);
					Assert.AreSame (changed_info.args.NewValue, expected_value);
					Assert.AreSame (changed_info.obj, canvas);
				}

				previous_expected_value = expected_value;

				Assert.AreSame (expected_value, actual_value, "Iteration #{0}", iterations);
				Assert.AreEqual (changes, info.Changes.Count, "Iteration #{0} there should be {1} changes, but there were {2} changes", iterations, changes, info.Changes.Count);
			}
		}
		
		[TestMethod ()]
		public void Register_Canvas_Custom_CustomCanvasType ()
		{
			Canvas_Custom_CustomCanvasType = new DependencyPropertyInfo ("Custom", typeof (Canvas), typeof (CustomCanvasType), false);
		}
#endregion
#region Canvas Height
		[TestMethod ()]
		public void Register_Canvas_Height_int ()
		{
			// Register a custom property with the same name and type as an existing builtin property
			Canvas_Height_int = new DependencyPropertyInfo ("Height", typeof (Canvas), typeof (int), false);
		}

		[TestMethod ()]
		public void Register_Canvas_Height_double ()
		{
			// Register a custom property with the same name and type as an existing builtin property
			Canvas_Height_double = new DependencyPropertyInfo ("Height", typeof (Canvas), typeof (double), false);
		}

		[TestMethod ()]
		public void Register_Canvas_Height_CustomClass ()
		{
			// Register a custom property with the same name (but not type) as an existing builtin AND another custom property
			Canvas_Height_CustomClass = new DependencyPropertyInfo ("Height", typeof (Canvas), typeof (CustomClass), false);
		}

		[TestMethod ()]
		public void Register_Canvas_Height_CustomCanvasType ()
		{
			// Register a custom property with the same name (but not type) as an existing builtin AND another custom property
			Canvas_Height_CustomCanvasType = new DependencyPropertyInfo ("Height", typeof (Canvas), typeof (CustomCanvasType), false);
		}
#endregion
#region CustomClass Height
		[TestMethod ()]
		public void Register_CustomClass_Height_int ()
		{
			CustomClass_Height_int = new DependencyPropertyInfo ("Height", typeof (CustomClass), typeof (int), false);
		}

		[TestMethod ()]
		public void Register_CustomClass_Height_double ()
		{
			CustomClass_Height_double = new DependencyPropertyInfo ("Height", typeof (CustomClass), typeof (double), false);
		}

		[TestMethod ()]
		public void Register_CustomClass_Height_CustomClass ()
		{
			CustomClass_Height_CustomClass = new DependencyPropertyInfo ("Height", typeof (CustomClass), typeof (CustomClass), false);
		}

		[TestMethod ()]
		public void Register_CustomClass_Height_CustomCanvasType ()
		{
			CustomClass_Height_CustomCanvasType = new DependencyPropertyInfo ("Height", typeof (CustomClass), typeof (CustomCanvasType), false);
		}
#endregion
#region CustomCanvasType Height
		[TestMethod ()]
		public void Register_CustomCanvasType_Height_int ()
		{
			CustomCanvasType the_object = new CustomCanvasType ();
			CustomCanvasType custom_canvas = new CustomCanvasType ();
			Canvas canvas = new Canvas ();
			DependencyProperty property;
			DependencyPropertyInfo info;
			DependencyPropertyInfo.ChangedInfo changed_info;
			InkPresenter ink = new InkPresenter (); // The only builtin type derived from Canvas
			object actual_value;
			object previous_expected_value = (int) 0;
			int iterations = 0;
			int changes = 0;

			CustomCanvasType_Height_int = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (int), false);
			info = CustomCanvasType_Height_int;

			property = info.Property;

			Assert.AreEqual (0, (int) the_object.GetValue (property));
			Assert.AreEqual (0, (int) ink.GetValue (property));

			Assert.Throws (delegate { the_object.SetValue (property, 1.1); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, "1"); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, ""); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new CustomClass ()); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, null); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new Canvas ()); }, typeof (ArgumentException));

			foreach (object expected_value in new object [] { 1, 1, 2 }) {
				iterations++;

				the_object.SetValue (property, expected_value);
				actual_value = the_object.GetValue (property);

				if ((int) expected_value != (int) previous_expected_value) {
					changes++;
					changed_info = info.Changes [info.Changes.Count - 1];
					Assert.AreEqual ((int) changed_info.args.OldValue, (int) previous_expected_value);
					Assert.AreEqual ((int) changed_info.args.NewValue, (int) expected_value);
					Assert.AreSame (changed_info.obj, the_object);
				}

				previous_expected_value = expected_value;

				Assert.AreEqual ((int) expected_value, (int) actual_value, "Iteration #{0}", iterations);
				Assert.AreEqual (changes, info.Changes.Count, "Iteration #{0} there should be {1} changes, but there were {2} changes", iterations, changes, info.Changes.Count);
			}
		}

		[TestMethod ()]
		public void Register_CustomCanvasType_Height_double ()
		{
			CustomCanvasType the_object = new CustomCanvasType ();
			CustomCanvasType custom_canvas = new CustomCanvasType ();
			Canvas canvas = new Canvas ();
			DependencyProperty property;
			DependencyPropertyInfo info;
			DependencyPropertyInfo.ChangedInfo changed_info;
			InkPresenter ink = new InkPresenter (); // The only builtin type derived from Canvas
			object actual_value;
			object previous_expected_value = (double) 0;
			int iterations = 0;
			int changes = 0;

			CustomCanvasType_Height_double = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (double), false);
			info = CustomCanvasType_Height_double;

			property = info.Property;

			Assert.AreEqual (0.0, (double) the_object.GetValue (property));
			Assert.AreEqual (0.0, (double) ink.GetValue (property));

			Assert.Throws (delegate { the_object.SetValue (property, 1); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, ""); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new CustomClass ()); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, null); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new Canvas ()); }, typeof (ArgumentException));

			foreach (object expected_value in new object [] { 1.1, 1.1, 2.2 }) {
				iterations++;

				the_object.SetValue (property, expected_value);
				actual_value = the_object.GetValue (property);

				if ((double) expected_value != (double) previous_expected_value) {
					changes++;
					changed_info = info.Changes [info.Changes.Count - 1];
					Assert.AreEqual ((double) changed_info.args.OldValue, (double) previous_expected_value);
					Assert.AreEqual ((double) changed_info.args.NewValue, (double) expected_value);
					Assert.AreSame (changed_info.obj, the_object);
				}

				previous_expected_value = expected_value;

				Assert.AreEqual ((double) expected_value, (double) actual_value, "Iteration #{0}", iterations);
				Assert.AreEqual (changes, info.Changes.Count, "Iteration #{0} there should be {1} changes, but there were {2} changes", iterations, changes, info.Changes.Count);
			}
		}

		[TestMethod ()]
		public void Register_CustomCanvasType_Height_void ()
		{
			Assert.Throws (delegate { CustomCanvasType_Height_void = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (void), false);}, typeof (NotSupportedException));
		}

		[TestMethod ()]
		public void Register_CustomCanvasType_Height_Canvas ()
		{
			CustomCanvasType_Height_Canvas = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (Canvas), false);
		}

		[TestMethod ()]
		public void Register_CustomCanvasType_Height_CustomClass ()
		{
			CustomCanvasType_Height_CustomClass = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomClass), false);
		}

		[TestMethod ()]
		public void Register_CustomCanvasType_Height_CustomCanvasType ()
		{
			CustomCanvasType_Height_CustomCanvasType = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomCanvasType), false);
		}

		[TestMethod ()]
		public void Register_CustomCanvasType_Height_CustomInterface ()
		{
			CustomCanvasType the_object = new CustomCanvasType ();
			CustomCanvasType custom_canvas = new CustomCanvasType ();
			Canvas canvas = new Canvas ();
			CustomStruct custom_struct_1 = new CustomStruct (1);
			CustomEnum custom_enum = CustomEnum.EnumValue1;
			CustomDelegate custom_delegate = delegate { };
			CustomInterface custom_interface_a = new CustomInterfaceImplA ();
			CustomInterface custom_interface_b = new CustomInterfaceImplB ();
			DependencyProperty property;
			DependencyPropertyInfo info;
			DependencyPropertyInfo.ChangedInfo changed_info;
			InkPresenter ink = new InkPresenter (); // The only builtin type derived from Canvas
			object actual_value;
			object previous_expected_value = null;
			int iterations = 0;
			int changes = 0;

			CustomCanvasType_Height_CustomInterface = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomInterface), false);
			info = CustomCanvasType_Height_CustomInterface;

			property = info.Property;

			Assert.AreEqual (null, the_object.GetValue (property), "Default value 1");
			Assert.AreEqual (null, ink.GetValue (property), "Default value 2");

			Assert.Throws (delegate { the_object.SetValue (property, 0); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, 1); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, ""); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new CustomClass ()); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new Canvas ()); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, custom_enum); }, typeof (ArgumentException));

			foreach (object expected_value in new object [] { null, custom_interface_a, null, custom_interface_b, custom_interface_b, null }) {
				iterations++;

				the_object.SetValue (property, expected_value);
				actual_value = the_object.GetValue (property);

				if (!object.Equals (expected_value, previous_expected_value)) {
					changes++;
					changed_info = info.Changes [info.Changes.Count - 1];
					Assert.AreEqual (changed_info.args.OldValue, previous_expected_value, "OldValue, iteration: " + iterations.ToString ());
					Assert.AreEqual (changed_info.args.NewValue, expected_value, "NewValue, iteration: " + iterations.ToString ());
					Assert.AreSame (changed_info.obj, the_object);
				}

				previous_expected_value = expected_value;

				Assert.AreEqual (expected_value, actual_value, "Iteration #{0}", iterations);
				Assert.AreEqual (changes, info.Changes.Count, "Iteration #{0} there should be {1} changes, but there were {2} changes", iterations, changes, info.Changes.Count);
			}
		}

		[TestMethod ()]
		public void Register_CustomCanvasType_Height_CustomStruct ()
		{
			CustomCanvasType the_object = new CustomCanvasType ();
			CustomCanvasType custom_canvas = new CustomCanvasType ();
			Canvas canvas = new Canvas ();
			CustomStruct custom_struct_1 = new CustomStruct (1);
			DependencyProperty property;
			DependencyPropertyInfo info;
			DependencyPropertyInfo.ChangedInfo changed_info;
			InkPresenter ink = new InkPresenter (); // The only builtin type derived from Canvas
			object actual_value;
			object previous_expected_value = new CustomStruct ();
			int iterations = 0;
			int changes = 0;

			CustomCanvasType_Height_CustomStruct = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomStruct), false);
			info = CustomCanvasType_Height_CustomStruct;

			property = info.Property;

			Assert.AreEqual (new CustomStruct (), (CustomStruct) the_object.GetValue (property));
			Assert.AreEqual (new CustomStruct (), (CustomStruct) ink.GetValue (property));

			Assert.Throws (delegate { the_object.SetValue (property, 1); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, ""); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new CustomClass ()); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, null); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new Canvas ()); }, typeof (ArgumentException));

			foreach (object expected_value in new object [] { custom_struct_1, custom_struct_1, new CustomStruct (), new CustomStruct () }) {
				iterations++;

				the_object.SetValue (property, expected_value);
				actual_value = the_object.GetValue (property);

				if (!object.Equals (expected_value, previous_expected_value)) {
					changes++;
					changed_info = info.Changes [info.Changes.Count - 1];
					Assert.AreEqual (changed_info.args.OldValue, previous_expected_value);
					Assert.AreEqual (changed_info.args.NewValue, expected_value);
					Assert.AreSame (changed_info.obj, the_object);
				}

				previous_expected_value = expected_value;

				Assert.AreEqual (expected_value, actual_value, "Iteration #{0}", iterations);
				Assert.AreEqual (changes, info.Changes.Count, "Iteration #{0} there should be {1} changes, but there were {2} changes", iterations, changes, info.Changes.Count);
			}
		}

		[TestMethod ()]
		public void Register_CustomCanvasType_Height_CustomEnum ()
		{
			CustomCanvasType the_object = new CustomCanvasType ();
			CustomCanvasType custom_canvas = new CustomCanvasType ();
			Canvas canvas = new Canvas ();
			CustomStruct custom_struct_1 = new CustomStruct (1);
			CustomEnum custom_enum = CustomEnum.EnumValue1;
			DependencyProperty property;
			DependencyPropertyInfo info;
			DependencyPropertyInfo.ChangedInfo changed_info;
			InkPresenter ink = new InkPresenter (); // The only builtin type derived from Canvas
			object actual_value;
			object previous_expected_value = (CustomEnum) 0;
			int iterations = 0;
			int changes = 0;

			CustomCanvasType_Height_CustomEnum = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomEnum), false);
			info = CustomCanvasType_Height_CustomEnum;

			property = info.Property;

			Assert.AreEqual ((CustomEnum) 0, the_object.GetValue (property), "Default value 1");
			Assert.AreEqual ((CustomEnum) 0, ink.GetValue (property), "Default value 2");

			Assert.Throws (delegate { the_object.SetValue (property, 0); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, 1); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, ""); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new CustomClass ()); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, null); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new Canvas ()); }, typeof (ArgumentException));

			foreach (object expected_value in new object [] { CustomEnum.EnumValue1, CustomEnum.EnumValue1, CustomEnum.EnumValue2, (CustomEnum) 0, custom_enum, custom_enum}) {
				iterations++;

				the_object.SetValue (property, expected_value);
				actual_value = the_object.GetValue (property);

				if (!object.Equals (expected_value, previous_expected_value)) {
					changes++;
					changed_info = info.Changes [info.Changes.Count - 1];
					Assert.AreEqual (changed_info.args.OldValue, previous_expected_value, "OldValue");
					Assert.AreEqual (changed_info.args.NewValue, expected_value, "NewValue");
					Assert.AreSame (changed_info.obj, the_object);
				}

				previous_expected_value = expected_value;

				Assert.AreEqual (expected_value, actual_value, "Iteration #{0}", iterations);
				Assert.AreEqual (changes, info.Changes.Count, "Iteration #{0} there should be {1} changes, but there were {2} changes", iterations, changes, info.Changes.Count);
			}
		}

		[TestMethod ()]
		public void Register_CustomCanvasType_Height_CustomDelegate ()
		{
			CustomCanvasType the_object = new CustomCanvasType ();
			CustomCanvasType custom_canvas = new CustomCanvasType ();
			Canvas canvas = new Canvas ();
			CustomStruct custom_struct_1 = new CustomStruct (1);
			CustomEnum custom_enum = CustomEnum.EnumValue1;
			CustomDelegate custom_delegate = delegate { };
			DependencyProperty property;
			DependencyPropertyInfo info;
			DependencyPropertyInfo.ChangedInfo changed_info;
			InkPresenter ink = new InkPresenter (); // The only builtin type derived from Canvas
			object actual_value;
			object previous_expected_value = null;
			int iterations = 0;
			int changes = 0;

			CustomCanvasType_Height_CustomDelegate = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomDelegate), false);
			info = CustomCanvasType_Height_CustomDelegate;

			property = info.Property;

			Assert.AreEqual (null, the_object.GetValue (property), "Default value 1");
			Assert.AreEqual (null, ink.GetValue (property), "Default value 2");

			Assert.Throws (delegate { the_object.SetValue (property, 0); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, 1); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, ""); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new CustomClass ()); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new Canvas ()); }, typeof (ArgumentException));

			foreach (object expected_value in new CustomDelegate [] { null, delegate {}, delegate {}, custom_delegate, custom_delegate }) {
				iterations++;

				the_object.SetValue (property, expected_value);
				actual_value = the_object.GetValue (property);

				if (!object.Equals (expected_value, previous_expected_value)) {
					changes++;
					changed_info = info.Changes [info.Changes.Count - 1];
					Assert.AreEqual (changed_info.args.OldValue, previous_expected_value, "OldValue");
					Assert.AreEqual (changed_info.args.NewValue, expected_value, "NewValue");
					Assert.AreSame (changed_info.obj, the_object);
				}

				previous_expected_value = expected_value;

				Assert.AreEqual (expected_value, actual_value, "Iteration #{0}", iterations);
				Assert.AreEqual (changes, info.Changes.Count, "Iteration #{0} there should be {1} changes, but there were {2} changes", iterations, changes, info.Changes.Count);
			}
		}

		[TestMethod ()]
		public void Register_CustomCanvasType_Height_CustomClassCtorA ()
		{
			CustomCanvasType_Height_CustomClassCtorA = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomClassCtorA), false);
		}

		[TestMethod ()]
		public void Register_CustomCanvasType_Height_CustomClassCtorB ()
		{
			CustomCanvasType_Height_CustomClassCtorB = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomClassCtorB), false);
		}

		///////////////////////////////////////////////////////

#endregion
#region CustomCanvasType2 Height

		[TestMethod ()]
		public void Register_CustomCanvasType2_Height_double ()
		{
			CustomCanvasType2 the_object = new CustomCanvasType2 ();
			CustomCanvasType custom_canvas = new CustomCanvasType ();
			Canvas canvas = new Canvas ();
			DependencyProperty property;
			DependencyPropertyInfo info;
			DependencyPropertyInfo.ChangedInfo changed_info;
			InkPresenter ink = new InkPresenter (); // The only builtin type derived from Canvas
			object actual_value;
			object previous_expected_value = (double) 0;
			int iterations = 0;
			int changes = 0;

			CustomCanvasType2_Height_double = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType2), typeof (double), false);
			info = CustomCanvasType2_Height_double;

			property = info.Property;

			Assert.AreEqual (0.0, (double) the_object.GetValue (property));
			Assert.AreEqual (0.0, (double) ink.GetValue (property));
			Assert.AreEqual (0.0, (double) custom_canvas.GetValue (property)); 

			Assert.Throws (delegate { the_object.SetValue (property, 1); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, ""); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new CustomClass ()); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, null); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new Canvas ()); }, typeof (ArgumentException));

			Assert.Throws (delegate { custom_canvas.SetValue (property, 1.1); }, typeof (ArgumentException));

			foreach (object expected_value in new object [] { 1.1 }) {
				iterations++;

				the_object.SetValue (property, expected_value);
				actual_value = the_object.GetValue (property);

				if ((double) expected_value != (double) previous_expected_value) {
					changes++;
					changed_info = info.Changes [info.Changes.Count - 1];
					Assert.AreEqual ((double) changed_info.args.OldValue, (double) previous_expected_value);
					Assert.AreEqual ((double) changed_info.args.NewValue, (double) expected_value);
					Assert.AreSame (changed_info.obj, the_object);
				}

				previous_expected_value = expected_value;

				Assert.AreEqual ((double) expected_value, (double) actual_value, "Iteration #{0}", iterations);
				Assert.AreEqual (changes, info.Changes.Count, "Iteration #{0} there should be {1} changes, but there were {2} changes", iterations, changes, info.Changes.Count);
			}
		}
#endregion
		#region FrameworkElement Height
		[TestMethod ()]
		public void Register_FrameworkElement_Height_int ()
		{
			FrameworkElement_Height_int = new DependencyPropertyInfo ("Height", typeof (FrameworkElement), typeof (int), false);
		}

		[TestMethod ()]
		public void Register_FrameworkElement_Height_double ()
		{
			FrameworkElement_Height_double = new DependencyPropertyInfo ("Height", typeof (FrameworkElement), typeof (double), false);
		}

		[TestMethod ()]
		public void Register_FrameworkElement_Height_CustomClass ()
		{
			FrameworkElement_Height_CustomClass = new DependencyPropertyInfo ("Height", typeof (FrameworkElement), typeof (CustomClass), false);
		}
#endregion

#region DefaultValue tests
		[TestMethod ()]
		public void DefaultValue_Run_Foreground ()
		{
			Assert.Throws<UnauthorizedAccessException> (delegate {
					Run r = new Run ();
					((SolidColorBrush)r.Foreground).Color = Colors.Blue;
				}, "modifying the default value of a property should throw an exception");
		}
#endregion

#region Custom types, etc
		

		public class CustomCanvasType : Canvas
		{
		}
		public class CustomCanvasType2 : Canvas
		{
		}
		public class CustomDerivedClass : CustomClass
		{
		}
		
		public class CustomClass 
		{
		}
		public struct CustomStruct
		{
			public int value;

			public CustomStruct (int v) { this.value = v; }

			public override string ToString ()
			{
				return "<CustomStruct (" + value.ToString () + ")>";
			}
		}
		public interface CustomInterface
		{
			void Method ();
		}
		public class CustomInterfaceImplA : CustomInterface
		{
			public void Method ()
			{
				throw new NotImplementedException ();
			}
		}
		public class CustomInterfaceImplB : CustomInterface
		{
			public void Method ()
			{
				throw new NotImplementedException ();
			}
		}

		public delegate void CustomDelegate ();
		public enum CustomEnum
		{
			EnumValue1 = 1,
			EnumValue2 = 2
		}
		// A class with no default ctor
		public class CustomClassCtorA
		{
			public CustomClassCtorA (string an_argument) { }
		}
		// A class with a private default ctor
		public class CustomClassCtorB
		{
			private CustomClassCtorB () {}
		}
#endregion

#region Managed Test
		[TestMethod]
		[MoonlightBug]
		public void ManagedTest_A ()
		{
			ManagedTestClass mtc;
			string format = "'{1}' => '{2}'";
			string sep = " ; ";

			// A

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_A1.xaml");
			Assert.AreEqual ("ok", mtc._A_, "A1-A");
			Assert.AreEqual ("b", mtc._b_, "A1-b");
			Assert.AreEqual ("c", mtc._c_, "A1-c");
			Assert.AreEqual ("C", mtc._C_, "A1-C");
			Assert.AreEqual ("D", mtc._D_1_, "A1-D1");
			Assert.AreEqual ("d", mtc._d_2_, "A1-D2");
			Assert.AreEqual ("d", mtc._d_3_, "A1-D3");
			Assert.AreEqual ("'A' => 'ok'", ManagedTestClass.A.ChangesToString (format, sep), "A1-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "A1-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "A1-c-Changes");
			Assert.AreEqual ("", ManagedTestClass.C.ChangesToString (format, sep), "A1-C-Changes");
			Assert.AreEqual ("", ManagedTestClass.D_1.ChangesToString (format, sep), "A1-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "A1-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "A1-d3-Changes");

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_A2.xaml");
			Assert.AreEqual ("ok", mtc._A_, "A2-A");
			Assert.AreEqual ("b", mtc._b_, "A2-b");
			Assert.AreEqual ("c", mtc._c_, "A2-c");
			Assert.AreEqual ("C", mtc._C_, "A2-C");
			Assert.AreEqual ("D", mtc._D_1_, "A2-D1");
			Assert.AreEqual ("d", mtc._d_2_, "A2-D2");
			Assert.AreEqual ("d", mtc._d_3_, "A2-D3");
			Assert.AreEqual ("'A' => 'ok'", ManagedTestClass.A.ChangesToString (format, sep), "A2-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "A2-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "A2-c-Changes");
			Assert.AreEqual ("", ManagedTestClass.C.ChangesToString (format, sep), "A2-C-Changes");
			Assert.AreEqual ("", ManagedTestClass.D_1.ChangesToString (format, sep), "A2-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "A2-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "A2-d3-Changes");


			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_A3.xaml");
			Assert.AreEqual ("ok-2", mtc._A_, "A3-A");
			Assert.AreEqual ("b", mtc._b_, "A3-b");
			Assert.AreEqual ("c", mtc._c_, "A3-c");
			Assert.AreEqual ("C", mtc._C_, "A3-C");
			Assert.AreEqual ("D", mtc._D_1_, "A3-D1");
			Assert.AreEqual ("d", mtc._d_2_, "A3-D2");
			Assert.AreEqual ("d", mtc._d_3_, "A3-D3");
			Assert.AreEqual ("'A' => 'ok-1' ; 'ok-1' => 'ok-2' ; 'ok-2' => 'ok-1' ; 'ok-1' => 'ok-2'", ManagedTestClass.A.ChangesToString (format, sep), "A3-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "A3-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "A3-c-Changes");
			Assert.AreEqual ("", ManagedTestClass.C.ChangesToString (format, sep), "A3-C-Changes");
			Assert.AreEqual ("", ManagedTestClass.D_1.ChangesToString (format, sep), "A3-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "A3-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "A3-d3-Changes");

		}

		[TestMethod]
		[MoonlightBug]
		public void ManagedTest_B ()
		{
			ManagedTestClass mtc;
			string format = "'{1}' => '{2}'";
			string sep = " ; ";

			// B

			Assert.Throws<XamlParseException> (delegate () { mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_B1.xaml"); });

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_B2.xaml");
			Assert.AreEqual ("A", mtc._A_, "B2-A");
			Assert.AreEqual ("ok", mtc._b_, "B2-b");
			Assert.AreEqual ("c", mtc._c_, "B2-c");
			Assert.AreEqual ("C", mtc._C_, "B2-C");
			Assert.AreEqual ("D", mtc._D_1_, "B2-D1");
			Assert.AreEqual ("d", mtc._d_2_, "B2-D2");
			Assert.AreEqual ("d", mtc._d_3_, "B2-D3");
			Assert.AreEqual ("", ManagedTestClass.A.ChangesToString (format, sep), "B2-A-Changes");
			Assert.AreEqual ("'b' => 'ok'", ManagedTestClass.b.ChangesToString (format, sep), "B2-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "B2-c-Changes");
			Assert.AreEqual ("", ManagedTestClass.C.ChangesToString (format, sep), "B2-C-Changes");
			Assert.AreEqual ("", ManagedTestClass.D_1.ChangesToString (format, sep), "B2-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "B2-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "B2-d3-Changes");

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_B3.xaml");
			Assert.AreEqual ("A", mtc._A_, "B3-A");
			Assert.AreEqual ("ok-2", mtc._b_, "B3-b");
			Assert.AreEqual ("c", mtc._c_, "B3-c");
			Assert.AreEqual ("C", mtc._C_, "B3-C");
			Assert.AreEqual ("D", mtc._D_1_, "B3-D1");
			Assert.AreEqual ("d", mtc._d_2_, "B3-D2");
			Assert.AreEqual ("d", mtc._d_3_, "B3-D3");
			Assert.AreEqual ("", ManagedTestClass.A.ChangesToString (format, sep), "B3-A-Changes");
			Assert.AreEqual ("'b' => 'ok-1' ; 'ok-1' => 'ok-2' ; 'ok-2' => 'ok-1' ; 'ok-1' => 'ok-2'", ManagedTestClass.b.ChangesToString (format, sep), "B3-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "B3-c-Changes");
			Assert.AreEqual ("", ManagedTestClass.C.ChangesToString (format, sep), "B3-C-Changes");
			Assert.AreEqual ("", ManagedTestClass.D_1.ChangesToString (format, sep), "B3-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "B3-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "B3-d3-Changes");
		}

		[TestMethod]
		[MoonlightBug]
		public void ManagedTest_C ()
		{
			ManagedTestClass mtc;
			string format = "'{1}' => '{2}'";
			string sep = " ; ";
			// C

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_C1.xaml");
			Assert.AreEqual ("A", mtc._A_, "C1-A");
			Assert.AreEqual ("b", mtc._b_, "C1-b");
			Assert.AreEqual ("c", mtc._c_, "C1-c");
			Assert.AreEqual ("ok", mtc._C_, "C1-C");
			Assert.AreEqual ("D", mtc._D_1_, "C1-D1");
			Assert.AreEqual ("d", mtc._d_2_, "C1-D2");
			Assert.AreEqual ("d", mtc._d_3_, "C1-D3");
			Assert.AreEqual ("", ManagedTestClass.A.ChangesToString (format, sep), "C1-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "C1-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "C1-c-Changes");
			Assert.AreEqual ("'C' => 'ok'", ManagedTestClass.C.ChangesToString (format, sep), "C1-C-Changes");
			Assert.AreEqual ("", ManagedTestClass.D_1.ChangesToString (format, sep), "C1-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "C1-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "C1-d3-Changes");

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_C2.xaml");
			Assert.AreEqual ("A", mtc._A_, "C2-A");
			Assert.AreEqual ("b", mtc._b_, "C2-b");
			Assert.AreEqual ("c", mtc._c_, "C2-c");
			Assert.AreEqual ("ok", mtc._C_, "C2-C");
			Assert.AreEqual ("D", mtc._D_1_, "C2-D1");
			Assert.AreEqual ("d", mtc._d_2_, "C2-D2");
			Assert.AreEqual ("d", mtc._d_3_, "C2-D3");
			Assert.AreEqual ("", ManagedTestClass.A.ChangesToString (format, sep), "C2-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "C2-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "C2-c-Changes");
			Assert.AreEqual ("'C' => 'ok'", ManagedTestClass.C.ChangesToString (format, sep), "C2-C-Changes");
			Assert.AreEqual ("", ManagedTestClass.D_1.ChangesToString (format, sep), "C2-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "C2-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "C2-d3-Changes");

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_C3.xaml");
			Assert.AreEqual ("A", mtc._A_, "C3-A");
			Assert.AreEqual ("b", mtc._b_, "C3-b");
			Assert.AreEqual ("c", mtc._c_, "C3-c");
			Assert.AreEqual ("ok-2", mtc._C_, "C3-C");
			Assert.AreEqual ("D", mtc._D_1_, "C3-D1");
			Assert.AreEqual ("d", mtc._d_2_, "C3-D2");
			Assert.AreEqual ("d", mtc._d_3_, "C3-D3");
			Assert.AreEqual ("", ManagedTestClass.A.ChangesToString (format, sep), "C3-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "C3-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "C3-c-Changes");
			Assert.AreEqual ("'C' => 'ok-1' ; 'ok-1' => 'ok-2' ; 'ok-2' => 'ok-1' ; 'ok-1' => 'ok-2'", ManagedTestClass.C.ChangesToString (format, sep), "C3-C-Changes");
			Assert.AreEqual ("", ManagedTestClass.D_1.ChangesToString (format, sep), "C3-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "C3-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "C3-d3-Changes");
		}

		[TestMethod]
		[MoonlightBug]
		public void ManagedTest_D ()
		{
			ManagedTestClass mtc;
			string format = "'{1}' => '{2}'";
			string sep = " ; ";

			// D

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_D1.xaml");
			Assert.AreEqual ("A", mtc._A_, "D1-A");
			Assert.AreEqual ("b", mtc._b_, "D1-b");
			Assert.AreEqual ("c", mtc._c_, "D1-c");
			Assert.AreEqual ("C", mtc._C_, "D1-C");
			Assert.AreEqual ("ok", mtc._D_1_, "D1-D1");
			Assert.AreEqual ("d", mtc._d_2_, "D1-D2");
			Assert.AreEqual ("d", mtc._d_3_, "D1-D3");
			Assert.AreEqual ("", ManagedTestClass.A.ChangesToString (format, sep), "D1-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "D1-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "D1-c-Changes");
			Assert.AreEqual ("", ManagedTestClass.C.ChangesToString (format, sep), "D1-C-Changes");
			Assert.AreEqual ("'D' => 'ok'", ManagedTestClass.D_1.ChangesToString (format, sep), "D1-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "D1-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "C3-d3-Changes");

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_D2.xaml");
			Assert.AreEqual ("A", mtc._A_, "D2-A");
			Assert.AreEqual ("b", mtc._b_, "D2-b");
			Assert.AreEqual ("c", mtc._c_, "D2-c");
			Assert.AreEqual ("C", mtc._C_, "D2-C");
			Assert.AreEqual ("ok", mtc._D_1_, "D2-D1");
			Assert.AreEqual ("d", mtc._d_2_, "D2-D2");
			Assert.AreEqual ("d", mtc._d_3_, "D2-D3");
			Assert.AreEqual ("", ManagedTestClass.A.ChangesToString (format, sep), "D2-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "D2-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "D2-c-Changes");
			Assert.AreEqual ("", ManagedTestClass.C.ChangesToString (format, sep), "D2-C-Changes");
			Assert.AreEqual ("'D' => 'ok'", ManagedTestClass.D_1.ChangesToString (format, sep), "D2-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "D2-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "D2-d3-Changes");

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_D3.xaml");
			Assert.AreEqual ("A", mtc._A_, "D3-A");
			Assert.AreEqual ("b", mtc._b_, "D3-b");
			Assert.AreEqual ("c", mtc._c_, "D3-c");
			Assert.AreEqual ("C", mtc._C_, "D3-C");
			Assert.AreEqual ("ok-2", mtc._D_1_, "D3-D1");
			Assert.AreEqual ("d", mtc._d_2_, "D3-D2");
			Assert.AreEqual ("d", mtc._d_3_, "D3-D3");
			Assert.AreEqual ("", ManagedTestClass.A.ChangesToString (format, sep), "D3-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "D3-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "D3-c-Changes");
			Assert.AreEqual ("", ManagedTestClass.C.ChangesToString (format, sep), "D3-C-Changes");
			Assert.AreEqual ("'D' => 'ok-1' ; 'ok-1' => 'ok-2' ; 'ok-2' => 'ok-1' ; 'ok-1' => 'ok-2'", ManagedTestClass.D_1.ChangesToString (format, sep), "D3-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "D3-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "D3-d3-Changes");

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_D4.xaml");
			Assert.AreEqual ("A", mtc._A_, "D4-A");
			Assert.AreEqual ("b", mtc._b_, "D4-b");
			Assert.AreEqual ("c", mtc._c_, "D4-c");
			Assert.AreEqual ("C", mtc._C_, "D4-C");
			Assert.AreEqual ("ok-2", mtc._D_1_, "D4-D1");
			Assert.AreEqual ("d", mtc._d_2_, "D4-D2");
			Assert.AreEqual ("d", mtc._d_3_, "D4-D3");
			Assert.AreEqual ("", ManagedTestClass.A.ChangesToString (format, sep), "D4-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "D4-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "D4-c-Changes");
			Assert.AreEqual ("", ManagedTestClass.C.ChangesToString (format, sep), "D4-C-Changes");
			Assert.AreEqual ("'D' => 'ok-1' ; 'ok-1' => 'ok-2' ; 'ok-2' => 'ok-1' ; 'ok-1' => 'ok-2'", ManagedTestClass.D_1.ChangesToString (format, sep), "D4-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "D4-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "D4-d3-Changes");

		}

		[TestMethod]
		public void ManagedTest_E ()
		{
			ManagedTestClass mtc;
			string format = "'{1}' => '{2}'";
			string sep = " ; ";
			// E

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_E1.xaml");
			Assert.AreEqual ("A", mtc._A_, "E1-A");
			Assert.AreEqual ("b", mtc._b_, "E1-b");
			Assert.AreEqual ("c", mtc._c_, "E1-c");
			Assert.AreEqual ("C", mtc._C_, "E1-C");
			Assert.AreEqual ("D", mtc._D_1_, "E1-D1");
			Assert.AreEqual ("d", mtc._d_2_, "E1-D2");
			Assert.AreEqual ("d", mtc._d_3_, "E1-D3");
			Assert.AreEqual ("E", mtc._E_1_, "E1-E1");
			Assert.AreEqual ("E", mtc._E_2_, "E1-E2");
			Assert.AreEqual ("ok", mtc._E_3_, "E1-E3");
			Assert.AreEqual ("", ManagedTestClass.A.ChangesToString (format, sep), "E1-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "E1-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "E1-c-Changes");
			Assert.AreEqual ("", ManagedTestClass.C.ChangesToString (format, sep), "E1-C-Changes");
			Assert.AreEqual ("", ManagedTestClass.D_1.ChangesToString (format, sep), "E1-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "E1-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "E1-d3-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_1.ChangesToString (format, sep), "E1-E1-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_2.ChangesToString (format, sep), "E1-E2-Changes");
			Assert.AreEqual ("'E' => 'ok'", ManagedTestClass.E_3.ChangesToString (format, sep), "E1-E3-Changes");

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_E2.xaml");
			Assert.AreEqual ("A", mtc._A_, "E2-A");
			Assert.AreEqual ("b", mtc._b_, "E2-b");
			Assert.AreEqual ("c", mtc._c_, "E2-c");
			Assert.AreEqual ("C", mtc._C_, "E2-C");
			Assert.AreEqual ("D", mtc._D_1_, "E2-D1");
			Assert.AreEqual ("d", mtc._d_2_, "E2-D2");
			Assert.AreEqual ("d", mtc._d_3_, "E2-D3");
			Assert.AreEqual ("E", mtc._E_1_, "E2-E1");
			Assert.AreEqual ("E", mtc._E_2_, "E2-E2");
			Assert.AreEqual ("ok", mtc._E_3_, "E2-E3");
			Assert.AreEqual ("", ManagedTestClass.A.ChangesToString (format, sep), "E2-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "E2-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "E2-c-Changes");
			Assert.AreEqual ("", ManagedTestClass.C.ChangesToString (format, sep), "E2-C-Changes");
			Assert.AreEqual ("", ManagedTestClass.D_1.ChangesToString (format, sep), "E2-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "E2-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "E2-d3-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_1.ChangesToString (format, sep), "E2-E1-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_2.ChangesToString (format, sep), "E2-E2-Changes");
			Assert.AreEqual ("'E' => 'ok'", ManagedTestClass.E_3.ChangesToString (format, sep), "E2-E3-Changes");


		}

		[TestMethod]
		[MoonlightBug]
		public void ManagedTest_F ()
		{
			ManagedTestClass mtc;
			string format = "'{1}' => '{2}'";
			string sep = " ; ";

			// F

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_F1.xaml");
			Assert.AreEqual ("A", mtc._A_, "F1-A");
			Assert.AreEqual ("b", mtc._b_, "F1-b");
			Assert.AreEqual ("c", mtc._c_, "F1-c");
			Assert.AreEqual ("C", mtc._C_, "F1-C");
			Assert.AreEqual ("D", mtc._D_1_, "F1-D1");
			Assert.AreEqual ("d", mtc._d_2_, "F1-D2");
			Assert.AreEqual ("d", mtc._d_3_, "F1-D3");
			Assert.AreEqual ("E", mtc._E_1_, "F1-E1");
			Assert.AreEqual ("E", mtc._E_2_, "F1-E2");
			Assert.AreEqual ("E", mtc._E_3_, "F1-E3");
			Assert.AreEqual ("F", mtc._F_1_, "F1-F1");
			Assert.AreEqual ("ok", mtc._F_2_, "F1-F2");
			Assert.AreEqual ("f", mtc._f_3_, "F1-F3");
			Assert.AreEqual ("", ManagedTestClass.A.ChangesToString (format, sep), "F1-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "F1-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "F1-c-Changes");
			Assert.AreEqual ("", ManagedTestClass.C.ChangesToString (format, sep), "F1-C-Changes");
			Assert.AreEqual ("", ManagedTestClass.D_1.ChangesToString (format, sep), "F1-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "F1-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "F1-d3-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_1.ChangesToString (format, sep), "F1-E1-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_2.ChangesToString (format, sep), "F1-E2-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_3.ChangesToString (format, sep), "F1-E3-Changes");
			Assert.AreEqual ("", ManagedTestClass.F_1.ChangesToString (format, sep), "F1-F1-Changes");
			Assert.AreEqual ("'F' => 'ok'", ManagedTestClass.F_2.ChangesToString (format, sep), "F1-F2-Changes");
			Assert.AreEqual ("", ManagedTestClass.f_3.ChangesToString (format, sep), "F1-F3-Changes");

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_F2.xaml");
			Assert.AreEqual ("A", mtc._A_, "F2-A");
			Assert.AreEqual ("b", mtc._b_, "F2-b");
			Assert.AreEqual ("c", mtc._c_, "F2-c");
			Assert.AreEqual ("C", mtc._C_, "F2-C");
			Assert.AreEqual ("D", mtc._D_1_, "F2-D1");
			Assert.AreEqual ("d", mtc._d_2_, "F2-D2");
			Assert.AreEqual ("d", mtc._d_3_, "F2-D3");
			Assert.AreEqual ("E", mtc._E_1_, "F2-E1");
			Assert.AreEqual ("E", mtc._E_2_, "F2-E2");
			Assert.AreEqual ("E", mtc._E_3_, "F2-E3");
			Assert.AreEqual ("F", mtc._F_1_, "F2-F1");
			Assert.AreEqual ("ok", mtc._F_2_, "F2-F2");
			Assert.AreEqual ("f", mtc._f_3_, "F2-F3");
			Assert.AreEqual ("", ManagedTestClass.A.ChangesToString (format, sep), "F2-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "F2-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "F2-c-Changes");
			Assert.AreEqual ("", ManagedTestClass.C.ChangesToString (format, sep), "F2-C-Changes");
			Assert.AreEqual ("", ManagedTestClass.D_1.ChangesToString (format, sep), "F2-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "F2-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "F2-d3-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_1.ChangesToString (format, sep), "F2-E1-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_2.ChangesToString (format, sep), "F2-E2-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_3.ChangesToString (format, sep), "F2-E3-Changes");
			Assert.AreEqual ("", ManagedTestClass.F_1.ChangesToString (format, sep), "F2-F1-Changes");
			Assert.AreEqual ("'F' => 'ok'", ManagedTestClass.F_2.ChangesToString (format, sep), "F2-F2-Changes");
			Assert.AreEqual ("", ManagedTestClass.f_3.ChangesToString (format, sep), "F2-F3-Changes");

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_F3.xaml");
			Assert.AreEqual ("A", mtc._A_, "F3-A");
			Assert.AreEqual ("b", mtc._b_, "F3-b");
			Assert.AreEqual ("c", mtc._c_, "F3-c");
			Assert.AreEqual ("C", mtc._C_, "F3-C");
			Assert.AreEqual ("D", mtc._D_1_, "F3-D1");
			Assert.AreEqual ("d", mtc._d_2_, "F3-D2");
			Assert.AreEqual ("d", mtc._d_3_, "F3-D3");
			Assert.AreEqual ("E", mtc._E_1_, "F3-E1");
			Assert.AreEqual ("E", mtc._E_2_, "F3-E2");
			Assert.AreEqual ("E", mtc._E_3_, "F3-E3");
			Assert.AreEqual ("F", mtc._F_1_, "F3-F1");
			Assert.AreEqual ("ok-2", mtc._F_2_, "F3-F2");
			Assert.AreEqual ("f", mtc._f_3_, "F3-F3");
			Assert.AreEqual ("", ManagedTestClass.A.ChangesToString (format, sep), "F3-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "F3-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "F3-c-Changes");
			Assert.AreEqual ("", ManagedTestClass.C.ChangesToString (format, sep), "F3-C-Changes");
			Assert.AreEqual ("", ManagedTestClass.D_1.ChangesToString (format, sep), "F3-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "F3-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "F3-d3-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_1.ChangesToString (format, sep), "F3-E1-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_2.ChangesToString (format, sep), "F3-E2-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_3.ChangesToString (format, sep), "F3-E3-Changes");
			Assert.AreEqual ("", ManagedTestClass.F_1.ChangesToString (format, sep), "F3-F1-Changes");
			Assert.AreEqual ("'F' => 'ok-1' ; 'ok-1' => 'ok-2' ; 'ok-2' => 'ok-1' ; 'ok-1' => 'ok-2'", ManagedTestClass.F_2.ChangesToString (format, sep), "F3-F2-Changes");
			Assert.AreEqual ("", ManagedTestClass.f_3.ChangesToString (format, sep), "F3-F3-Changes");

			mtc = new ManagedTestClass ("DependencyPropertyTest_ManagedTest_F4.xaml");
			Assert.AreEqual ("A", mtc._A_, "F4-A");
			Assert.AreEqual ("b", mtc._b_, "F4-b");
			Assert.AreEqual ("c", mtc._c_, "F4-c");
			Assert.AreEqual ("C", mtc._C_, "F4-C");
			Assert.AreEqual ("D", mtc._D_1_, "F4-D1");
			Assert.AreEqual ("d", mtc._d_2_, "F4-D2");
			Assert.AreEqual ("d", mtc._d_3_, "F4-D3");
			Assert.AreEqual ("E", mtc._E_1_, "F4-E1");
			Assert.AreEqual ("E", mtc._E_2_, "F4-E2");
			Assert.AreEqual ("E", mtc._E_3_, "F4-E3");
			Assert.AreEqual ("F", mtc._F_1_, "F4-F1");
			Assert.AreEqual ("ok-2", mtc._F_2_, "F4-F2");
			Assert.AreEqual ("f", mtc._f_3_, "F4-F3");
			Assert.AreEqual ("", ManagedTestClass.A.ChangesToString (format, sep), "F4-A-Changes");
			Assert.AreEqual ("", ManagedTestClass.b.ChangesToString (format, sep), "F4-b-Changes");
			Assert.AreEqual ("", ManagedTestClass.c.ChangesToString (format, sep), "F4-c-Changes");
			Assert.AreEqual ("", ManagedTestClass.C.ChangesToString (format, sep), "F4-C-Changes");
			Assert.AreEqual ("", ManagedTestClass.D_1.ChangesToString (format, sep), "F4-D1-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_2.ChangesToString (format, sep), "F4-d2-Changes");
			Assert.AreEqual ("", ManagedTestClass.d_3.ChangesToString (format, sep), "F4-d3-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_1.ChangesToString (format, sep), "F4-E1-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_2.ChangesToString (format, sep), "F4-E2-Changes");
			Assert.AreEqual ("", ManagedTestClass.E_3.ChangesToString (format, sep), "F4-E3-Changes");
			Assert.AreEqual ("", ManagedTestClass.F_1.ChangesToString (format, sep), "F4-F1-Changes");
			Assert.AreEqual ("'F' => 'ok-1' ; 'ok-1' => 'ok-2' ; 'ok-2' => 'ok-1' ; 'ok-1' => 'ok-2'", ManagedTestClass.F_2.ChangesToString (format, sep), "F4-F2-Changes");
			Assert.AreEqual ("", ManagedTestClass.f_3.ChangesToString (format, sep), "F4-F3-Changes");

		}
		
		[TestMethod]
		public void Managed_Interfaces ()
		{
			InterfaceDPs dp = new InterfaceDPs ();
			dp.IComparableProp = 5;
			Assert.AreEqual (5, (int) dp.IComparableProp, "#1");

			dp.IComparableProp = 1.0;
			Assert.AreEqual (1.0, (double) dp.IComparableProp, "#2");

			dp.IComparableProp = new InterfaceDPs ();
			Assert.IsInstanceOfType<InterfaceDPs> (dp.IComparableProp, "#3");

			dp.IComparableProp = new ManagedIComparable ();
			Assert.IsInstanceOfType<ManagedIComparable> (dp.IComparableProp, "#4");

			dp.IComparableChar = 'c';
			Assert.AreEqual ('c', dp.IComparableChar, "#5");

#if notyet
			// this should likely throw..
			dp.IEquatableProp = 5;
#endif

			dp.IEquatableProp = 5.0;
			Assert.AreEqual (5.0, (double) dp.IEquatableProp, "#5");

			dp.IEquatableProp = new ManagedIEquatable ();
			Assert.IsInstanceOfType<ManagedIEquatable> (dp.IEquatableProp, "#6");
		}
		
		[TestMethod]
		public void ManagedTest_GenericDPs ()
		{
			GenericDPS c = new GenericDPS ();
			c.ListFloat = new List<float> ();
			Assert.Throws <ArgumentException> (() => c.ListFloat = new List<int> ());
			c.ListInt = new List<int> ();
		}
		
		[TestMethod]
		public void Managed_AttachBinding ()
		{
			new ManagedDPPriority ();
			ManagedDPPriority c = (ManagedDPPriority) XamlReader.Load (@"
<x:ManagedDPPriority	xmlns=""http://schemas.microsoft.com/client/2007""
						xmlns:x=""clr-namespace:MoonTest.System.Windows;assembly=moon-unit""
						NormalProp=""{Binding}"" />");
			Assert.IsNotNull (c.NormalProp, "#1");
		}
		
		[TestMethod]
		public void ManagedPriority ()
		{
			new ManagedDPPriority ();
			ManagedDPPriority c = (ManagedDPPriority) XamlReader.Load (@"
<x:ManagedDPPriority	xmlns=""http://schemas.microsoft.com/client/2007""
						xmlns:x=""clr-namespace:MoonTest.System.Windows;assembly=moon-unit""
						BindingProp=""{Binding}"" />");
			Assert.IsNull (c.BindingProp, "#1");
			Assert.IsNull (c.GetValue (ManagedDPPriority.BindingPropProperty), "#2");
			Assert.IsInstanceOfType<Expression> (c.ReadLocalValue (ManagedDPPriority.BindingPropProperty), "#3");
		}

		[TestMethod]
		public void ManagedPriority2 ()
		{
			ManagedDPPriority c = new ManagedDPPriority ();
			c.SetValue (ManagedDPPriority.BindingPropProperty, new Binding ());
			Assert.IsTrue (c.PropertyChanged, "#1");
		}

		[TestMethod]
		[MoonlightBug]
		public void ManagedSameReference ()
		{
			string s = "Hi";
			ManagedTestClass c = new ManagedTestClass ();
			c._A_ = s;
			Assert.AreSame (s, c._A_, "#1");
		}
#endregion
	}
	
	public class ManagedIComparable : IComparable { public int CompareTo (object o) { return 0; } }
	public class ManagedIEquatable : IEquatable<double> { public bool Equals (double other) { return false; } }
	public class InterfaceDPs : Control, IComparable
	{
		public static DependencyProperty IComparablePropProperty = DependencyProperty.Register ("IComparableProp", typeof (IComparable), typeof (InterfaceDPs), null);
		public static DependencyProperty IComparableCharProperty = DependencyProperty.Register ("IComparableInt", typeof (IComparable<char>), typeof (InterfaceDPs), null);
		public static DependencyProperty IEquatablePropProperty = DependencyProperty.Register ("IEquatableProp", typeof (IEquatable<double>), typeof (InterfaceDPs), null);

		public IComparable IComparableProp
		{
			get { return (IComparable) GetValue (IComparablePropProperty); }
			set { SetValue (IComparablePropProperty, value); }
		}

		public IEquatable<double> IEquatableProp
		{
			get { return (IEquatable<double>) GetValue (IEquatablePropProperty); }
			set { SetValue (IEquatablePropProperty, value); }
		}

		public IComparable<char> IComparableChar
		{
			get { return (IComparable<char>) GetValue (IComparablePropProperty); }
			set { SetValue (IComparablePropProperty, value); }
		}

		public int CompareTo (object o) { return 0; }
	}

	public class ManagedDPPriority : Control
	{
		public static readonly DependencyProperty BindingPropProperty;
		static ManagedDPPriority ()
		{
			PropertyMetadata m = new PropertyMetadata ((o, e) => ((ManagedDPPriority) o).PropertyChanged = true);
			BindingPropProperty = DependencyProperty.Register ("BindingProp", typeof (Binding), typeof (ManagedDPPriority), m);
		}

		public Binding BindingProp {
			get; set;
		}
		
		public Binding NormalProp {
			get; set;
		}

		public bool PropertyChanged {
			get; set;
		}
	}
	
	public class DependencyPropertyInfo
	{
		public DependencyProperty Property;
		public string Name;
		public Type DeclaringType;
		public Type PropertyType;
		public PropertyMetadata Metadata;
		public bool Attached;

		public List<ChangedInfo> Changes = new List<ChangedInfo> ();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="format">{0} = index, {1} = old value, {2} = new value</param>
		/// <param name="separator"></param>
		/// <returns></returns>
		public string ChangesToString (string format, string separator)
		{
			StringBuilder result = new StringBuilder ();

			for (int i = 0; i < Changes.Count; i++) {
				ChangedInfo c = Changes [i];
				if (i != 0)
					result.Append (separator);
				result.AppendFormat (format, i, c.args.OldValue, c.args.NewValue);
			}

			return result.ToString ();
		}

		public DependencyPropertyInfo (string name, Type declaring_type, Type property_type, bool attached)
		{
			this.Name = name;
			this.DeclaringType = declaring_type;
			this.PropertyType = property_type;
			this.Metadata = new PropertyMetadata (PropertyChanged);
			this.Attached = attached;

			Create ();
		}

		public DependencyPropertyInfo (string name, Type declaring_type, Type property_type, bool attached, object default_value)
		{
			this.Name = name;
			this.DeclaringType = declaring_type;
			this.PropertyType = property_type;
			this.Metadata = new PropertyMetadata (default_value, PropertyChanged);
			this.Attached = attached;

			Create ();
		}

		private void Create ()
		{
			if (Attached)
				Property = DependencyProperty.RegisterAttached (Name, PropertyType, DeclaringType, Metadata);
			else
				Property = DependencyProperty.Register (Name, PropertyType, DeclaringType, Metadata);
		}

		private string ValueToString (object val)
		{
			if (val == null)
				return "<null>";
			else
				return val.ToString ();
		}

		private void PropertyChanged (DependencyObject obj, DependencyPropertyChangedEventArgs args)
		{
			Tester.WriteLine (string.Format ("DependencyPropertyInfo.PropertyChanged ({0}.{1}), old value: {2}, new value: {3}", DeclaringType.FullName, Name, ValueToString (args.OldValue), ValueToString (args.NewValue)));
			ChangedInfo info = new ChangedInfo ();
			info.obj = obj;
			info.args = args;
			Changes.Add (info);
		}

		public class ChangedInfo
		{
			public DependencyObject obj;
			public DependencyPropertyChangedEventArgs args;
		}
	}
	// This class is accessed from xaml, and xaml can't access nested classes.
	public class ManagedTestClass : Canvas
	{
		public static DependencyPropertyInfo A; // uppercase
		public static DependencyPropertyInfo b; // lowercase
		public static DependencyPropertyInfo C;
		public static DependencyPropertyInfo c; // uppercase and lowercase
		public static DependencyPropertyInfo D_1;
		public static DependencyPropertyInfo d_2;
		public static DependencyPropertyInfo d_3; // two with equal name + 1 with different case, accessed after D has been accessed
		public static DependencyPropertyInfo E_1;
		public static DependencyPropertyInfo E_2; // two with equal name
		public static DependencyPropertyInfo E_3; // two with equal name
		public static DependencyPropertyInfo F_1;
		public static DependencyPropertyInfo F_2;
		public static DependencyPropertyInfo f_3; // two with equal name + 1 with different name, accessed before f has been accessed

		public static Dictionary<DependencyPropertyInfo, int> Counters = new Dictionary<DependencyPropertyInfo, int> ();
		public static Dictionary<DependencyProperty, DependencyPropertyInfo> Hash = new Dictionary<DependencyProperty, DependencyPropertyInfo> ();

		static ManagedTestClass ()
		{
			A = new DependencyPropertyInfo ("A", typeof (ManagedTestClass), typeof (string), false, "A");
			b = new DependencyPropertyInfo ("b", typeof (ManagedTestClass), typeof (string), false, "b");
			C = new DependencyPropertyInfo ("C", typeof (ManagedTestClass), typeof (string), false, "C");
			c = new DependencyPropertyInfo ("c", typeof (ManagedTestClass), typeof (string), false, "c");
			D_1 = new DependencyPropertyInfo ("D", typeof (ManagedTestClass), typeof (string), false, "D");
			d_2 = new DependencyPropertyInfo ("d", typeof (ManagedTestClass), typeof (string), false, "d");
			d_3 = new DependencyPropertyInfo ("d", typeof (ManagedTestClass), typeof (string), false, "d");
			E_1 = new DependencyPropertyInfo ("E", typeof (ManagedTestClass), typeof (string), false, "E");
			E_2 = new DependencyPropertyInfo ("E", typeof (ManagedTestClass), typeof (string), false, "E");
			E_3 = new DependencyPropertyInfo ("E", typeof (ManagedTestClass), typeof (string), false, "E");
			F_1 = new DependencyPropertyInfo ("F", typeof (ManagedTestClass), typeof (string), false, "F");
			F_2 = new DependencyPropertyInfo ("F", typeof (ManagedTestClass), typeof (string), false, "F");
			f_3 = new DependencyPropertyInfo ("f", typeof (ManagedTestClass), typeof (string), false, "f");
		}

		public ManagedTestClass ()
		{
			ClearCounters ();
		}

		public ManagedTestClass (string xaml_resource_file)
			: this (new Uri ("/moon-unit;component/System.Windows/" + xaml_resource_file, UriKind.Relative))
		{
		}

		public ManagedTestClass (Uri uri) : this ()
		{
			Application.LoadComponent (this, uri);
		}

		public static void ClearCounters ()
		{
			A.Changes.Clear ();
			b.Changes.Clear ();
			c.Changes.Clear ();
			C.Changes.Clear ();
			D_1.Changes.Clear ();
			d_2.Changes.Clear ();
			d_3.Changes.Clear ();
			E_1.Changes.Clear ();
			E_2.Changes.Clear ();
			E_3.Changes.Clear ();
			F_1.Changes.Clear ();
			F_2.Changes.Clear ();
			f_3.Changes.Clear ();
		}

		// Weird naming to not get into any reflection-hackery SL might do.
		public string _A_ {
			get { return (string) GetValue (A.Property); }
			set { SetValue (A.Property, value); }
		}
		public string _b_ { get { return (string) GetValue (b.Property); } }
		public string _C_ { get { return (string) GetValue (C.Property); } }
		public string _c_ { get { return (string) GetValue (c.Property); } }
		public string _D_1_ { get { return (string) GetValue (D_1.Property); } }
		public string _d_2_ { get { return (string) GetValue (d_2.Property); } }
		public string _d_3_ { get { return (string) GetValue (d_3.Property); } }
		public string _E_1_ { get { return (string) GetValue (E_1.Property); } }
		public string _E_2_ { get { return (string) GetValue (E_2.Property); } }
		public string _E_3_ { get { return (string) GetValue (E_3.Property); } }
		public string _F_1_ { get { return (string) GetValue (F_1.Property); } }
		public string _F_2_ { get { return (string) GetValue (F_2.Property); } }
		public string _f_3_ { get { return (string) GetValue (f_3.Property); } }
	}
	
	public class GenericDPS : UserControl
	{
		public static readonly DependencyProperty ListFloatProperty = DependencyProperty.Register ("ListFloat", typeof (List<float>), typeof (GenericDPS), null);
		public static readonly DependencyProperty ListIntProperty = DependencyProperty.Register ("ListInt", typeof (List<int>), typeof (GenericDPS), null);

		public object ListFloat
		{
			get { return GetValue (ListFloatProperty); }
			set { SetValue (ListFloatProperty, value); }
		}

		public object ListInt
		{
			get { return GetValue (ListIntProperty); }
			set { SetValue (ListIntProperty, value); }
		}
	}
	
	public class CustomCanvas : Canvas
	{
		public static readonly DependencyProperty ChildProperty = DependencyProperty.Register ("Child", typeof (FrameworkElement), typeof (CustomCanvas), null);
		public static readonly DependencyProperty Child2Property = DependencyProperty.Register ("Child2", typeof (FrameworkElement), typeof (CustomCanvas), null);
		public FrameworkElement Child {
			get { return (FrameworkElement) GetValue (ChildProperty); }
			set { SetValue (ChildProperty, value); }
		}

		public FrameworkElement Child2 {
			get { return (FrameworkElement) GetValue (Child2Property); }
			set { SetValue (Child2Property, value); }
		}
	}
}
