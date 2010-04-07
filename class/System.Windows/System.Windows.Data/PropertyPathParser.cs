using System;
namespace System.Windows
{
	class PropertyPathParser {
		string Path {
			get; set;
		}

		public PropertyPathParser (string path)
		{
			Path = path;
		}

		public PropertyNodeType Step (out string typeName, out string propertyName, out string index)
		{
			var type = PropertyNodeType.None;
			if (Path.Length == 0) {
				typeName = null;
				propertyName = null;
				index = null;
				return type;
			}

			int end;
			if (Path.StartsWith ("(")) {
				type = PropertyNodeType.AttachedProperty;
				end = Path.IndexOf (")");
				if (end == -1)
					throw new ArgumentException ("Invalid property path. Attached property is missing the closing bracket");

				int splitIndex = Path.IndexOf ('.', 0, end);
				if (splitIndex == -1)
					throw new Exception ("Invalid property path. Attached property is missing a '.'");

				typeName = Path.Substring (1, splitIndex - 1);
				propertyName = Path.Substring (splitIndex + 1, end - splitIndex - 1);
				index = null;
				Path = Path.Substring (end + 1);
			} else if (Path.StartsWith ("[")) {
				type = PropertyNodeType.Indexed;
				end = Path.IndexOf ("]");

				typeName = null;
				propertyName = null;
				index = Path.Substring (1, end - 1);
				Path = Path.Substring (end + 1);
				// You can do stuff like: [someIndex].SomeProp
				// as well as: [SomeIndex]SomeProp
				if (Path.StartsWith ("."))
					Path = Path.Substring (1);
			} else {
				type = PropertyNodeType.Property;
				end = Path.IndexOfAny (new char [] { '.', '[' });
				if (end == -1) {
					propertyName = Path;
					Path = "";
				}
				else {
					propertyName = Path.Substring (0, end);
					if (Path [end] == '.')
						Path = Path.Substring (end + 1);
					else
						Path = Path.Substring (end);
				}

				typeName = null;
				index = null;
			}

			return type;
		}
	}
}

