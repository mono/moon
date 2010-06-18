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
using System.Runtime.CompilerServices;

namespace System.Windows.Data {
#if NET_2_1
	[TypeForwardedFrom ("System.Windows.Data, Version=2.0.5.0, Culture=Neutral, PublicKeyToken=31bf3856ad364e35")]
#endif
	public class PropertyGroupDescription : GroupDescription {

		IValueConverter converter;
		string propertyName;
		StringComparison stringComparison;

		[DefaultValue ((string) null)]
		public IValueConverter Converter {
			get { return converter; }
			set {
				if (converter != value) {
					converter = value;
					OnPropertyChanged (new PropertyChangedEventArgs ("Converter"));
				}
			}
		}

		[DefaultValue ((string) null)]
		public string PropertyName {
			get { return propertyName; }
			set {
				if (propertyName != value) {
					propertyName = value;
					PropertyPathWalker = null;
					OnPropertyChanged (new PropertyChangedEventArgs ("PropertyName"));
				}
			}
		}

		PropertyPathWalker PropertyPathWalker {
			get; set;
		}

		[DefaultValue (StringComparison.Ordinal)]
		public StringComparison StringComparison {
			get { return stringComparison; }
			set {
				if (stringComparison != value) {
					stringComparison = value;
					OnPropertyChanged (new PropertyChangedEventArgs ("StringComparison"));
				}
			}
		}

		public PropertyGroupDescription ()
			: this (null)
		{
		}

		public PropertyGroupDescription (string propertyName)
			: this (propertyName, null)
		{
		}

		public PropertyGroupDescription (string propertyName, IValueConverter converter)
			: this (propertyName, converter, StringComparison.Ordinal)
		{
		}

		public PropertyGroupDescription (string propertyName, IValueConverter converter, StringComparison stringComparison)
		{
			this.propertyName = propertyName;
			this.converter = converter;
			this.stringComparison = stringComparison;
		}

		public override object GroupNameFromItem (object item, int level, CultureInfo culture)
		{
			object value;
			if (string.IsNullOrEmpty (PropertyName)) {
				value = item;
			} else {
				PropertyPathWalker = PropertyPathWalker ?? new PropertyPathWalker (PropertyName);
				value = PropertyPathWalker.GetValue (item);
			}

			if (converter != null)
				value = converter.Convert (value, typeof (object), level, culture);
			return value;
		}

		public override bool NamesMatch (object groupName, object itemName)
		{
			var s1 = groupName as string;
			var s2 = itemName as string;
			if (s1 == null || s2 == null)
				return base.NamesMatch (groupName, itemName);
			return string.Equals (s1, s2, StringComparison);
		}
	}
}

