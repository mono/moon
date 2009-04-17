//
// INativeDependencyObjectWrapper.cs
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
using Microsoft.Internal;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.Windows.Documents;
using System.Windows.Threading;
using System.Threading;
using System.Collections.Generic;
using System.Linq.Expressions;
namespace Mono {

	internal class NativeDependencyObjectHelper {
#region "helpers for the INativeDependencyObjectWrapper interface"
		public static object GetValue (INativeDependencyObjectWrapper wrapper, DependencyProperty dp)
		{
			object result = null;
			
			if (dp == null)
				throw new ArgumentNullException ("property");
			
			//			CheckNativeAndThread (wrapper);

			IntPtr val = NativeMethods.dependency_object_get_value (wrapper.NativeHandle, Deployment.Current.Types.TypeToKind (wrapper.GetType ()), dp.Native);
			if (val == IntPtr.Zero)
				return dp.DefaultValue;
			
			result = Value.ToObject (dp.PropertyType, val);
			
			if (result == null && dp.PropertyType.IsValueType && !dp.IsNullable)
				result = dp.DefaultValue;
			
			return result;
		}

		public static void SetValue (INativeDependencyObjectWrapper wrapper, DependencyProperty dp, object value)
		{
			Type object_type;
			Value v;
			
			if (dp == null)
				throw new ArgumentNullException ("property");

			CheckNativeAndThread (wrapper);
			
			if (dp.DeclaringType != null && !dp.IsAttached) {
				if (!dp.DeclaringType.IsAssignableFrom (wrapper.GetType ()))
					throw new System.ArgumentException (string.Format ("The DependencyProperty '{2}', registered on type {0} can't be used to set a value on an object of type {1}", dp.DeclaringType.AssemblyQualifiedName, wrapper.GetType ().AssemblyQualifiedName, dp.Name));
			}
			
			if (value == null) {
				if (dp.PropertyType.IsValueType && !dp.IsNullable)
					throw new System.ArgumentException (string.Format ("null is not a valid value for '{0}'.", dp.Name));

				v = new Value { k = NativeMethods.dependency_property_get_property_type(dp.Native), IsNull = true };
				NativeMethods.dependency_object_set_value (wrapper.NativeHandle, dp.Native, ref v);
				return;
			}

			// XXX we need this to work with all
			// INativeDependencyObjectWrapper, I expect..
			// but right now the only one other than DO is
			// Application, which doesn't have any
			// settable properties, so we're safe for now.
			if (wrapper is DependencyObject)
				dp.Validate ((DependencyObject)wrapper, dp, value);

			object_type = value.GetType ();

			bool type_valid = false;
			if (dp.PropertyType.IsGenericTypeDefinition) {
				Type t = object_type;
				while (t != null) {
					if (dp.PropertyType.IsAssignableFrom (t)) {
						type_valid = true;
						break;
					}

					if (t.IsGenericType && !t.IsGenericTypeDefinition)
						t = t.GetGenericTypeDefinition ();
					else
						t = t.BaseType;
				}
			}
			else {
				type_valid = dp.PropertyType.IsAssignableFrom (object_type);
			}

			if (!type_valid) {
				throw new ArgumentException (string.Format ("The DependencyProperty '{3}.{2}', whose property type is {0} can't be set to value whose type is {1}", dp.PropertyType.FullName, object_type.FullName, dp.Name, dp.DeclaringType.FullName));
			}
				                     
			v = Value.FromObject (value, dp.PropertyType == typeof(object) && dp.BoxValueTypes);
			try {
				NativeMethods.dependency_object_set_value (wrapper.NativeHandle, dp.Native, ref v);
			} finally {
				NativeMethods.value_free_value (ref v);
			}
		}

		public static object ReadLocalValue (INativeDependencyObjectWrapper wrapper, DependencyProperty dp)
		{
			IntPtr val = NativeMethods.dependency_object_read_local_value (wrapper.NativeHandle, dp.Native);
			if (val == IntPtr.Zero) {
				return DependencyProperty.UnsetValue;
			} else {
				// We can get a style or bindingexpression or something else here
				// so the Value* will not always be of type 'DP.PropertyType'.
				return Value.ToObject (dp.PropertyType, val);
			}
		}

