//
// DependencyObject.cs
//
// Author:
//   Iain McCoy (iain@mccoy.id.au)
//   Moonlight Team (moonlight-list@lists.ximian.com)
//
// Copyright 2005 Iain McCoy
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

using Mono;
using System.Collections.Generic;
using System.Security;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Runtime.InteropServices;
using System.Windows.Documents;
using System.Windows.Threading;
using System.Threading;

namespace System.Windows {
	public abstract class DependencyObject {
		static Thread moonlight_thread;
		static Dictionary<IntPtr, DependencyObject> objects = new Dictionary<IntPtr, DependencyObject> ();
		internal IntPtr _native;

		internal EventHandlerList events;

		private GCHandle _handle;
		internal GCHandle GCHandle {
			get {
				if (!_handle.IsAllocated)
					_handle = GCHandle.Alloc (this);
				return _handle;
			}
		}

		internal IntPtr native {
			get {
				return _native;
			}

			set {
				_native = value;
				if (objects.ContainsKey (value))
					return;
				objects [value] = this;
			}
		}

		public static readonly DependencyProperty NameProperty = 
			   DependencyProperty.Lookup (Kind.DEPENDENCY_OBJECT, "Name", typeof (string));

		public string Name {
			get { return (string) GetValue(NameProperty); }
			set { SetValue (NameProperty, value); }
		}

		static DependencyObject ()
		{
			NativeMethods.runtime_init (0);
			moonlight_thread = Thread.CurrentThread;
			Helper.Agclr = typeof (DependencyObject).Assembly;
		}

		internal DependencyObject ()
		{
			events = new EventHandlerList ();
		}

		internal DependencyObject (IntPtr raw)
		{
			native = raw;
			events = new EventHandlerList ();
		}
		
		//
		// This is mostly copied from Gtk#'s Object.GetObject
		// we need to take into account in the future:
		//    WeakReferences
		//    ToggleReferences (talk to Mike)
		//
		// 
		internal static DependencyObject Lookup (Kind k, IntPtr ptr)
		{
			if (ptr == IntPtr.Zero)
				return null;

			DependencyObject reference;
			if (objects.TryGetValue (ptr, out reference))
				return reference;

			DependencyObject dop = (DependencyObject) CreateObject (k, ptr);
			if (dop == null){
				Report.Warning ("System.Windows: Returning a null object, did not know how to construct {0}", k);
				Report.Warning (Helper.GetStackTrace ());
			}

			return dop;
		}

		//
		// This version only looks up the object, if it has not been exposed,
		// we return null
		//
		internal static DependencyObject Lookup (IntPtr ptr)
		{
			if (ptr == IntPtr.Zero)
				return null;

			DependencyObject obj;
			if (!objects.TryGetValue (ptr, out obj))
				return null;

			return obj;
		}
		
