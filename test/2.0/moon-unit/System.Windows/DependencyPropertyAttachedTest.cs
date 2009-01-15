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
using System.Collections.Generic;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Threading;

#pragma warning disable 414
#pragma warning disable 219

namespace MoonTest.System.Windows
{
	[TestClass ()]
	public class DependencyPropertyAttachedTest
	{
		/*
		 * Declaring Type _ Property Name _ [index, not included in property name _] _ Property Type
		 */
		private DependencyPropertyInfo Canvas_Custom_double;
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
			Assert.Throws (delegate { DependencyProperty.RegisterAttached (null, typeof (int), typeof (Canvas), new PropertyMetadata (null)); }, typeof (ArgumentNullException));
			Assert.Throws (delegate { DependencyProperty.RegisterAttached (string.Empty, typeof (int), typeof (Canvas), new PropertyMetadata (null)); }, typeof (ArgumentException));
			Assert.Throws (delegate { DependencyProperty.RegisterAttached ("a", null, typeof (Canvas), new PropertyMetadata (null)); }, typeof (ArgumentNullException));
			Assert.Throws (delegate { DependencyProperty.RegisterAttached ("a", typeof (int), null, new PropertyMetadata (null)); }, typeof (ArgumentNullException));
			// null PropertyMetadata shouldn't throw.
			DependencyProperty.RegisterAttached ("a", typeof (int), typeof (Canvas), null);
		}

		[TestMethod ()]
		public void Register_HuhTest1 ()
		{
			DependencyPropertyInfo info = new DependencyPropertyInfo ("Custom", typeof (InkPresenter), typeof (int), true);
			DependencyProperty property = info.Property;
			Canvas canvas = new Canvas ();

			canvas.GetValue (property); // This should throw, the property doesn't exist on the canvas.

			Assert.Throws (delegate { canvas.GetValue (InkPresenter.StrokesProperty); }, typeof (Exception)); // And this throws a catastrophic error.
		}

		[TestMethod ()]
		public void Check_Default ()
		{
			DependencyProperty property = DependencyProperty.Register ("MyProp", typeof (int), typeof(Rectangle), new PropertyMetadata (100));
			Rectangle r = new Rectangle ();
			r.SetValue (property, 10);
			Assert.AreEqual (10, r.GetValue (property), "#1");
			r.ClearValue (property);
			Assert.AreEqual (100, r.GetValue (property), "#2");
		}

		object triggered;
		[TestMethod()]
		public void Register_Callback()
		{
			ManualResetEvent handle = new ManualResetEvent(false);
			DependencyProperty prop = DependencyProperty.Register("Callback", typeof(int), typeof(Rectangle),
										new PropertyMetadata((PropertyChangedCallback)delegate { triggered = new object(); }));
			Rectangle r = new Rectangle();
			r.SetValue(prop, 5);
			Assert.IsNotNull(triggered);
		}

		[TestMethod ()]
		public void Check_Default_Nullable ()
		{
			DependencyProperty property = DependencyProperty.Register ("MyPropNullable", typeof (int?), typeof(Rectangle), null);
			Rectangle r = new Rectangle ();
			r.SetValue (property, 10);
			Assert.AreEqual (10, r.GetValue (property), "#1");
			r.ClearValue (property);
			Assert.AreEqual (null, r.GetValue (property), "#2");
		}

		#region Canvas Custom
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

