//
// System.Windows.Media3D.Matrix3D struct
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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

namespace System.Windows.Media.Media3D {

	public unsafe struct Matrix3D : IFormattable {

		private double m_11;
		private double m_12;
		private double m_13;
		private double m_14;
		private double m_21;
		private double m_22;
		private double m_23;
		private double m_24;
		private double m_31;
		private double m_32;
		private double m_33;
		private double m_34;
		private double offset_x;
		private double offset_y;
		private double offset_z;
		private double m_44;

		// we can't have an empty ctor for a struct (CS0568) but the default matrix is identity
		private bool init;


		internal unsafe Matrix3D (IntPtr native)
		{
			if (native == IntPtr.Zero)
				throw new ArgumentNullException ("native");

			// FIXME: the generator butchers this pinvoke's name..
			double *dp = (double*) NativeMethods.matrix3_d_get_matrix_values (native);
			m_11 = dp [0];
			m_12 = dp [1];
			m_13 = dp [2];
			m_14 = dp [3];
			m_21 = dp [4];
			m_22 = dp [5];
			m_23 = dp [6];
			m_24 = dp [7];
			m_31 = dp [8];
			m_32 = dp [9];
			m_33 = dp [10];
			m_34 = dp [11];
			offset_x = dp [12];
			offset_y = dp [13];
			offset_z = dp [14];
			m_44 = dp [15];
			init = true;
		}

		public Matrix3D (double m11, double m12, double m13, double m14,
				 double m21, double m22, double m23, double m24,
				 double m31, double m32, double m33, double m34,
				 double offsetX, double offsetY, double offsetZ, double m44)
		{
			m_11 = m11;    
			m_12 = m12;    
			m_13 = m13;    
			m_14 = m14;    
			m_21 = m21;    
			m_22 = m22;    
			m_23 = m23;    
			m_24 = m24;    
			m_31 = m31;    
			m_32 = m32;    
			m_33 = m33;    
			m_34 = m34;    
			offset_x = offsetX;
			offset_y = offsetY;
			offset_z = offsetZ;
			m_44 = m44;    
			init = true;
		}


		public double M11 {
			get {
				if (!init) SetIdentity ();
				return m_11;
			}
			set {
				if (!init) SetIdentity ();
				m_11 = value;
			}
		}

		public double M12 {
			get {
				if (!init) SetIdentity ();
				return m_12;
			}
			set {
				if (!init) SetIdentity ();
				m_12 = value;
			}
		}

		public double M13 {
			get {
				if (!init) SetIdentity ();
				return m_13;
			}
			set {
				if (!init) SetIdentity ();
				m_13 = value;
			}
		}

		public double M14 {
			get {
				if (!init) SetIdentity ();
				return m_14;
			}
			set {
				if (!init) SetIdentity ();
				m_14 = value;
			}
		}

		public double M21 {
			get {
				if (!init) SetIdentity ();
				return m_21;
			}
			set {
				if (!init) SetIdentity ();
				m_21 = value;
			}
		}

		public double M22 {
			get {
				if (!init) SetIdentity ();
				return m_22;
			}
			set {
				if (!init) SetIdentity ();
				m_22 = value;
			}
		}

		public double M23 {
			get {
				if (!init) SetIdentity ();
				return m_23;
			}
			set {
				if (!init) SetIdentity ();
				m_23 = value;
			}
		}

		public double M24 {
			get {
				if (!init) SetIdentity ();
				return m_24;
			}
			set {
				if (!init) SetIdentity ();
				m_24 = value;
			}
		}

		public double M31 {
			get {
				if (!init) SetIdentity ();
				return m_31;
			}
			set {
				if (!init) SetIdentity ();
				m_31 = value;
			}
		}

		public double M32 {
			get {
				if (!init) SetIdentity ();
				return m_32;
			}
			set {
				if (!init) SetIdentity ();
				m_32 = value;
			}
		}

		public double M33 {
			get {
				if (!init) SetIdentity ();
				return m_33;
			}
			set {
				if (!init) SetIdentity ();
				m_33 = value;
			}
		}

		public double M34 {
			get {
				if (!init) SetIdentity ();
				return m_34;
			}
			set {
				if (!init) SetIdentity ();
				m_34 = value;
			}
		}

		public double OffsetX {
			get {
				if (!init) SetIdentity ();
				return offset_x;
			}
			set {
				if (!init) SetIdentity ();
				offset_x = value;
			}
		}

		public double OffsetY {
			get {
				if (!init) SetIdentity ();
				return offset_y;
			}
			set {
				if (!init) SetIdentity ();
				offset_y = value;
			}
		}

		public double OffsetZ {
			get {
				if (!init) SetIdentity ();
				return offset_z;
			}
			set {
				if (!init) SetIdentity ();
				offset_z = value;
			}
		}

		public double M44 {
			get {
				if (!init) SetIdentity ();
				return m_44;
			}
			set {
				if (!init) SetIdentity ();
				m_44 = value;
			}
		}

		public bool IsIdentity {
			get {
				if (!init)
					return true;

				return ((m_11 == 1.0 && m_12 == 0.0 && m_13 == 0.0 && m_14 == 0.0) &&
					(m_21 == 0.0 && m_22 == 1.0 && m_23 == 0.0 && m_24 == 0.0) &&
					(m_31 == 0.0 && m_32 == 0.0 && m_33 == 1.0 && m_34 == 0.0) &&
					(offset_x == 0.0 && offset_y == 0.0 && offset_z == 0.0 && m_44 == 1.0));
			}
		}


