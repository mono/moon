// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// Settings used to initialize the test harness.
    /// </summary>
    public class TestHarnessSettings
    {
        /// <summary>
        /// Gets the parameters from the response file.
        /// </summary>
        /// <value>The parameters.</value>
        public IDictionary<string, string> Parameters { get; private set; }

        /// <summary>
        /// Gets the components initialized by the entry-point assembly. These
        /// are the dynamically loaded objects that may be needed by the
        /// TestHarness.
        /// </summary>
        /// <value>The components.</value>
        public IDictionary<string, object> Components { get; private set; }

        /// <summary>
        /// Gets the log providers.
        /// </summary>
        public IList<LogProvider> LogProviders { get; private set; }

        /// <summary>
        /// Gets the list of test assemblies.
        /// </summary>
        /// <value>The test assembly.</value>
        [SuppressMessage("Microsoft.Usage", "CA2227:CollectionPropertiesShouldBeReadOnly", Justification = "Leaving open for extensibility purposes")]
        public IList<Assembly> TestAssemblies { get; private set; }
        
        /// <summary>
        /// Gets or sets the test service provider.  The test service lights up 
        /// advanced out-of-process communication, reporting, logging, and 
        /// other valuable services.
        /// </summary>
        public TestServiceProvider TestService { get; set; }

        /// <summary>
        /// Gets or sets the test harness.
        /// </summary>
        /// <value>The test harness.</value>
        public ITestHarness TestHarness { get; set; }

        /// <summary>
        /// Initializes a new instance of the TestHarnessSettings class.
        /// </summary>
        public TestHarnessSettings()
        {
            Components = new Dictionary<string, object>();
            LogProviders = new List<LogProvider>();
            Parameters = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            TestAssemblies = new List<Assembly>();
            TestService = new TestServiceProvider();
        }
    }
}