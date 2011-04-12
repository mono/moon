using System;
using System.ComponentModel;
using System.Globalization;
using System.Windows;
using System.Windows.Data;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Data
{
	public class ComputerConverter : TypeConverter {
		public static Action ConstructorFunc;
		public static Action<ITypeDescriptorContext, Type> CanConvertFromFunc;
		public static Action<ITypeDescriptorContext, Type> CanConvertToFunc;

		public static Action<ITypeDescriptorContext, CultureInfo, object> ConvertFromFunc;
		public static Action<ITypeDescriptorContext, CultureInfo, object, Type> ConvertToFunc;

		public ComputerConverter ()
		{
			if (ConstructorFunc != null)
				ConstructorFunc ();
		}

		public override bool CanConvertFrom (ITypeDescriptorContext context, Type sourceType)
		{
			if (CanConvertFromFunc != null)
				CanConvertFromFunc (context, sourceType);

			return sourceType == typeof (Robot);
		}

		public override bool CanConvertTo (ITypeDescriptorContext context, Type destinationType)
		{
			if (CanConvertToFunc != null)
				CanConvertToFunc (context, destinationType);

			return destinationType == typeof (Robot);
		}

		public override object ConvertFrom (ITypeDescriptorContext context, CultureInfo culture, object value)
		{
			if (ConvertFromFunc != null)
				ConvertFromFunc (context, culture, value);

			return new Computer { Name = ((Robot) value).Name };
		}

		public override object ConvertTo (ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType)
		{
			if (ConvertToFunc != null)
				ConvertToFunc (context, culture, value, destinationType);

			return new Robot { Name = ((Computer) value).Name };
		}
	}

	public class RobotConverter : TypeConverter {
		public static Action ConstructorFunc;
		public static Action<ITypeDescriptorContext, Type> CanConvertFromFunc;
		public static Action<ITypeDescriptorContext, Type> CanConvertToFunc;

		public static Action<ITypeDescriptorContext, CultureInfo, object> ConvertFromFunc;
		public static Action<ITypeDescriptorContext, CultureInfo, object, Type> ConvertToFunc;

		public RobotConverter ()
		{
			if (ConstructorFunc != null)
				ConstructorFunc ();
		}

		public override bool CanConvertFrom (ITypeDescriptorContext context, Type sourceType)
		{
			if (CanConvertFromFunc != null)
				CanConvertFromFunc (context, sourceType);

			return sourceType == typeof (string);
		}

		public override bool CanConvertTo (ITypeDescriptorContext context, Type destinationType)
		{
			if (CanConvertToFunc != null)
				CanConvertToFunc (context, destinationType);

			return destinationType == typeof (Robot);
		}

		public override object ConvertFrom (ITypeDescriptorContext context, CultureInfo culture, object value)
		{
			if (ConvertFromFunc != null)
				ConvertFromFunc (context, culture, value);

			return new Robot { Name = (string) value };
		}

		public override object ConvertTo (ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType)
		{
			if (ConvertToFunc != null)
				ConvertToFunc (context, culture, value, destinationType);

			return ((Robot) value).Name;
		}
	}

	[TypeConverter (typeof (RobotConverter))]
	public class Robot {
		public string Name { get; set; }
		public override string ToString ()
		{
			return string.Format ("Robot Name: {0}", Name);

		}
	}

	[TypeConverter (typeof (ComputerConverter))]
	public class Computer {
		public string Name { get; set; }
		public override string ToString ()
		{
			return string.Format ("Computer Name: {0}", Name);
		}
	}

	public class Hybrid : DependencyObject {
		public static readonly DependencyProperty MyNameProperty = DependencyProperty.Register ("MyName", typeof (string), typeof (Hybrid), null);
		public static readonly DependencyProperty ComputerProperty = DependencyProperty.Register ("Computer", typeof (Computer), typeof (Hybrid), null);
		public static readonly DependencyProperty RobotProperty = DependencyProperty.Register ("Robot", typeof (Robot), typeof (Hybrid), null);

		public string MyName
		{
			get { return (string) GetValue (MyNameProperty); }
			set { SetValue (MyNameProperty, value); }
		}

		public Computer Computer
		{
			get { return (Computer) GetValue (ComputerProperty); }
			set { SetValue (ComputerProperty, value); }
		}

		public Robot Robot
		{
			get { return (Robot) GetValue (RobotProperty); }
			set { SetValue (RobotProperty, value); }
		}
	}

	public class HookedConverter : TypeConverter {

		public static Func<ITypeDescriptorContext, Type, bool> CanConvertFromFunc;
		public static Func<ITypeDescriptorContext, Type, bool> CanConvertToFunc;

		public static Action ConstructorFunc;

		public static Func<ITypeDescriptorContext, CultureInfo, object, object> ConvertFromFunc;
		public static Func<ITypeDescriptorContext, CultureInfo, object, Type, object> ConvertToFunc;

		public static void Reset()
		{
			CanConvertFromFunc = null;
			CanConvertToFunc = null;
			ConstructorFunc = null;
			ConvertFromFunc = null;
			ConvertToFunc = null;
		}

		public HookedConverter()
		{

		}

		public HookedConverter(Type type)
		{
			Assert.Fail("ctor HookedConverter (Type) should not be called");
		}

		public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
		{
			return CanConvertFromFunc == null ? true : CanConvertFromFunc(context, sourceType);
		}

		public override bool CanConvertTo(ITypeDescriptorContext context, Type destinationType)
		{
			return CanConvertToFunc == null ? true : CanConvertToFunc(context, destinationType);
		}

		public override object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
		{
			return ConvertFromFunc(context, culture, value);
		}

		public override object ConvertTo(ITypeDescriptorContext context, CultureInfo culture, object value, Type destinationType)
		{
			return ConvertToFunc (context, culture, value, destinationType);
		}
	}

	[TestClass]
	public partial class BindingTest_TypeConverter {

		[TestInitialize]
		public void Initialize ()
		{
			HookedConverter.Reset ();

			ComputerConverter.ConstructorFunc = null;
			ComputerConverter.CanConvertFromFunc = null;
			ComputerConverter.CanConvertToFunc = null;
			ComputerConverter.ConvertFromFunc = null;
			ComputerConverter.ConvertToFunc = null;

			RobotConverter.ConstructorFunc = null;
			RobotConverter.CanConvertFromFunc = null;
			RobotConverter.CanConvertToFunc = null;
			RobotConverter.ConvertFromFunc = null;
			RobotConverter.ConvertToFunc = null;
		}


		public class IntPropNoConverter
		{
			public int IntProperty { get; set; }
		}

		public class IntPropWithConverter
		{
			[TypeConverter(typeof(HookedConverter))]
			public int IntProperty { get; set; }
		}

		[TypeConverter(typeof(HookedConverter))]
		public class IntPropClassConverter
		{
			public int IntProperty { get; set; }
		}

		public class IntPropClassConverterContainer
		{
			public IntPropClassConverter Value { get; set; }
		}

		public class IntDPNoConverter : DependencyObject
		{
			public static readonly DependencyProperty IntDPProperty =
				DependencyProperty.Register("IntDP", typeof(int), typeof(IntDPNoConverter), null);

			public int IntDP
			{
				get { return (int)GetValue(IntDPProperty); }
				set { SetValue(IntDPProperty, value); }
			}
		}

		public class IntDPWithConverter : DependencyObject
		{
			public static readonly DependencyProperty IntDPProperty =
				DependencyProperty.Register("IntDP", typeof(int), typeof(IntDPWithConverter), null);

			[TypeConverter(typeof(HookedConverter))]
			public int IntDP
			{
				get { return (int)GetValue(IntDPProperty); }
				set { SetValue(IntDPProperty, value); }
			}
		}

		[TypeConverter(typeof(HookedConverter))]
		public class IntDPClassConverter : DependencyObject
		{
			public static readonly DependencyProperty IntDPProperty =
				DependencyProperty.Register("IntDP", typeof(int), typeof(IntDPClassConverter), null);

			public int IntDP
			{
				get { return (int)GetValue(IntDPProperty); }
				set { SetValue(IntDPProperty, value); }
			}
		}

		public class IntDPClassConverterContainer
		{
			public IntDPClassConverter Value { get; set; }
		}
		
		public class ConvertedObjectDPNoConverter : DependencyObject
		{
			public static readonly DependencyProperty ConvertedObjectProperty =
				DependencyProperty.Register("ConvertedObject", typeof(ConvertedObject), typeof(ConvertedObjectDPNoConverter), null);

			public ConvertedObject ConvertedObject
			{
				get { return (ConvertedObject)GetValue(ConvertedObjectProperty); }
				set { SetValue(ConvertedObjectProperty, value); }
			}
		}

		public class ConvertedObjectDPWithConverter : DependencyObject
		{
			public static readonly DependencyProperty ConvertedObjectProperty =
				DependencyProperty.Register("ConvertedObject", typeof(ConvertedObject), typeof(ConvertedObjectDPWithConverter), null);

			[TypeConverter(typeof(HookedConverter))]
			public ConvertedObject ConvertedObject
			{
				get { return (ConvertedObject)GetValue(ConvertedObjectProperty); }
				set { SetValue(ConvertedObjectProperty, value); }
			}
		}

		[TypeConverter(typeof(HookedConverter))]
		public class ConvertedObjectDPClassConverter : DependencyObject
		{
			public static readonly DependencyProperty ConvertedObjectProperty =
				DependencyProperty.Register("ConvertedObject", typeof(ConvertedObject), typeof(ConvertedObjectDPClassConverter), null);

			public ConvertedObject ConvertedObject
			{
				get { return (ConvertedObject)GetValue(ConvertedObjectProperty); }
				set { SetValue(ConvertedObjectProperty, value); }
			}
		}

		public class ConvertedObjectDPClassConverterContainer
		{
			public ConvertedObjectDPClassConverter Value { get; set; }
		}

		[TestMethod]
		public void IntToConvertedDP_ConverterOnInt()
		{
			// Ignore the type converter defined in the IntProperty property.
			bool created = false;
			HookedConverter.ConstructorFunc = () => created = true;

			var source = new IntPropWithConverter { IntProperty = 5 };
			var target = new ConvertedObjectDPNoConverter { };

			BindingOperations.SetBinding(target, ConvertedObjectDPNoConverter.ConvertedObjectProperty, new Binding("IntProperty") { Source = source });
			Assert.IsFalse(created, "#1");
		}

		[TestMethod]
		public void IntToConvertedDP_ConverterOnIntClass()
		{
			// Ignore any type converters on the source class

			bool created = false;
			HookedConverter.ConstructorFunc = () => created = true;

			var source = new IntPropClassConverter { IntProperty = 5 };
			var target = new ConvertedObjectDPNoConverter { };

			BindingOperations.SetBinding(target, ConvertedObjectDPNoConverter.ConvertedObjectProperty, new Binding("IntProperty") { Source = source });
			Assert.IsFalse(created, "#1");
		}
	   
		[TestMethod]
		public void IntContainerToConvertedDP_ConverterOnIntClass()
		{
			// The field Value of type 'IntPropClassConverter' has a class level type converter, so we use that
			var source = new IntPropClassConverterContainer { Value = new IntPropClassConverter { IntProperty = 5 } };
			var target = new ConvertedObjectDPNoConverter { };

			HookedConverter.CanConvertFromFunc = (descriptor, type) => {
				Assert.AreEqual (typeof(ConvertedObject), type, "#1");
				return true;
			};
			HookedConverter.CanConvertToFunc = (descriptor, type) => {
				Assert.AreEqual (typeof (ConvertedObject), type, "#2");
				return true;
			};
			HookedConverter.ConvertFromFunc = (descriptor, culture, value) => {
				Assert.IsInstanceOfType<ConvertedObject>(value, "#3");
				return new IntPropClassConverter { IntProperty = (int) ((ConvertedObject)value).Value };
			};
			HookedConverter.ConvertToFunc = (descriptor, culture, value, type) => {
				Assert.IsInstanceOfType<IntPropClassConverter>(value, "#3");
				Assert.AreEqual(typeof (ConvertedObject), type, "#4");
				return new ConvertedObject { Value = ((IntPropClassConverter)value).IntProperty };
			};

			BindingOperations.SetBinding(target, ConvertedObjectDPNoConverter.ConvertedObjectProperty, new Binding("Value") { Mode = BindingMode.TwoWay, Source = source });
			Assert.AreEqual(5, target.ConvertedObject.Value, "#5");

			target.ConvertedObject = new ConvertedObject { Value = 6 };
			Assert.AreEqual(6, source.Value.IntProperty, "#6");
		}

		[TestMethod]
		public void IntContainerToConvertedDP_ConverterOnIntClass_NullInitialSource()
		{
			// If the source is initially null, we set null on the target without invoking the converter.
			// We don't even create one.
			var source = new IntPropClassConverterContainer { Value = null };
			var target = new ConvertedObjectDPNoConverter { ConvertedObject = new ConvertedObject() };

			HookedConverter.ConstructorFunc = () => Assert.Fail("Not invoked", "#ctor");

			BindingOperations.SetBinding(target, ConvertedObjectDPNoConverter.ConvertedObjectProperty, new Binding("Value") { Mode = BindingMode.TwoWay, Source = source });
			Assert.AreEqual(null, target.ConvertedObject, "#1");
		}

		[TestMethod]
		public void IntContainerToConvertedDP_ConverterOnIntClass_SetNullOnDestination()
		{
			// If the destination is set to null on a two-way binding we don't invoke the type converter
			var source = new IntPropClassConverterContainer { Value = new IntPropClassConverter { IntProperty = 5 } };
			var target = new ConvertedObjectDPNoConverter { };

			HookedConverter.ConvertFromFunc = (descriptor, culture, value) =>
			{
				Assert.Fail ("Should not be invoked");
				return null;
			};
			HookedConverter.ConvertToFunc = (descriptor, culture, value, type) =>
			{
				Assert.IsInstanceOfType<IntPropClassConverter>(value, "#3");
				Assert.AreEqual(typeof(ConvertedObject), type, "#4");
				return new ConvertedObject { Value = ((IntPropClassConverter)value).IntProperty };
			};

			BindingOperations.SetBinding(target, ConvertedObjectDPNoConverter.ConvertedObjectProperty, new Binding("Value") { Mode = BindingMode.TwoWay, Source = source });

			target.ConvertedObject = null;
			Assert.AreEqual(null, source.Value, "#1");
		}

		[TestMethod]
		public void IntToConvertedDP_ConverterOnDPProperty()
		{
			// Ignore type converters on the DP the binding is attached to
			bool created = false;
			HookedConverter.ConstructorFunc = () => created = true;

			var source = new IntPropNoConverter { IntProperty = 5 };
			var target = new ConvertedObjectDPWithConverter { };

			BindingOperations.SetBinding(target, ConvertedObjectDPWithConverter.ConvertedObjectProperty, new Binding("IntProperty") { Source = source });
			Assert.IsFalse(created, "#1");
		}

		[TestMethod]
		public void IntToConvertedDP_ConverterOnDPClass()
		{
			// Ignore type converters on the destination DPs declaring type.
			bool created = false;
			var source = new IntPropNoConverter { IntProperty = 5 };
			var target = new ConvertedObjectDPClassConverter { };

			HookedConverter.ConstructorFunc = () => created = true;
			BindingOperations.SetBinding(target, ConvertedObjectDPClassConverter.ConvertedObjectProperty, new Binding("IntProperty") { Source = source });
			Assert.IsFalse (created, "#1");
		}

		[TestMethod]
		public void ConvertedDPToInt_ConverterOnIntDP()
		{
			bool created = false;
			var source = new ConvertedObjectDPNoConverter { };
			var target = new IntDPWithConverter { IntDP = 5 };

			HookedConverter.ConstructorFunc = () => created = true;
			BindingOperations.SetBinding(target, IntDPWithConverter.IntDPProperty, new Binding("ConvertedObject") { Source = source });
			Assert.IsFalse (created, "#1");
		}

		[TestMethod]
		public void ConvertedDPToInt_ConverterOnIntClass()
		{
			bool created = false;
			var source = new ConvertedObjectDPNoConverter { };
			var target = new IntDPClassConverter { IntDP = 5 };

			HookedConverter.ConstructorFunc = () => created = true;
			BindingOperations.SetBinding(target, IntDPClassConverter.IntDPProperty, new Binding("ConvertedObject") { Source = source });
			Assert.IsFalse (created, "#1");
		}

		[TestMethod]
		public void ConvertedDPToInt_ConverterOnDPProperty()
		{
			bool created = false;
			var source = new ConvertedObjectDPWithConverter { };
			var target = new IntDPNoConverter { IntDP = 5 };

			HookedConverter.ConstructorFunc = () => created = true;
			BindingOperations.SetBinding(target, IntDPNoConverter.IntDPProperty, new Binding("ConvertedObject") { Source = source });
			Assert.IsFalse (created, "#1");
		}

		[TestMethod]
		public void ConvertedDPToInt_ConverterOnDPClass()
		{
			bool created = false;
			var source = new ConvertedObjectDPClassConverter { };
			var target = new IntDPNoConverter { IntDP = 5 };

			HookedConverter.ConstructorFunc = () => created = true;
			BindingOperations.SetBinding(target, IntDPNoConverter.IntDPProperty, new Binding("ConvertedObject") { Source = source });
			Assert.IsFalse (created, "#1");
		}

		[TestMethod]
		public void BindComputerToRobot_SetRobot ()
		{
			string failure = null;
			RobotConverter.ConstructorFunc = () => {
				failure = failure ?? "#1";
			};
			ComputerConverter.CanConvertFromFunc = (context, type) => {
				if (typeof (Robot) != type)
					failure = failure ?? "#2";
			};
			ComputerConverter.CanConvertToFunc = (context, type) => {
				if (typeof (Robot) != type)
					failure = failure ?? "#3";
			};
			ComputerConverter.ConvertToFunc = (context, culture, value, type) => {
				if (!(value is Computer))
					failure = failure ?? "#4";

				if (typeof (Robot) != type)
					failure = failure ?? "#5";
			};

			var h = new Hybrid ();
			BindingOperations.SetBinding (h, Hybrid.RobotProperty, new Binding ("Computer") { Source = h });
			h.Computer = new Computer { Name = "A" };

			if (failure != null)
				Assert.Fail (failure);

			Assert.IsNotNull (h.Robot, "#1");
			Assert.AreEqual ("A", h.Robot.Name, "#2");
		}

		[TestMethod]
		public void BindComputerToRobot_SetComputer ()
		{
			string failure = null;
			bool robotFirst = false;
			RobotConverter.ConstructorFunc = () => {
				robotFirst = true;
			};
			ComputerConverter.ConstructorFunc = () => {
				if (!robotFirst)
					failure = failure ?? "#1";
			};
			ComputerConverter.CanConvertFromFunc = (context, type) => {
				if (typeof (Robot) != type)
					failure = failure ?? "#2";
			};
			ComputerConverter.CanConvertToFunc = (context, type) => {
				if (typeof (Robot) != type)
					failure = failure ?? "#3";
			};
			ComputerConverter.ConvertFromFunc = (context, culture, value) => {
				if (!(value is Robot))
				failure = failure ?? "#4";
			};

			var h = new Hybrid ();
			BindingOperations.SetBinding (h, Hybrid.RobotProperty, new Binding ("Computer") { Source = h, Mode = BindingMode.TwoWay });
			h.Robot = new Robot { Name = "A" };

			if (failure != null)
				Assert.Fail (failure);

			Assert.IsNotNull (h.Computer, "#5");
			Assert.AreEqual ("A", h.Computer.Name, "#6");
		}

		[TestMethod]
		public void BindRobotToComputer_SetRobot ()
		{
			string failure = null;
			bool robotFirst = false;
			RobotConverter.ConstructorFunc = () => {
				robotFirst = true;
			};
			ComputerConverter.ConstructorFunc = () => {
				if (!robotFirst)
					failure = failure ?? "#1";
			};
			ComputerConverter.CanConvertFromFunc = (context, type) => {
				if (typeof (Robot) != type)
					failure = failure ?? "#2";
			};
			ComputerConverter.CanConvertToFunc = (context, type) => {
				if (typeof (Robot) != type)
					failure = failure ?? "#3";
			};
			ComputerConverter.ConvertFromFunc = (context, culture, value) => {
				if (!(value is Robot))
					failure = failure ?? "#4";
			};

			var h = new Hybrid ();
			BindingOperations.SetBinding (h, Hybrid.ComputerProperty, new Binding ("Robot") { Source = h, Mode = BindingMode.TwoWay });
			h.Robot = new Robot { Name = "A" };

			if (failure != null)
				Assert.Fail (failure);

			Assert.IsNotNull (h.Computer, "#5");
			Assert.AreEqual ("A", h.Computer.Name, "#6");
		}

		[TestMethod]
		public void BindRobotToComputer_SetComputer ()
		{
			string failure = null;
			bool computerFirst = false;
			ComputerConverter.ConstructorFunc = () => {
				computerFirst = true;
			};
			RobotConverter.ConstructorFunc = () => {
				if (!computerFirst)
					failure = failure ?? "#1";
			};
			ComputerConverter.CanConvertFromFunc = (context, type) => {
				if (typeof (Robot) != type)
					failure = failure ?? "#2";
			};
			ComputerConverter.CanConvertToFunc = (context, type) => {
				if (typeof (Robot) != type)
					failure = failure ?? "#3";
			};
			ComputerConverter.ConvertFromFunc = (context, culture, value) => {
				if (!(value is Robot))
					failure = failure ?? "#4";
			};

			var h = new Hybrid ();
			BindingOperations.SetBinding (h, Hybrid.ComputerProperty, new Binding ("Robot") { Source = h, Mode = BindingMode.TwoWay });
			h.Computer = new Computer { Name = "A" };

			if (failure != null)
				Assert.Fail (failure);

			Assert.IsNotNull (h.Robot, "#5");
			Assert.AreEqual ("A", h.Robot.Name, "#6");
		}
	}
}
