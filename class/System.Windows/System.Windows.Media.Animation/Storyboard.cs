//
// Storyboard.cs
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
using System.Windows.Media;
using System.Runtime.InteropServices;
using Mono;

namespace System.Windows.Media.Animation {

	public sealed partial class Storyboard : Timeline {
		// FIXME For TargetName and TargetProperty
		// FIXME Exception if setting on running
		// This check needs to go in native co
		private static readonly DependencyProperty ChildrenProperty = DependencyProperty.Lookup (Kind.TIMELINEGROUP, "Children", typeof (TimelineCollection));

		static Storyboard ()
		{
			TargetPropertyProperty.Validate += delegate (DependencyObject target, DependencyProperty property, object value) {
				PropertyPath path = (PropertyPath) value;
				if (path.Path == null && path.NativeDP == IntPtr.Zero)
					throw new NullReferenceException ();
			};
		}
		
		public void Begin ()
		{
			NativeMethods.storyboard_begin (native);
		}

		public void Pause ()
		{
			NativeMethods.storyboard_pause (native);
		}

		public void Resume ()
		{
			NativeMethods.storyboard_resume (native);
		}

		public void Seek (TimeSpan timespan)
		{
			NativeMethods.storyboard_seek (native, timespan.Ticks);
		}

		public void SeekAlignedToLastTick (TimeSpan seekTime)
		{
			NativeMethods.storyboard_seek_aligned_to_last_tick (native, seekTime.Ticks);
		}
		
		public void SkipToFill ()
		{
			NativeMethods.storyboard_skip_to_fill (native);
		}

		public void Stop ()
		{
			NativeMethods.storyboard_stop (native);
		}

		public TimeSpan GetCurrentTime ()
		{
			return new TimeSpan (NativeMethods.storyboard_get_current_time (native));
		}
		
		public ClockState GetCurrentState ()
		{
			return (ClockState) NativeMethods.storyboard_get_current_state (native);
		}
		
		public TimelineCollection Children {
			get {
				return (TimelineCollection) GetValue (ChildrenProperty);
			}
		}

		public static void SetTarget (Timeline timeline, DependencyObject target)
		{
			if (timeline == null)
				throw new ArgumentNullException ("timeline");
			if (target == null)
				throw new ArgumentNullException ("target");
			// FIXME Exception if setting on running
			timeline.ManualTarget = target;
		}

		public static void SetTargetName (Timeline element, string name)
		{
			// NOTE: this throws a NRE if element is null, while name == null is a valid value

			// FIXME Exception if setting on running
			element.SetValue (TargetNameProperty, name);
		}

		public static void SetTargetProperty (Timeline element, PropertyPath path)
		{
			if (element == null)
				throw new ArgumentNullException ("element");
			if (path == null)
				throw new ArgumentNullException ("path");
			// FIXME Exception if setting on running
			element.SetValue (TargetPropertyProperty, path);
		}

		public static string GetTargetName (Timeline element)
		{
			return (string) element.GetValue (TargetNameProperty);
		}

		public static PropertyPath GetTargetProperty (Timeline element)
		{
			if (element == null)
				throw new ArgumentNullException ("element");

			return (PropertyPath) element.GetValue (TargetPropertyProperty);
		}

		// Hack for VSM.
		internal static DependencyProperty GetTargetDependencyProperty (Timeline element)
		{
			if (element == null)
				throw new ArgumentNullException ("element");
			IntPtr ptr = NativeMethods.storyboard_get_target_dependency_property (element.native);
			return ptr == IntPtr.Zero ? null : DependencyProperty.Lookup (ptr);
		}

	}
}
