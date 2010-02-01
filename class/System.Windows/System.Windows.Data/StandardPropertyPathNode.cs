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

namespace System.Windows
{
	public class PropertyPathNode {

		public event EventHandler ValueChanged;

		object value;

		public PropertyPathNode Next {
			get; set;
		}

		public int PropertyIndex {
			get; private set;
		}

		public string PropertyName {
			get; private set;
		}

		public PropertyInfo PropertyInfo {
			get; set;
		}

		public object Source {
			get; set;
		}

		public object Value {
			get { return value; }
			set {
				if (this.value != value) {
					this.value = value;
					var h = ValueChanged;
					if (h != null && this.Next == null)
						h (this, EventArgs.Empty);
				}
			}
		}

		public PropertyPathNode (string propertyName)
			: this (propertyName, null)
		{
			
		}

		public PropertyPathNode (string propertyName, PropertyPathNode next)
		{
			Next = next;
			PropertyName = propertyName;
			PropertyIndex = -1;

			int close = propertyName.LastIndexOf (']');
			if (close > -1) {
				int open = propertyName.LastIndexOf ('[');
				PropertyIndex = int.Parse (propertyName.Substring (open + 1, close - open - 1));
				PropertyName = propertyName.Substring (0, open);
			}
		}

		public void Update (object source)
		{
			if (Source is INotifyPropertyChanged)
				((INotifyPropertyChanged) Source).PropertyChanged -= PropertyChanged;
			Source = source;
			if (Source is INotifyPropertyChanged)
				((INotifyPropertyChanged) Source).PropertyChanged += PropertyChanged;

			if (source == null) {
				PropertyInfo = null;
				Value = null;
			} else {
				PropertyInfo = source.GetType ().GetProperty (PropertyName);
				if (PropertyInfo == null) {
					Value = null;
				} else {
					source = PropertyInfo.GetValue (source, null);
					if (PropertyIndex != -1)
						source = ((IList) source)[PropertyIndex];
					Value = source;
				}
			}
			if (Next != null)
				Next.Update (Value);
		}

		void PropertyChanged (object o, PropertyChangedEventArgs e)
		{
			if (e.PropertyName == PropertyName && PropertyInfo != null) {
				Value = PropertyInfo.GetValue (Source, null);
				if (Next != null)
					Next.Update (Value);
			}
		}
	}
}

