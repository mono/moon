using System;
using System.Reflection;

namespace System.Windows
{
	public interface IPropertyPathNode {

		event EventHandler ValueChanged;

		bool IsBroken { get; }

		IPropertyPathNode Next { get; }

		void SetValue (object value);

		object Value { get; }

		PropertyInfo PropertyInfo { get; }

		Type ValueType { get; }

		void SetSource (object source);
	}
}