			Canvas_Custom_double = new DependencyPropertyInfo ("Custom", typeof (Canvas), typeof (double), true);
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
		public void Register_Canvas_Custom_CustomClass ()
		{
			Canvas_Custom_CustomClass = new DependencyPropertyInfo ("Custom", typeof (Canvas), typeof (CustomClass), true);
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

			Canvas_Custom_Canvas = new DependencyPropertyInfo ("Custom", typeof (Canvas), typeof (Canvas), true);
			info = Canvas_Custom_Canvas;

			property = info.Property;

			Assert.IsNull (canvas.GetValue (property));
			Assert.IsNull (ink.GetValue (property));

			Assert.Throws (delegate { canvas.SetValue (property, 1); }, typeof (ArgumentException));
			Assert.Throws (delegate { canvas.SetValue (property, ""); }, typeof (ArgumentException));
			Assert.Throws (delegate { canvas.SetValue (property, new CustomClass ()); }, typeof (ArgumentException));

			foreach (object expected_value in new object [] { null, new Canvas (), null, canvas, canvas, null, new CustomCanvasType (), custom_canvas, custom_canvas, ink }) {
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
			Canvas_Custom_CustomCanvasType = new DependencyPropertyInfo ("Custom", typeof (Canvas), typeof (CustomCanvasType), true);
		}
		#endregion
		#region Canvas Height
		[TestMethod ()]
		public void Register_Canvas_Height_int ()
		{
			// Register a custom property with the same name and type as an existing builtin property
			Canvas_Height_int = new DependencyPropertyInfo ("Height", typeof (Canvas), typeof (int), true);
		}

		[TestMethod ()]
		public void Register_Canvas_Height_double ()
		{
			// Register a custom property with the same name and type as an existing builtin property
			Canvas_Height_double = new DependencyPropertyInfo ("Height", typeof (Canvas), typeof (double), true);
		}

		[TestMethod ()]
		public void Register_Canvas_Height_CustomClass ()
		{
			// Register a custom property with the same name (but not type) as an existing builtin AND another custom property
			Canvas_Height_CustomClass = new DependencyPropertyInfo ("Height", typeof (Canvas), typeof (CustomClass), true);
		}

		[TestMethod ()]
		public void Register_Canvas_Height_CustomCanvasType ()
		{
			// Register a custom property with the same name (but not type) as an existing builtin AND another custom property
			Canvas_Height_CustomCanvasType = new DependencyPropertyInfo ("Height", typeof (Canvas), typeof (CustomCanvasType), true);
		}
		#endregion
		#region CustomClass Height
		[TestMethod ()]
		public void Register_CustomClass_Height_int ()
		{
			CustomClass_Height_int = new DependencyPropertyInfo ("Height", typeof (CustomClass), typeof (int), true);
		}

		[TestMethod ()]
		public void Register_CustomClass_Height_double ()
		{
			CustomClass_Height_double = new DependencyPropertyInfo ("Height", typeof (CustomClass), typeof (double), true);
		}

		[TestMethod ()]
		public void Register_CustomClass_Height_CustomClass ()
		{
			CustomClass_Height_CustomClass = new DependencyPropertyInfo ("Height", typeof (CustomClass), typeof (CustomClass), true);
		}

		[TestMethod ()]
		public void Register_CustomClass_Height_CustomCanvasType ()
		{
			CustomClass_Height_CustomCanvasType = new DependencyPropertyInfo ("Height", typeof (CustomClass), typeof (CustomCanvasType), true);
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

			CustomCanvasType_Height_int = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (int), true);
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

			CustomCanvasType_Height_double = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (double), true);
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
			Assert.Throws (delegate { CustomCanvasType_Height_void = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (void), true); }, typeof (NotSupportedException));
		}

		[TestMethod ()]
		public void Register_CustomCanvasType_Height_Canvas ()
		{
			CustomCanvasType_Height_Canvas = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (Canvas), true);
		}

		[TestMethod ()]
		public void Register_CustomCanvasType_Height_CustomClass ()
		{
			CustomCanvasType_Height_CustomClass = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomClass), true);
		}

		[TestMethod ()]
		public void Register_CustomCanvasType_Height_CustomCanvasType ()
		{
			CustomCanvasType_Height_CustomCanvasType = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomCanvasType), true);
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

			CustomCanvasType_Height_CustomInterface = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomInterface), true);
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

			CustomCanvasType_Height_CustomStruct = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomStruct), true);
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

			CustomCanvasType_Height_CustomEnum = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomEnum), true);
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

			foreach (object expected_value in new object [] { CustomEnum.EnumValue1, CustomEnum.EnumValue1, CustomEnum.EnumValue2, (CustomEnum) 0, custom_enum, custom_enum }) {
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

			CustomCanvasType_Height_CustomDelegate = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomDelegate), true);
			info = CustomCanvasType_Height_CustomDelegate;

			property = info.Property;

			Assert.AreEqual (null, the_object.GetValue (property), "Default value 1");
			Assert.AreEqual (null, ink.GetValue (property), "Default value 2");

			Assert.Throws (delegate { the_object.SetValue (property, 0); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, 1); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, ""); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new CustomClass ()); }, typeof (ArgumentException));
			Assert.Throws (delegate { the_object.SetValue (property, new Canvas ()); }, typeof (ArgumentException));

			foreach (object expected_value in new CustomDelegate [] { null, delegate { }, delegate { }, custom_delegate, custom_delegate }) {
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
			CustomCanvasType_Height_CustomClassCtorA = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomClassCtorA), true);
		}

		[TestMethod ()]
		public void Register_CustomCanvasType_Height_CustomClassCtorB ()
		{
			CustomCanvasType_Height_CustomClassCtorB = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType), typeof (CustomClassCtorB), true);
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

			CustomCanvasType2_Height_double = new DependencyPropertyInfo ("Height", typeof (CustomCanvasType2), typeof (double), true);
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

			//Assert.Throws (delegate { custom_canvas.SetValue (property, 1.1); }, typeof (ArgumentException));

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
			FrameworkElement_Height_int = new DependencyPropertyInfo ("Height", typeof (FrameworkElement), typeof (int), true);
		}

		[TestMethod ()]
		public void Register_FrameworkElement_Height_double ()
		{
			FrameworkElement_Height_double = new DependencyPropertyInfo ("Height", typeof (FrameworkElement), typeof (double), true);
		}

		[TestMethod ()]
		public void Register_FrameworkElement_Height_CustomClass ()
		{
			FrameworkElement_Height_CustomClass = new DependencyPropertyInfo ("Height", typeof (FrameworkElement), typeof (CustomClass), true);
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
			private CustomClassCtorB () { }
		}
		#endregion
	}
}
