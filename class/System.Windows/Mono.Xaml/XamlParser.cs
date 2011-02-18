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
using System.Globalization;
using System.ComponentModel;
using System.Collections.Generic;
using System.Runtime.InteropServices;

using System.Windows;
using System.Windows.Data;
using System.Windows.Media;
using System.Windows.Markup;
using System.Windows.Controls;
using System.Windows.Documents;


namespace Mono.Xaml {

	internal class XamlParser {

		internal static readonly BindingFlags METHOD_BINDING_FLAGS = BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic;
		internal static readonly BindingFlags FIELD_BINDING_FLAGS = BindingFlags.Static | BindingFlags.Public | BindingFlags.FlattenHierarchy;
		internal static readonly BindingFlags PROPERTY_BINDING_FLAGS = BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.FlattenHierarchy;
		internal static readonly BindingFlags EVENT_BINDING_FLAGS = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.DeclaredOnly | BindingFlags.Instance;

		internal static readonly NumberStyles CORLIB_INTEGER_STYLES = NumberStyles.AllowLeadingWhite |  NumberStyles.AllowTrailingWhite | NumberStyles.AllowLeadingSign | NumberStyles.AllowDecimalPoint;
		internal static readonly NumberStyles CORLIB_DOUBLE_STYLES = NumberStyles.AllowLeadingWhite |  NumberStyles.AllowTrailingWhite | NumberStyles.AllowLeadingSign | NumberStyles.AllowDecimalPoint | NumberStyles.AllowExponent;
		
		private XamlElement top_element;
		private XamlElement current_element;
		private XamlNode reader;
		private IXamlNode currentNode;
		bool bufferingTemplate;

		public XamlParser () : this (new XamlContext ())
		{
		}
		
		public XamlParser (XamlContext context) 
		{
			Context = context;
			NameScope = new NameScope ();
		}

		public XamlContext Context {
			get;
			private set;
		}

		public XamlNode Current {
			get { return reader; }
		}

		public XamlElement CurrentElement {
			get { return current_element; }
		}

		public object TopElement {
			get {
				if (Context.TopElement != null)
					return Context.TopElement;
				return top_element is XamlObjectElement ? ((XamlObjectElement) top_element).Object : null;
			}
		}

		public bool IsTopElement {
			get {
				return top_element == null;
			}
		}

		public object HydrateObject {
			get;
			set;
		}

		public object Owner {
			get;
			set;
		}

		public NameScope NameScope {
			get;
			private set;
		}

		public bool CreateNameScope {
			get { return !NameScope.Temporary; }
			set { NameScope.Temporary = !value; }
		}

		public bool ValidateTemplates {
			get;
			set;
		}

		public Uri ResourceBase {
			get;
			set;
		}

