// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using MethodInfo = System.Reflection.MethodInfo;

namespace Microsoft.Silverlight.Testing.Harness.Service
{
    /// <summary>
    /// The base class for test service providers.
    /// </summary>
    public abstract class ProviderBase
    {
        /// <summary>
        /// Initializes a new base provider class.
        /// </summary>
        /// <param name="provider">The owning test service provider.</param>
        /// <param name="displayName">The display name of the service.</param>
        protected ProviderBase(TestServiceProvider provider, string displayName)
        {
            DisplayName = displayName;
            TestService = provider ?? this as TestServiceProvider;
        }

        /// <summary>
        /// Event fired once initialization is complete.
        /// </summary>
        public event EventHandler InitializeCompleted;

        /// <summary>
        /// Gets the display name for the provider.
        /// </summary>
        public string DisplayName { get; protected set; }

        /// <summary>
        /// Gets the owning test service.
        /// </summary>
        public TestServiceProvider TestService { get; private set; }

        /// <summary>
        /// Initializes the provider.
        /// </summary>
        public virtual void Initialize()
        {
            Initialized = true;
            OnInitializeCompleted();
        }

        /// <summary>
        /// Invokes a method on this provider using reflection.
        /// </summary>
        /// <param name="methodName">The name of the method.</param>
        /// <param name="parameters">The optional parameters.</param>
        public void InvokeMethod(string methodName, params object[] parameters)
        {
            MethodInfo method = GetType().GetMethod(methodName);
            method.Invoke(this, parameters);
        }

        /// <summary>
        /// Gets a value indicating whether the provider has been initialized 
        /// yet.
        /// </summary>
        public bool Initialized { get; private set; }

        /// <summary>
        /// Call the InitializeCompleted event.
        /// </summary>
        protected void OnInitializeCompleted()
        {
            if (InitializeCompleted != null)
            {
                InitializeCompleted(this, EventArgs.Empty);
            }
        }

        /// <summary>
        /// Performs a callback.  Null action and/or result are permitted.
        /// </summary>
        /// <param name="action">The optional callback action.</param>
        /// <param name="result">The result to pass back.</param>
        protected static void Callback(Action<ServiceResult> action, ServiceResult result)
        {
            if (action != null)
            {
                action(result);
            }
        }
    }
}