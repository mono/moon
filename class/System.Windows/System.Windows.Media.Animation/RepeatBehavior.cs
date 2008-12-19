//
// RepeatBehavior.cs
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
namespace System.Windows.Media.Animation {

	public struct RepeatBehavior : IFormattable {
		const int COUNT = 1;
		const int TIMESPAN = 2;
		const int FOREVER = 3;

		internal int kind;
		private int padding;
		internal double count;
		internal TimeSpan duration;

		internal RepeatBehavior (int kind, double count, TimeSpan duration)
		{
			this.kind = kind;
			this.count = count;
			this.duration = duration;
			this.padding = 0;
		}

		public RepeatBehavior (double count)
		{
			kind = COUNT;
			this.count = count;
			duration = new TimeSpan (0);
			this.padding = 0;
		}

		public RepeatBehavior (TimeSpan duration)
		{
			kind = TIMESPAN;
			this.duration = duration;
			count = 0;
			this.padding = 0;
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
			case COUNT:
				return repeatBehavior1.count == repeatBehavior2.count;
			case TIMESPAN:
				return repeatBehavior1.duration == repeatBehavior2.duration;
			case FOREVER:
				return true;
			default:
				return false; // throw?
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
		
		public override string ToString ()
		{
			return base.ToString ();
		}

		public string ToString (IFormatProvider formatProvider)
		{
			throw new System.NotImplementedException ();
		}

		// This method shows up in gui-compare, don't know how I can remove it
		string System.IFormattable.ToString (string format, IFormatProvider formatProvider)
		{
			throw new System.NotImplementedException ();
		}

		
	}
}
