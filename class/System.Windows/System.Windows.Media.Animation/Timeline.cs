//
// Timeline.cs
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
using System.Windows;
using System.Windows.Media;
using Mono;

namespace System.Windows.Media.Animation {

	public abstract class Timeline : DependencyObject {

		static Timeline ()
		{
			AutoReverseProperty = DependencyProperty.Lookup (Kind.TIMELINE, "AutoReverse", typeof (bool));
			BeginTimeProperty = DependencyProperty.Lookup (Kind.TIMELINE, "BeginTime", typeof (Nullable<TimeSpan>));
			DurationProperty = DependencyProperty.Lookup (Kind.TIMELINE, "Duration", typeof (Duration));
			SpeedRatioProperty = DependencyProperty.Lookup (Kind.TIMELINE, "SpeedRatio", typeof (double));
			FillBehaviorProperty = DependencyProperty.Lookup (Kind.TIMELINE, "FillBehavior", typeof (FillBehavior));
			RepeatBehaviorProperty = DependencyProperty.Lookup (Kind.TIMELINE, "RepeatBehavior", typeof (RepeatBehavior));
		}			
		
		internal Timeline (IntPtr raw) : base (raw)
		{
		}
		
		public Timeline () : base (NativeMethods.timeline_new ())
		{
		}
		
		public bool AutoReverse {
			get {
				return (bool) GetValue (AutoReverseProperty);
			}
			
			set {
				SetValue (AutoReverseProperty, value);
			}
		}
		
		public Nullable<TimeSpan> BeginTime {
			get {
				return (Nullable<TimeSpan>) GetValue (BeginTimeProperty);
			}
			
			set {
				SetValue (BeginTimeProperty, value);
			}
		}
		public Duration Duration {
			get {
				return (Duration) GetValue (DurationProperty);
			}
			
			set {
				SetValue (DurationProperty, value);
			}
		}
		public FillBehavior FillBehavior {
			get {
				return (FillBehavior) GetValue (FillBehaviorProperty);
			}
			
			set {
				SetValue (FillBehaviorProperty, value);
			}
		}
		public RepeatBehavior RepeatBehavior {
			get {
				return (RepeatBehavior) GetValue (RepeatBehaviorProperty);
			}
			
			set {
				SetValue (RepeatBehaviorProperty, value);
			}
		}
		
		public double SpeedRatio {
			get {
				return (double) GetValue (SpeedRatioProperty);
			}
			
			set {
				SetValue (SpeedRatioProperty, value);
			}
		}
		
		public static readonly DependencyProperty AutoReverseProperty;
		public static readonly DependencyProperty BeginTimeProperty;
		public static readonly DependencyProperty DurationProperty;
		public static readonly DependencyProperty SpeedRatioProperty;
		public static readonly DependencyProperty FillBehaviorProperty;
		public static readonly DependencyProperty RepeatBehaviorProperty;

		internal override Kind GetKind ()
		{
			return Kind.TIMELINE;
		}
	}
}
