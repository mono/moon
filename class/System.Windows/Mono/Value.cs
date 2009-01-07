//
// Value.cs: represents the unmanaged Value structure from runtime.cpp
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
using System.Windows;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Runtime.InteropServices;

namespace Mono {

	internal struct UnmanagedColor {
		public double r;
		public double g;
		public double b;
		public double a;
	}

	[StructLayout(LayoutKind.Explicit)]
	internal struct ValUnion {
		[FieldOffset(0)] public double d;
		[FieldOffset(0)] public long i64;
		[FieldOffset(0)] public ulong ui64;
		[FieldOffset(0)] public int i32;
		[FieldOffset(0)] public IntPtr p;
	}

	internal struct Value {
		// Note: Keep these flags in sync with the native version
		const int NullFlag = 1;
		
		public Kind k;
		public int bitfield;
		public ValUnion u;

		public bool IsNull {
			get { return (bitfield & NullFlag) == NullFlag; }
			set {
				if (value)
					bitfield |= NullFlag;
				else
					bitfield &= ~NullFlag;
			}
		}

		public static Value Empty {
			get { return new Value (); }
		}

		static bool slow_codepath_error_shown = false;
		
		public static object ToObject (Type type, IntPtr value)
		{
			if (value == IntPtr.Zero)
				return null;
			
			unsafe {
				Value *val = (Value *) value;

				if (val->IsNull) {
					return null;
				}
				switch (val->k) {
				case Kind.INVALID:
					return null;
					
				case Kind.BOOL:
					return val->u.i32 != 0;

				case Kind.DOUBLE:
					return val->u.d;
					
				case Kind.UINT64:
					return val->u.ui64;
					
				case Kind.INT64:
					return val->u.i64;
					
				case Kind.TIMESPAN:
					return new TimeSpan (val->u.i64);
						
				case Kind.INT32:
					// marshall back to the .NET type that we simply serialised as int for unmanaged usage
					int i32 = val->u.i32;
					if (type == typeof (System.Windows.Input.Cursor))
						return (CursorType)i32 == CursorType.Default ? null : new Cursor ((CursorType) i32);
					else if (type == typeof (FontStretch))
						return new FontStretch ((FontStretchKind) i32);
					else if (type == typeof (FontStyle))
						return new FontStyle ((FontStyleKind) i32);
					else if (type == typeof (FontWeight))
						return new FontWeight ((FontWeightKind) i32);
					else if (type != null && type.IsEnum)
						return Enum.ToObject (type, i32);
					else if (type == typeof (char))
						return (char) i32;
					else
						return i32;

				case Kind.MANAGED:
					IntPtr managed_object = val->u.p;
					GCHandle handle = Helper.GCHandleFromIntPtr (managed_object);
					return handle.Target;
					
				case Kind.STRING: {
					string str = Helper.PtrToStringAuto (val->u.p);
					if (type == null)
						return str;
					
					// marshall back to the .NET type that we simply serialised as 'string' for unmanaged usage
					if (type == typeof (System.Windows.Markup.XmlLanguage))
						return XmlLanguage.GetLanguage (str);
					else if (type == typeof (System.Windows.Media.FontFamily))
						return new FontFamily (str);
					else if (type == typeof (System.Uri))
						return new Uri (str, UriKind.RelativeOrAbsolute);
					else
						return str;
				}
				
				case Kind.POINT: {
					Point *point = (Point*)val->u.p;
					return (point == null) ? new Point (0,0) : *point;
				}
				
				case Kind.RECT: {
					Rect *rect = (Rect*)val->u.p;
					return (rect == null) ? new Rect (0,0,0,0) : *rect;
				}

				case Kind.SIZE: {
					Size *size = (Size*)val->u.p;
					return (size == null) ? new Size (0,0) : *size;
				}

				case Kind.CORNERRADIUS: {
					CornerRadius *corner = (CornerRadius*)val->u.p;
					return (corner == null) ? new CornerRadius (0) : *corner;
				}

				case Kind.THICKNESS: {
					Thickness *thickness = (Thickness*)val->u.p;
					return (thickness == null) ? new Thickness (0) : *thickness;
				}
					
				case Kind.COLOR: {
					UnmanagedColor *color = (UnmanagedColor*)val->u.p;
					if (color == null)
						return new Color ();
					return Color.FromArgb ((byte)(255 * color->a), (byte)(255 * color->r), (byte)(255 * color->g), (byte)(255 * color->b));
				}
					
				case Kind.MATRIX: {
					double *dp = (double*)val->u.p;
					
					return new Matrix (dp [0], dp [1], dp [2], dp [3], dp [4], dp [5]);					
				}
					
				case Kind.DURATION: {
					Duration* duration = (Duration*)val->u.p;
					return (duration == null) ? Duration.Automatic : *duration;
				}
					
				case Kind.KEYTIME: {
					KeyTime* keytime = (KeyTime*)val->u.p;
					return (keytime == null) ? KeyTime.FromTimeSpan (TimeSpan.Zero) : *keytime;
				}
					
				case Kind.REPEATBEHAVIOR: {
					RepeatBehavior *repeat = (RepeatBehavior*)val->u.p;
					return (repeat == null) ? new RepeatBehavior () : *repeat;
				}
				}

				if (!slow_codepath_error_shown){
					Report.Warning ("DependencyObject type testing now using a very slow code path");
					slow_codepath_error_shown = true;
				}

				if (NativeMethods.type_is_dependency_object (val->k)){
					// Old fast test: if (val->k > Kind.DEPENDENCY_OBJECT){

 					if (val->u.p == IntPtr.Zero)
 						return null;
					
 					return DependencyObject.Lookup (val->k, val->u.p);
				}

				throw new Exception (String.Format ("Do not know how to convert {0}  {1}", val->k, (int) val->k));
			}
		}

