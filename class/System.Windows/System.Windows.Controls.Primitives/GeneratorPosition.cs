//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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

namespace System.Windows.Controls.Primitives {

	public struct GeneratorPosition {
		public GeneratorPosition (int index, int offset)
		{
			Index = index;
			Offset = offset;
		}
		
		public int Index {
			get; set;
		}
		
		public int Offset {
			get; set;
		}
		
		public override bool Equals (object obj)
		{
			return obj is GeneratorPosition && this == ((GeneratorPosition) obj);
		}
		
		public override int GetHashCode ()
		{
			return Index + Offset;
		}
		
		public override string ToString ()
		{
			return String.Format ("GeneratorPosition ({0},{1})", Index, Offset);
		}
		
		public static bool operator== (GeneratorPosition gp1, GeneratorPosition gp2)
		{
			return gp1.Index == gp2.Index && gp1.Offset == gp2.Offset;
		}
		
		public static bool operator!= (GeneratorPosition gp1, GeneratorPosition gp2)
		{
			return !(gp1 == gp2);
		}
	}
}


