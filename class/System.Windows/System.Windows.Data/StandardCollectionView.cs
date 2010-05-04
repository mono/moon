using System;
using System.ComponentModel;
using System.Collections.Specialized;
using System.Globalization;
using System.Collections.ObjectModel;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace System.Windows.Data {

	sealed class StandardCollectionView : ICollectionView, IDeferRefresh {

		public event NotifyCollectionChangedEventHandler CollectionChanged;
		public event EventHandler CurrentChanged;
		public event CurrentChangingEventHandler CurrentChanging;

		Predicate<object> filter;
		IList filteredList;

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
			get { return filteredList.Count; }
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
			get { return filter; }
			set {
				using (DeferRefresh ())
					filter = value;
			}
		}

		public ObservableCollection<GroupDescription> GroupDescriptions {
			get; private set;
		}

		public ReadOnlyObservableCollection<object> Groups {
			get; private set;
		}

		public bool IsCurrentAfterLast {
			get { return CurrentPosition == filteredList.Count || Count == 0; }
		}

		public bool IsCurrentBeforeFirst {
			get { return CurrentPosition == -1 || Count == 0; }
		}

		public bool IsEmpty {
			get; private set;
		}

		bool IsValidSelection {
			get { return CurrentPosition >= 0 && CurrentPosition < filteredList.Count; }
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
			SourceCollection = list;
			SortDescriptions = new SortDescriptionCollection ();
			GroupDescriptions = new ObservableCollection<GroupDescription> ();

			filteredList = new List <object> (list.Cast <object> ());
			CurrentPosition = -1;
			MoveCurrentToPosition (0);
		}

		public bool Contains (object item)
		{
			return filteredList.Contains (item);
		}

		public IDisposable DeferRefresh ()
		{
			return new Deferrer (this);
		}

		public IEnumerator GetEnumerator ()
		{
			return filteredList.GetEnumerator ();
		}

		public bool MoveCurrentTo (object item)
		{
			return MoveCurrentTo (filteredList.IndexOf (item));
		}

		bool MoveCurrentTo (int position)
		{
			return MoveCurrentTo (position, false);
		}

		bool MoveCurrentTo (int position, bool force)
		{
			if (CurrentPosition == position && !force)
				return IsValidSelection;

			var h = CurrentChanging;
			if (h != null) {
				CurrentChangingEventArgs e = new CurrentChangingEventArgs (true);
				h (this, e);
				if (e.Cancel)
					return true;
			}

			CurrentItem = position < 0 || position >= filteredList.Count ? null : filteredList [position];
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
			return MoveCurrentTo (filteredList.Count - 1);
		}

		public bool MoveCurrentToNext ()
		{
			return CurrentPosition != filteredList.Count && MoveCurrentTo (CurrentPosition + 1);
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

			filteredList.Clear ();
			foreach (var item in SourceCollection)
				if (filter == null || Filter (item))
					filteredList.Add (item);

			Groups = null;
			RootGroup.ClearItems ();
			if (GroupDescriptions.Count > 0 && filteredList.Count > 0) {
				foreach (var item in filteredList)
					AppendToGroup (item, 0, RootGroup);
				Groups = RootGroup.Items;
			}

			if (filteredList.Count > 0) {
				MoveCurrentTo (CurrentPosition, true);
			} else {
				MoveCurrentTo (-1);
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
