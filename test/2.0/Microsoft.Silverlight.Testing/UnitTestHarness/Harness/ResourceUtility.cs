// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Globalization;
using System.IO;
using System.Text;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// Helper functionality for dealing with embedded resources in an assembly.
    /// </summary>
    public static class ResourceUtility
    {
        /// <summary>
        /// Get a string containing the text of an embedded resource in an
        /// assembly.
        /// </summary>
        /// <param name="instance">
        /// An instance whose assembly can be retrieved through reflection.
        /// </param>
        /// <param name="fullEmbeddedResourceName">
        /// Complete embedded resource name.
        /// </param>
        /// <returns>
        /// A string containing the embedded resource's string contents.
        /// </returns>
        public static string GetAllText(object instance, string fullEmbeddedResourceName)
        {
            if (instance == null)
            {
                throw new ArgumentNullException("instance");
            }

            Type type = instance.GetType();
            using (Stream stream = type.Assembly.GetManifestResourceStream(fullEmbeddedResourceName))
            {
                if (stream != null)
                {
                    using (StreamReader reader = new StreamReader(stream))
                    {
                        return reader.ReadToEnd();
                    }
                }
            }
            
            StringBuilder message = new StringBuilder();
            message.AppendFormat(
                CultureInfo.InvariantCulture,
                "Embedded resource \"{0}\" not found in type \"{1}\"!",
                fullEmbeddedResourceName,
                type.Name);
            message.AppendLine();
            message.AppendLine();

            string[] embedded = type.Assembly.GetManifestResourceNames();
            foreach (string resource in embedded) 
            {
                message.AppendLine(
                    string.Format(CultureInfo.InvariantCulture, "\t{0} (Embedded Resource)", resource));
            }
                
            throw new FileNotFoundException(message.ToString());
        }
    }
}