		private void SetIdentity ()
		{
			m_11 = 1.0;
			m_12 = 0.0;
			m_13 = 0.0;
			m_14 = 0.0;
			m_21 = 0.0;
			m_22 = 1.0;
			m_23 = 0.0;
			m_24 = 0.0;
			m_31 = 0.0;
			m_32 = 0.0;
			m_33 = 1.0;
			m_34 = 0.0;
			offset_x = 0.0;
			offset_y = 0.0;
			offset_z = 0.0;
			m_44 = 1.0;
			init = true;
		}


		public override int GetHashCode ()
		{
			if (IsIdentity)
				return 0;

			return (m_11.GetHashCode () ^ m_12.GetHashCode () ^ m_13.GetHashCode () ^ m_14.GetHashCode () ^ 
				m_21.GetHashCode () ^ m_22.GetHashCode () ^ m_23.GetHashCode () ^ m_24.GetHashCode () ^ 
				m_31.GetHashCode () ^ m_32.GetHashCode () ^ m_33.GetHashCode () ^ m_34.GetHashCode () ^ 
				offset_x.GetHashCode () ^ offset_y.GetHashCode () ^ offset_z.GetHashCode () ^ m_44.GetHashCode ());
		}
		
		public override string ToString ()
		{
			if (IsIdentity)
				return "Identity";

			return String.Format ("{0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15}",
					      m_11, m_12, m_13, m_14,
					      m_21, m_22, m_23, m_24,
					      m_31, m_32, m_33, m_34,
					      offset_x, offset_y, offset_z, m_44);
		}

		public string ToString (IFormatProvider provider)
		{
			return (this as IFormattable).ToString (null, provider);
		}

		string IFormattable.ToString (string value, IFormatProvider formatProvider)
		{
			if (IsIdentity)
				return "Identity";

			if (String.IsNullOrEmpty (value))
				value = null;

			if (formatProvider != null) {
				ICustomFormatter cp = (ICustomFormatter) formatProvider.GetFormat (typeof (ICustomFormatter));
				if (cp != null) {
					string comma = cp.Format (null, ',', formatProvider);
					return String.Format ("{1}{0}{2}{0}{3}{0}{4}{0}{5}{0}{6}{0}{7}{0}{8}{0}{9}{0}{10}{0}{11}{0}{12}{0}{13}{0}{14}{0}{15}{0}{16}", 
							      comma,
							      cp.Format (value, m_11, formatProvider),
							      cp.Format (value, m_12, formatProvider),
							      cp.Format (value, m_13, formatProvider),
							      cp.Format (value, m_14, formatProvider),
							      cp.Format (value, m_21, formatProvider),
							      cp.Format (value, m_22, formatProvider),
							      cp.Format (value, m_23, formatProvider),
							      cp.Format (value, m_24, formatProvider),
							      cp.Format (value, m_31, formatProvider),
							      cp.Format (value, m_32, formatProvider),
							      cp.Format (value, m_33, formatProvider),
							      cp.Format (value, m_34, formatProvider),
							      cp.Format (value, offset_x, formatProvider),
							      cp.Format (value, offset_y, formatProvider),
							      cp.Format (value, offset_z, formatProvider),
							      cp.Format (value, m_44, formatProvider));
				}
			}

			return String.Format ("{0},{1},{2},{3},{4},{5},{6},{7},{8},{9},{10},{11},{12},{13},{14},{15}",
			      m_11.ToString (value, formatProvider), m_12.ToString (value, formatProvider), m_13.ToString (value, formatProvider), m_14.ToString (value, formatProvider),
			      m_21.ToString (value, formatProvider), m_22.ToString (value, formatProvider), m_23.ToString (value, formatProvider), m_24.ToString (value, formatProvider),
			      m_31.ToString (value, formatProvider), m_32.ToString (value, formatProvider), m_33.ToString (value, formatProvider), m_34.ToString (value, formatProvider),
			      offset_x.ToString (value, formatProvider), offset_y.ToString (value, formatProvider), offset_z.ToString (value, formatProvider), m_44.ToString (value, formatProvider));
		}
		
		// TODO comparing double is problematic, review MS precision

		public override bool Equals (object o)
		{
			if ((o == null) || (!(o is Matrix3D)))
				return false;
			return Equals ((Matrix3D)o);
		}

		public bool Equals (Matrix3D value)
		{
			return (this == value);
		}

		public static bool operator == (Matrix3D matrix1, Matrix3D matrix2)
		{
			if (!matrix1.init) {
				if (!matrix2.init)
					return true;
				matrix1.SetIdentity ();
			}
			if (!matrix2.init) {
				matrix2.SetIdentity ();
			}

			return (matrix1.m_11 == matrix2.m_11 && matrix1.m_12 == matrix2.m_12 && matrix1.m_13 == matrix2.m_13 && matrix1.m_14 == matrix2.m_14 &&
				matrix1.m_21 == matrix2.m_21 && matrix1.m_22 == matrix2.m_22 && matrix1.m_23 == matrix2.m_23 && matrix1.m_24 == matrix2.m_24 &&
				matrix1.m_31 == matrix2.m_31 && matrix1.m_32 == matrix2.m_32 && matrix1.m_33 == matrix2.m_33 && matrix1.m_34 == matrix2.m_34 &&
				matrix1.offset_x == matrix2.offset_x && matrix1.offset_y == matrix2.offset_y && matrix1.offset_z == matrix2.offset_z && matrix1.m_44 == matrix2.m_44);
		}

		public static bool operator != (Matrix3D matrix1, Matrix3D matrix2)
		{
			return !(matrix1 == matrix2);
		}


		public static Matrix3D Identity {
			get { return new Matrix3D (); }
		}
	}
}
