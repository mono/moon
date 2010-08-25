//
// XamlElement.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2007 Novell, Inc.
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

using Mono;

using System;
using System.IO;
using System.Xml;
using System.Text;
using System.Linq;
using System.Reflection;
using System.Collections;
using System.ComponentModel;
using System.Collections.Generic;

using System.Windows;
using System.Windows.Data;
using System.Windows.Markup;
using System.Windows.Controls;

namespace Mono.Xaml {

	// With types like structs and strings the underlying object can be changed between the time
	// we instantiate the object and the time we set properties on it.  This class allows us to
	// instantiate an unmutable object such as a string, change its underlying object to a new
	// string when its content data is set, and then perform property sets on the correct string
	// object later, instead of using the original string.
	internal class MutableObject {
		public object Object;

		public MutableObject (object o)
		{
			Object = o;
		}
	}

	internal abstract class XamlPropertySetter {

		protected XamlPropertySetter (XamlObjectElement element, string name, TypeConverter converter)
		{
			Element = element;
			Name = name;
			Converter = converter;
		}

		public XamlParser Parser {
			get { return Element.Parser; }
		}

		public XamlObjectElement Element {
			get;
			private set;
		}

		public string Name {
			get;
			private set;
		}

		public abstract Type Type {
			get;
		}

		public abstract Type DeclaringType {
			get;
		}

		public TypeConverter Converter {
			get;
			private set;
		}

		public void SetValue (object value)
		{
			SetValue (null, value);
		}

		public virtual object ConvertTextValue (string value)
		{
			return XamlTypeConverter.ConvertObject (Parser, Element, Type, Converter, Name, value);
		}

		public object ConvertValue (Type type, object value)
		{
			if (value == null)
				return null;

			if (value is Binding || value is TemplateBindingExpression)
				return value;

			MutableObject mutable = value as MutableObject;
			if (mutable != null)
				value = mutable.Object;

			Type valueType = value.GetType ();
			if (type.IsAssignableFrom (valueType))
				return value;

			TypeConverter converter = Converter;
			if (converter == null) {
				try {
					converter = new XamlTypeConverter (Parser, Element, Name, type);
				} catch (Exception e) {
					Console.Error.WriteLine ("Exception while creating type converter (this is a recoverable error.)");
					Console.Error.WriteLine (e);
					converter = null;
				}
			}

			if (converter != null && converter.CanConvertFrom (valueType))
				return converter.ConvertFrom (value);

			try {
				if (!valueType.IsSubclassOf (type))
					value = Convert.ChangeType (value, type, System.Globalization.CultureInfo.CurrentCulture);
			} catch {
			}
			
			// This will just let things fail
			return value;
		}

		/// Find the dependency property that corresponds to this
		/// property. Technically a property does not need a
		/// corresponding 
		public DependencyProperty LookupDependencyProperty ()
		{
			Type type = DeclaringType;

			Types.Ensure (type);

			Kind kind = Deployment.Current.Types.TypeToNativeKind (type);
			if (kind == Kind.INVALID)
				return null;

			try {
				return DependencyProperty.Lookup (kind, Name);
			} catch {
				return null;
			}
		}

		public void SetBinding (Binding binding, object obj)
		{
			DependencyProperty prop = LookupDependencyProperty ();

			if (prop == null)
				throw Parser.ParseException ("Invalid Binding, can not find DependencyProperty {0}.", Name);

			DependencyObject dob = obj as DependencyObject;

			if (dob == null)
				throw Parser.ParseException ("Bindings can not be used on non DependencyObject types.");

			BindingOperations.SetBinding (dob, prop, binding);
		}

		
		public void SetTemplateBinding (TemplateBindingExpression tb, object obj)
		{
			DependencyObject dob = obj as DependencyObject;
			FrameworkElement fwe = obj as FrameworkElement;

			if (dob == null)
				throw Parser.ParseException ("Invalid TemplateBinding, expressions must be bound to DependendyObjects.");

			// Applying a {TemplateBinding} to a DO which is not a FrameworkElement should silently discard the binding.
			if (fwe == null)
				return;

			if (Parser.Context == null || Parser.Context.Template == null)
				throw Parser.ParseException ("Invalid TemplateBinding, expressions can not be used outside of FrameworkTemplate.");

			FrameworkElement source = Parser.Context.TemplateBindingSource;
			if (source == null) 
				throw Parser.ParseException ("Invalid TemplateBinding, expression can not be used outside of a FrameworkTemplate.");

			DependencyProperty source_prop = DependencyProperty.Lookup (source.GetKind(), tb.SourcePropertyName);
			if (source_prop == null)
				throw Parser.ParseException ("Invalid TemplateBinding, property {0} could not be found.", tb.SourcePropertyName);

			DependencyProperty prop = LookupDependencyProperty ();
			if (prop == null)
				throw Parser.ParseException ("Invalid TemplateBinding, property {0} could not be found.", Name);

			tb.TargetProperty = prop;
			tb.SourceProperty = source_prop;

			fwe.SetTemplateBinding (prop, tb);
		}