		void XamlNode_OnElementStart (XamlNode node) {
			reader = node;
			currentNode = (IXamlNode) node;

			switch (node.NodeType) {
				case XmlNodeType.Element:
					ParseElement ();
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

		void XamlNode_OnElementEnd (XamlNode node)
		{
			reader = node;
			ParseEndElement ();
		}

		void XamlNode_OnAttribute (XamlNode node, XamlAttribute ai)
		{
			if (!(CurrentElement is XamlObjectElement))
				return;
			currentNode = ai;
			ParseAttribute (CurrentElement as XamlObjectElement, ai);
			currentNode = node;
		}

		public object Parse (XamlNode node)
		{
			try {

				node.Parse (XamlNode_OnElementStart, XamlNode_OnElementEnd, XamlNode_OnAttribute);

			} catch (XamlParseException pe) {
				Console.WriteLine ("Exception while parsing string ({0}:{1})", pe.LineNumber, pe.LinePosition);
				Console.WriteLine (pe);
				Console.WriteLine ("string:");
				Console.WriteLine (node.OuterXml);
				throw pe;
			} catch (Exception e) {
				
				Console.WriteLine ("Exception while parsing string:");
				Console.WriteLine (e);
				Console.WriteLine ("string:");
				Console.WriteLine (node.OuterXml);
				throw ParseException ("Caught exception: {0}", e.Message);
			}

			XamlObjectElement obj = top_element as XamlObjectElement;
			if (obj == null) {
				// We actually return the type of the property here
				// or the object that it wraps
				return null;
			}

			return obj.Object;
		}

		public object ParseString (string str)
		{
			try {
				XamlNode.Parse (str, XamlNode_OnElementStart, XamlNode_OnElementEnd, XamlNode_OnAttribute);
			} catch (XamlParseException pe) {
				Console.WriteLine ("Exception while parsing string ({0}:{1})", pe.LineNumber, pe.LinePosition);
				Console.WriteLine (pe);
				Console.WriteLine ("string:");
				Console.WriteLine (str);
				throw pe;
			} catch (Exception e) {
				IXmlLineInfo linfo = reader as IXmlLineInfo;
				int line = linfo.LineNumber;
				int col = linfo.LinePosition;
				Console.Error.WriteLine ("Exception while parsing string ({0}:{1}):", line, col);
				Console.Error.WriteLine (e);
				Console.WriteLine ("string:");
				Console.WriteLine (str);
				throw ParseException ("Caught exception: {0}", e.Message);
			}

			XamlObjectElement obj = top_element as XamlObjectElement;
			if (obj == null) {
				// We actually return the type of the property here
				// or the object that it wraps
				return null;
			}

			return obj.Object;
		}

		public object ParseFile (string file)
		{
			string xml = File.ReadAllText (file);
			return ParseString (xml);
		}

		public object ParseReader (TextReader stream)
		{
			string xml = stream.ReadToEnd ();
			return ParseString (xml);
		}

		public static Value CreateFromString (string xaml, bool create_namescope, bool validate_templates, IntPtr owner)
		{
			XamlParser p = new XamlParser () {
				CreateNameScope = create_namescope,
				ValidateTemplates = validate_templates,
				Owner = NativeDependencyObjectHelper.FromIntPtr (owner)
			};

			object v = p.ParseString (xaml);

			return ObjectToValue (v);
		}

		public static Value CreateFromFile (string file, bool create_namescope, bool validate_templates)
		{
			XamlParser p = new XamlParser () {
				CreateNameScope = create_namescope,
				ValidateTemplates = validate_templates,
			};

			object v = p.ParseFile (file);

			return ObjectToValue (v);
		}

		public unsafe static Value HydrateFromString (string xaml, Value *obj, bool create_namescope, bool validate_templates)
		{
			XamlParser p = new XamlParser () {
				CreateNameScope = create_namescope,
				ValidateTemplates = validate_templates,
				HydrateObject = Value.ToObject (null, obj),
			};

			object v = p.ParseString (xaml);

			return ObjectToValue (v);
		}

		public static Value ObjectToValue (object value)
		{
			Value v = Value.FromObject (value);
			return v;
		}

		internal object LookupNamedItem (XamlElement target, string name)
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
					ResourceDictionary rd = obj.Object as ResourceDictionary;
					if (rd != null) {
						o = rd [name];
						if (o != null)
							return o;
					}
				}
				lookup = lookup.Parent;
			}

			o = Context.LookupNamedItem (name);
			if (o != null)
				return o;

			o = Application.Current.Resources [name];
		
			return o;
		}

		internal void RegisterKeyItem (XamlObjectElement element, XamlElement target, string key)
		{
//			IDictionary rd = CurrentDictionary (element);
//			if (rd == null)
//				throw ParseException ("Attempt to use x:Key outside of an IDictionary.");

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

			DependencyObject dob = element.DependencyObject;
			if (dob != null)
				dob.SetNameOnScope (name, NameScope);
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
			XamlObjectElement obj = null;

			if (element == null || element.Parent == null)
				return null;

			XamlElement walk = element.Parent;

			XamlPropertyElement prop = null;
			while (walk != null) {
				prop = walk as XamlPropertyElement;
				if (prop == null) {
					obj = walk as XamlObjectElement;
					IDictionary rd = obj.Object as IDictionary;
					if (rd != null)
						return rd;
					walk = walk.Parent;
					continue;
				}

				//
				// We are in a different property
				//
				if (!(typeof (IDictionary).IsAssignableFrom (prop.Type))) {
					walk = walk.Parent;
					continue;
				}

				break;
			}

			if (prop == null)
				return null;

			obj = prop.Parent as XamlObjectElement;
			if (obj == null)
				return null;

			FrameworkElement fe = obj.Object as FrameworkElement;
			if (fe != null)
				return fe.Resources;

			Application app = obj.Object as Application;
			if (app != null)
				return app.Resources;

			return null;
		}

