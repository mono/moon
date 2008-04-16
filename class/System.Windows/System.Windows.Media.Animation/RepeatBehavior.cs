//
// RepeatBehavior.cs
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
namespace System.Windows.Media.Animation {

	public struct RepeatBehavior {
		const int COUNT = 1;
		const int TIMESPAN = 2;
		const int FOREVER = 3;

		internal int kind;
		internal double count;
		internal TimeSpan duration;

		internal RepeatBehavior (int kind, double count, TimeSpan duration)
		{
			this.kind = kind;
			this.count = count;
			this.duration = duration;
		}

		public RepeatBehavior (double count)
		{
			kind = COUNT;
			this.count = count;
			duration = new TimeSpan (0);
		}

		public RepeatBehavior (TimeSpan duration)
		{
			kind = TIMESPAN;
			this.duration = duration;
			count = 0;
		}

		public double Count {
			get {
				if (kind == COUNT)
					return count;
				throw new Exception ("This RepeatBehavior does not contain a Count");
			}
		}

		public TimeSpan Duration {
			get {
				if (kind == TIMESPAN)
					return duration;
				throw new Exception ("This RepeatBehavior does not contain a Duration");
			}
		}

		public static RepeatBehavior Forever {
			get { return new RepeatBehavior (FOREVER, 0, TimeSpan.Zero); }
		}


		public bool HasCount {
			get {
				return kind == COUNT;
			}
		}

		public bool HasDuration {
			get {
				return kind == TIMESPAN;
			}
		}

		public override bool Equals (object o)
		{
			if (! (o is RepeatBehavior))
				return false;

			return Equals (this, (RepeatBehavior) o);
		}

		public bool Equals (RepeatBehavior rb)
		{
			return Equals (this, rb);
		}

		public static bool Equals (RepeatBehavior ls, RepeatBehavior rs)
		{
			if (ls.kind != rs.kind)
				return false;

			switch (ls.kind) {
			case COUNT:
				return ls.count == rs.count;
			case TIMESPAN:
				return ls.duration == rs.duration;
			case FOREVER:
				return true;
			default:
				return false; // throw?
			}
		}

		public static bool operator == (RepeatBehavior ls, RepeatBehavior rs)
		{
			return Equals (ls, rs);
		}

		public static bool operator != (RepeatBehavior ls, RepeatBehavior rs)
		{
			return !Equals (ls, rs);
		}

		public override int GetHashCode ()
		{
			switch (kind) {
			case COUNT:
				return count.GetHashCode ();
			case TIMESPAN:
				return duration.GetHashCode ();
			case FOREVER:
				return FOREVER.GetHashCode ();
			default:
				return base.GetHashCode (); // throw?
			}
		}
	}
}