		static object CreateObject (Kind k, IntPtr raw)
		{
			NativeMethods.base_ref (raw);
			switch (k){
			case Kind.ARCSEGMENT: return new ArcSegment (raw);
			case Kind.ASSEMBLYPART: return new AssemblyPart (raw);
			case Kind.ASSEMBLYPART_COLLECTION: return new AssemblyPartCollection (raw);
			case Kind.BEGINSTORYBOARD: return new BeginStoryboard (raw);
			case Kind.BEZIERSEGMENT: return new BezierSegment (raw);
			case Kind.CANVAS: return new Canvas (raw);
			case Kind.COLORANIMATION: return new ColorAnimation (raw);
			case Kind.COLORKEYFRAME_COLLECTION: return new ColorKeyFrameCollection (raw);
			case Kind.DEPLOYMENT: return new Deployment (raw);
			case Kind.DISCRETECOLORKEYFRAME: return new DiscreteColorKeyFrame (raw);
			case Kind.DISCRETEDOUBLEKEYFRAME: return new DiscreteDoubleKeyFrame (raw);
			case Kind.DISCRETEPOINTKEYFRAME: return new DiscretePointKeyFrame (raw);
			case Kind.DOUBLEANIMATION: return new DoubleAnimation (raw);
			case Kind.DOUBLEANIMATIONUSINGKEYFRAMES: return new DoubleAnimationUsingKeyFrames (raw);
			case Kind.DOUBLEKEYFRAME_COLLECTION: return new DoubleKeyFrameCollection (raw);
			case Kind.ELLIPSEGEOMETRY: return new EllipseGeometry (raw);
			case Kind.ELLIPSE: return new Ellipse (raw);
			case Kind.EVENTTRIGGER: return new EventTrigger (raw);
			case Kind.GEOMETRY_COLLECTION: return new GeometryCollection (raw);
			case Kind.GEOMETRYGROUP: return new GeometryGroup (raw);
			case Kind.GLYPHS: return new Glyphs (raw);
			case Kind.GRADIENTSTOP_COLLECTION: return new GradientStopCollection (raw);
			case Kind.GRADIENTSTOP: return new GradientStop (raw);
			case Kind.GRID : return new Grid (raw);
			case Kind.IMAGEBRUSH: return new ImageBrush (raw);
			case Kind.IMAGE: return new Image (raw);
			case Kind.INLINES: return new InlineCollection (raw);
			case Kind.INKPRESENTER: return new InkPresenter (raw);
			case Kind.KEYSPLINE: return new KeySpline(raw);
			case Kind.LINEARGRADIENTBRUSH: return new LinearGradientBrush (raw);
			case Kind.LINEBREAK: return new LineBreak (raw);
			case Kind.LINEGEOMETRY: return new LineGeometry (raw);
			case Kind.LINE: return new Line (raw);
			case Kind.LINEARCOLORKEYFRAME: return new LinearColorKeyFrame (raw);
			case Kind.LINEARDOUBLEKEYFRAME: return new LinearDoubleKeyFrame (raw);
			case Kind.LINEARPOINTKEYFRAME: return new LinearPointKeyFrame (raw);
			case Kind.LINESEGMENT: return new LineSegment (raw);
			case Kind.MATRIXTRANSFORM: return new MatrixTransform (raw);
			case Kind.MEDIAELEMENT: return new MediaElement (raw);
			case Kind.PATHFIGURE_COLLECTION: return new PathFigureCollection (raw);
			case Kind.PATHFIGURE: return new PathFigure (raw);
			case Kind.PATHGEOMETRY: return new PathGeometry (raw);
			case Kind.PATH: return new Path (raw);
			case Kind.PATHSEGMENT_COLLECTION: return new PathSegmentCollection (raw);
			case Kind.POINTANIMATION: return new PointAnimation (raw);
			case Kind.POINTKEYFRAME_COLLECTION: return new PointKeyFrameCollection (raw);
			case Kind.POLYBEZIERSEGMENT: return new PolyBezierSegment (raw);
			case Kind.POLYGON: return new Polygon (raw);
			case Kind.POLYLINE: return new Polyline (raw);
			case Kind.POLYLINESEGMENT: return new PolyLineSegment (raw);
			case Kind.POLYQUADRATICBEZIERSEGMENT: return new PolyQuadraticBezierSegment (raw);
			case Kind.QUADRATICBEZIERSEGMENT: return new QuadraticBezierSegment (raw);
			case Kind.RADIALGRADIENTBRUSH: return new RadialGradientBrush (raw);
			case Kind.RECTANGLEGEOMETRY: return new RectangleGeometry (raw);
			case Kind.RECTANGLE: return new Rectangle (raw);
			case Kind.RESOURCE_DICTIONARY: return new ResourceDictionary (raw);
			case Kind.ROTATETRANSFORM: return new RotateTransform (raw);
			case Kind.RUN: return new Run (raw);
			case Kind.SCALETRANSFORM: return new ScaleTransform (raw);
			case Kind.SOLIDCOLORBRUSH: return new SolidColorBrush (raw);
			case Kind.SPLINECOLORKEYFRAME: return new SplineColorKeyFrame (raw);
			case Kind.SPLINEDOUBLEKEYFRAME: return new SplineDoubleKeyFrame (raw);
			case Kind.SPLINEPOINTKEYFRAME: return new SplinePointKeyFrame (raw);
			case Kind.STORYBOARD: return new Storyboard (raw);
			case Kind.STROKE_COLLECTION: return new StrokeCollection (raw);
			case Kind.STYLUSINFO: return new StylusInfo (raw);
			case Kind.STYLUSPOINT_COLLECTION: return new StylusPointCollection (raw);
			case Kind.STYLUSPOINT: return new StylusPoint (raw);
			case Kind.TEXTBLOCK: return new TextBlock (raw);
			case Kind.TIMELINE_COLLECTION: return new TimelineCollection (raw);
			case Kind.TIMELINEMARKER_COLLECTION: return new TimelineMarkerCollection (raw);
			case Kind.TRANSFORM_COLLECTION: return new TransformCollection (raw);
			case Kind.TRANSFORMGROUP: return new TransformGroup (raw);
			case Kind.TRANSLATETRANSFORM: return new TranslateTransform (raw);
			case Kind.TRIGGERACTION_COLLECTION: return new TriggerActionCollection (raw);
			case Kind.TRIGGER_COLLECTION: return new TriggerCollection (raw);
			case Kind.UIELEMENT_COLLECTION: return new UIElementCollection (raw);
							
			case Kind.CLOCKGROUP:
			case Kind.ANIMATIONCLOCK:
			case Kind.CLOCK: 
			case Kind.NAMESCOPE: 
			case Kind.TRIGGERACTION:
			case Kind.KEYFRAME_COLLECTION:
				throw new Exception (
					string.Format ("There is no managed equivalent of a {0} class.", k));
			case Kind.UIELEMENT:
			case Kind.PANEL:
			case Kind.TIMELINE: 
			case Kind.FRAMEWORKELEMENT:
			case Kind.BRUSH:
			case Kind.TILEBRUSH:
			case Kind.GENERALTRANSFORM:
			case Kind.TRANSFORM:
			case Kind.SHAPE:
			case Kind.GEOMETRY:
			case Kind.MEDIAATTRIBUTE_COLLECTION: 
				throw new Exception (
					String.Format ("Should never get an abstract class from unmanaged code {0}", k));

			default:
				throw new Exception (
					String.Format ("Kind missing from switch: {0}", k ));
			}
		}

