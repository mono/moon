//
// ManagedXamlParser.cs
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

	internal class XamlParser {

		internal static readonly BindingFlags METHOD_BINDING_FLAGS = BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic;
		internal static readonly BindingFlags PROPERTY_BINDING_FLAGS = BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.FlattenHierarchy;

		private XamlElement top_element;
		private XamlElement current_element;
		private XmlReader reader;

		public XamlParser () : this (new XamlContext ())
		{
		}
		
		public XamlParser (XamlContext context) 
		{
			Context = context;
			DefaultXmlns = String.Empty;
			Xmlns = new Dictionary<string,string> ();
			IgnorableXmlns = new List<string> ();
			NameScope = new NameScope ();

			NameScope.Temporary = true;
		}

		public XamlContext Context {
			get;
			private set;
		}

		public XamlElement CurrentElement {
			get { return current_element; }
		}

		public XamlElement TopElement {
			get {
				return top_element;
			}
		}

		public bool IsTopElement {
			get {
				return top_element == null;
			}
		}

		public string DefaultXmlns {
			get;
			private set;
		}

		public List<string> IgnorableXmlns {
			get;
			private set;
		}

		public Dictionary<string,string> Xmlns {
			get;
			private set;
		}

		public object HydrateObject {
			get;
			set;
		}

		public NameScope NameScope {
			get;
			private set;
		}

		public bool CreateNameScope {
			get;
			set;
		}

		public bool ValidateTemplates {
			get;
			set;
		}

		public object ParseString (string str)
		{
			object res = null;

			try {
				res = ParseReader (new StringReader (str));
			} catch (Exception e) {
				Console.WriteLine (e);
			}

			return res;
		}

		public object ParseFile (string file)
		{
			object res = null;

			using (FileStream s = File.OpenRead (file)) {
				try {
					res = ParseReader (new StreamReader (s));
				} catch (Exception e) {
					Console.WriteLine (e);
				}
			}

			return null;
		}

		public object ParseReader (TextReader stream)
		{
			reader = XmlReader.Create (stream);

			while (reader.Read ()) {
				switch (reader.NodeType) {
				case XmlNodeType.Element:
					ParseElement ();
					break;
				case XmlNodeType.EndElement:
					ParseEndElement ();
					break;
				case XmlNodeType.Text:
					ParseText ();
					break;
				case XmlNodeType.Whitespace:
					ParseWhitespace ();
					break;
				case XmlNodeType.SignificantWhitespace:
					ParseSignificantWhitespace ();
					break;
				}
			}

			XamlObjectElement obj = TopElement as XamlObjectElement;
			if (obj == null) {
				// We actually return the type of the property here
				// or the object that it wraps
				return null;
			}

			return obj.Object;
		}

		internal object LookupNamedItem (XamlObjectElement target, string name)
		{
			object o;
			XamlElement lookup = target;

			while (lookup != null) {
				XamlObjectElement obj = lookup as XamlObjectElement;
				if (obj != null) {
					FrameworkElement fe = obj.Object as FrameworkElement;
					if (fe != null) {
						o = fe.Resources [name];
						if (o != null)
							return o;
					}
				}
				lookup = lookup.Parent;
			}

			//
			// Didn't find it in our local tree, so try the context if one is available
			//

			if (Context != null) {
				o = Context.LookupNamedItem (name);
				if (o != null)
					return o;
			}

			o = Application.Current.Resources [name];
		
			return o;
		}

		internal void RegisterKeyItem (XamlObjectElement element, XamlElement target, string key)
		{
			IDictionary rd = CurrentDictionary (element);
			if (rd == null)
				throw ParseException ("Attempt to use x:Key outside of an IDictionary.");

			element.X_Key = key;
		}

		internal void RegisterNamedItem (XamlObjectElement element, string name)
		{
			IDictionary rd = CurrentDictionary (element);
			if (rd != null && element.X_Key != null) {
				throw ParseException ("The name already exists in the tree.");
			}

			if (element.X_Name != null) {
				throw ParseException ("Cannot specify both Name and x:Name attributes.");
			}

			element.X_Name = name;

			FrameworkElement fe = element.FrameworkElement;
			if (fe != null)
				fe.SetNameOnScope (name, NameScope);
		}

		internal FrameworkTemplate GetParentTemplate (XamlElement element)
		{
			XamlElement parent = element.Parent;

			while (parent != null && !IsTemplate (parent))
				parent = parent.Parent;

			if (parent != null)
				return ((XamlObjectElement) parent).Object as FrameworkTemplate;

			if (Context == null)
				return null;

			return Context.Template;
		}

		private bool IsTemplate (XamlElement element)
		{
			XamlObjectElement obj = element as XamlObjectElement;
			if (obj == null)
				return false;
			return typeof (FrameworkTemplate).IsAssignableFrom (obj.Type);
		}

		private IDictionary CurrentDictionary (XamlElement element)
		{
			IDictionary rd = null;
			XamlObjectElement obj = null;

			if (element == null || element.Parent == null)
				return null;

			XamlPropertyElement prop = element.Parent as XamlPropertyElement;
			if (prop == null) {
				//
				// We could be in a <ResourceDictionary> tag, so we get added via ContentProperty
				//
				obj = element.Parent as XamlObjectElement;
				rd = obj.Object as IDictionary;
				if (rd != null)
					return rd;
				return null;
			}

			//
			// We aren't even in a <Something.Resources> element
			//
			if (!(typeof (IDictionary).IsAssignableFrom (prop.Type)))
				return null;

			//
			// We need to be in a resource dictionary element, and we need that element to have a
			// valid parent container.
			//
			obj = prop.Parent as XamlObjectElement;
			if (obj == null)
				return null;

			FrameworkElement fe = obj.Object as FrameworkElement;
			rd = fe.Resources;
			return rd;
		}

		private void ParseElement ()
		{
			if (IsIgnorable ()) {
				return;
			}

			if (IsPropertyElement ()) {
				ParsePropertyElement ();
				return;
			}
			if (IsTemplateElement ()) {
				ParseTemplateElement ();
				return;
			}
			ParseObjectElement ();
		}

		
		private void ParseObjectElement ()
		{
			Type t = ResolveType ();
			if (t == null)
				throw ParseException ("Unable to find the type {0}.", reader.LocalName);

			object o = InstantiateType (t);

			XamlObjectElement element = new XamlObjectElement (this, reader.LocalName, o);

			SetElementTemplateScopes (element);
			OnElementBegin (element);
			
			ParseElementAttributes (element);

			// This is a self closing element ie <Rectangle />
			if (reader.IsEmptyElement)
				OnElementEnd ();
		}

		private void ParsePropertyElement ()
		{
			Type t = ResolveType ();
			if (t == null)
				throw ParseException ("Unable to find the property {0}.", reader.LocalName);

			XamlPropertySetter setter = null;
			if (CurrentElement != null) {
				XamlObjectElement parent = CurrentElement as XamlObjectElement;

				if (parent == null)
					throw ParseException ("Property {0} does not have a parent.", reader.LocalName);
				if (!t.IsAssignableFrom (parent.Type))
					throw ParseException ("Property {0} is not a descendant if its parent object {1}.", reader.LocalName, parent.Type);
				setter = CurrentElement.LookupProperty (reader);
				if (setter == null)
					throw ParseException ("Property {0} was not found on type {1}.", reader.LocalName, CurrentElement.Name);
			}

			XamlPropertyElement element = new XamlPropertyElement (this, reader.LocalName, setter);
			OnElementBegin (element);
		}

		private void ParseTemplateElement ()
		{
			Type t = ResolveType ();
			if (t == null)
				throw ParseException ("Unable to find the type {0}", t);
			object o = InstantiateType (t);

			XamlObjectElement element = new XamlObjectElement (this, reader.LocalName, o);
			OnElementBegin (element);
			
			ParseElementAttributes (element);

			string template_xml = reader.ReadInnerXml ();
			
			FrameworkTemplate template = o as FrameworkTemplate;

			unsafe {
				template.SetXamlBuffer (ParseTemplate, CreateXamlContext (template), template_xml);
			}

			//
			// ReadInnerXml will read our closing </ControlTemplate> tag also, so we manually close things
			//
			OnElementEnd ();
		}

		private static unsafe IntPtr ParseTemplate (Value *context_ptr, string resource_base, IntPtr surface, IntPtr binding_source, string xaml, ref MoonError error)
		{
			XamlContext context = Value.ToObject (typeof (XamlContext), context_ptr) as XamlContext;

			var parser = new XamlParser (context);

			var source = NativeDependencyObjectHelper.FromIntPtr (binding_source);
			if (source == null) {
				error = new MoonError (parser.ParseException ("Attempting to parse a template with an invalid binding source."));
				return IntPtr.Zero;
			}

			FrameworkElement fwe = source as FrameworkElement;
			if (fwe == null) {
				error = new MoonError (parser.ParseException ("Only FrameworkElements can be used as TemplateBinding sources."));
				return IntPtr.Zero;
			}

			context.IsExpandingTemplate = true;
			context.TemplateBindingSource = fwe;

			INativeEventObjectWrapper dob = null;
			try {
				dob = parser.ParseString (xaml) as INativeEventObjectWrapper;
			} catch (Exception e) {
				error = new MoonError (e);
				return IntPtr.Zero;
			}

			if (dob == null) {
				error = new MoonError (parser.ParseException ("Unable to parse template item."));
				return IntPtr.Zero;
			}

			return dob.NativeHandle;
		}

		private bool IsPropertyElement ()
		{
			return reader.LocalName.IndexOf ('.') > 0;
		}

		private bool IsTemplateElement ()
		{
			return reader.LocalName == "ControlTemplate";
		}

		private bool IsObjectElement ()
		{
			return CurrentElement is XamlObjectElement;
		}

		private void ParseEndElement ()
		{
			OnElementEnd ();
		}

		private void ParseText ()
		{
			XamlObjectElement obj = CurrentElement as XamlObjectElement;

			string value = HandleWhiteSpace (reader.Value);

			if (obj != null) {
				if (typeof (TextBlock).IsAssignableFrom (obj.Type)) {
					ParseTextBlockText (obj);
					return;
				}

				XamlReflectionPropertySetter content = obj.FindContentProperty ();

				if (content == null)
					throw ParseException ("Element {0} does not support text properties.", CurrentElement.Name);
				content.SetValue (value);
			}
		}

		private void ParseTextBlockText (XamlObjectElement block)
		{
		}

		private void ParseWhitespace ()
		{
		}

		private void ParseSignificantWhitespace ()
		{
		}

		private void ParseElementAttributes (XamlObjectElement element)
		{
			if (!reader.HasAttributes)
				return;

			try {
				int ac = reader.AttributeCount;
				for (int i = 0; i < reader.AttributeCount; i++) {
					reader.MoveToAttribute (i);
					ParseAttribute (element);
				}
			} finally {
				// We do this in a finally so error reporting doesn't get all jacked up
				reader.MoveToElement();
			}
		}

		private void ParseAttribute (XamlObjectElement element)
		{
			if (IsMcAttribute ()) {
				ParseMcAttribute (element);
				return;
			}
			if (IsXmlnsMapping ()) {
				ParseXmlnsMapping (element);
				return;
			}

			if (IsXAttribute ()) {
				ParseXAttribute (element);
				return;
			}

			if (IsXmlDirective ()) {
				ParseXmlDirective (element);
				return;
			}

			if (IsIgnorable ()) {
				return;
			}

			XamlPropertySetter prop = element.LookupProperty (reader);
			if (prop == null)
				throw ParseException ("The property {0} was not found.", reader.LocalName);
			object value = ParseAttributeValue (element, prop);
			prop.SetValue (value);
		}

		private void ParseMcAttribute (XamlElement element)
		{
			if (reader.LocalName == "Ignorable") {
				IgnorableXmlns.Add (reader.Value);
				return;
			}

			throw ParseException ("Undeclared prefix");
		}

		private void ParseXmlnsMapping (XamlElement element)
		{
			if (reader.Prefix == String.Empty) {
				DefaultXmlns = reader.Value;
				return;
			}

			Xmlns [reader.Name] = reader.Value;
		}

		private void ParseXAttribute (XamlObjectElement element)
		{
			switch (reader.LocalName) {
			case "Key":
				RegisterKeyItem (element, element.Parent, reader.Value);
				return;
			case "Name":
				RegisterNamedItem (element, reader.Value);
				return;
			case "Class":
				// The class attribute is handled when we initialize the element
				return;
			default:
				throw ParseException ("Unknown x: attribute ({0}).", reader.LocalName);
			}
		}

		private void ParseXmlDirective (XamlElement element)
		{
			if (reader.LocalName == "space") {
				// Do nothing XmlReader covers this for us
			}
		}

		private object ParseAttributeValue (XamlObjectElement element, XamlPropertySetter property)
		{
			object value = null;

			if (IsMarkupExpression (reader.Value))
				value = ParseAttributeMarkup (element, property);
			else {
				value = XamlTypeConverter.ConvertObject (this, element, property.Type, property.Converter, property.Name, reader.Value);
			}

			return value;
		}

		private object ParseAttributeMarkup (XamlObjectElement element, XamlPropertySetter property)
		{
			MarkupExpressionParser parser = new SL4MarkupExpressionParser (element.Object, property.Name, this, element);

			string expression = reader.Value;
			object o = parser.ParseExpression (ref expression);
			return o;
		}

		// Markup compatibility attribute
		private bool IsMcAttribute ()
		{
			return reader.Prefix == "mc";
		}

		private bool IsXmlnsMapping ()
		{
			return reader.Prefix == "xmlns" || reader.Name == "xmlns";
		}

		private bool IsXAttribute ()
		{
			return reader.Prefix == "x";
		}

		private bool IsXmlDirective ()
		{
			return reader.Prefix == "xml";
		}

		private bool IsMarkupExpression (string str)
		{
			int open_stache = str.IndexOf ('{');
			if (open_stache < 0)
				return false;
			for (int i = 0; i < open_stache; i++) {
				if (!Char.IsWhiteSpace (str [i]))
					return false;
			}
			
			int close_stache = str.LastIndexOf ('}');
			if (close_stache < 0)
				return false;
			for (int i = str.Length - 1; i > close_stache; i--) {
				if (!Char.IsWhiteSpace (str [i]))
					return false;
			}

			return true;
		}

		private bool IsValidXmlSpaceValue (string val)
		{
			return val == "preserve" || val == "default";
		}

		private bool IsIgnorable ()
		{
			return IgnorableXmlns.Contains (reader.Prefix);
		}

		private void OnElementBegin (XamlElement element)
		{
			InitializeElement (element);
			element.RaiseElementBegin ();

			if (top_element == null)
				InitTopElement (element);

			PushCurrentElement (element);
		}

		private void OnElementEnd ()
		{
			// if (reader.LocalName != CurrentElement.Name)
			//	throw new XamlParseException ("oh shit bitch, we are ending the wrong element!");

			CurrentElement.RaiseElementEnd ();
			EndInitializeElement (CurrentElement);

			XamlElement parent = CurrentElement.Parent;
			if (parent != null)
				ParentElement (CurrentElement, parent);

			PopCurrentElement ();
		}

		private void PushCurrentElement (XamlElement element)
		{
			if (element == null) {
				current_element = null;
				return;
			}

			if (current_element != null) {
				current_element.Children.Add (element);
				element.Parent = current_element;
			}

			if (top_element == null)
				top_element = element;

			current_element = element;
		}

		private void PopCurrentElement ()
		{
			current_element = current_element.Parent;
		}

		private void InitTopElement (XamlElement element)
		{
			XamlObjectElement obj = element as XamlObjectElement;

			
			if (obj != null) {

				if (HydrateObject != null) {
					if (!obj.Type.IsAssignableFrom (HydrateObject.GetType ()))
						throw ParseException ("Invalid top-level element found {0}, expecting {1}", obj.Type, HydrateObject.GetType ());
					obj.Object = HydrateObject;
				}

				if (typeof (DependencyObject).IsAssignableFrom (obj.Type)) {
					DependencyObject dob = (DependencyObject) obj.Object;
					NameScope.SetNameScope (dob, NameScope);
				}
			}
		}
	
		private void ParentElement (XamlElement element, XamlElement parent)
		{
			parent.AddChild (element);
		}

		private void ParentPropertyElement (XamlElement element, XamlPropertyElement prop)
		{
			XamlObjectElement obj = (XamlObjectElement) element;
		}
			       
		private void InitializeElement (XamlElement element)
		{
			XamlObjectElement obj = element as XamlObjectElement;
			if (obj == null)
				return;

			ISupportInitialize init = obj.Object as ISupportInitialize;
			if (init == null)
				return;
			init.BeginInit ();
		}

		private void EndInitializeElement (XamlElement element)
		{
			XamlObjectElement obj = element as XamlObjectElement;
			if (obj == null)
				return;

			ISupportInitialize init = obj.Object as ISupportInitialize;
			if (init == null)
				return;
			init.EndInit ();
		}

		private void SetElementTemplateScopes (XamlObjectElement element)
		{
			// This whole thing is basically copied from xaml.cpp AddCreatedItem

			DependencyObject el_dob = element.Object as DependencyObject;
			if (el_dob == null)
				return;

			// When instantiating a template, some elements are created which are not explicitly
			// mentioned in the xaml. Therefore we need to keep walking up the tree until we find
			// the last element which we set a value for Control::IsTemplateItem and propagate
			// it from there.


			XamlElement instance = CurrentElement;

			while (instance != null) {

				XamlObjectElement oe = element as XamlObjectElement;
				if (oe == null) {
					instance = instance.Parent;
					continue;
				}

				DependencyObject dob = oe.Object as DependencyObject;
				if (dob == null) {
					instance = instance.Parent;
					continue;
				}

				if (dob.ReadLocalValue (Control.IsTemplateItemProperty) == null) {
					instance = instance.Parent;
					continue;
				}

				el_dob.SetValue (Control.IsTemplateItemProperty, dob.GetValue (Control.IsTemplateItemProperty));
				el_dob.TemplateOwner = dob.TemplateOwner;

				break;
			}
		
			if (instance == null) {
				el_dob.SetValue (Control.IsTemplateItemProperty, Context.IsExpandingTemplate);
				el_dob.TemplateOwner = Context.TemplateBindingSource;
			}

			if (el_dob.GetValue (Control.IsTemplateItemProperty) != null) {
				UserControl uc = el_dob as UserControl;
				if (uc != null) {
				        // I can't come up with a test to verify this fix. However, it does
				        // fix a crasher in olympics when trying to play a new video from
				        // the recommendations list after the curreont video finishes
					NameScope ns = NameScope.GetNameScope (uc);
					NameScope.SetNameScope (uc.Content, ns);
					NameScope.SetNameScope (uc.Resources, ns);
				}
				NameScope.SetNameScope (el_dob, NameScope);
			}
		}

		public Type ResolveType (string str)
		{
			int colon = str.IndexOf (':');
			string xmlns = DefaultXmlns;
			string name = str;

			if (colon > 0) {
				string local = str.Substring (colon);
				name = str.Substring (++colon, str.Length - colon);
				if (!Xmlns.TryGetValue (local, out xmlns))
					throw ParseException ("Could not find namespace for type {0}.", str);
			}

			return ResolveType (xmlns, name);
		}

		public Type ResolveType ()
		{
			if (IsTopElement) {
				string user_class = ResolveUserClass ();
				if (user_class != null)
					return LoadType (null, null, user_class);
			}
			return ResolveType (reader.NamespaceURI, reader.LocalName);
		}

		public Type ResolveType (string xmlns, string full_name)
		{
			string ns = null;
			string asm_name = null;
			Assembly assembly = null;
			string name = full_name;
			int dot = name.IndexOf ('.');

			// We resolve attached property types with this function too
			// so make sure that we trim off the property part of the name
			if (dot > 0) {
				name = name.Substring (0, dot);
				full_name = name;
			}

			if (String.IsNullOrEmpty (xmlns))
				xmlns = DefaultXmlns;

			ns = ResolveClrNamespace (xmlns);
			asm_name = ResolveAssemblyName (xmlns);
			if (ns != null)
				full_name = String.Concat (ns, ".", name);

			//
			// If no assembly is specified we pass in null and LoadType will search for the type
			//
			if (asm_name != null)
				assembly = LoadAssembly (asm_name);

			return LoadType (assembly, xmlns, full_name);
		}

		private static string ResolveClrNamespace (string xmlns)
		{
			if (String.IsNullOrEmpty (xmlns))
				return null;

			int start = xmlns.IndexOf ("clr-namespace:");

			if (start < 0)
				return null;
			start += "clr-namespace:".Length;

			int end = xmlns.IndexOf (';', start);
			if (end == -1)
				end = xmlns.Length;
			
			return xmlns.Substring (start, end - start);
		}

		private static string ResolveAssemblyName (string xmlns)
		{
			if (String.IsNullOrEmpty (xmlns))
				return null;

			int start = xmlns.IndexOf ("assembly=");
			if (start < 0)
				return null;

			start += "assembly=".Length;
			int end = xmlns.IndexOf (';', start);
			if (end == -1)
				end = xmlns.Length;
			return xmlns.Substring (start, end - start);
		}

		private string ResolveUserClass ()
		{
			string xns = reader ["xmlns:x"];

			if (xns == null)
				return null;

			return reader ["Class", xns];
		}

		private Assembly DefaultAssembly ()
		{
			return Deployment.Current.EntryAssembly;
		}

		private Assembly LoadAssembly (string name)
		{
			if (name == "mscorlib")
				return typeof (object).Assembly;
			return Application.GetAssembly (name);
		}

		private bool IsValidType (Type t)
		{
			return (t != null && t.IsPublic);
		}

		public Type LoadType (Assembly assembly, string xmlns, string name)
		{
			Type t = null;

			if (assembly == null)
				t = FindType (xmlns, name);
			else
				t = assembly.GetType (name);

			if (IsValidType (t))
				return t;

			return null;
		}

		private Type FindType (string xmlns, string name)
		{
			Type t = FindDefaultType (xmlns, name);
			if (t != null)
				return t;

			var col = from a in Deployment.Current.Assemblies where (t = a.GetType (name)) != null select t;
			if (col.Count () > 0) {
				t = col.First ();
				if (IsValidType (t))
					return t;
			}

			return t;
		}

		private Type FindDefaultType (string xmlns, string name)
		{
			Assembly assembly = typeof (DependencyObject).Assembly;
			Type t = LoadPartialTypeFromAssembly (xmlns, name, assembly);

			if (IsValidType (t))
				return t;

			return null;
		}

		private Type LoadPartialTypeFromAssembly (string xmlns, string name, Assembly asm)
		{
			XmlnsDefinitionAttribute [] xmlnsdefs = XmlnsDefsForAssembly (xmlns, asm);

			foreach (XmlnsDefinitionAttribute def in xmlnsdefs) {
				string full_name = String.Concat (def.ClrNamespace, ".", name);
				Type t = asm.GetType (full_name);
				if (IsValidType (t))
					return t;
			}

			return null;
		}

		private XmlnsDefinitionAttribute [] XmlnsDefsForAssembly (string xmlns, Assembly asm)
		{
			// TODO: We can cache these easily enough
			XmlnsDefinitionAttribute [] defs = (XmlnsDefinitionAttribute []) asm.GetCustomAttributes (typeof (XmlnsDefinitionAttribute), false);

			var res = from x in defs where x.XmlNamespace == xmlns select x;
			return res.ToArray ();
		}

		private object InstantiateType (Type type)
		{
			object o;

			// Returns null if the type isn't a collection type.
			o = InstantiateCollectionType (type);

			if (o == null)
				o = Activator.CreateInstance (type);

			// TODO: Why did I need this? 
			INativeEventObjectWrapper evo = o as INativeEventObjectWrapper;
			if (evo != null)
				NativeMethods.event_object_ref (evo.NativeHandle);

			return o;
		}

		private bool IsCollectionType (Type type)
		{
			return typeof (IList).IsAssignableFrom (type) || typeof (IDictionary).IsAssignableFrom (type);
		}

		private object InstantiateCollectionType (Type t)
		{
			if (!(typeof (IList).IsAssignableFrom (t) || typeof (IDictionary).IsAssignableFrom (t)))
				return null;

			XamlReflectionPropertySetter prop = null;

			// CurrentElement hasn't been set yet, so we are getting our type's parent here.
			XamlPropertyElement pe = CurrentElement as XamlPropertyElement;
			XamlObjectElement oe = CurrentElement as XamlObjectElement;

			if (pe != null)
				prop = pe.Setter as XamlReflectionPropertySetter;

			if (pe == null && oe != null)
				prop = oe.FindContentProperty () as XamlReflectionPropertySetter;

			if (prop == null)
				return null;

			return prop.GetValue ();
		}

		internal XamlParseException ParseException (string message, params object [] p)
		{
			int pos = -1;
			int line = -1;

			IXmlLineInfo linfo = reader as IXmlLineInfo;
			if (linfo != null) {
				line = linfo.LineNumber;
				pos = linfo.LinePosition;
			}

			return new XamlParseException (line, pos, String.Format (message, p));
		}

		private XamlContext CreateXamlContext (FrameworkTemplate template)
		{
			return new XamlContext (Context, CreateResourcesList (), template);
		}

		private List<DependencyObject> CreateResourcesList ()
		{
			var list = new List<DependencyObject> ();

			XamlElement walk = CurrentElement;
			while (walk != null) {
				XamlObjectElement obj = walk as XamlObjectElement;

				if (obj != null) {
					ResourceDictionary rd = obj.Object as ResourceDictionary;
					if (rd != null)
						list.Add (rd);

					FrameworkElement fwe = obj.Object as FrameworkElement;
					if (fwe != null)
						list.Add (fwe);
				}
				
				walk = walk.Parent;
			}

			list.Reverse ();

			return list;
		}

		private string HandleWhiteSpace (string str)
		{
			if (reader.XmlSpace == XmlSpace.Preserve)
				return str;

			StringBuilder builder = new StringBuilder (str.Length);
			for (int i = 0; i < str.Length; i++) {
				bool ws = false;
				if (Char.IsWhiteSpace (str [i])) {
					do {
						i++;
					} while (i < str.Length -1 && Char.IsWhiteSpace (str [i]));
				}
				if (ws)
					builder.Append (' ');
				else
					builder.Append (str [i]);
			}

			return builder.ToString ();
		}
	}
}
