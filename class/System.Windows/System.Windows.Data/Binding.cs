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

namespace System.Windows.Data {

	public class Binding {
		
		IValueConverter converter;
		CultureInfo converterCulture;
		object converterParameter;
		BindingMode mode;
		bool notifyOnvalidationError;
		PropertyPath path;
		object source;
		bool validatesOnExceptions;
		
		public IValueConverter Converter {
			get { return converter; }
			set {
				CheckUsed ();
				converter = value;
			}
		}
		
		public CultureInfo ConverterCulture {
			get { return converterCulture; }
			set {
				CheckUsed ();
				converterCulture = value;
			}
		}
		
		public object ConverterParameter {
			get { return converterParameter; }
			set {
				CheckUsed ();
				converterParameter = value;
			}
		}
		
		public BindingMode Mode {
			get { return mode; }
			set {
				CheckUsed ();
				mode = value;
			}
		}
		
		public bool NotifyOnValidationError {
			get { return notifyOnvalidationError; }
			set {
				CheckUsed ();
				notifyOnvalidationError = value;
			}
		}
		
		[TypeConverter (typeof (PropertyPathConverter))]
		public PropertyPath Path {
			get { return path; }
			set {
				CheckUsed ();
				path = value;
			}
		}
		
		public object Source {
			get { return source; }
			set {
				CheckUsed ();
				source = value;
			}
		}

		internal bool Used {
			get; private set;
		}
		
		public bool ValidatesOnExceptions {
			get { return validatesOnExceptions; }
			set {
				CheckUsed ();
				validatesOnExceptions = value;
			}
		}
		
		public Binding ()
			: this ("")
		{
			
		}

		public Binding (string path)
		{
			Mode = BindingMode.OneWay;
			Path = new PropertyPath (path);
		}

		void CheckUsed ()
		{
			if (Used)
				throw new InvalidOperationException ("The Binding cannot be changed after it has been used");
		}

		internal void SetUsed ()
		{
			if (Used)
				return;
			
			Used = true;
		}
	}
}
