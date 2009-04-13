/*
 * App.xaml.cs
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Windows;
using System.Windows.Browser;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Threading;

using Mono.Moonlight.UnitTesting;

namespace Mono.Moonlight
{
	public partial class SampleTest : Application
	{
		public SampleTest ()
		{
			this.Startup += this.Application_Startup;

			InitializeComponent();
		}

		private void Application_Startup(object sender, StartupEventArgs e)
		{
			this.RootVisual = new SampleTestControl ();

			TestPlugin.TestPluginReady += delegate (object dummy, EventArgs args) 
			{
				TestLogger.LogDebug ("Hello!");
				TestPlugin.CaptureSingleImage ("SampleTest.png", 100, 100, 0);
				TestPlugin.SignalShutdown ();
			};
		}
	}

}