		public abstract void SetValue (XamlObjectElement obj, object value);
	}

	internal class XamlReflectionPropertySetter : XamlPropertySetter {

		private object target;
		private bool is_mutable;
		private PropertyInfo prop;

		public XamlReflectionPropertySetter (XamlObjectElement element, object target, PropertyInfo prop) : base (element, prop.Name, Helper.GetConverterFor (prop, prop.PropertyType))
		{
			this.target = target;
			this.prop = prop;

			if (target is MutableObject)
				is_mutable = true;
		}

		public override Type Type {
			get { return prop.PropertyType; }
		}

		public override Type DeclaringType {
			get { return prop.DeclaringType; }
		}

		public PropertyInfo PropertyInfo {
			get { return prop; }
		}

		public object Target {
			get {
				if (is_mutable) {
					MutableObject obj = (MutableObject) target;
					return obj.Object;
				}
				return target;
			}
		}

		public object GetValue ()
		{
			return prop.GetValue (Target, null);
		}

		public override void SetValue (XamlObjectElement obj, object value)
		{
			MutableObject mutable = value as MutableObject;
			if (mutable != null)
				value = mutable.Object;

			if (!typeof (Binding).IsAssignableFrom (Type)) {
				Binding binding = value as Binding;
				if (binding != null) {
					SetBinding (binding, Target);
					return;
				}
			}

			if (!typeof (TemplateBindingExpression).IsAssignableFrom (Type)) {
				TemplateBindingExpression tb = value as TemplateBindingExpression;
				if (tb != null) {
					SetTemplateBinding (tb, Target);
					return;
				}
			}

			// We do this before lists to cover the case where you are setting a list to a list or
			// a resource dictionary to a resource dictionary, ect
			// as opposed to adding items to the list or dictionary.
			//
			// null is a legal value here because they may have done something like foo="{x:Null}"
			//
			if (value == null || Type.IsAssignableFrom (value.GetType ())) {
				prop.SetValue (Target, ConvertValue (Type, value), null);
				return;
			}

			if (typeof (IList).IsAssignableFrom (Type)) {
				AddToCollection (obj, value);
				return;
			}

			if (typeof (IDictionary).IsAssignableFrom (Type)) {
				AddToDictionary (obj, value);
				return;
			}

			throw Parser.ParseException ("Unable to set property {0} to value {1}.", Name, value);
		}

		private void AddToCollection (XamlObjectElement obj, object value)
		{
			IList list = prop.GetValue (target, null) as IList;
			if (list == null) {
				throw Parser.ParseException ("Collection property in non collection type.");
			}

			list.Add (value);
		}

		private void AddToDictionary (XamlObjectElement obj, object value)
		{
			IDictionary rd = prop.GetValue (target, null) as IDictionary;
			if (rd == null)
				throw Parser.ParseException ("Collection property in non collection type.");

			string key = obj.GetDictionaryKey ();
			if (key == null)
				throw Parser.ParseException ("You must specify an x:Key or x:Name for elements in a ResourceDictionary");

			rd.Add (key, value);
		}
	}

	internal class XamlReflectionEventSetter : XamlPropertySetter {

		private object target;
		private EventInfo evnt;

		public XamlReflectionEventSetter (XamlObjectElement element, object target, EventInfo evnt) : base (element, evnt.Name,
				Helper.GetConverterFor (evnt, evnt.EventHandlerType))
		{
			this.target = target;
			this.evnt = evnt;
		}

