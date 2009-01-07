//
// Cursors.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2007 Novell, Inc.
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

namespace System.Windows.Input {

	public static class Cursors {

		static Cursors ()
		{
			Arrow = new Cursor (CursorType.Arrow);
			Eraser = new Cursor (CursorType.Eraser);
			Hand = new Cursor (CursorType.Hand);
			IBeam = new Cursor (CursorType.IBeam);
			None = new Cursor (CursorType.None);
			SizeNS = new Cursor (CursorType.SizeNS);
			SizeWE = new Cursor (CursorType.SizeWE);
			Stylus = new Cursor (CursorType.Stylus);
			Wait = new Cursor (CursorType.Wait);
		}
		
		internal static Cursor FromEnum (CursorType type)
		{
			switch (type){
			case CursorType.Arrow:
				return Arrow;
			case CursorType.Eraser:
				return Eraser;
			case CursorType.Hand:
				return Hand;
			case CursorType.IBeam:
				return IBeam;
			case CursorType.None:
				return None;
			case CursorType.SizeNS:
				return SizeNS;
			case CursorType.SizeWE:
				return SizeWE;
			case CursorType.Stylus:
				return Stylus;
			case CursorType.Wait:
				return Wait;
			case CursorType.Default:
			default:
				return null;
			}
		}

		public static Cursor None {
			get; private set;
		}
		public static Cursor Arrow {
			get; private set;
		}
		public static Cursor SizeWE {
			get; private set;
		}
		public static Cursor Eraser {
			get; private set;
		}
		public static Cursor Hand {
			get; private set;
		}
		public static Cursor Wait {
			get; private set;
		}
		public static Cursor SizeNS {
			get; private set;
		}
		public static Cursor IBeam {
			get; private set;
		}
		public static Cursor Stylus {
			get; private set;
		}
	}
}
