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
using System.Linq.Expressions;
using System.Reflection;
using System.Collections;
using System.ComponentModel;
using System.Collections.Generic;

using System.Windows;
using System.Windows.Data;
using System.Windows.Markup;
using System.Windows.Controls;

namespace Mono.Xaml {

	internal abstract class XamlElement {

		public XamlElement (XamlParser parser, string name)
		{
			Parser = parser;
			Name = name;
		}

		public XamlParser Parser {
			get;
			private set;
		}

		public string Name {
			get;
			private set;
		}

		public XamlElement Parent {
			get;
			set;
		}

		public abstract Type Type {
			get;
		}

		/// NOTE: This means has the opening element ended. ie imagining the . is the current
		/// parsing marker, all of the following should return true:
		/// <Element>.</Element>
		/// <Element />.
		/// and these would return false:
		/// <Element . Foo="bar" />
		/// <Element .>
		public bool Ended {
			get;
			set;
		}

		public List<XamlElement> Children = new List<XamlElement> ();

		
		public void RaiseElementBegin ()
		{
			if (BeginElement != null)
				BeginElement (this, EventArgs.Empty);
		}

		public void RaiseElementEnd ()
		{
			if (EndElement != null)
				EndElement (this, EventArgs.Empty);
		}

		public event EventHandler BeginElement;
		public event EventHandler EndElement;

		public abstract void AddChild (XamlElement child);
		public abstract XamlPropertySetter LookupProperty (XmlReader reader);
	}

	internal class XamlPropertyElement : XamlElement {

		private bool set_on_add;

		public XamlPropertyElement (XamlParser parser, string name, XamlPropertySetter setter) : base (parser, name)
		{
			Setter = setter;

			DetermineIfSetOnAdd ();
		}

		public XamlPropertySetter Setter {
			get;
			private set;
		}

		public override Type Type {
			get { return Setter.Type; }
		}

		public override void AddChild (XamlElement child)
		{
			if (!set_on_add)
				return;

			XamlObjectElement element = child as XamlObjectElement;
			if (element == null)
				return;

			Setter.SetValue (element, element.Object);
		}

		public override XamlPropertySetter LookupProperty (XmlReader reader)
		{
			throw Parser.ParseException ("Property element {0} found inside property element {1}.", reader.LocalName, Name);
		}

		public void DoSet ()
		{
			if (!set_on_add) {
				foreach (XamlObjectElement obj in Children) {
					Setter.SetValue (obj, obj.Object);
				}
			}
		}

		private void DetermineIfSetOnAdd ()
		{
			if (typeof (IList).IsAssignableFrom (Type)) {
				set_on_add = true;
				return;
			}

			if (typeof (IDictionary).IsAssignableFrom (Type)) {
				set_on_add = true;
				return;
			}
		}
	}

	internal class XamlObjectElement : XamlElement {

		private Type type;

		public XamlObjectElement (XamlParser parser, string name, Type type, object o) : base (parser, name)
		{
			this.type = type;

			Object = o;
		}

		public object Object {
			get;
			private set;
		}

		public string X_Key {
			get;
			set;
		}

		public string X_Name {
			get;
			set;
		}

		public FrameworkElement FrameworkElement {
			get { return Object as FrameworkElement; }
		}

		public DependencyObject DependencyObject {
			get { return Object as DependencyObject; }
		}

		public override Type Type {
			get { return type; }
		}

		public string GetDictionaryKey ()
		{
			if (X_Key != null)
				return X_Key;
			if (X_Name != null)
				return X_Name;

			Style s = Object as Style;
			if (s != null && s.TargetType != null)
				return s.TargetType.ToString ();

			return null;
		}

		public override void AddChild (XamlElement child)
		{
			XamlPropertyElement prop = child as XamlPropertyElement;
			if (prop != null) {
				AddChildProperty (prop);
				return;
			}

			AddChildObject ((XamlObjectElement) child);
		}

		private void AddChildProperty (XamlPropertyElement prop)
		{
			prop.DoSet ();
		}

