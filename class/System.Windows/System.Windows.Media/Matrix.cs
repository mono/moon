//
// System.Windows.Media.Matrix struct
//
// Authors:
//	Sebastien Pouliot  <sebastien@ximian.com>
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
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

namespace System.Windows.Media {

	public struct Matrix {

		private double m_11;
		private double m_12;
		private double m_21;
		private double m_22;
		private double offset_x;
		private double offset_y;
		// we can't have an empty ctor for a struct (CS0568) but the default matrix is identity
		private bool init;


		public Matrix (double m11, double m12, double m21, double m22, double offsetX, double offsetY)
		{
			m_11 = m11;
			m_12 = m12;
			m_21 = m21;
			m_22 = m22;
			offset_x = offsetX;
			offset_y = offsetY;
			init = true;
		}


		public double M11 {
			get {
				if (!init)
					SetIdentity ();
				return m_11;
			}
			set {
				if (!init)
					SetIdentity ();
				m_11 = value;
			}
		}

		public double M12 {
			get {
				if (!init)
					SetIdentity ();
				return m_12;
			}
			set {
				if (!init)
					SetIdentity ();
				m_12 = value;
			}
		}

		public double M21 {
			get {
				if (!init)
					SetIdentity ();
				return m_21;
			}
			set {
				if (!init)
					SetIdentity ();
				m_21 = value;
			}
		}

		public double M22 {
			get {
				if (!init)
					SetIdentity ();
				return m_22;
			}
			set {
				if (!init)
					SetIdentity ();
				m_22 = value;
			}
		}

		public double OffsetX {
			get {
				if (!init)
					SetIdentity ();
				return offset_x;
			}
			set {
				if (!init)
					SetIdentity ();
				offset_x = value;
			}
		}

		public double OffsetY {
			get {
				if (!init)
					SetIdentity ();
				return offset_y;
			}
			set {
				if (!init)
					SetIdentity ();
				offset_y = value;
			}
		}

		public bool IsIdentity {
			get {
				if (!init)
					return true;

				return ((m_11 == 1.0) && (m_12 == 0.0) && (m_21 == 0.0) && (m_22 == 1.0) &&
					(offset_x == 0.0) && (offset_y == 0.0));
			}
		}


		public void SetIdentity ()
		{
			m_11 = 1.0;
			m_12 = 0.0;
			m_21 = 0.0;
			m_22 = 1.0;
			offset_x = 0.0;
			offset_y = 0.0;
		}


		public override int GetHashCode ()
		{
			if (IsIdentity)
				return 0;

			return m_11.GetHashCode () ^ m_12.GetHashCode () ^ m_21.GetHashCode () ^ m_22.GetHashCode () ^
				offset_x.GetHashCode () ^ offset_y.GetHashCode ();
		}
		
		public override string ToString ()
		{
			if (IsIdentity)
				return "Identity";
			return String.Format ("{0},{1},{2},{3},{4},{5}", m_11, m_12, m_21, m_22, offset_x, offset_y);
		}

		// TODO comparing double is problematic, review MS precision

		public override bool Equals (object o)
		{
			if ((o == null) || (!(o is Matrix)))
				return false;
			return Equals ((Matrix)o);
		}

		public bool Equals (Matrix value)
		{
			return (this == value);
		}

		public static bool Equals (Matrix matrix1, Matrix matrix2)
		{
			return (matrix1 == matrix2);
		}

		public static bool operator == (Matrix matrix1, Matrix matrix2)
		{
			return ((matrix1.m_11 == matrix2.m_11) && (matrix1.m_12 == matrix2.m_12) &&
				(matrix1.m_21 == matrix2.m_21) && (matrix1.m_22 == matrix2.m_22) &&
				(matrix1.offset_x == matrix2.offset_x) && (matrix1.offset_y == matrix2.offset_y));
		}

		public static bool operator != (Matrix matrix1, Matrix matrix2)
		{
			return ((matrix1.m_11 != matrix2.m_11) || (matrix1.m_12 != matrix2.m_12) ||
				(matrix1.m_21 != matrix2.m_21) || (matrix1.m_22 != matrix2.m_22) ||
				(matrix1.offset_x != matrix2.offset_x) || (matrix1.offset_y != matrix2.offset_y));
		}


		public static Matrix Identity {
			get { return new Matrix (); }
		}
	}
}
