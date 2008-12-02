// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using System.Linq;
using Microsoft.Silverlight.Testing.Harness.Service;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// A system that provides test services.
    /// </summary>
    public class TestServiceProvider : ProviderBase
    {
        /// <summary>
        /// The dictionary of services registered with this provider instance.
        /// </summary>
        private Dictionary<object, ProviderBase> _services;

        /// <summary>
        /// Gets or sets a unique test run identifier, if any is present.
        /// </summary>
        public string UniqueTestRunIdentifier { get; set; }

        /// <summary>
        /// Initializes a new test service provider instance.
        /// </summary>
        public TestServiceProvider() : base(null, "All Test Services")
        {
            _services = new Dictionary<object, ProviderBase>();
        }

        /// <summary>
        /// Initializes the provider and all of its test services.
        /// </summary>
        public override void Initialize()
        {
            InitializeAllServices();
        }

        /// <summary>
        /// Initialize all services and features synchronously.
        /// </summary>
        public void InitializeAllServices()
        {
            // Find and wire up the first un-initialized provider
            if (_services.Values.Count(i => ! i.Initialized) > 0)
            {
                ProviderBase pb = _services.Values.First(i => ! i.Initialized);
                pb.InitializeCompleted += (sender, e) => InitializeAllServices();
                pb.Initialize();
            }
            else
            {
                // Fires the initialization finished event
                base.Initialize();
            }
        }

        /// <summary>
        /// Register a new service that the test service should expose.
        /// </summary>
        /// <param name="feature">Known feature type.</param>
        /// <param name="serviceInstance">Instance of the feature's 
        /// <see cref='ProviderBase' /> type.</param>
        public void RegisterService(TestServiceFeature feature, ProviderBase serviceInstance)
        {
            _services.Add(feature, serviceInstance);
        }

        /// <summary>
        /// Register a new service that the test service should expose.
        /// </summary>
        /// <param name="featureName">String name of the feature if the known 
        /// enum value does not exist.</param>
        /// <param name="serviceInstance">Instance of the feature's 
        /// <see cref='ProviderBase' /> type.</param>
        public void RegisterService(string featureName, ProviderBase serviceInstance)
        {
            _services.Add(featureName, serviceInstance);
        }

        /// <summary>
        /// Unregisters a feature.
        /// </summary>
        /// <param name="feature">Known feature type.</param>
        public void UnregisterService(TestServiceFeature feature)
        {
            _services.Remove(feature);
        }

        /// <summary>
        /// Unregisters a feature.
        /// </summary>
        /// <param name="featureName">Known feature type string.</param>
        public void UnregisterService(string featureName)
        {
            _services.Remove(featureName);
        }

        /// <summary>
        /// Check if a requested feature is supported by the test service 
        /// provider.
        /// </summary>
        /// <param name="feature">Feature of interest.</param>
        /// <returns>A value indicating whether the feature exists.</returns>
        public bool HasService(TestServiceFeature feature)
        {
            return (GetServiceInternal(feature) != null);
        }

        /// <summary>
        /// Check if a requested feature is supported by the test service 
        /// provider.
        /// </summary>
        /// <param name="featureName">Feature of interest.</param>
        /// <returns>A value indicating whether the feature exists.</returns>
        public bool HasService(string featureName)
        {
            return (GetServiceInternal(featureName) != null);
        }

        /// <summary>
        /// Retrieve a feature.  An exception will be thrown if the service 
        /// does not exist.
        /// </summary>
        /// <typeparam name="TService">Type of a service, ProviderBase.</typeparam>
        /// <param name="feature">The feature of interest.</param>
        /// <returns>Returns the feature, cast properly.</returns>
        [SuppressMessage("Microsoft.Design", "CA1004:GenericMethodsShouldProvideTypeParameter", Justification = "Simplifies calling code.")]
        public TService GetService<TService>(TestServiceFeature feature)
            where TService : ProviderBase
        {
            RequireService(feature);
            //return GetService(feature) as TService;
            //workaround for bug #448560:
            object o = GetService (feature);
            object dummy = null;
            if (o is TService)
                return (TService) o;
            else
                return (TService) dummy;
        }

        /// <summary>
        /// Retrieve a feature.
        /// </summary>
        /// <param name="feature">Feature of interest.</param>
        /// <returns>The feature's provider.</returns>
        public ProviderBase GetService(TestServiceFeature feature)
        {
            return GetServiceInternal(feature);
        }

        /// <summary>
        /// Retrieve a feature.
        /// </summary>
        /// <param name="featureName">Feature of interest.</param>
        /// <returns>The service or null if one was not present.</returns>
        public ProviderBase GetService(string featureName)
        {
            return GetServiceInternal(featureName);
        }

        /// <summary>
        /// Require a feature, or throw an exception if it isn't available.
        /// </summary>
        /// <param name="feature">Feature of interest.</param>
        /// <param name="requiredType">The required type.</param>
        /// <returns>The service or null if one was not present.</returns>
        public ProviderBase RequireService(TestServiceProvider feature, Type requiredType)
        {
            return RequireServiceInternal(feature, requiredType);
        }

        /// <summary>
        /// Require a feature, or throw an exception if it isn't available.
        /// </summary>
        /// <param name="featureName">Feature of interest.</param>
        /// <param name="requiredType">The required type.</param>
        /// <returns>The service or null if one was not present.</returns>
        public ProviderBase RequireService(string featureName, Type requiredType)
        {
            return RequireServiceInternal(featureName, requiredType);
        }

        /// <summary>
        /// Require a feature or interest.
        /// </summary>
        /// <param name="feature">Feature of interest.</param>
        /// <returns>The service or null if one was not present.</returns>
        public ProviderBase RequireService(TestServiceFeature feature)
        {
            return RequireServiceInternal(feature);
        }

        /// <summary>
        /// Requires a service.
        /// </summary>
        /// <param name="featureName">Feature of interest.</param>
        /// <returns>The service or null if one was not present.</returns>
        public ProviderBase RequireService(string featureName)
        {
            return RequireServiceInternal(featureName);
        }

        /// <summary>
        /// Check for and required the presence of a service.  Throws an 
        /// InvalidOperationException message if the service is unavailable.
        /// </summary>
        /// <param name="feature">Feature of interest.</param>
        /// <returns>The service or null if one was not present.</returns>
        private ProviderBase RequireServiceInternal(object feature)
        {
            ProviderBase service = GetServiceInternal(feature);
            if (service == null)
            {
                throw new InvalidOperationException(
                    String.Format(CultureInfo.InvariantCulture, "The test service feature \"{0}\" is not available, and is required for an operation.", feature.ToString()));
            }
            return service;
        }

        /// <summary>
        /// Require a specific feature, and that it can be cast properly.
        /// </summary>
        /// <param name="feature">Feature of interest.</param>
        /// <param name="cast">The type to verify assignment for a cast.</param>
        /// <returns>The service or null if one was not present.</returns>
        private ProviderBase RequireServiceInternal(object feature, Type cast)
        {
            ProviderBase pb = RequireServiceInternal(feature);
            Type pbt = pb.GetType();
            if (!cast.IsAssignableFrom(pbt))
            {
                throw new InvalidOperationException("A required test service feature was present, but not of the proper type for this application.");
            }
            return pb;
        }

        /// <summary>
        /// Looks for a specific service.
        /// </summary>
        /// <param name="feature">Feature of interest.</param>
        /// <returns>The service or null if one was not present.</returns>
        private ProviderBase GetServiceInternal(object feature)
        {
            if (_services.ContainsKey(feature))
            {
                return _services[feature];
            }
            return null;
        }
    }
}