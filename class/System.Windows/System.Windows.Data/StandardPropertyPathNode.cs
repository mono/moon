//
// PropertyPathNode.cs
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
using System.Collections;
using System.ComponentModel;
using System.Reflection;

namespace System.Windows.Data
{
	class StandardPropertyPathNode : PropertyPathNode {

		public string PropertyName {
			get; private set;
		}

		public StandardPropertyPathNode (string propertyName, IPropertyPathNode next)
			: base (next)
		{
			PropertyName = propertyName;
		}

		protected override void OnSourceChanged (object oldSource, object newSource)
		{
			if (Source == null) {
				PropertyInfo = null;
			} else {
				PropertyInfo = Source.GetType ().GetProperty (PropertyName);
			}

			if (PropertyInfo == null) {
				ValueType = null;
				Value = null;
			} else {
				ValueType = PropertyInfo.PropertyType;
				Value = PropertyInfo.GetValue (Source, null);
			}
		}

		protected override void OnSourcePropertyChanged (object o, PropertyChangedEventArgs e)
		{
			if (e.PropertyName == PropertyName && PropertyInfo != null) {
				Value = PropertyInfo.GetValue (Source, null);
				if (Next != null)
					Next.SetSource (Value);
			}
		}

		public override void SetValue (object value)
		{
			if (PropertyInfo != null)
				PropertyInfo.SetValue (Source, value, null);
		}
	}
}