		internal void Free ()
		{
			if (this.native != IntPtr.Zero) {
				NativeMethods.base_unref (this.native);
				this.native = IntPtr.Zero;
			}
		}
		
		~DependencyObject ()
		{
			Free ();
			if (_handle.IsAllocated) {
				_handle.Free();
			}
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public virtual object GetValue (DependencyProperty property)
		{
			object result = null;
			
			if (property == null)
				throw new ArgumentNullException ("property");
			
			CheckNativeAndThread ();
			
			IntPtr val = NativeMethods.dependency_object_get_value (native, Types.TypeToKind (GetType ()), property.Native);
			if (val != IntPtr.Zero)
				result = ValueToObject (property.PropertyType, val);
			
			if (result == null && property.PropertyType.IsValueType)
				result = Activator.CreateInstance (property.PropertyType);
			
			return result;
		}

#if NET_2_1
		[SecuritySafeCritical]
#endif
		public object GetAnimationBaseValue (DependencyProperty property)
		{
			throw new System.NotImplementedException ();
		}
		
#if NET_2_1
		[SecuritySafeCritical]
#endif
		public object ReadLocalValue (DependencyProperty property)
		{
			throw new System.NotImplementedException ();
		}
		
		
#if NET_2_1
		[SecuritySafeCritical]
#endif
		public void ClearValue (DependencyProperty property)
		{
			throw new System.NotImplementedException ();
		}
		
		[System.ComponentModel.EditorBrowsable (System.ComponentModel.EditorBrowsableState.Advanced)]
		public Dispatcher Dispatcher {
			get { throw new System.NotImplementedException (); }
		}
		
		static bool slow_codepath_error_shown = false;
		
		internal static object ValueToObject (Type type, IntPtr value)
		{
			if (value == IntPtr.Zero)
				return null;
			
			unsafe {
				Value *val = (Value *) value;
				
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
					if (type == typeof (System.Windows.Input.Cursor))
						return new Cursor ((CursorType)val->u.i32);
					else
						return val->u.i32;

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
						return new Uri (str);
					else
						return str;
				}
				
				case Kind.POINT: {
					UnmanagedPoint *point = (UnmanagedPoint*)val->u.p;
					return new Point (point->x, point->y);
				}
				
				case Kind.RECT: {
					UnmanagedRect *rect = (UnmanagedRect*)val->u.p;
					return new Rect (rect->left, rect->top, rect->width, rect->height);
				}

				case Kind.SIZE: {
					UnmanagedSize *size = (UnmanagedSize*)val->u.p;
					return new Size (size->width, size->height);
				}

				case Kind.THICKNESS: {
					UnmanagedThickness *thickness = (UnmanagedThickness*)val->u.p;
					return new Thickness (thickness->left, thickness->top, thickness->right, thickness->bottom);
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
					UnmanagedDuration* duration = (UnmanagedDuration*)val->u.p;
					if (duration == null)
						return Duration.Automatic;

					return new Duration (duration->kind, new TimeSpan (duration->timespan));
				}
					
				case Kind.KEYTIME: {
					UnmanagedKeyTime* keytime = (UnmanagedKeyTime*)val->u.p;
					if (keytime == null)
						return KeyTime.FromTimeSpan (TimeSpan.Zero);
					return new KeyTime ((KeyTimeType) keytime->kind, keytime->percent, new TimeSpan (keytime->timespan));
				}
					
				case Kind.REPEATBEHAVIOR: {
					UnmanagedRepeatBehavior *repeat = (UnmanagedRepeatBehavior*)val->u.p;
					if (repeat == null)
						return new RepeatBehavior ();

					return new RepeatBehavior (repeat->kind, repeat->count, new TimeSpan (repeat->timespan));
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

				throw new Exception (String.Format ("Do not know how to convert {0}", val->k));
			}
		}

		internal static Value GetAsValue (object v)
		{
			return GetAsValue (v, false);
		}
		
		//
		// How do we support "null" values, should the caller take care of that?
		//
		internal static Value GetAsValue (object v, bool as_managed_object)
		{
			Value value = new Value ();
			
			if (as_managed_object) {
				// TODO: We probably need to marshal types that can animate as the 
				// corresponding type (Point, Double, Color, etc).
				// TODO: We need to store the GCHandle somewhere so that we can free it,
				// or register a callback on the surface for the unmanaged code to call.
				GCHandle handle = GCHandle.Alloc (v);
				value.k = Kind.MANAGED;
				value.u.p = Helper.GCHandleToIntPtr (handle);
				return value;
			}
			
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
					IntPtr dov_native = dov.native;
					objects [dov_native] = dov;
					value.k = dov.GetKind ();
					value.u.p = dov_native;
					NativeMethods.base_ref (dov_native);
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
					value.u.p = Helper.AllocHGlobal (sizeof (UnmanagedDuration));
					UnmanagedDuration* duration = (UnmanagedDuration*) value.u.p;
					duration->kind = d.KindInternal;
					duration->timespan = d.TimeSpanInternal.Ticks;
				}
				else if (v is KeyTime) {
					KeyTime k = (KeyTime) v;
					value.k = Kind.KEYTIME;
					value.u.p = Helper.AllocHGlobal (sizeof (UnmanagedKeyTime));
					UnmanagedKeyTime* keytime = (UnmanagedKeyTime*) value.u.p;
					keytime->kind = (int) k.type;
					keytime->percent = k.percent;
					keytime->timespan = k.time_span.Ticks;
				}
				else if (v is RepeatBehavior) {
					RepeatBehavior d = (RepeatBehavior) v;
					value.k = Kind.REPEATBEHAVIOR;
					value.u.p = Helper.AllocHGlobal (sizeof (UnmanagedRepeatBehavior));
					UnmanagedRepeatBehavior* rep = (UnmanagedRepeatBehavior*) value.u.p;
					rep->kind = d.kind;
					rep->count = d.count;
					rep->timespan = d.duration.Ticks;
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
				else {
					throw new Exception (
						String.Format ("Do not know how to encode {0} yet", v.GetType ()));
				}
			}
			return value;
		}

