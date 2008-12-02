// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics.CodeAnalysis;
using System.Globalization;
using System.Windows.Browser;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// Set of extensions for working with the helper enums, values, 
    /// types, and strings for use on web pages.
    /// </summary>
    public static class Extensions
    {
        /// <summary>
        /// Append the managed HtmlControl to the HtmlElement.  Extension method
        /// for HtmlElement.
        /// </summary>
        /// <param name="element">The HtmlElement.</param>
        /// <param name="control">The managed HtmlControlBase / HtmlControl.</param>
        public static void AppendChild(this HtmlElement element, HtmlControlBase control)
        {
            HtmlControlBase.AppendAndInitialize(element, control);
        }

        /// <summary>
        /// Returns the ScriptProperty name.  This is a camel-case value. Only 
        /// alters the first character.
        /// </summary>
        /// <param name="property">The property name.</param>
        /// <returns>Returns the value, as a string, with the first letter 
        /// capitalized.</returns>
        public static string ScriptPropertyName(this HtmlProperty property)
        {
            return CamelCase(property.ToString());
        }

        /// <summary>
        /// A plural 's' as the suffix, when not equal to one.
        /// </summary>
        /// <param name="value">The string value.</param>
        /// <param name="number">The number to check.</param>
        /// <returns>Returns an empty string or the English plural 's'.</returns>
        public static string Plural(this string value, int number)
        {
            return number != 1 ? value + "s" : value;
        }

        /// <summary>
        /// Replace any underscores with dashes.
        /// </summary>
        /// <param name="s">The input string.</param>
        /// <returns>Returns a string's underscores with dashes.</returns>
        private static string ReplaceUnderscores(string s)
        {
            return s.Replace('_', '-');
        }

        /// <summary>
        /// Returns a property name for use in browser scripting engines. This 
        /// is as a string value, with the first character lowercase.
        /// </summary>
        /// <param name="enumValue">The enumeration value.</param>
        /// <returns>Returns the value of the current enumeration item, with the
        /// first character lowercase.</returns>
        public static string ScriptPropertyName(this Enum enumValue)
        {
            return CamelCase(enumValue.ToString());
        }

        /// <summary>
        /// Used in helping to camel-case a string.  This will make the first 
        /// character of a string lowercase.
        /// </summary>
        /// <param name="value">The input string.</param>
        /// <returns>Returns the string, with the first character lowercase.</returns>
        public static string CamelCase(this string value)
        {
            return ModifyFirstCharacterCase(value, false);
        }

        /// <summary>
        /// Camel case and replaces underscores in the HtmlAttribute enum.
        /// </summary>
        /// <param name="attribute">Attribute object.</param>
        /// <returns>Returns the string.</returns>
        public static string CamelCaseAndReplace(this HtmlAttribute attribute)
        {
            return ReplaceUnderscores(attribute.ToString()).CamelCase();
        }

        /// <summary>
        /// Capitalizes the first character in a string.
        /// </summary>
        /// <param name="value">The input string.</param>
        /// <returns>Returns the string, with the first character 
        /// capitalized.</returns>
        public static string CapitalizeFirstCharacter(this string value)
        {
            return ModifyFirstCharacterCase(value, true);
        }

        /// <summary>
        /// Modifies the capitalization of the string.  The first character will
        /// be capitalized, or lowercase, depending on the passed-in bool value 
        /// that is the second character.
        /// </summary>
        /// <param name="value">The input string.</param>
        /// <param name="capitalize">A value indicating whether to capitalize 
        /// the first character or not.  Setting to false will un-capitalize 
        /// (lowercase) the same character instead.</param>
        /// <returns>Returns the string, with a modified first character.</returns>
        [SuppressMessage("Microsoft.Globalization", "CA1308:NormalizeStringsToUppercase", Justification = "The purpose of the method is to modify the case one way or the other.")]
        private static string ModifyFirstCharacterCase(string value, bool capitalize)
        {
            if (!String.IsNullOrEmpty(value))
            {
                string first = capitalize ? 
                    value.Substring(0, 1).ToUpper(CultureInfo.InvariantCulture) 
                    : value.Substring(0, 1).ToLower(CultureInfo.InvariantCulture);
                value = first + value.Substring(1);
            }
            return value;
        }

        /// <summary>
        /// Returns the border style enum as a script property name.
        /// </summary>
        /// <param name="style">The style enum value.</param>
        /// <returns>Returns a script-ready name for the property.</returns>
        public static string CssPropertyName(this BorderStyle style)
        {
            return style.ToString().CamelCase();
        }
    }
}