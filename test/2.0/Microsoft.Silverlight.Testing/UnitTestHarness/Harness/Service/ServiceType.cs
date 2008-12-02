// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Browser;
using Microsoft.Silverlight.Testing.Harness;

namespace Microsoft.Silverlight.Testing.Harness.Service
{
    /// <summary>
    /// The type of test service in use.  Used by the more advanced service 
    /// scenarios in SilverlightTestServiceProvider.
    /// </summary>
    public enum ServiceType
    {
        /// <summary>
        /// No service, or unknown service type.
        /// </summary>
        None,

        /// <summary>
        /// A direct connection, be it the file system, isolated storage, or 
        /// similar.
        /// </summary>
        Direct,

        /// <summary>
        /// A web service.
        /// </summary>
        WebService,
    }
}