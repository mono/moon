using System;

namespace System.Windows.Data
{
	class StandardCollectionViewGroup : CollectionViewGroup
	{
		public override bool IsBottomLevel {
			get; private set;
		}

		public StandardCollectionViewGroup (object name)
			: base (name)
		{
			IsBottomLevel = true;
		}

		internal void AddItem (object item)
		{
			ProtectedItems.Add (item);
			ProtectedItemCount ++;
		}

		internal void RemoveItem (object item)
		{
			if (ProtectedItems.Remove (item))
				ProtectedItemCount --;
		}
	}
}

