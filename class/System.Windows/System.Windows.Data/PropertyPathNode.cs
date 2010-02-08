using System;
using System.Collections;
using System.ComponentModel;
using System.Reflection;

namespace System.Windows.Data
{
	abstract class PropertyPathNode : IPropertyPathNode {

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

		public IPropertyPathNode Next {
			get; private set;
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
				if (this.value != value) {
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

		protected PropertyPathNode (IPropertyPathNode next)
		{
			Next = next;
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
			if (Source != source) {
				var oldSource = Source;
				if (Source is INotifyPropertyChanged)
					((INotifyPropertyChanged) Source).PropertyChanged -= OnSourcePropertyChanged;
				Source = source;
				if (Source is INotifyPropertyChanged)
					((INotifyPropertyChanged) Source).PropertyChanged += OnSourcePropertyChanged;

				OnSourceChanged (oldSource, Source);
				if (Next != null)
					Next.SetSource (Value);
			}
		}
	}
}