		private void ParseElement ()
		{
			if (IsPropertyElement ()) {
				ParsePropertyElement ();
				return;
			}

			if (IsStaticResourceElement ()) {
				if (!reader.Continue)
					reader.Continue = true;
				else
					ParseStaticResourceElement ();
				return;
			}

			ParseObjectElement ();
		}

		
		private void ParseObjectElement ()
		{
			if (reader.ManagedType == null)
				reader.ManagedType = ResolveType ();

			if (reader.ManagedType == null)
				throw ParseException ("Unable to find the type {0}.", reader.LocalName);

			object o = InstantiateType (reader.ManagedType);

			if (o == null)
				throw ParseException ("Could not create object for element {0}.", reader.LocalName);

			XamlObjectElement element = new XamlObjectElement (this, reader.LocalName, reader.ManagedType, o);

			if (IsBufferedTemplateElement (element)) {
				reader.Ignore = true;
				ParseTemplateElement (element);
				return;
			}

			if (typeof (System.Windows.FrameworkElement).IsAssignableFrom (reader.ManagedType)) {
				element.EndElement += delegate (object sender, EventArgs e) {
					FrameworkElement fwe = element.Object as FrameworkElement;
					fwe.ApplyDefaultStyle ();
				};
			}

			SetResourceBase (element);
			SetElementTemplateScopes (element);
			OnElementBegin (element);
		}

		private void ParsePropertyElement ()
		{
			if (reader.ManagedType == null)
				reader.ManagedType = ResolveType ();

			if (reader.ManagedType == null)
				throw ParseException ("Unable to find the property {0}.", reader.LocalName);

			XamlPropertySetter setter = null;
			if (CurrentElement != null) {
				XamlObjectElement parent = CurrentElement as XamlObjectElement;

				if (parent == null)
					throw ParseException ("Property {0} does not have a parent.", reader.LocalName);
				setter = CurrentElement.LookupProperty (reader);
				if (setter == null)
					throw ParseException ("Property {0} was not found on type {1}.", reader.LocalName, CurrentElement.Name);
			} else
				throw ParseException ("A property element cannot be at the root of a document.");

			XamlPropertyElement element = new XamlPropertyElement (this, reader.LocalName, setter);
			OnElementBegin (element);
		}

		private void ParseTemplateElement (XamlObjectElement element)
		{
			OnElementBegin (element);


			FrameworkTemplate template = (FrameworkTemplate) element.Object;

			unsafe {
				template.SetXamlBuffer (ParseTemplate, CreateXamlContext (template));
			}
		}

		private static unsafe IntPtr ParseTemplate (Value *context_ptr, IntPtr resource_base, IntPtr surface, IntPtr binding_source, string xaml, ref MoonError error)
		{
			XamlContext context = Value.ToObject (typeof (XamlContext), context_ptr) as XamlContext;

			var parser = new XamlParser (context) {
				ResourceBase = UriHelper.FromNativeUri (resource_base),
			};

			FrameworkElement fwe = null;
			var source = NativeDependencyObjectHelper.FromIntPtr (binding_source);

			if (source != null) {
				fwe = source as FrameworkElement;
				if (fwe == null) {
					error = new MoonError (parser.ParseException ("Only FrameworkElements can be used as TemplateBinding sources."));
					return IntPtr.Zero;
				}
			}

			context.IsExpandingTemplate = true;
			context.TemplateOwner = source as DependencyObject;
			context.TemplateBindingSource = fwe;
			parser.HydrateObject = context.Template;

			
			INativeEventObjectWrapper dob = null;
			try {
				FrameworkTemplate template = parser.Parse (context.Node) as FrameworkTemplate;
				
				if (template != null) {
					dob = template.Content as INativeEventObjectWrapper;
					template.Content = null;
				}

				// No errors, but the template was just empty.
				if (dob == null)
					return IntPtr.Zero;
			} catch (Exception e) {
				error = new MoonError (e);
				return IntPtr.Zero;
			} finally {
				context.IsExpandingTemplate = false;
				context.TemplateOwner = null;
				context.TemplateBindingSource = null;
			}

			// XamlParser needs to ref its return value otherwise we can end up returning a an object to native
			// code with a refcount of '1' and it could then get GC'ed before we use it.
			Mono.NativeMethods.event_object_ref (dob.NativeHandle);
			return dob.NativeHandle;
		}

