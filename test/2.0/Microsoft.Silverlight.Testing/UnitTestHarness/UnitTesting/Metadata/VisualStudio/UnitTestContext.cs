// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
using System.Globalization;
using VS = Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Microsoft.Silverlight.Testing.UnitTesting.Metadata.VisualStudio
{
    /// <summary>
    /// A wrapper for the unit test context capability of Visual Studio Team 
    /// Test's unit test framework.
    /// </summary>
    public class UnitTestContext : VS.TestContext
    {
        /// <summary>
        /// Create a new unit test context wrapper for the test method.
        /// </summary>
        /// <param name="testMethod">Test method.</param>
        internal UnitTestContext(TestMethod testMethod)
        {
            _tm = testMethod;
        }

        /// <summary>
        /// Constructor of a new unit test context.
        /// </summary>
        private UnitTestContext() { }

        /// <summary>
        /// Test method.
        /// </summary>
        private TestMethod _tm;

        /// <summary>
        /// Used to write trace messages while the test is running.
        /// </summary>
        /// <param name="format">Format string.</param>
        /// <param name="args">The arguments.</param>
        public override void WriteLine(string format, params object[] args)
        {
            string s = args.Length == 0 ? format : String.Format(CultureInfo.InvariantCulture, format, args);
            _tm.OnWriteLine(s);
        }

        /// <summary>
        /// Throw a not supported exception.
        /// </summary>
        /// <param name="functionality">Functionality that is not supported (string).</param>
        /// <returns>A new NotSupportedException.</returns>
        private static Exception NotSupportedException(string functionality)
        {
            return new NotSupportedException(String.Format(CultureInfo.InvariantCulture, Microsoft.Silverlight.Testing.Properties.UnitTestMessage.UnitTestContext_FeatureNotSupported, functionality));
        }

        /// <summary>
        /// Adds a file name to the list in TestResult.ResultFileNames.
        /// </summary>
        /// <param name="fileName">Filename to add as a result.</param>
        public override void AddResultFile(string fileName)
        {
            throw NotSupportedException("AddResultFile");
        }

        /// <summary>
        /// Begins a timer with the specified name.
        /// </summary>
        /// <param name="timerName">The name of the timer to create.</param>
        public override void BeginTimer(string timerName)
        {
            throw NotSupportedException("BeginTimer");
        }

        /// <summary>
        /// Ends a timer with the specified name.
        /// </summary>
        /// <param name="timerName">Name of the timer.</param>
        public override void EndTimer(string timerName)
        {
            throw NotSupportedException("EndTimer");
        }

        /// <summary>
        /// Gets test properties.
        /// </summary>
        public override IDictionary Properties
        {
            get
            {
                if (_propertyCache == null)
                {
                    _propertyCache = new Dictionary<string, string>();
                    foreach (var prop in _tm.Properties)
                    {
                        _propertyCache.Add(prop.Name, prop.Value);
                    }
                }
                return _propertyCache;
            }
        }

        /// <summary>
        /// Cache of properties.
        /// </summary>
        private Dictionary<string, string> _propertyCache;

        /// <summary>
        /// Gets current data row when test is used for data driven testing.
        /// </summary>
        public override DataRow DataRow
        {
            get { throw NotSupportedException("DataRow"); }
        }

        /// <summary>
        /// Gets current data connection row when test is used for data driven testing.
        /// </summary>
        public override DbConnection DataConnection
        {
            get { throw NotSupportedException("DataConnection"); }
        }

        /// <summary>
        /// Gets the name of the test method.
        /// </summary>
        public override string TestName
        {
            get { return _tm.Name; }
        }

        /// <summary>
        /// Gets the current enum outcome - passed, failed, or inconclusive.
        /// </summary>
        public override VS.UnitTestOutcome CurrentTestOutcome
        {
            get { return VS.UnitTestOutcome.Unknown; }
        }
    }
}