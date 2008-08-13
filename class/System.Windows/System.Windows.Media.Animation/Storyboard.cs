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
using System.Security;
using System.Windows;
using System.Windows.Media;
using System.Runtime.InteropServices;
using Mono;

namespace System.Windows.Media.Animation {

	public sealed partial class Storyboard : Timeline {
		// FIXME For TargetName and TargetProperty
		// FIXME Exception if setting on running
		// This check needs to go in native co

#if NET_2_1
		[SecuritySafeCritical ()]
#endif
		public void Begin ()
		{
			NativeMethods.storyboard_begin (native);
		}

#if NET_2_1
		[SecuritySafeCritical ()]
#endif
		public void Pause ()
		{
			NativeMethods.storyboard_pause (native);
		}


#if NET_2_1
		[SecuritySafeCritical ()]
#endif
		public void Resume ()
		{
			NativeMethods.storyboard_resume (native);
		}

#if NET_2_1
		[SecuritySafeCritical ()]
#endif
		public void Seek (TimeSpan timespan)
		{
			NativeMethods.storyboard_seek (native, timespan.Ticks);
		}

		public void SeekAlignedToLastTick (TimeSpan seekTime)
		{
			throw new NotImplementedException ();
		}
		
		public void SkipToFill ()
		{
			throw new NotImplementedException ();
		}

#if NET_2_1
		[SecuritySafeCritical ()]
#endif
		public void Stop ()
		{
			NativeMethods.storyboard_stop (native);
		}

		public TimelineCollection Children {
			get { throw new NotImplementedException (); }
		}
		
		public static void SetTarget (Timeline timeline, DependencyObject target)
		{
			// FIXME Exception if setting on running
			NativeMethods.timeline_set_manual_target (timeline.native, target.native);
		}

		public static void SetTargetName (Timeline element, string name)
		{
			// FIXME Exception if setting on running
			element.SetValue (TargetNameProperty, name);
		}

		public static void SetTargetProperty (Timeline element, PropertyPath path)
		{
			// FIXME Exception if setting on running
			element.SetValue (TargetPropertyProperty, path.Path);
		}

		public static string GetTargetName (Timeline element)
		{
			return (string) element.GetValue (TargetNameProperty);
		}

		public static PropertyPath GetTargetProperty (Timeline element)
		{
			string path = (string) element.GetValue (TargetPropertyProperty);
			return new PropertyPath (path);
		}

		public TimeSpan GetCurrentTime ()
		{
			throw new NotImplementedException ();
		}
		
		public ClockState GetCurrentState ()
		{
			throw new NotImplementedException ();
		}
	}
}
