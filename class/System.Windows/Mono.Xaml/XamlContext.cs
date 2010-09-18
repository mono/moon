//
// XamlContext.cs
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

using System;
using System.IO;
using System.Reflection;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Windows;

using Mono;

namespace Mono.Xaml
{
	internal class XamlContext {

		internal XamlContext ()
		{
			Resources = new List<DependencyObject> ();

			DefaultXmlns = String.Empty;
			IgnorableXmlns = new List<string> ();
			Xmlns = new Dictionary<string,string> ();
		}

		internal XamlContext (XamlContext parent, XamlElement top_element, List<DependencyObject> resources, FrameworkTemplate template)
		{
			Parent = parent;
			TopElement = top_element;
			Resources = resources;
			Template = template;

			//
			// Its just not worth the lookup time to try accessing these on the parents, so copy them over.
			//
			DefaultXmlns = parent.DefaultXmlns;
			IgnorableXmlns = new List<string> (parent.IgnorableXmlns);
			Xmlns = new Dictionary<string,string> (parent.Xmlns);
		}

		public XamlContext Parent {
			get;
			private set;
		}

		public XamlElement TopElement {
			get;
			private set;
		}

		public List<DependencyObject> Resources {
			get;
			private set;
		}

		public FrameworkTemplate Template {
			get;
			private set;
		}

		public FrameworkElement TemplateBindingSource {
			get;
			set;
		}

		public DependencyObject TemplateOwner {
			get;
			set;
		}

		public bool IsExpandingTemplate {
			get;
			set;
		}

		public string DefaultXmlns {
			get;
			set;
		}

		public List<string> IgnorableXmlns {
			get;
			private set;
		}

		public Dictionary<string,string> Xmlns {
			get;
			private set;
		}

		public object LookupNamedItem (string name)
		{
			object res = null;

			foreach (DependencyObject dob in Resources) {
				ResourceDictionary rd = dob as ResourceDictionary;
				if (rd != null) {
					res = rd [name];
					if (res != null)
						return res;
					continue;
				}
				FrameworkElement fwe = (FrameworkElement) dob;
				res = fwe.Resources [name];
				if (res != null)
					return res;
			}

			if (Parent != null)
				return Parent.LookupNamedItem (name);

			return null;
		}

		~XamlContext ()
		{
			Free ();
		}

		internal void Free ()
		{
		}
	}
}


