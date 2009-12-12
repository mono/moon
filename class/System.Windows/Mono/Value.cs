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
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Media3D;
using System.Windows.Documents;
using System.Windows.Media.Animation;
using System.Runtime.InteropServices;
using System.Reflection;

namespace Mono {

	internal struct UnmanagedFontFamily {
		public IntPtr source;
	}

	internal struct UnmanagedFontWeight {
		public FontWeightKind weight;
	}

	internal struct UnmanagedFontStyle {
		public FontStyleKind style;
	}

	internal struct UnmanagedFontStretch {
		public FontStretchKind stretch;
	}

	internal struct UnmanagedFontSource {
		public IntPtr stream;
	}

	internal struct UnmanagedStreamCallbacks {
		public IntPtr handle;
		public IntPtr CanSeek;
		public IntPtr CanRead;
		public IntPtr Length;
		public IntPtr Position;
		public IntPtr Read;
		public IntPtr Write;
		public IntPtr Seek;
		public IntPtr Close;
	}

	[StructLayout(LayoutKind.Sequential)]
	internal struct UnmanagedPropertyPath {
		public IntPtr pathString;
		public IntPtr expandedPathString;
		public IntPtr property;
	}

	internal struct UnmanagedColor {
		public double r;
		public double g;
		public double b;
		public double a;
		
		public Color ToColor ()
		{
			return Color.FromArgb ((byte)(255 * a), (byte)(255 * r), (byte)(255 * g), (byte)(255 * b));
		}
	}

	internal struct UnmanagedUri {
		public bool isAbsolute;
		public IntPtr scheme;
		public IntPtr user;
		public IntPtr auth;
		public IntPtr passwd;
		public IntPtr host;
		public int port;
		public IntPtr path;
		public IntPtr _params;
		public IntPtr query;
		public IntPtr fragment;
		public IntPtr originalString;

