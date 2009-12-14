// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.Generic;
using System.Globalization;
using System.Reflection;
using System.Text;
using System.Windows.Media;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace System.Windows.Controls.Test
{
    /// <summary>
    /// Common testing utilities and constants.
    /// </summary>
    public static class Common
    {
        /// <summary>
        /// Delta used for high precision assertions.
        /// </summary>
        public const double HighPrecisionDelta = 0.000000000001;

        /// <summary>
        /// Standard number of iterations to execute timing tests.
        /// </summary>
        public const int NumberOfIterationsForTiming = 100;

        /// <summary>
        /// Measure the duration of an action in milliseconds using a low
        /// accuracy timing method.
        /// </summary>
        /// <param name="action">Action to measure.</param>
        /// <returns>Duration of the action in milliseconds.</returns>
        public static double MeasureTestDuration(Action action)
        {
            if (action == null)
            {
                throw new ArgumentNullException("action");
            }

            DateTime start = DateTime.UtcNow;
            action();
            DateTime end = DateTime.UtcNow;

            return (end - start).TotalMilliseconds;
        }

        /// <summary>
        /// Ensure the action completes within the specified duration (in
        /// milliseconds).
        /// </summary>
        /// <param name="duration">
        /// Maximum allowable duration of the action in milliseconds.
        /// </param>
        /// <param name="action">
        /// Action to complete within the specified duration.
        /// </param>        
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode", Justification = "Most tests are still unwritten.")]
        public static void AssertTestDuration(double duration, Action action)
        {
            double actualDuration = MeasureTestDuration(action);
            if (actualDuration > duration)
            {
                throw new AssertFailedException(string.Format(
                    CultureInfo.InvariantCulture,
                    "Action completed in {0}ms ({2}ms longer than the maximum allowable {1}ms).",
                    actualDuration,
                    duration,
                    actualDuration - duration));
            }
        }

        /// <summary>
        /// Ensure the action completes within the specified average duration
        /// (in milliseconds) across the desired number of iterations.
        /// </summary>
        /// <param name="duration">
        /// Maximum allowable average duration of the action in milliseconds.
        /// </param>
        /// <param name="action">
        /// Action to complete within the specified duration.
        /// </param>        
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode", Justification = "Most tests are still unwritten.")]
        public static void AssertAverageDuration(double duration, Action action)
        {
            AssertAverageDuration(duration, NumberOfIterationsForTiming, action);
        }

        /// <summary>
        /// Ensure the action completes within the specified average duration
        /// (in milliseconds) across the desired number of iterations.
        /// </summary>
        /// <param name="duration">
        /// Maximum allowable average duration of the action in milliseconds.
        /// </param>
        /// <param name="iterations">
        /// Number of iterations to measure the test duration.
        /// </param>
        /// <param name="action">
        /// Action to complete within the specified duration.
        /// </param>        
        public static void AssertAverageDuration(double duration, int iterations, Action action)
        {
            if (iterations <= 0)
            {
                throw new ArgumentException("iterations must be greater than 0.", "iterations");
            }

            double totalDuration = 0;
            for (int i = 0; i < iterations; i++)
            {
                totalDuration += MeasureTestDuration(action);
            }

            double averageDuration = totalDuration / ((double) iterations);
            if (averageDuration > duration)
            {
                throw new AssertFailedException(string.Format(
                    CultureInfo.InvariantCulture,
                    "Action completed in {0}ms on average over {2} iterations ({3}ms longer than the maximum allowable {1}ms).",
                    averageDuration,
                    duration,
                    iterations,
                    averageDuration - duration));
            }
        }

        /// <summary>
        /// Create a reference to the element that will be added to the testing
        /// surface and then removed when the reference is disposed.
        /// </summary>
        /// <param name="element">Element create the reference for.</param>
        /// <returns>LiveReference to track the element.</returns>
        public static LiveReference CreateLiveReference(this UIElement element, Microsoft.Silverlight.Testing.SilverlightTest testReference)
        {
            return new LiveReference(testReference, element);
        }

        /// <summary>
        /// Get the template parts declared on a type.
        /// </summary>
        /// <param name="controlType">Type with template parts defined.</param>
        /// <returns>Template parts defined on the type.</returns>
        public static IDictionary<string, Type> GetTemplateParts(this Type controlType)
        {
            Dictionary<string, Type> templateParts = new Dictionary<string, Type>();
            foreach (Attribute attribute in controlType.GetCustomAttributes(typeof(TemplatePartAttribute), false))
            {
                TemplatePartAttribute templatePart = attribute as TemplatePartAttribute;
                if (templatePart != null)
                {
                    templateParts.Add(templatePart.Name, templatePart.Type);
                }
            }
            return templateParts;
        }

        /// <summary>
        /// Assert that a template part is defined.
        /// </summary>
        /// <param name="templateParts">Template parts defined on a type.</param>
        /// <param name="name">Name of the template part.</param>
        /// <param name="type">Type of the template part.</param>
        public static void AssertTemplatePartDefined(this IDictionary<string, Type> templateParts, string name, Type type)
        {
            Assert.IsNotNull(templateParts);
            Assert.IsTrue(templateParts.ContainsKey(name),
                "No template part named {0} was defined!", name);
            Assert.AreEqual(type, templateParts[name],
                "The template part {0} is of type {1}, not {2}!", name, templateParts[name].FullName, type.FullName);
        }

        /// <summary>
        /// Determines if the specified brushes are equal.
        /// </summary>
        /// <param name="first">The first brush to compare.</param>
        /// <param name="second">The second brush to compare.</param>
        /// <returns>True if the brushes are equal, false otherwise.</returns>
        public static bool AreBrushesEqual(Brush first, Brush second)
        {
            // If the default comparison is true, that's good enough.
            if (object.Equals(first, second))
            {
                return true;
            }

            // Do a field by field comparison if they're not the same reference
            // 
            SolidColorBrush firstSolidColorBrush = first as SolidColorBrush;
            if (firstSolidColorBrush != null)
            {
                SolidColorBrush secondSolidColorBrush = second as SolidColorBrush;
                if (secondSolidColorBrush != null)
                {
                    return object.Equals(firstSolidColorBrush.Color, secondSolidColorBrush.Color);
                }
            }

            return false;
        }

        /// <summary>
        /// Verifies that the specified brushes are equal.
        /// </summary>
        /// <param name="expected">
        /// The first brush to compare.  This is the brush the unit test
        /// expects.
        /// </param>
        /// <param name="actual">
        /// The second brush to compare.  This is the brush the unit test
        /// produced.
        /// </param>
        public static void AssertBrushesAreEqual(Brush expected, Brush actual)
        {
            if (!AreBrushesEqual(expected, actual))
            {
                throw new AssertFailedException(string.Format(CultureInfo.InvariantCulture,
                    "Brushes are not equal.  Expected:{0}.  Actual:{1}.",
                    expected,
                    actual));
            }
        }

        /// <summary>
        /// Verifies that the test delegates raise the specified exception.
        /// </summary>
        /// <typeparam name="TException">Type of exception</typeparam>
        /// <param name="exceptionPrototype">Exception prototype, with the expected exception message populated.</param>
        /// <param name="tests">Action delegates to expect exceptions from.</param>
        public static void AssertExpectedException<TException>(TException exceptionPrototype, params Action[] tests)
            where TException : Exception
        {
            foreach (var test in tests)
            {
                AssertExpectedException<TException>(exceptionPrototype, test);
            }
        }

        /// <summary>
        /// Verifies that the test delegate raises the specified exception.
        /// </summary>
        /// <typeparam name="TException">Type of exception</typeparam>
        /// <param name="exceptionPrototype">Exception prototype, with the expected exception message populated.</param>
        /// <param name="test">Action delegate to expect exception from.</param>
        public static void AssertExpectedException<TException>(TException exceptionPrototype, Action test)
            where TException : Exception
        {
            TException exception = null;

            try
            {
                test();
            }
            catch (TException e)
            {
                // looking for exact matches
                if (e.GetType() == typeof(TException))
                {
                    exception = e;
                }
            }

            if (exception == null)
            {
                Assert.Fail("Expected {0} with message \"{1}\". \nActual: none.", typeof(TException).FullName, exceptionPrototype.Message);
            }
            else if (exception.Message != exceptionPrototype.Message)
            {
                Assert.Fail("Expected {0} with message \"{1}\". \nActual: {2} => \"{3}\".", typeof(TException).FullName, exceptionPrototype.Message, exception.GetType().FullName, exception.Message);
            }
        }

        public static void AssertExpectedExceptionWithoutMessageControl<TException>(TException exceptionPrototype, Action test)
            where TException : Exception
        {
            TException exception = null;

            try
            {
                test();
            }
            catch (TException e)
            {
                // looking for exact matches
                if (e.GetType() == typeof(TException))
                {
                    exception = e;
                }
            }

            if (exception == null)
            {
                Assert.Fail("Expected {0}. \nActual: none.", typeof(TException).FullName);
            }
        }

        static Random random = new Random();

        public static string RandomString(int length)
        {
            StringBuilder stringBuilder = new StringBuilder();
            for (int i = 0; i < length; i++)
            {
                stringBuilder.Append((char)random.Next(65, 90));
            }
            return stringBuilder.ToString();
        }

        public static bool RandomBoolean()
        {
            return (random.Next(0, 2) == 0 ? true : false);
        }

        public static int RandomInt32()
        {
            return (random.Next());
        }

        public static int RandomInt32(int minValue, int maxValue)
        {
            return (random.Next(minValue, maxValue));
        }

        public static double RandomDouble()
        {
            return (random.NextDouble());
        }

        public static decimal RandomDecimal()
        {
            return (new decimal(RandomDouble() / RandomInt32()));
        }

        public static DateTime RandomDateTime()
        {
            return new DateTime(random.Next(1800, 3000), random.Next(1, 12), random.Next(1, 28));
        }

        public static T RandomEnum<T>()
        {
            if (typeof(T).IsEnum)
            {
                FieldInfo[] enumValues = typeof(T).GetFields(BindingFlags.Public | BindingFlags.Static);
                int randomInt = RandomInt32(0, enumValues.Length);
                return ((T)(Enum.Parse(typeof(T), enumValues[randomInt].Name, false)));
            }
            else
            {
                throw new ArgumentException("Type passed is not an enum.");
            }
        }

        public static object RandomEnumFromNullableEnum<T>()
        {
            Assert.IsTrue(typeof(T).IsGenericType);
            Assert.IsTrue(typeof(T).GetGenericTypeDefinition() == typeof(Nullable<>));
            Assert.AreEqual(1, typeof(T).GetGenericArguments().Length);
            Assert.IsTrue(typeof(T).GetGenericArguments()[0].IsEnum);

            FieldInfo[] enumValues = typeof(T).GetGenericArguments()[0].GetFields(BindingFlags.Public | BindingFlags.Static);
            int randomInt = RandomInt32(0, enumValues.Length);
            return Enum.Parse(typeof(T).GetGenericArguments()[0], enumValues[randomInt].Name, false);
        }
        
        public static SByte RandomSByte()
        {
            return (sbyte)random.Next(-128, 127);
        }

        public static object RandomTypeValue<TRandomType>()
        {
            object returnValue = null;
            Type objectType = typeof(TRandomType);
            if (objectType == typeof(string))
            {
                returnValue = RandomString(RandomInt32(10, 20));
            }
            else if (objectType == typeof(bool))
            {
                returnValue = RandomBoolean();
            }
            else if (objectType == typeof(bool?))
            {
                returnValue = RandomInt32(1, 3) == 1 ? (bool?)null : RandomBoolean();
            }
            else if (objectType == typeof(SByte))
            {
                returnValue = RandomSByte();
            }
            else if (objectType == typeof(SByte?))
            {
                returnValue = RandomBoolean() ? RandomSByte() : (SByte?)null;
            }
            else if (objectType == typeof(int))
            {
                returnValue = RandomInt32();
            }
            else if (objectType == typeof(int?))
            {
                returnValue = RandomInt32(1, 3) == 1 ? (int?)null : RandomInt32();
            }
            else if (objectType == typeof(double))
            {
                returnValue = RandomDouble();
            }
            else if (objectType == typeof(double?))
            {
                returnValue = RandomInt32(1, 3) == 1 ? (double?)null : RandomDouble();
            }
            else if (objectType == typeof(decimal))
            {
                returnValue = RandomDecimal();
            }
            else if (objectType == typeof(decimal?))
            {
                returnValue = RandomInt32(1, 3) == 1 ? (decimal?)null : RandomDecimal();
            }
            else if (objectType == typeof(DateTime))
            {
                returnValue = RandomDateTime();
            }
            else if (objectType == typeof(DateTime?))
            {
                returnValue = RandomInt32(1, 3) == 1 ? (DateTime?)null : RandomDateTime();
            }
            else if (objectType.IsEnum)
            {
                returnValue = RandomEnum<TRandomType>();
            }
            //Nullable enum
            else if (objectType.IsGenericType &&
                        objectType.GetGenericTypeDefinition() == typeof(Nullable<>) &&
                        objectType.GetGenericArguments().Length == 1 &&
                        objectType.GetGenericArguments()[0].IsEnum)
            {
                if (RandomInt32(1, 3) == 1)
                {
                    returnValue = null;
                }
                else
                {
                    returnValue = RandomEnumFromNullableEnum<TRandomType>();
                }
            }
            else
            {
                returnValue = null;
            }

            return returnValue;
        }
    }
}
