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

using Mono;

namespace System.Windows.Data
{
	abstract class PropertyPathNode : IPropertyPathNode, IListenINPC {

		public event EventHandler ValueChanged;

		object value;

		public DependencyProperty DependencyProperty {
			get; protected set;
		}

		public virtual bool IsBroken {
			get {
				// If any node in the middle of the chain has a null source,
				// then the final value cannot be retrieved so the chain is broken
				return Source == null || (PropertyInfo == null && DependencyProperty == null);
			}
		}

		public IWeakListener Listener {
			get; set;
		}

		public IPropertyPathNode Next {
			get; set;
		}

		public PropertyInfo PropertyInfo {
			get; protected set;
		}

		public object Source {
			get; set;
		}

		public object Value {
			get { return value; }
			protected set {
				if (!Helper.AreEqual (value, this.value)) {
					this.value = value;
					var h = ValueChanged;
					if (h != null && this.Next == null)
						h (this, EventArgs.Empty);
				}
			}
		}

		public Type ValueType {
			get; protected set;
		}

		protected PropertyPathNode ()
		{
			
		}

		protected virtual void OnSourceChanged (object oldSource, object newSource)
		{
			
		}

		protected virtual void OnSourcePropertyChanged (object o, PropertyChangedEventArgs e)
		{
			
		}

		public abstract void SetValue (object value);

		public void SetSource (object source)
		{
			if (Source != source || source == null) {
				var oldSource = Source;
				if (Listener != null) {
					Listener.Detach ();
					Listener = null;
				}

				Source = source;
				if (Source is INotifyPropertyChanged)
					Listener = new WeakINPCListener ((INotifyPropertyChanged) Source, this);

				OnSourceChanged (oldSource, Source);
				UpdateValue ();
				if (Next != null)
					Next.SetSource (Value);
			}
		}

		public abstract void UpdateValue ();

		void IListenINPC.OnPropertyChanged (object o, PropertyChangedEventArgs e)
		{
			OnSourcePropertyChanged (o, e);
		}
	}
}
