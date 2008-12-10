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
		PropertyPath path;
		object source;
		
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
		
		public BindingMode Mode {
			get { return (BindingMode) NativeMethods.binding_get_binding_mode (Native); }
			set {
				CheckSealed ();
				NativeMethods.binding_set_binding_mode (Native, (int) value);
			}
		}

		internal IntPtr Native {
			get; set;
		}
		
		public bool NotifyOnValidationError {
			get { return NativeMethods.binding_get_notify_on_validation_error (Native); }
			set {
				CheckSealed ();
				NativeMethods.binding_set_notify_on_validation_error (Native, value);
			}
		}
		
		[TypeConverter (typeof (PropertyPathConverter))]
		public PropertyPath Path {
			get {
				if (Sealed && path == null) {
					path = new PropertyPath (NativeMethods.binding_get_property_path (Native));
				}
				return path;
			}
			set {
				CheckSealed ();
				path = value;
			}
		}

		internal bool Sealed {
			get {
				return NativeMethods.binding_get_is_sealed (Native);
			}
			private set {
				NativeMethods.binding_set_is_sealed (Native, value);
			}
		}
		
		public object Source {
			get { return source; }
			set {
				CheckSealed ();
				source = value;
			}
		}
		
		public bool ValidatesOnExceptions {
			get { return NativeMethods.binding_get_validates_on_exceptions (Native); }
			set {
				CheckSealed ();
				NativeMethods.binding_set_validates_on_exceptions (Native, value);
			}
		}
		
		public Binding ()
			: this ("")
		{
			
		}

		public Binding (string path)
			: this (NativeMethods.binding_new ())
		{
			Mode = BindingMode.OneWay;
			Path = new PropertyPath (path);
		}

		internal Binding (IntPtr native)
		{
			Native = native;
		}

		~Binding ()
		{
			if (Native != IntPtr.Zero) {
				Mono.NativeMethods.event_object_unref (Native);
				Native = IntPtr.Zero;
			}
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
			if (path != null)
				NativeMethods.binding_set_property_path (Native, path.Path);
		}
	}
}
