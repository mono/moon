//
// CollectionViewSource.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#pragma warning disable 67 // "The event 'E' is never used" shown for Filter

using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Globalization;
using System.Windows;

namespace System.Windows.Data {

	public partial class CollectionViewSource : DependencyObject, IDeferRefresh {
		public static readonly DependencyProperty SourceProperty =
			DependencyProperty.Register ("SourceProperty", typeof (object), typeof (CollectionViewSource),
							 new PropertyMetadata (null, SourceChanged));

		static void SourceChanged (DependencyObject o, DependencyPropertyChangedEventArgs e)
		{
			((CollectionViewSource) o).OnSourceChanged (e.OldValue, e.NewValue);
		}

		public static readonly DependencyProperty ViewProperty =
			DependencyProperty.Register ("ViewProperty", typeof (ICollectionView), typeof (CollectionViewSource), null);

		public event FilterEventHandler Filter;

		public CultureInfo Culture {
			get; set;
		}

		int IDeferRefresh.DeferLevel {
			get; set;
		}

		public ObservableCollection<GroupDescription> GroupDescriptions {
			get; private set;
		}

		public SortDescriptionCollection SortDescriptions {
			get; private set;
		}

		public object Source {
			get { return (object) GetValue (SourceProperty); }
			set { SetValue (SourceProperty, value); }
		}

		public ICollectionView View {
			get { return (ICollectionView) GetValue (ViewProperty); }
			private set { SetValueImpl (ViewProperty, value); }
		}

		public CollectionViewSource ()
		{
			SortDescriptions = new SortDescriptionCollection ();
			GroupDescriptions = new ObservableCollection<GroupDescription> ();

			GroupDescriptions.CollectionChanged += UpdateViewGroupDescriptions;
		}

		public IDisposable DeferRefresh ()
		{
			return new Deferrer (this);
		}

		protected virtual void OnCollectionViewTypeChanged (Type oldCollectionViewType, Type newCollectionViewType)
		{
			
		}

		protected virtual void OnSourceChanged (object oldSource, object newSource)
		{
			View = new StandardCollectionView ((IList) newSource);
		}

		void IDeferRefresh.Refresh ()
		{
			// FIXME: What does deferring a CollectionViewSource do?
		}

		void UpdateViewGroupDescriptions (object sender, NotifyCollectionChangedEventArgs e)
		{
			using (View.DeferRefresh ()) {
				View.GroupDescriptions.Clear ();
				for (int i = 0; i < GroupDescriptions.Count; i++)
					View.GroupDescriptions.Add (GroupDescriptions [i]);
	
				// FIXME: This is what the tests say but I must be missing something. This is crazy.
				View.Filter = null;
			}
		}
	}
}
