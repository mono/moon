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
			get; private set;
		}

		public bool IsCurrentBeforeFirst {
			get; private set;
		}

		public bool IsEmpty {
			get; private set;
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
				return false;
			
			CurrentItem = list [position];
			CurrentPosition = position;
			return true;
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
			return MoveCurrentTo (CurrentPosition + 1);
		}

		public bool MoveCurrentToPosition (int position)
		{
			return MoveCurrentTo (position);
		}

		public bool MoveCurrentToPrevious ()
		{
			return MoveCurrentTo (CurrentPosition - 1);
		}

		public void Refresh ()
		{
			throw new System.NotImplementedException();
		}
	}
}