		private bool IsPropertyElement ()
		{
			return reader.LocalName.IndexOf ('.') > 0;
		}

		private bool IsBufferedTemplateElement (XamlObjectElement element)
		{
			if (element == null)
				return false;

			if (!typeof (FrameworkTemplate).IsAssignableFrom (element.Type))
				return false;

			// If we are parsing a template (In the ParseTemplate callback,
			// the top level template elementis not buffered because we are
			// trying to parse the contents of that template element
			if (IsTopElement && Context.IsExpandingTemplate)
				return false;

			return true;
		}

		private bool IsObjectElement ()
		{
			return CurrentElement is XamlObjectElement;
		}

		private bool IsStaticResourceElement ()
		{
			return reader.LocalName == "StaticResource";
		}

		private bool IsTextBlockElement (XamlObjectElement obj)
		{
			return (typeof (TextBlock).IsAssignableFrom (obj.Type));
		}

		private void ParseEndElement ()
		{
			OnElementEnd ();
		}

		private void ParseText ()
		{
			string value = HandleWhiteSpace (reader.Value);

			XamlObjectElement obj = CurrentElement as XamlObjectElement;
			if (obj != null) {
				if (IsTextBlockElement (obj)) {
					ParseTextBlockText (obj, value);
					return;
				}

				if (typeof (Paragraph).IsAssignableFrom (obj.Type)) {
					ParseParagraphText (obj, value);
					return;
				}

				if (typeof (Span).IsAssignableFrom (obj.Type)) {
					ParseSpanText (obj, value);
					return;
				}

				XamlReflectionPropertySetter content = obj.FindContentProperty ();
				if (content == null) {

					if (IsLegalCorlibType (obj.Type)) {
						MutableObject mutable = (MutableObject) obj.Object;
						mutable.Object = CorlibTypeValueFromString (obj.Type, value);
						return;
					}

					if (IsLegalStructType (obj.Type)) {
						MutableObject mutable = (MutableObject) obj.Object;
						mutable.Object = KnownStructValueFromString (obj, value);
						return;
					}

					if (IsSpecialCasedType (obj.Type)) {
						MutableObject mutable = (MutableObject) obj.Object;
						mutable.Object = SpecialCasedTypeValueFromString (obj, value);
						return;
					}

					if (IsTypeConvertedType (obj.Type)) {
						var converter = Helper.GetConverterFor (obj.Type);
						if (converter.CanConvertFrom (typeof (string))) {
							MutableObject mutable = (MutableObject) obj.Object;
							mutable.Object = converter.ConvertFrom (null, Helper.DefaultCulture, value);
							return;
						}
					}

					throw ParseException ("Element {0} does not support text properties.", CurrentElement.Name);
				}
				
				object converted = content.ConvertTextValue (value);
				content.SetValue (converted);
				return;
			}

			XamlPropertyElement prop = CurrentElement as XamlPropertyElement;
			if (prop != null) {
				object converted = prop.Setter.ConvertTextValue (value);
				prop.Setter.SetValue (converted);
			}
		}

		private void ParseTextBlockText (XamlObjectElement block, string value)
		{
			Run run = ParseRunText (value);
			TextBlock textblock = block.Object as TextBlock;

			textblock.Inlines.Add (run);
		}

		private void ParseParagraphText (XamlObjectElement obj, string value)
		{
			Paragraph para = (Paragraph) obj.Object;
			Run run = ParseRunText (value);

			para.Inlines.Add (run);
		}

		private void ParseSpanText (XamlObjectElement obj, string value)
		{
			Span span = (Span) obj.Object;
			Run run = ParseRunText (value);

			span.Inlines.Add (run);
		}

		private Run ParseRunText (string value)
		{
			Run run = new Run ();

			run.Text = value;
			return run;
		}

		private void ParseWhitespace ()
		{
		}

		private void ParseSignificantWhitespace ()
		{
		}

		private void ParseStaticResourceElement ()
		{
			string key = reader.GetAttribute ("ResourceKey");
			if (key == null)
				throw ParseException ("No ResourceKey found on StaticResource element.");

			object obj = LookupNamedItem (CurrentElement, key);
			XamlObjectElement element = new XamlObjectElement (this, reader.LocalName, obj.GetType (), obj);

			OnElementBegin (element);
		}

