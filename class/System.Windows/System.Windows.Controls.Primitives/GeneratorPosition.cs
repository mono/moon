

namespace System.Windows.Controls.Primitives {

	public struct GeneratorPosition {

		private int index;
		private int offset;

		public GeneratorPosition (int index, int offset)
		{
			this.index = index;
			this.offset = offset;
		}

		public int Index {
			get { return index; }
			set { index = value; }
		}

		public int Offset {
			get { return offset; }
			set { offset = value; }
		}
	}
}


