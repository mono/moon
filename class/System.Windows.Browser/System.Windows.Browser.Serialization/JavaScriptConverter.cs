using System;
using System.Collections.Generic;
using System.Security.Permissions;

namespace System.Windows.Browser.Serialization
{
	public abstract class JavaScriptConverter
	{
		protected JavaScriptConverter ()
		{
		}
		
		public abstract object Deserialize (
			IDictionary<string, object> dictionary,
			Type type,
			JavaScriptSerializer serializer);
		
		public abstract IDictionary<string, object> Serialize (
			object obj, JavaScriptSerializer serializer);
		
		public abstract IEnumerable<Type> SupportedTypes { get; }
	}
}
