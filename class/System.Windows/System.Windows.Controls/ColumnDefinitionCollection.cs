//
// System.Windows.Media.PathSegmentCollection class
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
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

using System.Windows;
using Mono;

namespace System.Windows.Controls {

	public sealed partial class ColumnDefinitionCollection : PresentationFrameworkCollection<ColumnDefinition> {
		public ColumnDefinitionCollection () : base (NativeMethods.column_definition_collection_new ())
		{
		}
		
		internal ColumnDefinitionCollection (IntPtr raw) : base (raw)
		{
		}
		
		internal override Kind GetKind ()
		{
			return Kind.COLUMNDEFINITION_COLLECTION;
		}

		public override void Add (ColumnDefinition value)
		{
			AddImpl (value);
		}
		
		public override bool Contains (ColumnDefinition value)
		{
			return ContainsImpl (value);
		}
		
		public override int IndexOf (ColumnDefinition value)
		{
			return IndexOfImpl (value);
		}
		
		public override void Insert (int index, ColumnDefinition value)
		{
			InsertImpl (index, value);
		}
		
		public override bool Remove (ColumnDefinition value)
		{
			return RemoveImpl (value);
		}
		
		public override ColumnDefinition this[int index] {
			get { return GetItemImpl (index); }
			set { SetItemImpl (index, value); }
		}
	}
}
