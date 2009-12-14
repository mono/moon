// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows;
using System;
using Microsoft.Silverlight.Testing;

namespace System.Windows.Controls.Extended.Test
{
    public partial class App : Application
    {
        public App()
        {
            this.Startup += this.OnStartup;
            this.Exit += this.OnExit;

            InitializeComponent();
        }
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Usage", "CA1806:DoNotIgnoreMethodResults", MessageId = "Microsoft.Silverlight.Testing.TransparentTestDriver", Justification = "Required to instantiate test framework")]
        private void OnStartup(object sender, EventArgs e)
        {
            // Initialize and start the test framework
#if MOONLIGHT
			this.RootVisual = Mono.Moonlight.UnitTesting.Testing.CreateTestPage (this);
#else
            this.RootVisual = UnitTestSystem.CreateTestPage();
#endif
        }

        private void OnExit(object sender, EventArgs e)
        {

        }
    }
}
