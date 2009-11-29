
using System.Collections.Specialized;

namespace System.Windows.Controls.Primitives {

	
	public class ItemsChangedEventArgs : EventArgs {

		private NotifyCollectionChangedAction action;

		private int item_count;
		private int item_ui_count;

		private GeneratorPosition position;
		private GeneratorPosition old_position;
		

		public NotifyCollectionChangedAction Action {
			get { return action; }
		}

		public int ItemCount {
			get { return item_count; }
		}

		public int ItemUICount {
			get { return item_ui_count; }
		}

		public GeneratorPosition OldPosition {
			get { return old_position; }
		}

		public GeneratorPosition Position {
			get { return position; }
		}
	}
}


