

namespace System.Windows.Controls.Primitives {

	
	public class ItemsChangedEventArgs : EventArgs {

		// public NotifyCollectionChangedAction Action { get; }

		public int ItemCount {
			get { return 0; }
		}

		public int ItemUICount {
			get { return 0; }
		}

		public GeneratorPosition OldPosition {
			get { return new GeneratorPosition (); }
		}

		public GeneratorPosition Position {
			get { return new GeneratorPosition (); }
		}
	}
}


