using System;
using System.ComponentModel;
using System.Globalization;
using System.Windows;
using System.Windows.Data;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Data
{
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
	public partial class BindingTest {

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

		[TestInitialize]
		public void Setup()
		{
			HookedConverter.Reset();
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
	}
}
