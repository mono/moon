// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Linq;
using System.Reflection;

namespace System.Windows.Controls.Data.Test
{
    [AttributeUsage(AttributeTargets.Property)]
    public class PropertyTestExpectedResultsAttribute : Attribute
    {
        public string TestId { get; set; }
        public bool IsReadOnly { get; set; }

        public static PropertyTestExpectedResultsAttribute GetExpectedResults(PropertyInfo propertyInfo, string testId)
        {
            PropertyTestExpectedResultsAttribute result = propertyInfo.GetCustomAttributes(true).OfType<PropertyTestExpectedResultsAttribute>().SingleOrDefault(attr => attr.TestId == testId);

            if (result == null)
            {
                result = new PropertyTestExpectedResultsAttribute();
            }

            return result;
        }
    }
}