		public static object GetAnimationBaseValue (INativeDependencyObjectWrapper wrapper, DependencyProperty dp)
		{
			throw new NotImplementedException ();
		}

		public static void ClearValue (INativeDependencyObjectWrapper wrapper, DependencyProperty dp)
		{
			NativeMethods.dependency_object_clear_value (wrapper.NativeHandle, dp.Native, true);
		}
#endregion


		internal static Dictionary<IntPtr, ToggleRef> objects = new Dictionary<IntPtr, ToggleRef> ();


		public static void AddNativeMapping (IntPtr native, INativeDependencyObjectWrapper wrapper)
		{
			if (native == IntPtr.Zero)
				return;

			if (objects.ContainsKey (native)) {
				// XXX shouldn't this be an error?
				return;
			}

			ToggleRef tref = new ToggleRef (wrapper);
			objects[native] = tref;
			tref.Initialize ();
		}

		//
		// This is mostly copied from Gtk#'s Object.GetObject
		// we need to take into account in the future:
		//    WeakReferences
		//    ToggleReferences (talk to Mike)
		//
		// 
		internal static INativeDependencyObjectWrapper Lookup (Kind k, IntPtr ptr)
		{
			if (ptr == IntPtr.Zero)
				return null;

			ToggleRef reference;
			if (objects.TryGetValue (ptr, out reference))
				return reference.Target;

			INativeDependencyObjectWrapper wrapper = (INativeDependencyObjectWrapper) CreateObject (k, ptr);
			if (wrapper == null){
				Report.Warning ("System.Windows: Returning a null object, did not know how to construct {0}", k);
				Report.Warning (Environment.StackTrace);
			}

			return wrapper;
		}

		internal static INativeDependencyObjectWrapper FromIntPtr (IntPtr ptr)
		{
			if (ptr == IntPtr.Zero)
				return null;

			Kind k = NativeMethods.event_object_get_object_type (ptr);

			return Lookup (k, ptr);
		}

		//
		// This version only looks up the object, if it has not been exposed,
		// we return null
		//
		internal static INativeDependencyObjectWrapper Lookup (IntPtr ptr)
		{
			if (ptr == IntPtr.Zero)
				return null;

			ToggleRef tref;
			if (objects.TryGetValue (ptr, out tref))
				return tref.Target;
			return null;
		}