		private void AddChildObject (XamlObjectElement obj)
		{
			object value = obj.Object;
			MutableObject mutable = value as MutableObject;
			if (mutable != null)
				value = mutable.Object;

			IList list = Object as IList;
			if (list != null) {
				list.Add (value);
				return;
			}

			IDictionary dict = Object as IDictionary;
			if (dict != null) {
				dict.Add (obj.GetDictionaryKey (), value);
				return;
			}

			XamlReflectionPropertySetter content_property = FindContentProperty ();
			if (content_property == null)
				throw Parser.ParseException ("Unable to add element {0} to element {1}.", obj.Name, Name);

			content_property.SetValue (value);
		}

		public XamlReflectionPropertySetter FindContentProperty ()
		{
			Type type = null;
			ContentPropertyAttribute cnt = FindContentPropertyAttribute (out type);
			if (cnt == null) {
				// If there wasn't an explicitly set content property (via attributes)
				// attempt to use a property named Content. Will return null if the
				// property is not found.
				return XamlReflectionPropertyForName (Type, "Content");
			}


			return XamlReflectionPropertyForName (type, cnt.Name);
		}

		private ContentPropertyAttribute FindContentPropertyAttribute (out Type type)
		{
			type = Type;

			while (type != typeof (object) && type != typeof (ValueType)) {
				ContentPropertyAttribute [] atts = (ContentPropertyAttribute []) type.GetCustomAttributes (typeof (ContentPropertyAttribute), false);
				if (atts.Length == 0) {
					type = type.BaseType;
					continue;
				}
				
				return atts [0];
			}

			type = null;
			return null;
		}

		public override XamlPropertySetter LookupProperty (XmlReader reader)
		{
			if (IsNameProperty (reader))
				return new XamlNamePropertySetter (this, (DependencyObject) Object);

			XamlPropertySetter prop = LookupAttachedProperty (reader);
			if (prop != null)
				return prop;
	
			prop = LookupReflectionProperty (reader);

			return prop;
		}

		private bool IsNameProperty (XmlReader reader)
		{
			return (reader.LocalName == "Name" && Type.IsSubclassOf (typeof (DependencyObject)));
		}

		private bool IsAttachedProperty (XmlReader reader)
		{
			int dot = reader.LocalName.IndexOf ('.');

			if (dot < 0)
				return false;

			if (reader.NodeType == XmlNodeType.Attribute)
				return true;

			Type t = Parser.ResolveType ();
			if (t.IsAssignableFrom (Type))
				return false;
			return true;
		}

		private XamlPropertySetter LookupReflectionProperty (XmlReader reader)
		{
			XamlPropertySetter prop = XamlReflectionPropertyForName (Type, reader.LocalName);
			if (prop != null)
				return prop;

			object target = Object;
			MutableObject obj = Object as MutableObject;
			if (obj != null)
				target = obj.Object;
			
			XamlPropertySetter evnt = XamlReflectionEventForName (target, reader.LocalName);
			if (evnt != null)
				return evnt;

			XamlPropertySetter attached = XamlImplicitAttachedPropertyForName (target, reader.LocalName);
			if (attached != null)
				return attached;

			return null;
		}

		private XamlReflectionPropertySetter XamlReflectionPropertyForName (Type target, string name)
		{
			Accessors accessors;
			var key = new CachedAccessorKey (name, target);
			if (!XamlContext.PropertyAccessorCache.TryGetValue (key, out accessors)) {
				PropertyInfo p = target.GetProperty (PropertyName (name), XamlParser.PROPERTY_BINDING_FLAGS);
				if (p != null) {
					accessors = new Accessors (
						CreateGetter (p.GetGetMethod (true)),
						CreateSetter (p.GetSetMethod (true)),
						p.PropertyType,
						p.Name,
						Helper.GetConverterCreatorFor (p, p.PropertyType),
						p.DeclaringType
					);
				}
				XamlContext.PropertyAccessorCache.Add (key, accessors);
			}

			return XamlReflectionPropertySetter.Create (this, Object, accessors);
		}

		public XamlReflectionEventSetter XamlReflectionEventForName (object target, string name)
		{
			EventInfo e = target.GetType ().GetEvent (name);
			if (e == null)
				return null;

			return new XamlReflectionEventSetter (this, target, e);
		}

