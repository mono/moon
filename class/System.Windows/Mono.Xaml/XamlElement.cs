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

		public XamlPropertyElement (XamlParser parser, string name, XamlPropertySetter setter) : base (parser, name)
		{
			Setter = setter;
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

		}

		public override XamlPropertySetter LookupProperty (XmlReader reader)
		{
			throw Parser.ParseException ("Property element {0} found inside property element {1}.", reader.LocalName, Name);
		}

		public void DoSet ()
		{
			foreach (XamlObjectElement obj in Children) {
				Setter.SetValue (obj, obj.Object);
			}
		}
	}

	internal class XamlObjectElement : XamlElement {

		public XamlObjectElement (XamlParser parser, string name, object o) : base (parser, name)
		{
			Object = o;
		}

		public object Object {
			get;
			set;
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
			get { return Object.GetType (); }
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
			IList list = Object as IList;
			if (list != null) {
				list.Add (obj.Object);
				return;
			}

			IDictionary dict = Object as IDictionary;
			if (dict != null) {
				dict.Add (obj.GetDictionaryKey (), obj.Object);
				return;
			}

			XamlReflectionPropertySetter content_property = FindContentProperty ();
			if (content_property == null)
				throw Parser.ParseException ("Unable to add element {0} to element {1}.", obj.Name, Name);

			content_property.SetValue (obj.Object);
		}

		public XamlReflectionPropertySetter FindContentProperty ()
		{
			ContentPropertyAttribute [] atts = (ContentPropertyAttribute []) Type.GetCustomAttributes (typeof (ContentPropertyAttribute), true);
			if (atts.Length == 0) {
				// If there wasn't an explicitly set content property (via attributes)
				// attempt to use a property named Content. Will return null if the
				// property is not found.
				Console.WriteLine ("using baked in content property.");
				return XamlReflectionPropertyForName (Object, "Content");
			}

			ContentPropertyAttribute cpa = (ContentPropertyAttribute ) atts [0];
			return XamlReflectionPropertyForName (Object, cpa.Name);
		}

		public override XamlPropertySetter LookupProperty (XmlReader reader)
		{
			if (IsAttachedProperty (reader))
				return LookupAttachedProperty (reader);

			XamlPropertySetter prop = LookupReflectionProperty (reader);
			return prop;
		}

		private bool IsAttachedProperty (XmlReader reader)
		{
			int dot = reader.LocalName.IndexOf ('.');

			if (dot < 0)
				return false;

			Type t = Parser.ResolveType ();
			if (t.IsAssignableFrom (Type))
				return false;
			return true;
		}

		private XamlPropertySetter LookupReflectionProperty (XmlReader reader)
		{
			Type t = Object.GetType ();
			XamlPropertySetter prop = XamlReflectionPropertyForName (Object, reader.LocalName);
			if (prop != null)
				return prop;

			XamlPropertySetter evnt = XamlReflectionEventForName (Object, reader.LocalName);
			if (evnt != null)
				return evnt;

			return null;
		}

		private XamlReflectionPropertySetter XamlReflectionPropertyForName (object target, string name)
		{
			PropertyInfo p = target.GetType ().GetProperty (PropertyName (name), XamlParser.PROPERTY_BINDING_FLAGS);
			if (p == null)
				return null;

			return new XamlReflectionPropertySetter (this, target, p);
		}

		public XamlReflectionEventSetter XamlReflectionEventForName (object target, string name)
		{
			EventInfo e = target.GetType ().GetEvent (name);
			if (e == null)
				return null;

			return new XamlReflectionEventSetter (this, target, e);
		}

		private XamlPropertySetter LookupAttachedProperty (XmlReader reader)
		{
			Type t = Parser.ResolveType ();
			string name = AttachedPropertyName (reader.LocalName);
			MethodInfo getter = ResolveAttachedPropertyGetter (name, t);

			if (getter == null) {
				return null;
			}

			MethodInfo setter = ResolveAttachedPropertySetter (name, t, getter.ReturnType);

			return new XamlAttachedPropertySetter (this, name, getter, setter);
		}

		private string AttachedPropertyName (string name)
		{
			int dot = name.IndexOf ('.');
			return name.Substring (++dot, name.Length - dot);
		}

		private string PropertyName (string name)
		{
			int dot = name.IndexOf ('.');
			if (dot < 0)
				return name;
			return name.Substring (++dot, name.Length - dot);
		}

		private MethodInfo ResolveAttachedPropertyGetter (string base_name, Type type)
		{
			Type [] arg_types = new Type [] { typeof (DependencyObject) };
			string full_name = String.Concat ("Get", base_name);

			MethodInfo [] methods = type.GetMethods (XamlParser.METHOD_BINDING_FLAGS);
			foreach (MethodInfo method in methods) {
				if (method.Name != full_name)
					continue;

				ParameterInfo [] the_params = method.GetParameters ();
				if (the_params.Length != arg_types.Length)
					continue;

				bool match = true;
				for (int i = 0; i < arg_types.Length; i++) {
					if (!arg_types [i].IsAssignableFrom (the_params [i].ParameterType)) {
						match = false;
						break;
					}
				}

				if (match)
					return method;
			}

			return null;
		}

		private MethodInfo ResolveAttachedPropertySetter (string base_name, Type type, Type return_type)
		{
			return type.GetMethod (String.Concat ("Set", base_name), XamlParser.METHOD_BINDING_FLAGS);
		}
	}
}

