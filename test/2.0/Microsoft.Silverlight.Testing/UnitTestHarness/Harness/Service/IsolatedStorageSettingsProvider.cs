// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

// NOTE: This file should only be included in builds targeted for Silverlight.

using System;
using System.Collections.Generic;
using System.IO;
using System.IO.IsolatedStorage;
using System.Text;

namespace Microsoft.Silverlight.Testing.Harness.Service
{
    /// <summary>
    /// A type that stores global settings in the isolated storage for the 
    /// application. An implementation of the <see cref='SettingsProvider' /> 
    /// type.
    /// </summary>
    public class IsolatedStorageSettingsProvider : SettingsProvider
    {
        /// <summary>
        /// The unique key used for storing the test framework's settings 
        /// dictionary.
        /// </summary>
        private const string TestFrameworkSettingsKeyName = "_TestFx_Settings";

        /// <summary>
        /// The underlying settings object.
        /// </summary>
        private IsolatedStorageSettings _settings;

        /// <summary>
        /// Initializes a new isolated storage settings provider.
        /// </summary>
        /// <param name="testService">The test service instance.</param>
        public IsolatedStorageSettingsProvider(TestServiceProvider testService)
            : base(testService, "Isolated Storage Settings")
        {
            SourceName = "Local application storage";
        }

        /// <summary>
        /// Initializes the isolated storage settings provider.
        /// </summary>
        public override void Initialize()
        {
            InitializeSettings();
            base.Initialize();
        }

        /// <summary>
        /// Saves the current settings values.
        /// </summary>
        /// <param name="callback">The service completion callback.</param>
        public override void SaveSettings(Action<ServiceResult> callback)
        {
            ServiceResult result;
            try
            {
                if (_settings != null)
                {
                    _settings.Save();
                }

                result = null;
            }
            catch (IsolatedStorageException e)
            {
                result = ServiceResult.CreateExceptionalResult(e);
            }
            callback(result);
        }

        /// <summary>
        /// Recalls the stored settings values from isolated storage.
        /// </summary>
        private void LoadSettings()
        {
            IDictionary<string, string> stored = (IDictionary<string, string>)_settings[TestFrameworkSettingsKeyName];
            Settings.Clear();
            foreach (string key in stored.Keys)
            {
                Settings[key] = stored[key];
            }
        }

        /// <summary>
        /// Initialize the isolated storage application settings object.
        /// </summary>
        private void InitializeSettings()
        {
            _settings = IsolatedStorageSettings.SiteSettings;
            if (_settings.Contains(TestFrameworkSettingsKeyName))
            {
                LoadSettings();
            }
            else
            {
                _settings[TestFrameworkSettingsKeyName] = Settings;
            }

            // Isolated storage provides the ability to save settings
            IsReadOnly = false;
        }
    }
}