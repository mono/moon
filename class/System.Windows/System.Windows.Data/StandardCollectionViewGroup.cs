using System;

namespace System.Windows.Data {

	class StandardCollectionViewGroup : CollectionViewGroup {

		bool isBottomLevel;
		public override bool IsBottomLevel {
			get { return isBottomLevel; }
		}

		public StandardCollectionViewGroup (object name)
			: this (name, false)
		{
			
		}
		
		public StandardCollectionViewGroup (object name, bool isBottomLevel)
			: base (name)
		{
			this.isBottomLevel = isBottomLevel;
		}

		internal void AddItem (object item)
		{
			ProtectedItems.Add (item);
			ProtectedItemCount ++;
		}

		internal void ClearItems ()
		{
			ProtectedItems.Clear ();
		}

		internal void RemoveItem (object item)
		{
			if (ProtectedItems.Remove (item))
				ProtectedItemCount --;
		}
	}
}

