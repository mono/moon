using System;

namespace System.Windows.Data {

	class StandardCollectionViewGroup : CollectionViewGroup {

		bool isBottomLevel;
		public override bool IsBottomLevel {
			get { return isBottomLevel; }
		}

		StandardCollectionViewGroup Parent {
			get; set;
		}

		public StandardCollectionViewGroup (object name)
			: this (null, name)
		{
			
		}

		public StandardCollectionViewGroup (StandardCollectionViewGroup parent, object name)
			: this (parent, name, false)
		{
			
		}

		public StandardCollectionViewGroup (StandardCollectionViewGroup parent, object name, bool isBottomLevel)
			: base (name)
		{
			this.isBottomLevel = isBottomLevel;
			Parent = parent;
		}

		internal void AddItem (object item)
		{
			ProtectedItems.Add (item);
			if (!(item is StandardCollectionViewGroup))
				IncrementCount ();
		}

		internal void ClearItems ()
		{
			ProtectedItems.Clear ();
			ProtectedItemCount = 0;
		}

		internal void DecrementCount ()
		{
			ProtectedItemCount --;
			if (Parent != null)
				Parent.DecrementCount ();
		}

		internal void IncrementCount ()
		{
			ProtectedItemCount ++;
			if (Parent != null)
				Parent.IncrementCount ();
		}

		internal void RemoveItem (object item)
		{
			if (ProtectedItems.Remove (item))
				if (!(item is StandardCollectionViewGroup))
					DecrementCount ();
		}
	}
}

