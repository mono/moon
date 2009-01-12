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
using System.Windows.Data;
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
	public abstract partial class DependencyObject {
		static Thread moonlight_thread;
		static Dictionary<IntPtr, DependencyObject> objects = new Dictionary<IntPtr, DependencyObject> ();
		static Dictionary<IntPtr, object> managedObjects = new Dictionary<IntPtr, object> ();
		internal IntPtr _native;

		internal EventHandlerList events;

		[ThreadStatic] static private Dispatcher dispatcher;
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
		
		static DependencyObject ()
		{
			moonlight_thread = Thread.CurrentThread;
			Helper.Agclr = typeof (DependencyObject).Assembly;
		}

		protected DependencyObject ()
		{
			native = NativeMethods.dependency_object_new ();
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

		internal static DependencyObject FromIntPtr (IntPtr ptr)
		{
			if (ptr == IntPtr.Zero)
				return null;

			Kind k = NativeMethods.dependency_object_get_object_type (ptr);

			return DependencyObject.Lookup (k, ptr);
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
		
		internal static void TrackNativeReference (DependencyObject obj)
		{
			IntPtr obj_native = obj.native;
			objects [obj_native] = obj;
			NativeMethods.event_object_ref (obj_native);
		}

		static object CreateObject (Kind k, IntPtr raw)
		{
			NativeMethods.event_object_ref (raw);
			switch (k){
			case Kind.ARCSEGMENT: return new ArcSegment (raw);
			case Kind.ASSEMBLYPART: return new AssemblyPart (raw);
			case Kind.ASSEMBLYPART_COLLECTION: return new AssemblyPartCollection (raw);
			case Kind.BEGINSTORYBOARD: return new BeginStoryboard (raw);
			case Kind.BEZIERSEGMENT: return new BezierSegment (raw);
			case Kind.BORDER: return new Border (raw);
			case Kind.CANVAS: return new Canvas (raw);
			case Kind.COLORANIMATION: return new ColorAnimation (raw);
			case Kind.COLORKEYFRAME_COLLECTION: return new ColorKeyFrameCollection (raw);
			case Kind.COLUMNDEFINITION_COLLECTION: return new ColumnDefinitionCollection (raw);
			case Kind.CONTROLTEMPLATE: return new ControlTemplate (raw);
			case Kind.DATATEMPLATE: return new DataTemplate (raw);
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
			case Kind.INLINE_COLLECTION: return new InlineCollection (raw);
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
			case Kind.MULTISCALEIMAGE: return new MultiScaleImage (raw);
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
			case Kind.ROWDEFINITION_COLLECTION: return new RowDefinitionCollection (raw);
			case Kind.RUN: return new Run (raw);
			case Kind.SETTERBASE_COLLECTION: return new SetterBaseCollection (raw);
			case Kind.SETTER: return new Setter (raw);
			case Kind.SCALETRANSFORM: return new ScaleTransform (raw);
			case Kind.SOLIDCOLORBRUSH: return new SolidColorBrush (raw);
			case Kind.SPLINECOLORKEYFRAME: return new SplineColorKeyFrame (raw);
			case Kind.SPLINEDOUBLEKEYFRAME: return new SplineDoubleKeyFrame (raw);
			case Kind.SPLINEPOINTKEYFRAME: return new SplinePointKeyFrame (raw);
			case Kind.STORYBOARD: return new Storyboard (raw);
			case Kind.STROKE_COLLECTION: return new StrokeCollection (raw);
			case Kind.STYLE: return new Style (raw);
			case Kind.STYLUSPOINT_COLLECTION: return new StylusPointCollection (raw);
			case Kind.STYLUSPOINT: return new StylusPoint (raw);
			case Kind.TEXTBLOCK: return new TextBlock (raw);
			case Kind.TEXTBOX: return new TextBox (raw);
			case Kind.TIMELINE_COLLECTION: return new TimelineCollection (raw);
			case Kind.TIMELINEMARKER_COLLECTION: return new TimelineMarkerCollection (raw);
			case Kind.TRANSFORM_COLLECTION: return new TransformCollection (raw);
			case Kind.TRANSFORMGROUP: return new TransformGroup (raw);
			case Kind.TRANSLATETRANSFORM: return new TranslateTransform (raw);
			case Kind.TRIGGERACTION_COLLECTION: return new TriggerActionCollection (raw);
			case Kind.TRIGGER_COLLECTION: return new TriggerCollection (raw);
			case Kind.UIELEMENT_COLLECTION: return new UIElementCollection (raw);
			case Kind.USERCONTROL: return new UserControl (raw);
							
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
			case Kind.SETTERBASE:
			case Kind.FRAMEWORKTEMPLATE:
				throw new Exception (
					String.Format ("Should never get an abstract class from unmanaged code {0}", k));

			default:
				throw new Exception (
					String.Format ("DependencyObject::CreateObject(): Kind missing from switch: {0}", k));
			}
		}

		internal void Free ()
		{
			if (this.native != IntPtr.Zero) {
				NativeMethods.event_object_unref(this.native);
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

		public object GetValue (DependencyProperty dp)
		{
			object result = null;
			
			if (dp == null)
				throw new ArgumentNullException ("property");
			
			CheckNativeAndThread ();

			IntPtr val = NativeMethods.dependency_object_get_value (native, Types.TypeToKind (GetType ()), dp.Native);
			if (val != IntPtr.Zero)
				result = Value.ToObject (dp.PropertyType, val);
			
			if (result == null && dp.PropertyType.IsValueType)
				result = dp.DefaultValue;
			
			return result;
		}

		public object GetAnimationBaseValue (DependencyProperty dp)
		{
			throw new System.NotImplementedException ();
		}

		public object ReadLocalValue (DependencyProperty dp)
		{
			return ReadLocalValueImpl (dp);
		}

		internal virtual object ReadLocalValueImpl (DependencyProperty dp)
		{
			IntPtr val = NativeMethods.dependency_object_get_local_value (native, dp.Native);
			if (val == IntPtr.Zero) {
				return DependencyProperty.UnsetValue;
			} else {
				// We can get a style or bindingexpression or something else here
				// so the Value* will not always be of type 'DP.PropertyType'.
				return Value.ToObject (dp.PropertyType, val);
			}
		}
		
		public void ClearValue (DependencyProperty dp)
		{
			ClearValueImpl (dp);
		}

		internal virtual void ClearValueImpl (DependencyProperty dp)
		{
			NativeMethods.dependency_object_clear_value (native, dp.Native, true);
		}
		
		[System.ComponentModel.EditorBrowsable (System.ComponentModel.EditorBrowsableState.Advanced)]
		public Dispatcher Dispatcher {
			get {
				if (dispatcher == null)
					dispatcher = new Dispatcher ();

				return dispatcher;
			}
		}
		

		public void SetValue (DependencyProperty dp, object value)
		{
			if (dp == null)
				throw new ArgumentNullException ("property");
			if (dp.IsReadOnly)
				throw new InvalidOperationException ();
			SetValueImpl (dp, value);
		}

		internal virtual void SetValueImpl (DependencyProperty dp, object value)
		{
			Type object_type;
			Value v;
			
			if (dp == null)
				throw new ArgumentNullException ("property");

			CheckNativeAndThread ();
			
			if (dp.DeclaringType != null && !dp.IsAttached) {
				if (!dp.DeclaringType.IsAssignableFrom (GetType ()))
					throw new System.ArgumentException (string.Format ("The DependencyProperty '{2}', registered on type {0} can't be used to set a value on an object of type {1}", dp.DeclaringType.AssemblyQualifiedName, GetType ().AssemblyQualifiedName, dp.Name));
			}
			
			if (value == null) {
				if (dp.PropertyType.IsValueType && !dp.IsNullable)
					throw new System.ArgumentException (string.Format ("null is not a valid value for '{0}'.", dp.Name));

				Console.WriteLine ("Kind: {0}", NativeMethods.dependency_property_get_property_type(dp.Native));
				v = new Value { k = NativeMethods.dependency_property_get_property_type(dp.Native), IsNull = true };
				NativeMethods.dependency_object_set_value (native, dp.Native, ref v);
				return;
			}
			
			dp.Validate (this, dp, value);
			
			object_type = value.GetType ();
			if (!dp.PropertyType.IsAssignableFrom (object_type))
				throw new ArgumentException (string.Format ("The DependencyProperty '{2}', whose property type is {0} can't be set to value whose type is {1}", dp.PropertyType.FullName, object_type.FullName, dp.Name));
				                     
			v = Value.FromObject (value, (dp is CustomDependencyProperty) || (dp.PropertyType == typeof (object) || dp.PropertyType == typeof (Type)));
			try {
				NativeMethods.dependency_object_set_value (native, dp.Native, ref v);
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

		[System.ComponentModel.EditorBrowsable (System.ComponentModel.EditorBrowsableState.Never)]
		public bool CheckAccess ()
		{
			return Thread.CurrentThread == moonlight_thread;
		}
		
		private void CheckNativeAndThread ()
		{
			if (native == IntPtr.Zero) {
				throw new Exception (
					string.Format ("Uninitialized object: this object ({0}) has not set its native handle set", GetType ().FullName));
			}

			if (!CheckAccess ())
				throw new UnauthorizedAccessException ("Invalid access of Moonlight from an external thread");
		}

#if NET_2_1
		internal
#else
		public
#endif
		static void Initialize ()
		{
			// Here just to ensure that the static ctor is executed and
			// runtime init is initialized from some entry points
		}
	}
}
