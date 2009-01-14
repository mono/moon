using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

using Microsoft.Silverlight.Testing;

namespace Mono.Moonlight.UnitTesting
{
	[AttributeUsage (AttributeTargets.Method, AllowMultiple=false)]
	public class MoonlightBugAttribute : BugAttribute
	{
		public MoonlightBugAttribute ()
			: base (PlatformID.Unix)
		{
		}

		public MoonlightBugAttribute (params PlatformID [] platforms)
			: base (platforms)
		{
		}

		public MoonlightBugAttribute (string description)
			: base (description, new PlatformID [] { PlatformID.Unix })
		{
		}

		public MoonlightBugAttribute (string description, params PlatformID [] platforms)
			: base (description, platforms)
		{
		}
	}
}
