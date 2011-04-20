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

		public event EventHandler IsBrokenChanged;
		public event EventHandler ValueChanged;

		public DependencyProperty DependencyProperty {
			get; protected set;
		}

		public bool IsBroken {
			get; private set;
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
			get; private set;
		}

		public Type ValueType {
			get; protected set;
		}

		protected PropertyPathNode ()
		{
			IsBroken = true;
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

		protected virtual bool CheckIsBroken ()
		{
			return Source == null || (PropertyInfo == null && DependencyProperty == null);
		}

		protected void UpdateValueAndIsBroken (object newValue, bool isBroken)
		{
			bool emitBrokenChanged = IsBroken != isBroken;
			bool emitValueChanged = !Helper.AreEqual (Value, newValue);

			IsBroken = isBroken;
			Value = newValue;

			// If Value changes it implicitly covers the case where
			// IsBroken changed too, so don't emit both events.
			if (emitValueChanged && ValueChanged != null) {
				ValueChanged (this, EventArgs.Empty);
			} else if (emitBrokenChanged && IsBrokenChanged != null) {
				IsBrokenChanged (this, EventArgs.Empty);
			}
		}

		void IListenINPC.OnPropertyChanged (object o, PropertyChangedEventArgs e)
		{
			OnSourcePropertyChanged (o, e);
		}
	}
}
