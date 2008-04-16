//
// Value.cs: represents the unmanaged Value structure from runtime.cpp
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
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
using System.Runtime.InteropServices;

namespace Mono {

	public struct UnmanagedRepeatBehavior {
		public int kind;
		int padding;
		public double count;
		public long timespan;
	}

	public struct UnmanagedKeyTime {
		public int kind;
		int padding;
		public double percent;
		public long timespan;
	}

	public struct UnmanagedDuration {
		public int kind;
		int padding;
		public long timespan;
	}

	public struct UnmanagedColor {
		public double r;
		public double g;
		public double b;
		public double a;
	}

	[StructLayout(LayoutKind.Explicit)]
	public struct UnmanagedArray {
		[FieldOffset(0)] public int count;
		[FieldOffset(4)] public int refcount;
		[FieldOffset(8)] public double first_d;
		[FieldOffset(8)] public UnmanagedPoint first_pnt;
	}

	public struct UnmanagedPoint {
		public double x;
		public double y;
	}

	public struct UnmanagedRect {
		public double left;
		public double top;
		public double width;
		public double height;
	}

	[StructLayout(LayoutKind.Explicit)]
	public struct ValUnion {
		[FieldOffset(0)] public double d;
		[FieldOffset(0)] public long i64;
		[FieldOffset(0)] public ulong ui64;
		[FieldOffset(0)] public int i32;
		[FieldOffset(0)] public IntPtr p;
	}

	[StructLayout(LayoutKind.Explicit)]
	public struct Value {
		[FieldOffset (0)]
		public Kind k;

		[FieldOffset (4)]
		int padding;

		[FieldOffset (8)]
		public ValUnion u;
	}
}
