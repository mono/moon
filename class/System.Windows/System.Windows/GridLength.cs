//
// GridLength.cs
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
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
using System.Runtime.InteropServices;

namespace System.Windows {

	[StructLayout (LayoutKind.Sequential)]
	public struct GridLength {
		double val;
		GridUnitType type;
		
		static GridLength auto;
		
		static GridLength ()
		{
			auto = new GridLength (0, GridUnitType.Auto);
		}

		public GridLength (double pixels)
		{
			val = pixels;
			type = GridUnitType.Pixel;
		}

		public GridLength (double value, GridUnitType type)
		{
			val = value;
			this.type = type;
		}

		public double Value {
			get {
				return val;
			}

			set {
				val = value;
			}
		}

		public GridUnitType GridUnitType {
			get {
				return type;
			}
		}
		
		public bool IsStar {
			get {
				return GridUnitType == GridUnitType.Star;
			}
		}

		public bool IsAuto {
			get {
				return GridUnitType == GridUnitType.Auto;
			}
		}

		public bool IsAbsolute {
			get {
				return GridUnitType == GridUnitType.Pixel;
			}
		}

		public static GridLength Auto {
			get {
				return auto;
			}
		}
	}
}