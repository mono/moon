using System;

namespace Mono {

	class XmlNsKey {
		string xmlns;
		string name;

		public XmlNsKey (string xmlns, string name)
		{
			this.xmlns = xmlns;
			this.name = name;
		}

		public override int GetHashCode ()
		{
			int code = name.GetHashCode ();

			if (xmlns != null)
				code += xmlns.GetHashCode ();

			return code;
		}

		public override bool Equals (object o)
		{
			XmlNsKey key = o as XmlNsKey;

			if (key == null)
				return false;

			return key.xmlns == this.xmlns && key.name == this.name;
		}
	}
}

