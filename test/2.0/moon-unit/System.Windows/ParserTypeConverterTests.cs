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
using Mono.Moonlight.UnitTesting;
using System.ComponentModel;
using System.Windows.Markup;

namespace MoonTest.System.Windows {

	[TestClass]
	public class ParserTypeConverterTests {

		[TestMethod]
		public void ConverterCallOnAttachedGetter ()
		{
			// Do look up TypeConverters declared on the static getter method.
			var x = (ClassWithoutTypeConverter)XamlReader.Load(@"
<clr:ClassWithoutTypeConverter
	xmlns=""http://schemas.microsoft.com/client/2007""
	xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
	xmlns:clr=""clr-namespace:MoonTest.System.Windows;assembly=moon-unit"">
	<clr:ClassWithoutTypeConverter.ConverterOnAttachedGetter>
		test
	</clr:ClassWithoutTypeConverter.ConverterOnAttachedGetter>
</clr:ClassWithoutTypeConverter>
");
			Assert.IsInstanceOfType<ConvertedObject>(x.GetValue(ClassWithoutTypeConverter.ConverterOnAttachedGetterProperty), "#1");
		}

		[TestMethod]
		public void ConverterCallOnAttachedSetter ()
		{
			// Don't look up TypeConverters declared on the static setter method.
			Assert.Throws<XamlParseException>( () => XamlReader.Load(@"
<clr:ClassWithoutTypeConverter
	xmlns=""http://schemas.microsoft.com/client/2007""
	xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
	xmlns:clr=""clr-namespace:MoonTest.System.Windows;assembly=moon-unit"">
	<clr:ClassWithoutTypeConverter.ConverterOnAttachedSetter>
		test
	</clr:ClassWithoutTypeConverter.ConverterOnAttachedSetter>
</clr:ClassWithoutTypeConverter>
"));
		}

		[TestMethod]
		public void TypeConverterOnClass_AttachedProperty_OnDeclaringClass  ()
		{
			// Attached DPs don't look up type converters on their declaring class
			// when set on an instance of their declaring type
			Assert.Throws<XamlParseException>( () => XamlReader.Load(@"
<clr:ClassWithTypeConverter
	xmlns=""http://schemas.microsoft.com/client/2007""
	xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
	xmlns:clr=""clr-namespace:MoonTest.System.Windows;assembly=moon-unit"">
	<clr:ClassWithTypeConverter.Attached>
		test
	</clr:ClassWithTypeConverter.Attached>
</clr:ClassWithTypeConverter>
"));
		}

		[TestMethod]
		public void TypeConverterOnClass_AttachedProperty_OnOtherClass  ()
		{
			// Attached DPs don't look up type converters on their declaring class
			// when set on an object of another type
			Assert.Throws<XamlParseException>( () => XamlReader.Load(@"
<Rectangle
	xmlns=""http://schemas.microsoft.com/client/2007""
	xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
	xmlns:clr=""clr-namespace:MoonTest.System.Windows;assembly=moon-unit"">
	<clr:ClassWithTypeConverter.Attached>
		test
	</clr:ClassWithTypeConverter.Attached>
</Rectangle>
"));
		}
	}

	public class ClassWithoutTypeConverter : DependencyObject
	{
		public static readonly DependencyProperty ConverterOnAttachedGetterProperty =
		   DependencyProperty.RegisterAttached("ConverterOnAttachedGetter", typeof(ConvertedObject), typeof(ClassWithoutTypeConverter), null);

		public static readonly DependencyProperty ConverterOnAttachedSetterProperty =
			DependencyProperty.RegisterAttached("ConverterOnAttachedSetter", typeof(ConvertedObject), typeof(ClassWithoutTypeConverter), null);

		[TypeConverter (typeof (ConvertedObjectConverter))]
		public static ConvertedObject GetConverterOnAttachedGetter(DependencyObject o)
		{
			return (ConvertedObject)o.GetValue(ConverterOnAttachedGetterProperty);
		}

		public static void SetConverterOnAttachedGetter(DependencyObject o, ConvertedObject value)
		{
			o.SetValue(ConverterOnAttachedGetterProperty, value);
		}

		public static ConvertedObject GetConverterOnAttachedSetter(DependencyObject o)
		{
			return (ConvertedObject)o.GetValue(ConverterOnAttachedSetterProperty);
		}

		[TypeConverter (typeof (ConvertedObjectConverter))]
		public static void SetConverterOnAttachedSetter(DependencyObject o, ConvertedObject value)
		{
			o.SetValue(ConverterOnAttachedSetterProperty, value);
		}
	}

	[TypeConverter(typeof(ConvertedObjectConverter))]
	public class ClassWithTypeConverter
	{
		public static readonly DependencyProperty AttachedProperty =
			DependencyProperty.RegisterAttached("Attached", typeof(ConvertedObject), typeof(ClassWithTypeConverter), null);

		public static ConvertedObject GetAttached(DependencyObject o)
		{
			return (ConvertedObject)o.GetValue(AttachedProperty);
		}
		public static void SetAttached(DependencyObject o, ConvertedObject value)
		{
			o.SetValue(AttachedProperty, value);
		}
	}

	public class ConvertedObject
	{
		public object Value { get; set; }
	}

	public class ConvertedObjectConverter : TypeConverter
	{
		public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
		{
			return true;
		}

		public override bool CanConvertTo(ITypeDescriptorContext context, Type destinationType)
		{
			return true;
		}
		public override object ConvertFrom(ITypeDescriptorContext context, global::System.Globalization.CultureInfo culture, object value)
		{
			return new ConvertedObject { Value = value };
		}
		public override object ConvertTo(ITypeDescriptorContext context, global::System.Globalization.CultureInfo culture, object value, Type destinationType)
		{
			return new ConvertedObject { Value = value };
		}
	}
}