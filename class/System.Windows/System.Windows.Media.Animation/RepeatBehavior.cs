//
// RepeatBehavior.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2007, 2009 Novell, Inc.
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
namespace System.Windows.Media.Animation {

	public struct RepeatBehavior : IFormattable {

		// note: Count is default for the (default) empty ctor, so it needs to have a 0 value
		enum RepeatBehaviorKind {
			Count,
			TimeSpan,
			Forever
		}

		private RepeatBehaviorKind kind;
		private int padding;
		private double count;
		private TimeSpan duration;

		private RepeatBehavior (RepeatBehaviorKind kind, double count, TimeSpan duration)
		{
			this.kind = kind;
			this.count = count;
			this.duration = duration;
			this.padding = 0;
		}

		public RepeatBehavior (double count)
		{
			if ((count < 0) || Double.IsNaN (count) || Double.IsInfinity (count))
				throw new ArgumentOutOfRangeException ("count");

			kind = RepeatBehaviorKind.Count;
			this.count = count;
			duration = TimeSpan.Zero;
			padding = 0;
		}

		public RepeatBehavior (TimeSpan duration)
		{
			if (duration.Ticks < 0)
				throw new ArgumentOutOfRangeException ("duration");

			kind = RepeatBehaviorKind.TimeSpan;
			this.duration = duration;
			count = 0;
			padding = 0;
		}

		public double Count {
			get {
				if (kind == RepeatBehaviorKind.Count)
					return count;
				throw new InvalidOperationException ("This RepeatBehavior does not contain a Count");
			}
		}

		public TimeSpan Duration {
			get {
				if (kind == RepeatBehaviorKind.TimeSpan)
					return duration;
				throw new InvalidOperationException ("This RepeatBehavior does not contain a Duration");
			}
		}

		public static RepeatBehavior Forever {
			get { return new RepeatBehavior (RepeatBehaviorKind.Forever, 0, TimeSpan.Zero); }
		}


		public bool HasCount {
			get { return kind == RepeatBehaviorKind.Count; }
		}

		public bool HasDuration {
			get { return kind == RepeatBehaviorKind.TimeSpan; }
		}

		public override bool Equals (object value)
		{
			if (!(value is RepeatBehavior))
				return false;

			return Equals (this, (RepeatBehavior) value);
		}

		public bool Equals (RepeatBehavior repeatBehavior)
		{
			return Equals (this, repeatBehavior);
		}

		public static bool Equals (RepeatBehavior repeatBehavior1, RepeatBehavior repeatBehavior2)
		{
			if (repeatBehavior1.kind != repeatBehavior2.kind)
				return false;

			switch (repeatBehavior1.kind) {
			case RepeatBehaviorKind.Count:
				return repeatBehavior1.count == repeatBehavior2.count;
			case RepeatBehaviorKind.TimeSpan:
				return repeatBehavior1.duration == repeatBehavior2.duration;
			case RepeatBehaviorKind.Forever:
				return true;
			default:
				return false;
			}
		}

		public static bool operator == (RepeatBehavior repeatBehavior1, RepeatBehavior repeatBehavior2)
		{
			return Equals (repeatBehavior1, repeatBehavior2);
		}

		public static bool operator != (RepeatBehavior repeatBehavior1, RepeatBehavior repeatBehavior2)
		{
			return !Equals (repeatBehavior1, repeatBehavior2);
		}

		public override int GetHashCode ()
		{
			switch (kind) {
			case RepeatBehaviorKind.Count:
				return count.GetHashCode ();
			case RepeatBehaviorKind.TimeSpan:
				return duration.GetHashCode ();
			case RepeatBehaviorKind.Forever:
				return RepeatBehaviorKind.Forever.GetHashCode ();
			default:
				return base.GetHashCode ();
			}
		}
		
		public override string ToString ()
		{
			switch (kind) {
			case RepeatBehaviorKind.Forever:
				return "Forever";
			case RepeatBehaviorKind.Count:
				return String.Format ("{0}x", count);
			case RepeatBehaviorKind.TimeSpan:
				return duration.ToString ();
			default:
				return base.ToString ();
			}
		}

		public string ToString (IFormatProvider formatProvider)
		{
			return (this as IFormattable).ToString (null, formatProvider);
		}

		string System.IFormattable.ToString (string format, IFormatProvider formatProvider)
		{
			if (kind != RepeatBehaviorKind.Count)
				return ToString ();

			if (String.IsNullOrEmpty (format))
				format = null;

			if (formatProvider != null) {
				ICustomFormatter cp = (ICustomFormatter) formatProvider.GetFormat (typeof (ICustomFormatter));
				if (cp != null) {
					return String.Format ("{0}x", cp.Format (format, count, formatProvider));
				}
			}

			return String.Format ("{0}x", count.ToString (format, formatProvider));
		}
	}
}