		public void SetValue (DependencyProperty property, object obj)
		{
			Type object_type;
			Value v;
			
			if (property == null)
				throw new ArgumentNullException ("property");

			CheckNativeAndThread ();
			
			if (property.DeclaringType != null) {
				if (property.DeclaringType != GetType () && !property.DeclaringType.IsAssignableFrom (GetType ()))
					throw new System.ArgumentException (string.Format ("A DependencyProperty registered on type {0} can't be used to set a value on an object of type {1}", property.DeclaringType.FullName, GetType ().FullName));
			}
			
			if (obj == null) {
				if (property.PropertyType.IsValueType)
					throw new System.ArgumentException (string.Format ("null is not a valid value for '{0}'.", property.Name));
				
				NativeMethods.dependency_object_set_value (native, property.Native, IntPtr.Zero);
				return;
			}

			object_type = obj.GetType ();
			if (!(object_type == property.PropertyType || property.PropertyType.IsAssignableFrom (object_type)))
				throw new ArgumentException (string.Format ("A DependencyProperty whose property type is {0} can't be set to value whose type is {1}", property.PropertyType.FullName, object_type.FullName));
			
			v = GetAsValue (obj, property is CustomDependencyProperty);
			try {
				NativeMethods.dependency_object_set_value (native, property.Native, ref v);
			} finally {
				NativeMethods.value_free_value (ref v);
			}

		}

		internal DependencyObject DepObjectFindName (string name)
		{
			Kind k;
			IntPtr o = NativeMethods.dependency_object_find_name (native, name, out k);
			if (o == IntPtr.Zero)
				return null;

			return Lookup (k, o);
		}

		internal virtual Kind GetKind ()
		{
			return Kind.DEPENDENCY_OBJECT;
		}

#if SL_2_1
		[SecuritySafeCritical ()]
#endif
		[System.ComponentModel.EditorBrowsable (System.ComponentModel.EditorBrowsableState.Never)]
		public bool CheckAccess ()
		{
			return Thread.CurrentThread == moonlight_thread;
		}
		
		private void CheckNativeAndThread ()
		{
			if (native == IntPtr.Zero) {
				throw new Exception (
					string.Format ("Uninitialized object: this object ({0}) has not set its native handle or overwritten SetValue", GetType ().FullName));
			}

			if (!CheckAccess ())
				throw new UnauthorizedAccessException ("Invalid access of Moonlight from an external thread");
		}

		internal static void Ping ()
		{
			// Here just to ensure that the static ctor is executed and
			// runtime init is initialized from some entry points
		}
	}
}
