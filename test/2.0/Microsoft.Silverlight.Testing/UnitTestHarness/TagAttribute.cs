// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Globalization;
using System.Linq;

namespace Microsoft.Silverlight.Testing
{
    /// <summary>
    /// Tag attribute used to associate individual test cases with tags to
    /// easily test related functionality.
    /// </summary>
    /// <remarks>
    /// The infrastructure associated with the TagAttribute is not yet in place.
    /// </remarks>
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Method, AllowMultiple = true, Inherited = true)]
    public sealed partial class TagAttribute : Attribute
    {
        /// <summary>
        /// List of reserved words that cannot be used as tags.
        /// </summary>
        /// <remarks>
        /// This list should be shared with the tag parser implementation when
        /// available.
        /// </remarks>
        private static string[] ReservedWords = new string[] { "All" };

        /// <summary>
        /// List of reserved characters that cannot be used in tags.
        /// </summary>
        /// <remarks>
        /// This list should be shared with the tag parser implementation when
        /// available.
        /// </remarks>
        private static char[] ReservedCharacters = new char[] { '+', '-', '*', '!', '(', ')' };

        /// <summary>
        /// Gets the tag associated with the test method or class.
        /// </summary>
        public string Tag { get; private set; }

        /// <summary>
        /// Initializes a new instance of the TagAttribute class.
        /// </summary>
        /// <param name="tag">
        /// Tag associated with the test method or class.
        /// </param>
        public TagAttribute(string tag)
        {
            if (tag == null)
            {
                throw new ArgumentNullException("tag");
            }
            else if (tag.Length == 0)
            {
                throw new ArgumentException(Properties.UnitTestMessage.TagAttribute_ctor_EmptyTag, "tag");
            }
            else if (ReservedWords.Contains(tag, StringComparer.OrdinalIgnoreCase))
            {
                throw new ArgumentException(
                    string.Format(CultureInfo.InvariantCulture, Properties.UnitTestMessage.TagAttribute_ctor_ReservedTag, tag),
                    "tag");
            }
            
            int invaldIndex = tag.IndexOfAny(ReservedCharacters);
            if (invaldIndex >= 0)
            {
                throw new ArgumentException(
                    string.Format(CultureInfo.InvariantCulture, Properties.UnitTestMessage.TagAttribute_ctor_ReservedCharacter, tag, tag[invaldIndex]),
                    "tag");
            }

            Tag = tag;
        }
    }
}