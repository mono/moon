//
// PropertyGroupDescription.cs
//
// Authors:
//	Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc.  http://www.novell.com
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
using System.ComponentModel;
using System.Globalization;

namespace System.Windows.Data {
	public class PropertyGroupDescription : GroupDescription {
		public PropertyGroupDescription ()
		{
			Console.WriteLine ("System.Windows.Data.PropertyGroupDescription:.ctor: NIEX");
			throw new NotImplementedException ();
		}

		public PropertyGroupDescription (string propertyName)
		{
			Console.WriteLine ("System.Windows.Data.PropertyGroupDescription:.ctor: NIEX");
			throw new NotImplementedException ();
		}

		public PropertyGroupDescription (string propertyName, IValueConverter converter)
		{
			Console.WriteLine ("System.Windows.Data.PropertyGroupDescription:.ctor: NIEX");
			throw new NotImplementedException ();
		}

		public PropertyGroupDescription (string propertyName, IValueConverter converter, StringComparison stringComparison)
		{
			Console.WriteLine ("System.Windows.Data.PropertyGroupDescription:.ctor: NIEX");
			throw new NotImplementedException ();
		}

		public override object GroupNameFromItem (object item, int level, CultureInfo culture)
		{
			Console.WriteLine ("System.Windows.Data.PropertyGroupDescription:GroupNameFromItem: NIEX");
			throw new NotImplementedException ();
		}

		public override bool NamesMatch (object groupName, object itemName)
		{
			Console.WriteLine ("System.Windows.Data.PropertyGroupDescription:NamesMatch: NIEX");
			throw new NotImplementedException ();
		}

		private void OnPropertyChanged (string propertyName)
		{
			Console.WriteLine ("System.Windows.Data.PropertyGroupDescription:OnPropertyChanged: NIEX");
			throw new NotImplementedException ();
		}

		[DefaultValue ((string) null)]
		public IValueConverter Converter {
			get {
				Console.WriteLine ("System.Windows.Data.PropertyGroupDescription:get_Converter: NIEX");
				throw new NotImplementedException ();
			}
			set {
				Console.WriteLine ("System.Windows.Data.PropertyGroupDescription:set_Converter: NIEX");
				throw new NotImplementedException ();
			}
		}

		[DefaultValue ((string) null)]
		public string PropertyName {
			get {
				Console.WriteLine ("System.Windows.Data.PropertyGroupDescription:get_PropertyName: NIEX");
				throw new NotImplementedException ();
			}
			set {
				Console.WriteLine ("System.Windows.Data.PropertyGroupDescription:set_PropertyName: NIEX");
				throw new NotImplementedException ();
			}
		}

		[DefaultValue (StringComparison.Ordinal)]
		public StringComparison StringComparison {
			get {
				Console.WriteLine ("System.Windows.Data.PropertyGroupDescription:get_StringComparison: NIEX");
				throw new NotImplementedException ();
			}
			set {
				Console.WriteLine ("System.Windows.Data.PropertyGroupDescription:set_StringComparison: NIEX");
				throw new NotImplementedException ();
			}
		}
	}
}

