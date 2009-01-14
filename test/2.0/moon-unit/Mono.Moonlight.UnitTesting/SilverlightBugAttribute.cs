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
	[AttributeUsage (AttributeTargets.Method, AllowMultiple = false)]
	public class SilverlightBugAttribute : BugAttribute
	{
		public SilverlightBugAttribute () : base (PlatformID.Win32NT, PlatformID.MacOSX)
		{
		}

		public SilverlightBugAttribute (params PlatformID [] platforms)
			: base (platforms)
		{
		}

		public SilverlightBugAttribute (string description)
			: base (description, PlatformID.Win32NT, PlatformID.MacOSX)
		{
		}

		public SilverlightBugAttribute (string description, params PlatformID [] platforms)
			: base (description, platforms)
		{
		}
	}
}
