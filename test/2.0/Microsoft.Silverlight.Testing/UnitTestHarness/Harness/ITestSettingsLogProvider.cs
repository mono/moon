// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// Interface for LogProviders that want access to external test settings.
    /// </summary>
    public interface ITestSettingsLogProvider 
    {
        /// <summary>
        /// Initializes the provider.
        /// </summary>
        /// <param name="settings">The settings.</param>
        void Initialize(TestHarnessSettings settings);
    }
}