		private XamlPropertySetter XamlImplicitAttachedPropertyForName (object target, string name)
		{
			Accessors accessors;
			var dictKey = new CachedAccessorKey (name, Type);

			if (!XamlContext.AttachedPropertyAccessorCache.TryGetValue (dictKey, out accessors)) {
				accessors = CreateAttachedAccessors (this, Type, name);
				XamlContext.AttachedPropertyAccessorCache.Add (dictKey, accessors);
			}

			return XamlAttachedPropertySetter.Create (this, accessors);
		}

		private XamlPropertySetter LookupAttachedProperty (XmlReader reader)
		{
			Accessors accessors;
			var key = new CachedAccessorKey (reader.LocalName, Parser.ResolveType ());

			if (!XamlContext.AttachedPropertyAccessorCache.TryGetValue (key, out accessors)) {
				if (key.Type != null && key.Name != null) {
					var name = AttachedPropertyName (key.Name);
					accessors = CreateAttachedAccessors (this, key.Type, name);
				}
				XamlContext.AttachedPropertyAccessorCache.Add (key, accessors);
			}

			return XamlAttachedPropertySetter.Create (this, accessors);
		}

		static Accessors CreateAttachedAccessors (XamlObjectElement parent, Type t, string name)
		{
			MethodInfo getter = ResolveAttachedPropertyGetter (name, t);
			MethodInfo setter = ResolveAttachedPropertySetter (name, t, getter == null ? null : getter.ReturnType);

			if (getter == null && setter == null)
				return null;

			if (setter == null && !IsCollectionType (getter.ReturnType))
				return null;

			Func<TypeConverter> converter = null;
			if (getter != null)
				converter = Helper.GetConverterCreatorFor (getter, getter.ReturnType);

			return new Accessors (
				CreateAttachedGetter (getter),
				CreateAttachedSetter (setter),
				getter != null ? getter.ReturnType : setter.GetParameters () [1].ParameterType,
				name,
				converter,
				getter != null ? getter.DeclaringType : setter.DeclaringType
			);
		}

		private static bool IsCollectionType (Type t)
		{
			return (typeof (IDictionary).IsAssignableFrom (t) || typeof (IList).IsAssignableFrom (t));
		}

		public static string AttachedPropertyName (string name)
		{
			int dot = name.IndexOf ('.');
			if (dot < 1)
				return null;
			return name.Substring (++dot, name.Length - dot);
		}

		private string PropertyName (string name)
		{
			int dot = name.IndexOf ('.');
			if (dot < 0)
				return name;
			return name.Substring (++dot, name.Length - dot);
		}

		private static MethodInfo ResolveAttachedPropertyGetter (string base_name, Type type)
		{
			Type [] arg_types = new Type [] { typeof (DependencyObject) };
			string full_name = String.Concat ("Get", base_name);

			return ResolveAttachedPropertyMethod (full_name, type, arg_types);
		}

		private static MethodInfo ResolveAttachedPropertySetter (string base_name, Type type, Type return_type)
		{
			Type [] arg_types = new Type [] { typeof (DependencyObject), return_type };
			string full_name = String.Concat ("Set", base_name);

			return ResolveAttachedPropertyMethod (full_name, type, arg_types);
		}

		private static MethodInfo ResolveAttachedPropertyMethod (string name, Type type, Type [] arg_types)
		{
			MethodInfo [] methods = type.GetMethods (XamlParser.METHOD_BINDING_FLAGS);
			foreach (MethodInfo method in methods) {
				if (method.Name != name)
					continue;

				ParameterInfo [] the_params = method.GetParameters ();
				if (the_params.Length != arg_types.Length)
					continue;

				bool match = true;
				for (int i = 0; i < arg_types.Length; i++) {
					if (arg_types [i] != null && !arg_types [i].IsAssignableFrom (the_params [i].ParameterType)) {
						match = false;
						break;
					}
				}

				if (match)
					return method;
			}

			return null;
		}

