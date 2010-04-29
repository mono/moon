using System;
using System.ComponentModel;
using System.Collections.Specialized;
using System.Globalization;
using System.Collections.ObjectModel;
using System.Collections;

namespace System.Windows.Data {

	class StandardCollectionView : ICollectionView {

		public event NotifyCollectionChangedEventHandler CollectionChanged;
		public event EventHandler CurrentChanged;
		public event CurrentChangingEventHandler CurrentChanging;

		IList list;
		ObservableCollection<object> groups;

		public bool CanFilter {
			get; private set;
		}

		public bool CanGroup {
			get; private set;
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
			groups = new ObservableCollection<object> ();
			Groups = new  ReadOnlyObservableCollection<object>(groups);
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
			throw new System.NotImplementedException();
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
			throw new System.NotImplementedException();
		}
	}
}
