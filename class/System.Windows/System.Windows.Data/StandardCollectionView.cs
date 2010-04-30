using System;
using System.ComponentModel;
using System.Collections.Specialized;
using System.Globalization;
using System.Collections.ObjectModel;
using System.Collections;

namespace System.Windows.Data {

	sealed class StandardCollectionView : ICollectionView, IDeferRefresh {

		public event NotifyCollectionChangedEventHandler CollectionChanged;
		public event EventHandler CurrentChanged;
		public event CurrentChangingEventHandler CurrentChanging;

		IList list;

		public bool CanFilter {
			get; private set;
		}

		public bool CanGroup {
			get { return true; }
		}

		public bool CanSort {
			get; private set;
		}

		int Count {
			get { return list.Count; }
		}

		public CultureInfo Culture {
			get; set;
		}

		public object CurrentItem {
			get; private set;
		}

		public int CurrentPosition {
			get; private set;
		}

		int IDeferRefresh.DeferLevel {
			get; set;
		}

		public Predicate<object> Filter {
			get; set;
		}

		public ObservableCollection<GroupDescription> GroupDescriptions {
			get; private set;
		}

		public ReadOnlyObservableCollection<object> Groups {
			get; private set;
		}

		public bool IsCurrentAfterLast {
			get { return CurrentPosition == list.Count || Count == 0; }
		}

		public bool IsCurrentBeforeFirst {
			get { return CurrentPosition == -1 || Count == 0; }
		}

		public bool IsEmpty {
			get; private set;
		}

		bool IsValidSelection {
			get { return CurrentPosition >= 0 && CurrentPosition < list.Count; }
		}

		StandardCollectionViewGroup RootGroup {
			get; set;
		}

		public SortDescriptionCollection SortDescriptions {
			get; private set;
		}

		public IEnumerable SourceCollection {
			get; private set;
		}

		public StandardCollectionView (IList list)
		{
			this.list = list;
			SourceCollection = list;
			SortDescriptions = new SortDescriptionCollection ();
			GroupDescriptions = new ObservableCollection<GroupDescription> ();

			CurrentPosition = -1;
			MoveCurrentToPosition (0);
		}

		public bool Contains (object item)
		{
			return list.Contains (item);
		}

		public IDisposable DeferRefresh ()
		{
			return new Deferrer (this);
		}

		public IEnumerator GetEnumerator ()
		{
			return list.GetEnumerator ();
		}

		public bool MoveCurrentTo (object item)
		{
			return MoveCurrentTo (list.IndexOf (item));
		}

		bool MoveCurrentTo (int position)
		{
			if (CurrentPosition == position)
				return IsValidSelection;

			var h = CurrentChanging;
			if (h != null) {
				CurrentChangingEventArgs e = new CurrentChangingEventArgs (true);
				h (this, e);
				if (e.Cancel)
					return true;
			}

			CurrentItem = position < 0 || position >= list.Count ? null : list [position];
			CurrentPosition = position;

			var h2 = CurrentChanged;
			if (h2 != null)
				h2 (this, EventArgs.Empty);

			return IsValidSelection;
		}

		public bool MoveCurrentToFirst ()
		{
			return MoveCurrentTo (0);
		}

		public bool MoveCurrentToLast ()
		{
			return MoveCurrentTo (list.Count - 1);
		}

		public bool MoveCurrentToNext ()
		{
			return CurrentPosition != list.Count && MoveCurrentTo (CurrentPosition + 1);
		}

		public bool MoveCurrentToPosition (int position)
		{
			return MoveCurrentTo (position);
		}

		public bool MoveCurrentToPrevious ()
		{
			return CurrentPosition != -1 && MoveCurrentTo (CurrentPosition - 1);
		}

		public void Refresh ()
		{
			if (((IDeferRefresh) this).DeferLevel != 0)
				return;

			if (RootGroup == null)
				RootGroup = new StandardCollectionViewGroup (null);

			RootGroup.ClearItems ();
			if (GroupDescriptions.Count == 0) {
				Groups = null;
			} else {
				Groups = RootGroup.Items;
				foreach (var item in list) {
					AppendToGroup (item, 0, RootGroup);
				}
			}
		}

		void AppendToGroup (object item, int depth, StandardCollectionViewGroup group)
		{
			if (depth < GroupDescriptions.Count) {
				var desc = GroupDescriptions [depth];
				var name = desc.GroupNameFromItem (item, depth, null);
				StandardCollectionViewGroup subGroup = null;
				foreach (StandardCollectionViewGroup g in group.Items) {
					if (desc.NamesMatch (g.Name, name)) {
						subGroup = g;
						break;
					}
				}
				if (subGroup == null) {
					subGroup = new StandardCollectionViewGroup (group, name, depth == (GroupDescriptions.Count - 1));
					group.AddItem (subGroup);
				}

				AppendToGroup (item, depth + 1, subGroup);
			} else {
				group.AddItem (item);
			}
		}
	}
}
