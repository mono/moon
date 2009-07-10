//
// Duration.cs
//
// This class implementation is not very complete, nor has it been
// tested, this is here just so we can get other things elsewhere
// to compile and run for now
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
using Mono;
using System;

namespace System.Windows {

	public struct Duration  {

		enum DurationKind {
			TimeSpan = 0,
			Automatic = 1,
			Forever = 2,
		}

		DurationKind kind;
		int padding;
		TimeSpan time_span;

		static Duration automatic = new Duration (DurationKind.Automatic);
		static Duration forever = new Duration (DurationKind.Forever);
		
		internal int KindInternal {
			get { return (int) kind; }
		}
		
		internal TimeSpan TimeSpanInternal {
			get { return time_span; }
		}
		
		Duration (DurationKind k)
		{
			kind = k;
			time_span = new TimeSpan (0);
			padding = 0;
		}
		
		internal Duration (int k, TimeSpan ts)
		{
			kind = (DurationKind) k;
			time_span = ts;
			padding = 0;
		}
		
		public Duration (TimeSpan timeSpan)
		{
			time_span = timeSpan;
			kind = DurationKind.TimeSpan;
			padding = 0;
		}

		public static bool Equals (Duration t1, Duration t2)
		{
			if (t1.kind == DurationKind.TimeSpan && t2.kind == DurationKind.TimeSpan)
				return t1.time_span == t2.time_span;

			return t1.kind == t2.kind;
		}
		
		public override bool Equals (object value)
		{
			if (!(value is Duration))
				return false;

			return Equals (this, (Duration) value);
		}
		
		public bool Equals (Duration duration)
		{
			return Equals (this, duration);
		}

		public static int Compare (Duration t1, Duration t2)
		{
			if (t1.kind == t2.kind && t1.kind == DurationKind.TimeSpan)
				return TimeSpan.Compare (t1.time_span, t2.time_span);

			return t1.kind - t2.kind;
		}
			
		public override int GetHashCode ()
		{
			if (kind == DurationKind.TimeSpan)
				return time_span.GetHashCode ();
			else
				return (int) kind;
		}

		public Duration Add (Duration duration)
		{
			if (kind == duration.kind && kind == DurationKind.TimeSpan)
				return new Duration (time_span.Add (duration.time_span));

			return this;
		}

		public Duration Subtract (Duration duration)
		{
			if (kind == duration.kind && kind == DurationKind.TimeSpan)
				return new Duration (time_span.Subtract (duration.time_span));

			return this;
		}

		public static Duration Plus (Duration duration)
		{
			if (duration.kind == DurationKind.TimeSpan && duration.time_span < TimeSpan.Zero)
				return new Duration (-duration.time_span);
			else
				return duration;
		}

		public override string ToString ()
		{
			return kind == DurationKind.TimeSpan ?
				time_span.ToString () : (kind == DurationKind.Automatic ? "Automatic" : "Forever");
		}

		public static implicit operator Duration (TimeSpan timeSpan)
		{
			return new Duration (timeSpan);
		}

		public static Duration operator + (Duration t1, Duration t2)
		{
			return t1.Add (t2);
		}
		
		public static Duration operator - (Duration t1, Duration t2)
		{
			return t1.Subtract (t2);
		}
		
		public static bool operator == (Duration t1, Duration t2)
		{
			if (t1.kind == DurationKind.TimeSpan && t2.kind == DurationKind.TimeSpan)
				return t1.time_span == t2.time_span;

			return t1.kind == t2.kind;
		}
		
		public static bool operator != (Duration t1, Duration t2)
		{
			if (t1.kind != t2.kind)
				return true;
			if (t1.kind == DurationKind.TimeSpan)
				return t1.time_span != t2.time_span;
			return false;
		}
		
		public static bool operator > (Duration t1, Duration t2)
		{
			return Compare (t1, t2) > 0;
		}
		
		public static bool operator >= (Duration t1, Duration t2)
		{
			return Compare (t1, t2) >= 0;
		}
		
		public static bool operator < (Duration t1, Duration t2)
		{
			return Compare (t1, t2) < 0;
		}
		
		public static bool operator <= (Duration t1, Duration t2)
		{
			return Compare (t1, t2) <= 0;
		}
		
		public static Duration operator + (Duration duration)
		{
			return Plus (duration);
		}

		public static Duration Automatic {
			get { return automatic; }
		}
		
		public static Duration Forever {
			get { return forever; } 
		}

		public bool HasTimeSpan {
			get {
				return (kind == DurationKind.TimeSpan);
			}
		}

		public TimeSpan TimeSpan {
			get {
				if (kind == DurationKind.TimeSpan)
					return time_span;

				throw new InvalidOperationException ("This is not a TimeSpan duration");
			}
		}
	}
}
