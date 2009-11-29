/*
 * StyleTypedPropertyAttribute.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;

namespace System.Windows
{
	[AttributeUsageAttribute (AttributeTargets.Class, AllowMultiple=true)]
	public sealed class StyleTypedPropertyAttribute : Attribute 
	{
		string property;
		Type style_target_type;
		
		public StyleTypedPropertyAttribute()
		{
		}

		public string Property {
			get { return property; }
			set { property = value; }
		}

		public Type StyleTargetType {
			get { return style_target_type; }
			set { style_target_type = value; }
		}
	}
}