		public unsafe static IntPtr FromUri (Uri managed_uri)
		{
			IntPtr uri = Marshal.AllocHGlobal (sizeof (UnmanagedUri));
	
			UnmanagedUri *uuri = (UnmanagedUri*)uri;
			uuri->scheme = IntPtr.Zero;
			uuri->user = IntPtr.Zero;
			uuri->auth = IntPtr.Zero;
			uuri->passwd = IntPtr.Zero;
			uuri->host = IntPtr.Zero;
			uuri->path = IntPtr.Zero;
			uuri->_params = IntPtr.Zero;
			uuri->query = IntPtr.Zero;
			uuri->fragment = IntPtr.Zero;
			uuri->originalString = IntPtr.Zero;
	
			NativeMethods.uri_parse (uri, managed_uri.OriginalString, false);
	
			uuri->isAbsolute = managed_uri.IsAbsoluteUri;

			return uri;
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	internal struct ManagedTypeInfo {
		public IntPtr assembly_name;
		public IntPtr full_name;
	}
	
	[StructLayout(LayoutKind.Explicit)]
	internal struct ValUnion {
		[FieldOffset(0)] public float f;
		[FieldOffset(0)] public double d;
		[FieldOffset(0)] public long i64;
		[FieldOffset(0)] public ulong ui64;
		[FieldOffset(0)] public int i32;
		[FieldOffset(0)] public uint ui32;
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

		public static unsafe object ToObject (Type type, Value* value)
		{
			if (value == null || value->IsNull) {
				return null;
			}
			switch (value->k) {
			case Kind.INVALID:
				return null;
					
			case Kind.DEPENDENCYPROPERTY:
				return DependencyProperty.Lookup (value->u.p);
				
			case Kind.BOOL:
				return value->u.i32 != 0;

			case Kind.DOUBLE:
				return value->u.d;
					
			case Kind.FLOAT:
				return value->u.f;
					
			case Kind.UINT64:
				return value->u.ui64;
					
			case Kind.INT64:
				return value->u.i64;
					
			case Kind.TIMESPAN:
				return new TimeSpan (value->u.i64);
						
			case Kind.INT32:
				// marshall back to the .NET type that we simply serialised as int for unmanaged usage
				int i32 = value->u.i32;
				if (type == typeof (System.Windows.Input.Cursor))
					return Cursors.FromEnum ((CursorType)i32);
				else if (type == typeof (TextDecorationCollection))
					return (i32 == (int) TextDecorationKind.Underline) ? TextDecorations.Underline : null;
				else if (type != null && type.IsEnum)
					return Enum.ToObject (type, i32);
				else
					return i32;

			case Kind.CHAR:
				return (char) value->u.ui32;

			case Kind.SURFACE:
				return NativeDependencyObjectHelper.FromIntPtr (value->u.p);

			case Kind.MANAGED:
				IntPtr managed_object = value->u.p;
				GCHandle handle = GCHandle.FromIntPtr (managed_object);
				return handle.Target;

			case Kind.STRING: {
				string str = Marshal.PtrToStringAuto (value->u.p);
				if (type == null)
					return str;
				
				// marshall back to the .NET type that we simply serialised as 'string' for unmanaged usage
				if (type == typeof (System.Windows.Markup.XmlLanguage))
					return XmlLanguage.GetLanguage (str);
				else
					return str;
			}

			case Kind.URI: {
				UnmanagedUri *uri = (UnmanagedUri*)value->u.p;
				return uri->originalString == IntPtr.Zero
					? new Uri("", UriKind.Relative)
					: new Uri (Marshal.PtrToStringAuto (uri->originalString),
						   uri->isAbsolute ? UriKind.Absolute : UriKind.Relative);
			}

			case Kind.XMLLANGUAGE: {
				string str = Marshal.PtrToStringAuto (value->u.p);
				return XmlLanguage.GetLanguage (str);
			}

			case Kind.FONTFAMILY: {
				UnmanagedFontFamily *family = (UnmanagedFontFamily*)value->u.p;
				return new FontFamily (family == null ? null : Marshal.PtrToStringAuto (family->source));
			}

			case Kind.FONTSTRETCH: {
				UnmanagedFontStretch *stretch = (UnmanagedFontStretch*)value->u.p;
				return new FontStretch (stretch == null ? FontStretchKind.Normal : stretch->stretch);
			}

			case Kind.FONTSTYLE: {
				UnmanagedFontStyle *style = (UnmanagedFontStyle*)value->u.p;
				return new FontStyle (style == null ? FontStyleKind.Normal : style->style);
			}

			case Kind.FONTWEIGHT: {
				UnmanagedFontWeight *weight = (UnmanagedFontWeight*)value->u.p;
				return new FontWeight (weight == null ? FontWeightKind.Normal : weight->weight);
			}

			case Kind.FONTSOURCE: {
				UnmanagedFontSource *source = (UnmanagedFontSource *) value->u.p;
				ManagedStreamCallbacks callbacks;
				StreamWrapper wrapper;
					
				callbacks = (ManagedStreamCallbacks) Marshal.PtrToStructure (source->stream, typeof (ManagedStreamCallbacks));
					
				wrapper = (StreamWrapper) GCHandle.FromIntPtr (callbacks.handle).Target;
					
				return new FontSource (wrapper.stream);
			}

			case Kind.PROPERTYPATH: {
				UnmanagedPropertyPath *propertypath = (UnmanagedPropertyPath *) value->u.p;
				if (propertypath == null)
					return new PropertyPath (null);
				if (propertypath->property != IntPtr.Zero)
					return null;
				return new PropertyPath (Marshal.PtrToStringAuto (propertypath->pathString));
			}

			case Kind.POINT: {
				Point *point = (Point*)value->u.p;
				return (point == null) ? new Point (0,0) : *point;
			}
				
			case Kind.RECT: {
				Rect *rect = (Rect*)value->u.p;
				return (rect == null) ? new Rect (0,0,0,0) : *rect;
			}

			case Kind.SIZE: {
				Size *size = (Size*)value->u.p;
				return (size == null) ? new Size (0,0) : *size;
			}

			case Kind.CORNERRADIUS: {
				CornerRadius *corner = (CornerRadius*)value->u.p;
				return (corner == null) ? new CornerRadius (0) : *corner;
			}

			case Kind.THICKNESS: {
				Thickness *thickness = (Thickness*)value->u.p;
				return (thickness == null) ? new Thickness (0) : *thickness;
			}
					
			case Kind.COLOR: {
				UnmanagedColor *color = (UnmanagedColor*)value->u.p;
				if (color == null)
					return new Color ();
				return color->ToColor ();
			}

			case Kind.MATRIX:
			case Kind.UNMANAGEDMATRIX: {
				return new Matrix (value->u.p);
			}

			case Kind.MATRIX3D:
			case Kind.UNMANAGEDMATRIX3D: {
				return new Matrix3D (value->u.p);
			}

			case Kind.DURATION: {
				Duration* duration = (Duration*)value->u.p;
				return (duration == null) ? Duration.Automatic : *duration;
			}
					
			case Kind.KEYTIME: {
				KeyTime* keytime = (KeyTime*)value->u.p;
				return (keytime == null) ? KeyTime.FromTimeSpan (TimeSpan.Zero) : *keytime;
			}

			case Kind.GRIDLENGTH: {
				GridLength* gridlength = (GridLength*)value->u.p;
				return (gridlength == null) ? new GridLength () : *gridlength;
			}
					
			case Kind.REPEATBEHAVIOR: {
				RepeatBehavior *repeat = (RepeatBehavior*)value->u.p;
				return (repeat == null) ? new RepeatBehavior () : *repeat;
			}

			case Kind.MEDIAATTRIBUTE_COLLECTION: {
				MediaAttributeCollection attrs = (MediaAttributeCollection) NativeDependencyObjectHelper.Lookup (value->k, value->u.p);
				return attrs.AsDictionary ();
			}

			case Kind.MANAGEDTYPEINFO: {
				ManagedTypeInfo *type_info = (ManagedTypeInfo *) value->u.p;

				if (type_info == null)
					return null;

				string assembly_name = Marshal.PtrToStringAuto (type_info->assembly_name);
				string full_name = Marshal.PtrToStringAuto (type_info->full_name);

				Assembly asm = Application.GetAssembly (assembly_name);
				if (asm != null)
					return asm.GetType (full_name);

				return null;
			}
			}

			if (!slow_codepath_error_shown){
				Report.Warning ("DependencyObject type testing now using a very slow code path");
				slow_codepath_error_shown = true;
			}

			if (NativeMethods.type_is_dependency_object (value->k)){
				// Old fast test: if (value->k > Kind.DEPENDENCY_OBJECT){

				if (value->u.p == IntPtr.Zero)
					return null;
					
				return NativeDependencyObjectHelper.Lookup (value->k, value->u.p);
			}

			throw new Exception (String.Format ("Do not know how to convert {0}  {1}", value->k, (int) value->k));
		}

		public static unsafe object ToObject (Type type, IntPtr value)
		{
			return ToObject (type, (Value *) value);
		}

		public static Value FromObject (object v)
		{
			return FromObject (v, false);
		}
		
		//
		// How do we support "null" values, should the caller take care of that?
		//
		public static Value FromObject (object v, bool box_value_types)
		{
			Value value = new Value ();
			
			unsafe {
				// get rid of this case right away.
				if (box_value_types && v.GetType().IsValueType) {
					//Console.WriteLine ("Boxing a value of type {0}:", v.GetType());

					GCHandle handle = GCHandle.Alloc (v);
					value.k = Kind.MANAGED;
					value.u.p = GCHandle.ToIntPtr (handle);
					return value;
				}

				if (v is IEasingFunction && !(v is EasingFunctionBase))
					v = new EasingFunctionWrapper (v as IEasingFunction);

				if (v is INativeDependencyObjectWrapper) {
					INativeDependencyObjectWrapper dov = (INativeDependencyObjectWrapper) v;

					if (dov.NativeHandle == IntPtr.Zero)
						throw new Exception (String.Format (
							"Object {0} has not set its native property", dov.GetType()));

					NativeMethods.event_object_ref (dov.NativeHandle);

					value.k = dov.GetKind ();
					value.u.p = dov.NativeHandle;

				} else if (v is DependencyProperty) {
					value.k = Kind.DEPENDENCYPROPERTY;
					value.u.p = ((DependencyProperty)v).Native;
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
				else if (v is float) {
					value.k = Kind.FLOAT;
					value.u.f = (float) v;
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
				else if (v is uint) {
					value.k = Kind.UINT32;
					value.u.ui32 = (uint) v;
				}
				else if (v is char) {
					value.k = Kind.CHAR;
					value.u.ui32 = (uint) (char) v;
				}
				else if (v is string) {
					value.k = Kind.STRING;

					value.u.p = StringToIntPtr ((string) v);
				}
				else if (v is Rect) {
					Rect rect = (Rect) v;
					value.k = Kind.RECT;
					value.u.p = Marshal.AllocHGlobal (sizeof (Rect));
					Marshal.StructureToPtr (rect, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is Size) {
					Size size = (Size) v;
					value.k = Kind.SIZE;
					value.u.p = Marshal.AllocHGlobal (sizeof (Size));
					Marshal.StructureToPtr (size, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is CornerRadius) {
					CornerRadius corner = (CornerRadius) v;
					value.k = Kind.CORNERRADIUS;
					value.u.p = Marshal.AllocHGlobal (sizeof (CornerRadius));
					Marshal.StructureToPtr (corner, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is Point) {
					Point pnt = (Point) v;
					value.k = Kind.POINT;
					value.u.p = Marshal.AllocHGlobal (sizeof (Point));
					Marshal.StructureToPtr (pnt, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is Thickness) {
					Thickness thickness = (Thickness)v;
					value.k = Kind.THICKNESS;
					value.u.p = Marshal.AllocHGlobal (sizeof (Thickness));
					Marshal.StructureToPtr (thickness, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is Color) {
					Color c = (Color) v;
					value.k = Kind.COLOR;
					value.u.p = Marshal.AllocHGlobal (sizeof (UnmanagedColor));
					UnmanagedColor* color = (UnmanagedColor*) value.u.p;
					color->r = c.R / 255.0f;
					color->g = c.G / 255.0f;
					color->b = c.B / 255.0f;
					color->a = c.A / 255.0f;
				}
				else if (v is Matrix) {
					// hack around the fact that managed Matrix is a struct while unmanaged Matrix is a DO
					// i.e. the unmanaged and managed structure layouts ARE NOT equal
					return FromObject (new UnmanagedMatrix ((Matrix) v), box_value_types);
				}
				else if (v is Duration) {
					Duration d = (Duration) v;
					value.k = Kind.DURATION;
					value.u.p = Marshal.AllocHGlobal (sizeof (Duration));
					Marshal.StructureToPtr (d, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is KeyTime) {
					KeyTime k = (KeyTime) v;
					value.k = Kind.KEYTIME;
					value.u.p = Marshal.AllocHGlobal (sizeof (KeyTime));
					Marshal.StructureToPtr (k, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is RepeatBehavior) {
					RepeatBehavior d = (RepeatBehavior) v;
					value.k = Kind.REPEATBEHAVIOR;
					value.u.p = Marshal.AllocHGlobal (sizeof (RepeatBehavior));
					Marshal.StructureToPtr (d, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is FontFamily) {
					FontFamily family = (FontFamily) v;
					value.k = Kind.FONTFAMILY;
					value.u.p = Marshal.AllocHGlobal (sizeof (UnmanagedFontFamily));
					Marshal.StructureToPtr (family, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}

				else if (v is FontSource) {
					FontSource source = (FontSource) v;
					
					value.k = Kind.FONTSOURCE;
					
					if (source.wrapper != null) {
						value.u.p = Marshal.AllocHGlobal (sizeof (UnmanagedFontSource));
						UnmanagedFontSource *ufs = (UnmanagedFontSource *) value.u.p;
						ManagedStreamCallbacks callbacks = source.wrapper.GetCallbacks ();
						ufs->stream = Marshal.AllocHGlobal (sizeof (UnmanagedStreamCallbacks));
						Marshal.StructureToPtr (callbacks, ufs->stream, false);
					} else {
						value.IsNull = true;
					}
				}

				else if (v is PropertyPath) {
					PropertyPath propertypath = (PropertyPath) v;
					value.k = Kind.PROPERTYPATH;
					value.u.p = Marshal.AllocHGlobal (sizeof (UnmanagedPropertyPath));

					UnmanagedPropertyPath *upp = (UnmanagedPropertyPath *) value.u.p;
					upp->property = propertypath.NativeDP;
					if (upp->property == IntPtr.Zero)
						upp->pathString = StringToIntPtr (propertypath.Path);
					else
						upp->pathString = IntPtr.Zero;
					upp->expandedPathString = IntPtr.Zero;
				}
				else if (v is Uri) {
					Uri uri = (Uri) v;

					value.k = Kind.URI;
					value.u.p = UnmanagedUri.FromUri (uri);
				}
				else if (v is XmlLanguage) {
					XmlLanguage lang = (XmlLanguage) v;
					
					value.k = Kind.STRING;
					
					value.u.p = StringToIntPtr (lang.IetfLanguageTag);
				}
				else if (v is Cursor) {
					Cursor c = (Cursor) v;

					value.k = Kind.INT32;

					value.u.i32 = (int)c.cursor;
				}
				else if (v is GridLength) {
					GridLength gl = (GridLength) v;
					value.k = Kind.GRIDLENGTH;
					value.u.p = Marshal.AllocHGlobal (sizeof (GridLength));
					Marshal.StructureToPtr (gl, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is FontStretch) {
					FontStretch stretch = (FontStretch) v;
					value.k = Kind.FONTSTRETCH;
					value.u.p = Marshal.AllocHGlobal (sizeof (UnmanagedFontStretch));
					Marshal.StructureToPtr (stretch, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is FontStyle) {
					FontStyle style = (FontStyle) v;
					value.k = Kind.FONTSTYLE;
					value.u.p = Marshal.AllocHGlobal (sizeof (UnmanagedFontStyle));
					Marshal.StructureToPtr (style, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is FontWeight) {
					FontWeight weight = (FontWeight) v;
					value.k = Kind.FONTWEIGHT;
					value.u.p = Marshal.AllocHGlobal (sizeof (UnmanagedFontWeight));
					Marshal.StructureToPtr (weight, value.u.p, false); // Unmanaged and managed structure layout is equal.
				}
				else if (v is PixelFormat) {
					// FIXME we need to make an unmanaged PixelFormat struct and
					// marshal it like the FontStretch/Style/Weight structs above.
					value.k = Kind.INT32;
					value.u.i32 = v.GetHashCode ();
				}
				else if (v is TextDecorationCollection) {
					value.k = Kind.INT32;
					value.u.i32 = (int) (v as TextDecorationCollection).Kind;
				}
				else if (v is Type) {
					Type t = v as Type;
					ManagedTypeInfo mti = new ManagedTypeInfo ();

					mti.assembly_name = StringToIntPtr (t.Assembly.GetName ().Name);
					mti.full_name = StringToIntPtr (t.FullName);

					value.k = Kind.MANAGEDTYPEINFO;
					value.u.p = Marshal.AllocHGlobal (sizeof (ManagedTypeInfo));
					Marshal.StructureToPtr (mti, value.u.p, false);
				}
				else {
					//Console.WriteLine ("Do not know how to encode {0} yet, boxing it", v.GetType ());

					// TODO: We probably need to marshal types that can animate as the 
					// corresponding type (Point, Double, Color, etc).
					// TODO: We need to store the GCHandle somewhere so that we can free it,
					// or register a callback on the surface for the unmanaged code to call.
					GCHandle handle = GCHandle.Alloc (v);
					value.k = Kind.MANAGED;
					value.u.p = GCHandle.ToIntPtr (handle);
				}
			}
			return value;
		}

		public static IntPtr StringToIntPtr (string str)
		{
			if (str == null)
				return IntPtr.Zero;

			byte [] bytes = System.Text.Encoding.UTF8.GetBytes (str);
			IntPtr result = Marshal.AllocHGlobal (bytes.Length + 1);
			Marshal.Copy (bytes, 0, result, bytes.Length);
			Marshal.WriteByte (result, bytes.Length, 0);

			return result;
		}

	}
}
