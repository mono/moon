using System;
using System.Security.Permissions;

namespace System.Windows.Browser.Serialization
{
	public class SimpleTypeResolver : JavaScriptTypeResolver
	{
		[MonoTODO]
		public override Type ResolveType (string id)
		{
			if (id == null)
				throw new ArgumentNullException ("id");

			return Type.GetType (id);
		}
		
		[MonoTODO]
		public override string ResolveTypeId (Type type)
		{
			if (type == null)
				throw new ArgumentNullException ("type");

			return type.FullName;
		}
	}
}