		public static Value FromObject (object v)
		{
			return FromObject (v, false);
		}
		
		//
		// How do we support "null" values, should the caller take care of that?
		//
		public static Value FromObject (object v, bool as_managed_object)
		{
			Value value = new Value ();
			
			unsafe {
				if (v is DependencyObject) {
					DependencyObject dov = (DependencyObject) v;

					if (dov.native == IntPtr.Zero)
						throw new Exception (String.Format (
							"Object {0} has not set its native property", dov.GetType()));

					//
					// Keep track of this object, so we know how to map it
					// if it comes back. 
					//
					DependencyObject.TrackNativeReference (dov);

					value.k = dov.GetKind ();
					value.u.p = dov.native;

				}
				else if (v is int || (v.GetType ().IsEnum && Enum.GetUnderlyingType (v.GetType()) == typeof(int))) {
					value.k = Kind.INT32;
					value.u.i32 = (int) v;
				}
				else if (v is bool) {
					value.k = Kind.BOOL;
					value.u.i32 = ((bool) v) ? 1 : 0;
				}
				else if (v is double) {
					value.k = Kind.DOUBLE;
					value.u.d = (double) v;
				}
				else if (v is long) {
					value.k = Kind.INT64;
					value.u.i64 = (long) v;
				}
				else if (v is TimeSpan) {
					TimeSpan ts = (TimeSpan) v;
					value.k = Kind.TIMESPAN;
					value.u.i64 = ts.Ticks;
				}
				else if (v is ulong) {
					value.k = Kind.UINT64;
					value.u.ui64 = (ulong) v;
				}
				else if (v is char) {
					value.k = Kind.CHAR;
					value.u.i32 = (int) (char)v;
				}
				else if (v is string) {
					value.k = Kind.STRING;

					byte[] bytes = System.Text.Encoding.UTF8.GetBytes ((string)v);
					IntPtr result = Helper.AllocHGlobal (bytes.Length + 1);
					Marshal.Copy (bytes, 0, result, bytes.Length);
					Marshal.WriteByte (result, bytes.Length, 0);

					value.u.p = result;
				}
				else if (v is Rect) {
					Rect rect = (Rect) v;
					value.k = Kind.RECT;
					value.u.p = Helper.AllocHGlobal (sizeof (Rect));
					Marshal.StructureToPtr (rect, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is Size) {
					Size size = (Size) v;
					value.k = Kind.SIZE;
					value.u.p = Helper.AllocHGlobal (sizeof (Size));
					Marshal.StructureToPtr (size, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is CornerRadius) {
					CornerRadius corner = (CornerRadius) v;
					value.k = Kind.CORNERRADIUS;
					value.u.p = Helper.AllocHGlobal (sizeof (CornerRadius));
					Marshal.StructureToPtr (corner, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is Point) {
					Point pnt = (Point) v;
					value.k = Kind.POINT;
					value.u.p = Helper.AllocHGlobal (sizeof (Point));
					Marshal.StructureToPtr (pnt, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is Thickness) {
					Thickness thickness = (Thickness)v;
					value.k = Kind.THICKNESS;
					value.u.p = Helper.AllocHGlobal (sizeof (Thickness));
					Marshal.StructureToPtr (thickness, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is Color) {
					Color c = (Color) v;
					value.k = Kind.COLOR;
					value.u.p = Helper.AllocHGlobal (sizeof (UnmanagedColor));
					UnmanagedColor* color = (UnmanagedColor*) value.u.p;
					color->r = c.R / 255.0f;
					color->g = c.G / 255.0f;
					color->b = c.B / 255.0f;
					color->a = c.A / 255.0f;
				}
				else if (v is Matrix) {
					Matrix mat = (Matrix) v;
					value.k = Kind.MATRIX;
					value.u.p = Helper.AllocHGlobal (sizeof (double) * 6);
					Marshal.StructureToPtr (mat, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is Duration) {
					Duration d = (Duration) v;
					value.k = Kind.DURATION;
					value.u.p = Helper.AllocHGlobal (sizeof (Duration));
					Marshal.StructureToPtr (d, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is KeyTime) {
					KeyTime k = (KeyTime) v;
					value.k = Kind.KEYTIME;
					value.u.p = Helper.AllocHGlobal (sizeof (KeyTime));
					Marshal.StructureToPtr (k, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is RepeatBehavior) {
					RepeatBehavior d = (RepeatBehavior) v;
					value.k = Kind.REPEATBEHAVIOR;
					value.u.p = Helper.AllocHGlobal (sizeof (RepeatBehavior));
					Marshal.StructureToPtr (d, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is FontFamily) {
					FontFamily family = (FontFamily) v;
					
					value.k = Kind.STRING;
					
					byte[] bytes = System.Text.Encoding.UTF8.GetBytes (family.Source);
					IntPtr result = Helper.AllocHGlobal (bytes.Length + 1);
					Marshal.Copy (bytes, 0, result, bytes.Length);
					Marshal.WriteByte (result, bytes.Length, 0);
					
					value.u.p = result;
				}
				else if (v is Uri) {
					Uri uri = (Uri) v;
					
					value.k = Kind.STRING;
					
					byte[] bytes = System.Text.Encoding.UTF8.GetBytes (uri.OriginalString);
					IntPtr result = Helper.AllocHGlobal (bytes.Length + 1);
					Marshal.Copy (bytes, 0, result, bytes.Length);
					Marshal.WriteByte (result, bytes.Length, 0);
					
					value.u.p = result;
				}
				else if (v is XmlLanguage) {
					XmlLanguage lang = (XmlLanguage) v;
					
					value.k = Kind.STRING;
					
					byte[] bytes = System.Text.Encoding.UTF8.GetBytes (lang.IetfLanguageTag);
					IntPtr result = Helper.AllocHGlobal (bytes.Length + 1);
					Marshal.Copy (bytes, 0, result, bytes.Length);
					Marshal.WriteByte (result, bytes.Length, 0);
					
					value.u.p = result;
				}
				else if (v is Cursor) {
					Cursor c = (Cursor) v;

					value.k = Kind.INT32;

					value.u.i32 = (int)c.cursor;
				}
				else if (v is GridLength) {
					GridLength gl = (GridLength) v;
					value.k = Kind.GRIDLENGTH;
					value.u.p = Helper.AllocHGlobal (sizeof (GridLength));
					Marshal.StructureToPtr (gl, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if ((v is FontStretch) || (v is FontStyle) || (v is FontWeight)) {
					value.k = Kind.INT32;
					value.u.i32 = v.GetHashCode (); // unit tested as such
				}
				else if (as_managed_object) {
					// TODO: We probably need to marshal types that can animate as the 
					// corresponding type (Point, Double, Color, etc).
					// TODO: We need to store the GCHandle somewhere so that we can free it,
					// or register a callback on the surface for the unmanaged code to call.
					GCHandle handle = GCHandle.Alloc (v);
					value.k = Kind.MANAGED;
					value.u.p = Helper.GCHandleToIntPtr (handle);
				}
				else {
					throw new Exception (
						String.Format ("Do not know how to encode {0} yet", v.GetType ()));
				}
			}
			return value;
		}


	}
}
