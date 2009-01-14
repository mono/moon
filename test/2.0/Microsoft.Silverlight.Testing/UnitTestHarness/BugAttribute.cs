// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;

namespace Microsoft.Silverlight.Testing
{
    /// <summary>
    /// An attribute that contains known bug information that can be logged 
    /// during a unit test run. 
    /// 
    /// When applied to a test method, it will invert the result. This makes it 
    /// possible to perform automated runs and continuous integration while 
    /// validating and tracking known issues.
    /// 
    /// The attribute should have the Fixed bool set to True, or be removed, 
    /// once the issue is resolved.
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
    public class BugAttribute : Attribute
    {
        /// <summary>
        /// Gets the bug description.
        /// </summary>
        public string Description { get; private set; }

        /// <summary>
        /// Gets or sets a value indicating whether the known issue has been 
        /// fixed. If it has, the attribute is ignored and will not affect 
        /// test results.
        /// </summary>
        public bool Fixed { get; set; }

        /// <summary>
        /// Gets or sets a value indicating on which platforms this is 
        /// a known issue.
        /// </summary>
        public PlatformID [] Platforms { get; set; }
        
        /// <summary>
        /// Construct a new BugAttribute with no bug description.
        /// </summary>
        public BugAttribute() { }

        /// <summary>
        /// Construct a new BugAttribute with no bug description and optionally only applicable on certain platforms.
        /// </summary>
        public BugAttribute(params PlatformID [] platforms)
        {
            Platforms = platforms;
        }

        /// <summary>
        /// Construct a new BugAttribute with a bug description and optionally only applicable on certain platforms.
        /// </summary>
        /// <param name="description">Known issue text.</param>
        public BugAttribute(string description, params PlatformID [] platforms) 
        {
            Description = description;
            Platforms = platforms;
        }

        /// <summary>
        /// Return the bug information.
        /// </summary>
        /// <returns>Known issue as a string.</returns>
        public override string ToString()
        {
            return Fixed ? "Fixed: " + Description : Description;
        }
    }
}