		private static Func<object, object> CreateGetter(MethodInfo getMethod)
		{
			if (getMethod == null)
				return null;

			// If the method is private we won't be able to generate an optimised delegate for it
			// so just return delegate which invokes the MethodInfo.
			if (!getMethod.IsPublic)
				return (t) => InvokeGetter (getMethod, t);

			var target = System.Linq.Expressions.Expression.Parameter(typeof(object), "a");
			System.Linq.Expressions.Expression getter =
				System.Linq.Expressions.Expression.Call(
					System.Linq.Expressions.Expression.Convert(target, getMethod.DeclaringType),
				getMethod
			);

			getter = System.Linq.Expressions.Expression.Convert(getter, typeof(object));
			try {
				return System.Linq.Expressions.Expression.Lambda<Func<object, object>>(getter, target).Compile ();
			} catch {
				return (t) => InvokeGetter (getMethod, t);
			}
		}

		private static Func<object, object> CreateAttachedGetter(MethodInfo getMethod)
		{
			if (getMethod == null)
				return null;

			// If the method is private we won't be able to generate an optimised delegate for it
			// so just return delegate which invokes the MethodInfo.
			if (!getMethod.IsPublic)
				return (t) => InvokeAttachedGetter (getMethod, t);

			var target = System.Linq.Expressions.Expression.Parameter(typeof(object), "a");
			System.Linq.Expressions.Expression getter =
				System.Linq.Expressions.Expression.Call(
					getMethod,
					System.Linq.Expressions.Expression.Convert(target, getMethod.GetParameters ()[0].ParameterType)
			);

			getter = System.Linq.Expressions.Expression.Convert(getter, typeof(object));
			try {
				return System.Linq.Expressions.Expression.Lambda<Func<object, object>>(getter, target).Compile();
			} catch {
				return (t) => InvokeAttachedGetter (getMethod, t);
			}
		}

		private static Action<object, object> CreateSetter(MethodInfo setMethod)
		{
			if (setMethod == null)
				return null;

			var parameterType = setMethod.GetParameters ()[0].ParameterType;
			var target = System.Linq.Expressions.Expression.Parameter(typeof(object), "a");
			var value = System.Linq.Expressions.Expression.Parameter(typeof(object), "b");

			// If the object we're setting the value on is a value type we need to use
			// the PropertyInfo to set the value, otherwise we'll end up modifying a
			// copy of the value type instead of the actual one.
			if (parameterType.IsValueType || !setMethod.IsPublic)
				return (t, v) => InvokeSetter (setMethod, t, v);

			var setter = System.Linq.Expressions.Expression.Call (
				System.Linq.Expressions.Expression.Convert (target, setMethod.DeclaringType),
				setMethod,
				System.Linq.Expressions.Expression.Convert (value, parameterType)
			);

			try {
				return System.Linq.Expressions.Expression.Lambda<Action<object, object>>(setter, target, value).Compile();
			} catch {
				return (t, v) => InvokeSetter (setMethod, t, v);
			}
		}

		private static Action<object, object> CreateAttachedSetter(MethodInfo setMethod)
		{
			if (setMethod == null)
				return null;

			// If the method is private we won't be able to generate an optimised delegate for it
			// so just return delegate which invokes the MethodInfo.
			if (!setMethod.IsPublic)
				return (t, v) => InvokeAttachedSetter (setMethod, t, v);

			var target = System.Linq.Expressions.Expression.Parameter(typeof(object), "a");
			var value = System.Linq.Expressions.Expression.Parameter(typeof(object), "b");

			var setter = System.Linq.Expressions.Expression.Call(
				setMethod,
				System.Linq.Expressions.Expression.Convert(target, setMethod.GetParameters()[0].ParameterType),
				System.Linq.Expressions.Expression.Convert(value, setMethod.GetParameters()[1].ParameterType)
			);
			try {
				return System.Linq.Expressions.Expression.Lambda<Action<object, object>>(setter, target, value).Compile();
			} catch {
				return (t, v) => InvokeAttachedSetter (setMethod, t, v);
			}
		}

		static object InvokeAttachedGetter (MethodInfo getter, object target)
		{
			return getter.Invoke (null, new object [] { target });
		}

		static void InvokeAttachedSetter (MethodInfo setter, object target, object value)
		{
			setter.Invoke (null, new object [] { target, value });
		}

		static object InvokeGetter (MethodInfo getter, object target)
		{
			return getter.Invoke (target, null);
		}

		static void InvokeSetter (MethodInfo setter, object target, object value)
		{
			setter.Invoke (target, new object [] { value });
		}

	}
}