		static object CreateObject (Kind k, IntPtr raw)
		{
			NativeMethods.event_object_ref (raw);
			switch (k){
			case Kind.ARCSEGMENT: return new ArcSegment (raw);
			case Kind.APPLICATION: return new Application (raw);
			case Kind.ASSEMBLYPART: return new AssemblyPart (raw);
			case Kind.ASSEMBLYPART_COLLECTION: return new AssemblyPartCollection (raw);
			case Kind.BACKEASE: return new BackEase (raw);
			case Kind.BEGINSTORYBOARD: return new BeginStoryboard (raw);
			case Kind.BEZIERSEGMENT: return new BezierSegment (raw);
				//			case Kind.BINDINGEXPRESSION: return new BindingExpression (raw);
			case Kind.BITMAPIMAGE: return new BitmapImage (raw);
			case Kind.BITMAPSOURCE: return new BitmapSource (raw);
			case Kind.BOUNCEEASE: return new BounceEase (raw);
			case Kind.BORDER: return new Border (raw);
			case Kind.CANVAS: return new Canvas (raw);
			case Kind.CIRCLEEASE: return new CircleEase (raw);
			case Kind.COLORANIMATION: return new ColorAnimation (raw);
			case Kind.COLORANIMATIONUSINGKEYFRAMES: return new ColorAnimationUsingKeyFrames (raw);
			case Kind.COLORKEYFRAME_COLLECTION: return new ColorKeyFrameCollection (raw);
			case Kind.COLUMNDEFINITION: return new ColumnDefinition (raw);
			case Kind.COLUMNDEFINITION_COLLECTION: return new ColumnDefinitionCollection (raw);
			case Kind.CONTENTCONTROL: return new ContentControl (raw);
			case Kind.CONTROLTEMPLATE: return new ControlTemplate (raw);
			case Kind.CUBICEASE: return new CubicEase (raw);
			case Kind.DATATEMPLATE: return new DataTemplate (raw);
			case Kind.DEEPZOOMIMAGETILESOURCE: return new DeepZoomImageTileSource (raw);
			case Kind.DEPLOYMENT: return new Deployment (raw);
			case Kind.DISCRETECOLORKEYFRAME: return new DiscreteColorKeyFrame (raw);
			case Kind.DISCRETEDOUBLEKEYFRAME: return new DiscreteDoubleKeyFrame (raw);
			case Kind.DISCRETEPOINTKEYFRAME: return new DiscretePointKeyFrame (raw);
			case Kind.DISCRETEOBJECTKEYFRAME: return new DiscreteObjectKeyFrame (raw);
			case Kind.DOUBLEANIMATION: return new DoubleAnimation (raw);
			case Kind.DOUBLEANIMATIONUSINGKEYFRAMES: return new DoubleAnimationUsingKeyFrames (raw);
			case Kind.DOUBLEKEYFRAME_COLLECTION: return new DoubleKeyFrameCollection (raw);
			case Kind.DOUBLE_COLLECTION: return new DoubleCollection (raw);
			case Kind.DRAWINGATTRIBUTES: return new DrawingAttributes (raw);
			case Kind.EASINGCOLORKEYFRAME: return new EasingColorKeyFrame (raw);
			case Kind.EASINGDOUBLEKEYFRAME: return new EasingDoubleKeyFrame (raw);
			case Kind.EASINGPOINTKEYFRAME: return new EasingPointKeyFrame (raw);
			case Kind.ELASTICEASE: return new ElasticEase (raw);
			case Kind.ELLIPSEGEOMETRY: return new EllipseGeometry (raw);
			case Kind.ELLIPSE: return new Ellipse (raw);
			case Kind.EVENTTRIGGER: return new EventTrigger (raw);
			case Kind.EXPONENTIALEASE: return new ExponentialEase (raw);
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
			case Kind.INPUTMETHOD: return new InputMethod (raw);
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
			case Kind.MEDIAATTRIBUTE: return new MediaAttribute (raw);
			case Kind.MEDIAELEMENT: return new MediaElement (raw);
			case Kind.MULTISCALEIMAGE: return new MultiScaleImage (raw);
			case Kind.MULTISCALESUBIMAGE: return new MultiScaleSubImage (raw);
			case Kind.MULTISCALESUBIMAGE_COLLECTION: return new MultiScaleSubImageCollection (raw);
			case Kind.OBJECTANIMATIONUSINGKEYFRAMES: return new ObjectAnimationUsingKeyFrames (raw);
			case Kind.PASSWORDBOX: return new PasswordBox (raw);
			case Kind.PATHFIGURE_COLLECTION: return new PathFigureCollection (raw);
			case Kind.PATHFIGURE: return new PathFigure (raw);
			case Kind.PATHGEOMETRY: return new PathGeometry (raw);
			case Kind.PATH: return new Path (raw);
			case Kind.PATHSEGMENT_COLLECTION: return new PathSegmentCollection (raw);
			case Kind.POINTANIMATION: return new PointAnimation (raw);
			case Kind.POINTANIMATIONUSINGKEYFRAMES: return new PointAnimationUsingKeyFrames (raw);
			case Kind.POINTKEYFRAME_COLLECTION: return new PointKeyFrameCollection (raw);
			case Kind.POINT_COLLECTION: return new PointCollection (raw);
			case Kind.POLYBEZIERSEGMENT: return new PolyBezierSegment (raw);
			case Kind.POLYGON: return new Polygon (raw);
			case Kind.POLYLINE: return new Polyline (raw);
			case Kind.POLYLINESEGMENT: return new PolyLineSegment (raw);
			case Kind.POLYQUADRATICBEZIERSEGMENT: return new PolyQuadraticBezierSegment (raw);
			case Kind.POPUP: return new Popup (raw);
			case Kind.POWEREASE: return new PowerEase (raw);
			case Kind.QUADRATICBEZIERSEGMENT: return new QuadraticBezierSegment (raw);
			case Kind.QUADRATICEASE: return new QuadraticEase (raw);
			case Kind.QUARTICEASE: return new QuarticEase (raw);
			case Kind.QUINTICEASE: return new QuinticEase (raw);
			case Kind.RADIALGRADIENTBRUSH: return new RadialGradientBrush (raw);
			case Kind.RECTANGLEGEOMETRY: return new RectangleGeometry (raw);
			case Kind.RECTANGLE: return new Rectangle (raw);
			case Kind.RESOURCE_DICTIONARY: return new ResourceDictionary (raw);
			case Kind.ROTATETRANSFORM: return new RotateTransform (raw);
			case Kind.ROWDEFINITION: return new RowDefinition (raw);
			case Kind.ROWDEFINITION_COLLECTION: return new RowDefinitionCollection (raw);
			case Kind.RUN: return new Run (raw);
			case Kind.SETTERBASE_COLLECTION: return new SetterBaseCollection (raw);
			case Kind.SETTER: return new Setter (raw);
			case Kind.SCALETRANSFORM: return new ScaleTransform (raw);
			case Kind.SINEEASE: return new SineEase (raw);
			case Kind.SOLIDCOLORBRUSH: return new SolidColorBrush (raw);
			case Kind.SPLINECOLORKEYFRAME: return new SplineColorKeyFrame (raw);
			case Kind.SPLINEDOUBLEKEYFRAME: return new SplineDoubleKeyFrame (raw);
			case Kind.SPLINEPOINTKEYFRAME: return new SplinePointKeyFrame (raw);
			case Kind.STORYBOARD: return new Storyboard (raw);
			case Kind.STROKE_COLLECTION: return new StrokeCollection (raw);
			case Kind.STYLE: return new Style (raw);
			case Kind.STYLUSINFO: return new StylusInfo (raw);
			case Kind.STYLUSPOINT_COLLECTION: return new StylusPointCollection (raw);
			case Kind.STYLUSPOINT: return new StylusPoint (raw);
			case Kind.TEXTBLOCK: return new TextBlock (raw);
			case Kind.TEXTBOX: return new TextBox (raw);
			case Kind.TEXTBOXVIEW: return new TextBoxView (raw);
			case Kind.TIMELINE_COLLECTION: return new TimelineCollection (raw);
			case Kind.TIMELINEMARKER_COLLECTION: return new TimelineMarkerCollection (raw);
			case Kind.TRANSFORM_COLLECTION: return new TransformCollection (raw);
			case Kind.TRANSFORMGROUP: return new TransformGroup (raw);
			case Kind.TRANSLATETRANSFORM: return new TranslateTransform (raw);
			case Kind.TRIGGERACTION_COLLECTION: return new TriggerActionCollection (raw);
			case Kind.TRIGGER_COLLECTION: return new TriggerCollection (raw);
			case Kind.UIELEMENT_COLLECTION: return new UIElementCollection (raw);
			case Kind.USERCONTROL: return new UserControl (raw);
			case Kind.VIDEOBRUSH: return new VideoBrush (raw);
			case Kind.WRITEABLEBITMAP: return new WriteableBitmap (raw);
				
			case Kind.CLOCKGROUP:
			case Kind.ANIMATIONCLOCK:
			case Kind.CLOCK: 
			case Kind.NAMESCOPE: 
			case Kind.TRIGGERACTION:
			case Kind.KEYFRAME_COLLECTION:
			case Kind.MEDIAATTRIBUTE_COLLECTION:
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
			case Kind.SETTERBASE:
			case Kind.FRAMEWORKTEMPLATE:
			case Kind.EASINGFUNCTIONBASE:
				throw new Exception (
					String.Format ("Should never get an abstract class from unmanaged code {0}", k));

			default:
				throw new Exception (
					String.Format ("NativeDependencyObjectHelper::CreateObject(): Kind missing from switch: {0}", k));
			}
		}

		private static void CheckNativeAndThread (INativeDependencyObjectWrapper wrapper)
		{
			if (wrapper.NativeHandle == IntPtr.Zero) {
				throw new Exception (
					string.Format ("Uninitialized object: this object ({0}) has not set its native handle set", wrapper.GetType ().FullName));
			}

			if (!wrapper.CheckAccess ())
				throw new UnauthorizedAccessException ("Invalid access of Moonlight from an external thread");
		}

	}

}