		private void ParseAttribute (XamlObjectElement element, XamlAttribute ai)
		{

			if (ai.IsNsXaml) {
				ParseXAttribute (element, ai);
				return;
			}

			if (ai.IsXmlDirective) {
				ParseXmlDirective (element, ai);
				return;
			}

			XamlPropertySetter prop = element.LookupProperty (ai);
			if (prop == null)
				throw ParseException ("The property {0} was not found on element {1}.", ai.LocalName, element.Name);

			object value = ParseAttributeValue (element, prop, ai);
			prop.SetValue (element, value);
		}

		private void ParseXAttribute (XamlObjectElement element, XamlAttribute ai)
		{
			switch (ai.LocalName) {
			case "Key":
				RegisterKeyItem (element, element.Parent, ai.Value);
				return;
			case "Name":
				RegisterNamedItem (element, ai.Value);
				return;
			case "Class":
				// The class attribute is handled when we initialize the element
				return;
			case "ClassModifier":
			case "FieldModifier":
				// There are no docs on whether these change anything
				// internally on silverlight.
				// But I think they are only a tooling issue.
				return;
			case "Uid":
                               // This attribute is just ignored, but allowed.
                               return;
			default:
				throw ParseException ("Unknown x: attribute ({0}).", ai.LocalName);
			}
		}

		private void ParseXmlDirective (XamlElement element, XamlAttribute ai)
		{
			if (ai.LocalName == "space") {
				// Do nothing XmlReader covers this for us
			}
		}

		private object ParseAttributeValue (XamlObjectElement element, XamlPropertySetter property, XamlAttribute ai)
		{
			object value = null;

			if (IsMarkupExpression (ai.Value))
				value = ParseAttributeMarkup (element, property, ai);
			else {
				try {
					value = property.ConvertTextValue (ai.Value);
				} catch (Exception ex) {
					throw ParseException ("Could not convert attribute value '{0}' on element {1}.", ai.Value, element.Name, ex);
				}
			}
			return value;
		}

		private object ParseAttributeMarkup (XamlObjectElement element, XamlPropertySetter property, XamlAttribute ai)
		{
			MarkupExpressionParser parser = new SL4MarkupExpressionParser (element.Object, property.Name, this, element);

			string expression = ai.Value;
			object o = null;

			try {
				o = parser.ParseExpression (ref expression);
			} catch (Exception e) {
				throw ParseException ("Could not convert attribute value '{0}' on element {1}.", ai.Value, element.Name, e);
			}

			if (o == null && !MarkupExpressionParser.IsExplicitNull (expression))
				throw ParseException ("Unable to convert attribute value: '{0}'.", ai.Value);

			return property.ConvertValue (property.Type, o);
		}

		private bool IsMarkupExpression (string str)
		{
			return str.Length == 0 ? false : str [0] == '{';
		}

