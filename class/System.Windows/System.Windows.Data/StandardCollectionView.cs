using System;
using System.ComponentModel;
using System.Collections.Specialized;
using System.Globalization;
using System.Collections.ObjectModel;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace System.Windows.Data {

	sealed class StandardCollectionView : ICollectionView, INotifyPropertyChanged, IDeferRefresh {

		public event NotifyCollectionChangedEventHandler CollectionChanged;
		public event EventHandler CurrentChanged;
		public event CurrentChangingEventHandler CurrentChanging;
		public event PropertyChangedEventHandler PropertyChanged;

		CultureInfo culture;
		object currentItem;
		int currentPosition;
		Predicate<object> filter;
		List <object> filteredList;
		bool isCurrentAfterLast;
		bool isCurrentBeforeFirst;

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
			get { return culture; }
			set {
				if (culture != value) {
					culture = value;
					RaisePropertyChanged ("Culture");
				}
			}
		}

		public object CurrentItem {
			get { return currentItem; }
			set {
				if (currentItem != value) {
					currentItem = value;
					RaisePropertyChanged ("CurrentItem");
				}
			}
		}

		public int CurrentPosition {
			get { return currentPosition; }
			set {
				if (currentPosition != value) {
					currentPosition = value;
					RaisePropertyChanged ("CurrentPosition");
				}
			}
		}

		int IDeferRefresh.DeferLevel {
			get; set;
		}

		public Predicate<object> Filter {
			get { return filter; }
			set {
				filter = value;
				Refresh ();
			}
		}

		public ObservableCollection<GroupDescription> GroupDescriptions {
			get; private set;
		}

		public ReadOnlyObservableCollection<object> Groups {
			get; private set;
		}

		public bool IsCurrentAfterLast {
			get { return isCurrentAfterLast; }
			private set {
				if (isCurrentAfterLast != value) {
					isCurrentAfterLast = value;
					RaisePropertyChanged ("IsCurrentAfterLast");
				}
			}
		}

		public bool IsCurrentBeforeFirst {
			get { return isCurrentBeforeFirst; }
			private set {
				if (isCurrentBeforeFirst != value) {
					isCurrentBeforeFirst = value;
					RaisePropertyChanged ("IsCurrentBeforeFirst");
				}
			}
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

		public StandardCollectionView (IEnumerable list)
		{
			SourceCollection = list;
			SortDescriptions = new SortDescriptionCollection ();
			GroupDescriptions = new ObservableCollection<GroupDescription> ();

			filteredList = new List <object> (list.Cast <object> ());
			CurrentPosition = -1;
			MoveCurrentToPosition (0);

			if (list is INotifyCollectionChanged)
				((INotifyCollectionChanged) list).CollectionChanged += (o, e) => Refresh ();

			GroupDescriptions.CollectionChanged += (o, e) => Refresh ();
			((INotifyCollectionChanged) SortDescriptions).CollectionChanged += (o, e) => Refresh ();
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

			IsCurrentAfterLast = position == filteredList.Count || Count == 0;
			IsCurrentBeforeFirst = position == -1 || Count == 0;
			CurrentPosition = position;
			CurrentItem = position < 0 || position >= filteredList.Count ? null : filteredList [position];

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

		void RaisePropertyChanged (string propertyName)
		{
			var h = PropertyChanged;
			if (h != null)
				h (this, new PropertyChangedEventArgs (propertyName));
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

			if (SortDescriptions.Count > 0)
				filteredList.Sort (new PropertyComparer (SortDescriptions));

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
				var name = desc.GroupNameFromItem (item, depth, Culture);
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
