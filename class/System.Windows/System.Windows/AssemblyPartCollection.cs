//
// AssemblyPartCollection.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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
using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Input;
using Mono;

namespace System.Windows {
	public sealed partial class AssemblyPartCollection : PresentationFrameworkCollection<AssemblyPart> {
		public AssemblyPartCollection () : base (NativeMethods.assembly_part_collection_new ())
		{
		}
		
		internal AssemblyPartCollection (IntPtr raw) : base (raw)
		{
		}
		
		internal override Kind GetKind ()
		{
			return Kind.ASSEMBLYPART_COLLECTION;
		}

		public override void Add (AssemblyPart value)
		{
			AddImpl (value);
		}
		
		public override bool Contains (AssemblyPart value)
		{
			return ContainsImpl (value);
		}
		
		public override int IndexOf (AssemblyPart value)
		{
			return IndexOfImpl (value);
		}
		
		public override void Insert (int index, AssemblyPart value)
		{
			InsertImpl (index, value);
		}
		
		public override bool Remove (AssemblyPart value)
		{
			return RemoveImpl (value);
		}
		
		public override AssemblyPart this[int index] {
			get { return GetItemImpl (index); }
			set { SetItemImpl (index, value); }
		}
	}
}
