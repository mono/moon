using System;
using System.Collections;
using System.Collections.Specialized;
using System.Reflection;

namespace System.Windows.Data
{
	class IndexedPropertyPathNode : PropertyPathNode {

		public int Index {
			get; private set;
		}

		public IndexedPropertyPathNode (int index, IPropertyPathNode next)
			: base (next)
		{
			Index = index;
		}

		bool GetIndexer ()
		{
			var members = Source.GetType ().GetDefaultMembers ();
			if (members.Length == 1  && members [0] is PropertyInfo) {
				PropertyInfo = (PropertyInfo) members [0];
				return true;
			}
			return false;
		}

		void OnCollectionChanged (object o, NotifyCollectionChangedEventArgs e)
		{
			object newVal = ((IList)Source) [Index];
			if (Value != newVal) {
				Value = newVal;
				if (Next != null)
					Next.SetSource (Value);
			}
		}

		protected override void OnSourceChanged (object oldSource, object newSource)
		{
			base.OnSourceChanged (oldSource, newSource);

			if (oldSource is INotifyCollectionChanged)
				((INotifyCollectionChanged) oldSource).CollectionChanged -= OnCollectionChanged;
			if (newSource is INotifyCollectionChanged)
				((INotifyCollectionChanged) newSource).CollectionChanged += OnCollectionChanged;

			IList source = Source as IList;

			if (source == null || !GetIndexer ()) {
				PropertyInfo = null;
				ValueType = null;
				Value = null;
			} else {
				ValueType = PropertyInfo.PropertyType;
				Value = source [Index];
			}
		}

		public override void SetValue (object value)
		{
			if (PropertyInfo != null)
				((IList) Source)[Index] = value;
		}
	}
}
