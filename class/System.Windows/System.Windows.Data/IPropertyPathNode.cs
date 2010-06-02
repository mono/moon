using System;
using System.Reflection;

namespace System.Windows.Data
{
	interface IPropertyPathNode {

		event EventHandler ValueChanged;

		bool IsBroken { get; }

		IPropertyPathNode Next { get; set; }

		void SetValue (object value);

		object Source { get; }

		object Value { get; }

		PropertyInfo PropertyInfo { get; }

		Type ValueType { get; }

		void SetSource (object source);
	}
}
