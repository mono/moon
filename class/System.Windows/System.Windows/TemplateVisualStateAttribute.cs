/*
 * TemplateVisualStateAttribute.cs.
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
	public class TemplateVisualStateAttribute : Attribute
	{
		public TemplateVisualStateAttribute()
		{
		}

		public string Name { get; set; }
		public string GroupName { get; set; }
	}
}
