//
// Binding.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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

using System.ComponentModel;
using System.Globalization;
using System.Windows;
using System.Collections.Generic;

using Mono;

namespace System.Windows.Data {
	public class Binding {
		IValueConverter converter;
		CultureInfo converterCulture;
		object converterParameter;
		string elementName;
		PropertyPath path;
		BindingMode mode;
		bool notifyonerror;
		bool validatesonex;
		bool issealed;
		object source;
		UpdateSourceTrigger trigger;
		RelativeSource relative_source;
		
		public IValueConverter Converter {
			get { return converter; }
			set {
				CheckSealed ();
				converter = value;
			}
		}
		
		public CultureInfo ConverterCulture {
			get { return converterCulture; }
			set {
				CheckSealed ();
				converterCulture = value;
			}
		}
		
		public object ConverterParameter {
			get { return converterParameter; }
			set {
				CheckSealed ();
				converterParameter = value;
			}
		}

		public string ElementName {
			get { return elementName; }
			set {
				CheckSealed ();
				if (Source != null || RelativeSource != null)
					throw new InvalidOperationException ("ElementName cannot be set if either RelativeSource or Source is set");
				elementName = value;
			}
		}

		public BindingMode Mode {
			get { return mode; }
			set {
				CheckSealed ();
				mode = value;
			}
		}

		public bool NotifyOnValidationError {
			get { return notifyonerror; }
			set {
				CheckSealed ();
				notifyonerror = value;
			}
		}

		public RelativeSource RelativeSource {
			get { return relative_source; }
			set {
				// FIXME: Check that the standard validation is done here
				CheckSealed ();
				if (source != null || ElementName != null)
					throw new InvalidOperationException ("RelativeSource cannot be set if either ElementName or Source is set");
				relative_source = value;
			}
		}

		[TypeConverter (typeof (PropertyPathConverter))]
		public PropertyPath Path {
			get {
				return path;
			}
			set {
				if (value.NativeDP != IntPtr.Zero)
					throw new ArgumentException ("PropertyPaths which are instantiated with a DependencyProperty are not supported");

				CheckSealed ();
				path = value;
			}
		}

		internal bool Sealed {
			get {
				return issealed;
			}
			private set {
				issealed = value;
			}
		}
		
		public object Source {
			get { return source; }
			set {
				CheckSealed ();
				if (ElementName != null || RelativeSource != null)
					throw new InvalidOperationException ("Source cannot be set if either ElementName or RelativeSource is set");
				source = value;
			}
		}

		public UpdateSourceTrigger UpdateSourceTrigger {
			get { return trigger; }
			set { trigger = value; }
		}
		
		public bool ValidatesOnExceptions {
			get { return validatesonex; }
			set {
				CheckSealed ();
				validatesonex = value;
			}
		}
		
		public Binding ()
			: this ("")
		{
			
		}

		public Binding (string path)
		{
			if (path == null)
				throw new ArgumentNullException ("path");
			
			Mode = BindingMode.OneWay;
			Path = new PropertyPath (path);
			trigger = UpdateSourceTrigger.Default;
		}

		void CheckSealed ()
		{
			if (Sealed)
				throw new InvalidOperationException ("The Binding cannot be changed after it has been used");
		}

		internal void Seal ()
		{
			if (Sealed)
				return;
			
			Sealed = true;
		}
	}
}