		private bool IsValidXmlSpaceValue (string val)
		{
			return val == "preserve" || val == "default";
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
				if (typeof (DependencyObject).IsAssignableFrom (obj.Type)) {
					object target = obj.Object;
					if (target is MutableObject)
						target = ((MutableObject)target).Object;
					DependencyObject dob = (DependencyObject) target;
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
			//XamlObjectElement obj = (XamlObjectElement) element;
		}
			       
		private void InitializeElement (XamlElement element)
		{
			XamlObjectElement obj = element as XamlObjectElement;
			if (obj == null)
				return;

			ISupportInitialize init = obj.Object as ISupportInitialize;
			if (init == null)
				return;

			try {
				init.BeginInit ();
			} catch (Exception e) {
				Console.WriteLine ("Exception in initializer");
				Console.WriteLine (e);
			}
		}

		private void EndInitializeElement (XamlElement element)
		{
			XamlObjectElement obj = element as XamlObjectElement;
			if (obj == null)
				return;

			ISupportInitialize init = obj.Object as ISupportInitialize;
			if (init == null)
				return;

			try {
				init.EndInit ();
			} catch (Exception e) {
				Console.WriteLine ("Exception in initializer.");
				Console.WriteLine (e);
			}
		}

		private void SetResourceBase (XamlObjectElement element)
		{
			if (ResourceBase == null)
				return;

			DependencyObject dob = element.DependencyObject;
			if (dob == null)
				return;

			dob.ResourceBase = ResourceBase;
		}

		private void SetElementTemplateScopes (XamlObjectElement element)
		{
			// This whole thing is basically copied from xaml.cpp AddCreatedItem
			object is_template = null;

			DependencyObject el_dob = element.Object as DependencyObject;
			if (el_dob == null)
				return;

			// When instantiating a template, some elements are created which are not explicitly
			// mentioned in the xaml. Therefore we need to keep walking up the tree until we find
			// the last element which we set a value for Control::IsTemplateItem and propagate
			// it from there.


			XamlElement instance = CurrentElement;

			while (instance != null) {

				XamlObjectElement oe = instance as XamlObjectElement;
				if (oe == null) {
					instance = instance.Parent;
					continue;
				}

				DependencyObject dob = oe.Object as DependencyObject;
				if (dob == null) {
					instance = instance.Parent;
					continue;
				}

				is_template = dob.ReadLocalValue (Control.IsTemplateItemProperty);
				if (is_template == null || is_template == DependencyProperty.UnsetValue) {
					instance = instance.Parent;
					continue;
				}

				el_dob.SetValue (Control.IsTemplateItemProperty, dob.GetValue (Control.IsTemplateItemProperty));
				if (dob.TemplateOwner != null)
					el_dob.TemplateOwner = dob.TemplateOwner;

				break;
			}
		
			if (instance == null) {
				el_dob.SetValue (Control.IsTemplateItemProperty, Context.IsExpandingTemplate);
				el_dob.TemplateOwner = Context.TemplateOwner;
			}

			is_template = el_dob.ReadLocalValue (Control.IsTemplateItemProperty);
			if (is_template != null && is_template != DependencyProperty.UnsetValue && ((bool) is_template)) {
				UserControl uc = el_dob as UserControl;
				if (uc != null) {
				        // I can't come up with a test to verify this fix. However, it does
				        // fix a crasher in olympics when trying to play a new video from
				        // the recommendations list after the curreont video finishes
					NameScope ns = NameScope.GetNameScope (uc);
					if (uc.Content != null)
						NameScope.SetNameScope (uc.Content, ns);
					if (uc.Resources != null)
						NameScope.SetNameScope (uc.Resources, ns);
				}
				NameScope.SetNameScope (el_dob, NameScope);
			}
		}

		public Type ResolveType (string str)
		{
			int colon = str.IndexOf (':');
			string xmlns = reader.DefaultXmlns;
			string name = str;

			if (colon > 0) {
				string local = str.Substring (0, colon);
				name = str.Substring (++colon, str.Length - colon);
				if (!reader.Namespaces.TryGetValue (local, out xmlns))
					throw ParseException ("Could not find namespace for type {0} ({1}, {2}).", str, name, local);
			}

			return ResolveType (xmlns, name);
		}

		public Type ResolveType ()
		{
			if (IsTopElement) {
				string user_class = ResolveUserClass ();
				if (user_class != null) {
					Type t = LoadType (null, null, user_class);
					if (t == null)
						throw ParseException ("Unable to load type '{0}'.", user_class);
					return t;
				}
			}
			return ResolveType (currentNode.NamespaceURI, currentNode.LocalName);
		}

		public Type ResolveType (string xmlns, string full_name)
		{
			Type t;
			var dictKey = new XmlNsKey (xmlns, full_name);
			if (Context.XmlnsCachedTypes.TryGetValue (dictKey, out t))
				return t;

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
				xmlns = reader.DefaultXmlns;

			ns = ResolveClrNamespace (xmlns);
			asm_name = ResolveAssemblyName (xmlns);
			if (ns != null)
				full_name = String.Concat (ns, ".", name);

			//
			// If no assembly is specified we pass in null and LoadType will search for the type
			//
			if (asm_name != null)
				assembly = LoadAssembly (asm_name);

			t = LoadType (assembly, xmlns, full_name);
			if (!Context.XmlnsCachedTypes.ContainsKey (dictKey))
				Context.XmlnsCachedTypes.Add (dictKey, t);
			return t;
		}

		private static string ResolveClrNamespace (string xmlns)
		{
			if (String.IsNullOrEmpty (xmlns))
				return null;

			int start = xmlns.IndexOf ("clr-namespace:");

			if (start != 0)
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
			if (currentNode is XamlAttribute)
				return null;
			return reader.Class;
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
			bool res = (t != null && t.IsPublic);

			if (!res && t != null && !t.IsPublic) {
				if (typeof (DependencyObject).Assembly != t.Assembly)
					return true;
				if (typeof (DependencyObject).Assembly == t.Assembly && IsValidInternalType (t))
					return true;
			}
			return res;
		}

		private bool IsValidInternalType (Type t)
		{
			if (t == typeof (StaticResource))
				return true;

			return false;
		}

		private Type LoadTypeFromAssembly (Assembly assembly, string name)
		{
			Type t = assembly.GetType (name);
			
			if (!IsValidType (t))
				return null;
			
			return t;
		}

		private Type LoadTypeFromXmlNs (string xmlns, string name)
		{
			XmlNsKey key = new XmlNsKey (xmlns, name);
			Type t;
			
			if (Context.XmlnsCachedTypes.TryGetValue (key, out t))
				return t;

			t = FindType (xmlns, name);
			if (!IsValidType (t))
				t = null;
			
			Context.XmlnsCachedTypes.Add (key, t);
			return t;
		}

		public Type LoadType (Assembly assembly, string xmlns, string name)
		{
			if (assembly != null)
				return LoadTypeFromAssembly (assembly, name);
			else
				return LoadTypeFromXmlNs (xmlns, name);
		}

		private Type FindType (string xmlns, string name)
		{
			Type t = FindDefaultType (xmlns, name);
			if (t != null)
				return t;

			if (!name.Contains ('.'))
				t = FindPartialType (xmlns, name);

			if (t == null) {
				var col = from a in Deployment.Current.Assemblies where (t = a.GetType (name)) != null select t;
				if (col.Count () > 0) {
					t = col.First ();
					if (IsValidType (t)) 
						return t;					
				}
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

		private Type FindPartialType (string xmlns, string name)
		{
			if (Deployment.Current.Assemblies == null)
				return null;

			foreach (Assembly assembly in Deployment.Current.Assemblies) {
				Type t = LoadPartialTypeFromAssembly (xmlns, name, assembly);

				if (IsValidType (t))
					return t;
			}

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

			if (IsTopElement && HydrateObject != null) {
				if (!type.IsAssignableFrom (HydrateObject.GetType ()))
					throw ParseException ("Invalid top-level element found {0}, expecting {1}", type, HydrateObject.GetType ());
				return HydrateObject;
			}
			
			// Returns null if the type isn't a collection type.
			o = InstantiateCollectionType (type);

			if (o == null && HasDefaultConstructor (type))
				o = Activator.CreateInstance (type);

			if (o == null && IsSpecialInternalType (type))
			{
				o = Activator.CreateInstance (type,
					BindingFlags.CreateInstance | BindingFlags.NonPublic |
					BindingFlags.Instance | BindingFlags.OptionalParamBinding,
					null, new object[] {}, null);
			}

			if (o == null && IsLegalCorlibType (type))
				o = DefaultValueForCorlibType (type);

			if (o == null && IsLegalStructType (type))
				o = DefaultValueForStructType (type);

			if (o == null && IsSpecialCasedType (type))
				o = DefaultValueForSpecialCasedType (type);

			// If an object does not have a content property, we can use
			// a class level type converter to generate an object of this type
			if (!(o is MutableObject) && IsTypeConvertedType (type))
				o = new MutableObject (o);

			return o;
		}

		private static bool HasDefaultConstructor (Type t)
		{
			ConstructorInfo [] ctors = t.GetConstructors ();

			var def = ctors.Where (c => c.GetParameters ().Length == 0).FirstOrDefault ();

			return def != null;
		}

		private static bool IsLegalCorlibType (Type t)
		{
			bool res = false;
			
			if (t == typeof (string))
				res = true;
			else if (t == typeof (int))
				res = true;
			else if (t == typeof (double))
				res = true;
			else if (t == typeof (bool))
				res = true;

			return res;
		}

		private static object DefaultValueForCorlibType (Type t)
		{
			object res = false;
			
			if (t == typeof (string))
				res = String.Empty;
			else if (t == typeof (int))
				res = 0;
			else if (t == typeof (double))
				res = 0.0;
			else if (t == typeof (bool))
				res = false;

			if (res != null)
				res = new MutableObject (res);

			return res;
		}

		private object CorlibTypeValueFromString (Type dest, string value)
		{
			if (dest == typeof (string))
				return value;
			else if (dest == typeof (int)) {
				int res;

				if (value.Length == 0)
					return 0;

				var sub = value.Trim();
				for (int i = 0; i < sub.Length; i++) {
					if (!Char.IsDigit (sub, i) && i > 0) {
						sub = sub.Substring (0, i--);
					}
				}

				if (!Int32.TryParse (sub, CORLIB_INTEGER_STYLES, CultureInfo.InvariantCulture, out res)) {
						throw ParseException ("Invalid int value {0}.", sub);
				}

				return (int) res;
			} else if (dest == typeof (double)) {
				double res;

				if (value.Length == 0)
					return 0.0;

				if (!Double.TryParse (value, CORLIB_DOUBLE_STYLES, CultureInfo.InvariantCulture, out res))
					throw ParseException ("Invalid double value {0}.", value);

				return res;
			} else if (dest == typeof (bool)) {
				bool res;

				if (value.Length == 0)
					return false;

				if (!Boolean.TryParse (value, out res)) {
					double d;
					if (!Double.TryParse (value, CORLIB_DOUBLE_STYLES, CultureInfo.InvariantCulture, out d))
						throw ParseException ("Invalid bool value {0}.", value);
					res = d != 0.0;
				}

				return res;
			}

			throw ParseException ("Invalid corlib type used.");
		}

		private static bool IsLegalStructType (Type t)
		{
			if (t == typeof (char))
				return false;
			if (t == typeof (byte))
				return false;
			if (t == typeof (DateTime))
				return false;	
			if (t == typeof (short))
				return false;
			if (t == typeof (long))
				return false;
			if (t == typeof (Single))
				return false;
			if (t == typeof (Decimal))
				return false;

			return t.IsValueType;
		}

		private static object DefaultValueForStructType (Type t)
		{
			object o = Activator.CreateInstance (t);

			return new MutableObject (o);
		}

		private object KnownStructValueFromString (XamlObjectElement element, string value)
		{
			return XamlTypeConverter.ConvertObject (this, element, element.Type, null, null, value);
		}

		private static bool IsSpecialCasedType (Type t)
		{
			//
			// Sadly i think this list will grow.
			//

			if (t == typeof (FontFamily))
				return true;
			if (t == typeof (System.Windows.Input.Cursor))
				return true;
			
			return false;
		}

		private bool IsSpecialInternalType (Type t)
		{
			if (t == typeof (Section) && Owner is RichTextBox)
				return true;
			return false;
		}

		private static object DefaultValueForSpecialCasedType (Type t)
		{
			return new MutableObject (null);
		}

		private static bool IsTypeConvertedType (Type t)
		{
			return Helper.GetConverterFor (t) != null;
		}

		private object SpecialCasedTypeValueFromString (XamlObjectElement element, string value)
		{
			return XamlTypeConverter.ConvertObject (this, element, element.Type, null, element.Name, value);
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

			object res = prop.GetValue ();

			if (res != null && !t.IsAssignableFrom (res.GetType ()))
				return null;

			return res;
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
			return new XamlContext (Context, TopElement, CreateResourcesList (), template, reader);
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

			if (str.Length == 0)
				return str;

			int i = 0;
			while (i < str.Length && Char.IsWhiteSpace (str [i]))
				i++;

			StringBuilder builder = new StringBuilder (str.Length);
			for ( ; i < str.Length; i++) {
				if (Char.IsWhiteSpace (str [i])) {
					while (i < str.Length - 1 && Char.IsWhiteSpace (str [i + 1])) {
						i++;
					}
					if (i == str.Length - 1)
						break;
					builder.Append (' ');
					continue;
				}
				builder.Append (str [i]);
			}

			return builder.ToString ();
		}

		public static DependencyProperty LookupDependencyProperty (Kind kind, string name)
		{
			DependencyProperty dp;
			
			if (!DependencyProperty.TryLookup (kind, name, out dp)) {
				var type = Deployment.Current.Types.KindToType (kind);
				var field = type.GetField (name + "Property", FIELD_BINDING_FLAGS);
				if (field != null && typeof (DependencyProperty).IsAssignableFrom (field.FieldType))
					dp = (DependencyProperty) field.GetValue (null);
				else
					dp = null;
			}

			return dp;
		}
	}
}