		public override Type Type {
			get { return evnt.EventHandlerType; }
		}

		public override Type DeclaringType {
			get { return evnt.DeclaringType; }
		}

		public EventInfo EventInfo {
			get { return evnt; }
		}

		public override object ConvertTextValue (string value)
		{
			// Just leave them as strings, we do the method
			// lookup when SetValue is called.

			return value;
		}

		public override void SetValue (XamlObjectElement obj, object value)
		{
			MethodInfo invoker_info = evnt.EventHandlerType.GetMethod ("Invoke");
			ParameterInfo [] event_params = invoker_info.GetParameters ();
			string handler_name = value as string;
			XamlObjectElement subscriber = Parser.TopElement as XamlObjectElement;

			if (subscriber == null)
				throw Parser.ParseException ("Attempt to set an event handler on an invalid object.");

			if (String.IsNullOrEmpty (handler_name))
				throw Parser.ParseException ("Attmept to set an event handler to null.");


			Delegate d = null;
			MethodInfo [] methods = subscriber.Type.GetMethods (XamlParser.EVENT_BINDING_FLAGS);
			MethodInfo candidate = null;
			bool name_match = false;

			for (int i = 0; i < methods.Length; i++) {
				MethodInfo m = methods [i];
				ParameterInfo [] parameters;
				
				if (m.Name != handler_name)
					continue;

				if (name_match)
					throw Parser.ParseException ("Multiple event handlers found with same name.");

				name_match = true;

				if (m.ReturnType != typeof (void))
					continue;

				parameters = m.GetParameters ();
				if (parameters.Length != event_params.Length)
					continue;

				bool match = true;
				for (int p = 0; p < parameters.Length; p++) {
					if (!event_params [p].ParameterType.IsSubclassOf (parameters [p].ParameterType) && parameters [p].ParameterType != event_params [p].ParameterType) {
						Console.Error.WriteLine ("mismatch:  {0}  and {1}", parameters [p].ParameterType, event_params [p].ParameterType);
						match = false;
						break;
					}
				}

				if (!match)
					continue;

				if (candidate != null)
					throw Parser.ParseException ("Multiple event handler candidates found for event {0}", Name);

				candidate = m;
			}

			if (candidate == null)
				throw Parser.ParseException ("Event handler not found for event {0}.", Name);

			d = Delegate.CreateDelegate (evnt.EventHandlerType, subscriber.Object, candidate, false);
			if (d == null)
				throw Parser.ParseException ("Unable to create event delegate for event {0}.", Name);

			evnt.AddEventHandler (target, d);
		}
	}

	internal class XamlAttachedPropertySetter : XamlPropertySetter {

		private MethodInfo getter;
		private MethodInfo setter;

		public XamlAttachedPropertySetter (XamlObjectElement element, string name, MethodInfo getter, MethodInfo setter) : base (element, name, Helper.GetConverterFor (setter, getter.ReturnType))
		{
			this.getter = getter;
			this.setter = setter;
		}

		public override Type Type {
			get { return getter.ReturnType; }
		}

		public override Type DeclaringType {
			get { return getter.DeclaringType; }
		}

		public override void SetValue (XamlObjectElement obj, object value)
		{
			if (!typeof (Binding).IsAssignableFrom (Type)) {
				Binding binding = value as Binding;
				if (binding != null) {
					SetBinding (binding, obj.Object);
					return;
				}
			}

			if (!typeof (TemplateBindingExpression).IsAssignableFrom (Type)) {
				TemplateBindingExpression tb = value as TemplateBindingExpression;
				if (tb != null) {
					SetTemplateBinding (tb, obj.Object);
					return;
				}
			}

			if (value == null || Type.IsAssignableFrom (value.GetType ())) {
				setter.Invoke (null, new object [] { Element.Object, ConvertValue (Type, value) });
				return;
			}
				
			if (typeof (IList).IsAssignableFrom (Type)) {
				AddToCollection (value);
				return;
			}
		}

		public void AddToCollection (object value)
		{
			IList list = getter.Invoke (null, new object [] { Element.Object }) as IList;

			if (list == null)
				throw Parser.ParseException ("Attempt to add attached property to empty list.");

			list.Add (value);
		}

	}
}


