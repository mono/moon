// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.ComponentModel;
using System.Diagnostics.CodeAnalysis;
using System.Globalization;

namespace Microsoft.Silverlight.Testing.Html
{
    /// <summary>
    /// A struct that is a length measurement for web page use.
    /// </summary>
    public struct Unit
    {
        /// <summary>
        /// An Empty, unassigned Unit instance.
        /// </summary>
        public static readonly Unit Empty = new Unit();

        /// <summary>
        /// The type of unit.
        /// </summary>
        private readonly UnitType _type;

        /// <summary>
        /// The unit's value.
        /// </summary>
        private readonly double _value;

        /// <summary>
        /// Initializes a new instance of the Unit structure with the specified 
        /// int as the value and with Pixel as the unit's default type.
        /// </summary>
        /// <param name="value">The unit's value.</param>
        public Unit(int value)
        {
            _value = value;
            _type = UnitType.Pixel;
        }

        /// <summary>
        /// Initializes a new instance of the Unit struct with the double as the
        /// unit and Pixel as the unit's default type.
        /// </summary>
        /// <param name="value">The unit's value.</param>
        public Unit(double value)
        {
            _value = value;
            _type = UnitType.Pixel;
        }

        /// <summary>
        /// Initializes a new instance of the Unit struct with the specified 
        /// double number as the value and the specified UnitType as the unit 
        /// type.
        /// </summary>
        /// <param name="value">The unit's value.</param>
        /// <param name="type">The type of the unit.</param>
        public Unit(double value, UnitType type)
        {
            _type = type;
            if (type == UnitType.Pixel)
            {
                _value = (int)value;
                return;
            }
            _value = value;
        }

        /// <summary>
        /// Initializes a new instance of the Unit struct with the specified 
        /// text string that contains the unit value and unit type.  The default
        /// unit type is pixel.
        /// </summary>
        /// <param name="value">The unit's value.</param>
        public Unit(string value)
            : this(value, CultureInfo.CurrentCulture, UnitType.Pixel)
        { }

        /// <summary>
        /// Initializes a new instance of the Unit struct with the specified 
        /// text string that contains the unit value and unit type.
        /// </summary>
        /// <param name="value">The unit's value.</param>
        /// <param name="culture">The culture type in use.</param>
        public Unit(string value, CultureInfo culture)
            : this(value, culture, UnitType.Pixel)
        { }

        /// <summary>
        /// Initializes a new Unit type with the given value, culture type and 
        /// UnitType values.
        /// </summary>
        /// <param name="value">The unit's value.</param>
        /// <param name="culture">The culture in use.</param>
        /// <param name="defaultType">The default UnitType to use.</param>
        [SuppressMessage("Microsoft.Globalization", "CA1304:SpecifyCultureInfo", MessageId = "System.String.ToLower", Justification = "Source ported and tested as is.")]
        internal Unit(string value, CultureInfo culture, UnitType defaultType)
        {
            if ((value == null) || (value.Length == 0))
            {
                _value = 0;
                _type = (UnitType)0;
            }
            else
            {
                if (culture == null)
                {
                    culture = CultureInfo.CurrentCulture;
                }

                string trimLcase = value.Trim().ToLower();
                int len = trimLcase.Length;

                int lastDigit = -1;
                for (int i = 0; i < len; i++)
                {
                    char ch = trimLcase[i];
                    if (((ch < '0') || (ch > '9')) && (ch != '-') && (ch != '.') && (ch != ','))
                    {
                        break;
                    }
                    lastDigit = i;
                }
                if (lastDigit == -1)
                {
                    throw new FormatException("No digits parsed.");
                }
                if (lastDigit < len - 1)
                {
                    _type = (UnitType)GetTypeFromString(trimLcase.Substring(lastDigit + 1).Trim());
                }
                else
                {
                    _type = defaultType;
                }

                string numericPart = trimLcase.Substring(0, lastDigit + 1);
                // Cannot use Double.FromString
                try
                {
                    _value = double.Parse(numericPart, culture);
                    if (_type == UnitType.Pixel)
                    {
                        _value = (int)_value;
                    }
                }
                catch
                {
                    throw new FormatException("Numeric parsing failure.");
                }
            }
        }

        /// <summary>
        /// Gets a value indicating whether the Unit is empty.
        /// </summary>
        public bool IsEmpty
        {
            get { return _type == (UnitType)0; }
        }

        /// <summary>
        /// Gets the type of the Unit.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1721:PropertyNamesShouldNotMatchGetMethods", Justification = "This property maintains compatibility with the framework's Unit type.")]
        public UnitType Type
        {
            get
            {
                return IsEmpty ? UnitType.Pixel : _type;
            }
        }

        /// <summary>
        /// Gets the value of the Unit.
        /// </summary>
        public double Value
        {
            get { return _value; }
        }

        /// <summary>
        /// Gets the hash code for the value.
        /// </summary>
        /// <returns>Returns the hash code for the value.</returns>
        public override int GetHashCode()
        {
            return _type.GetHashCode() << 2 ^ _value.GetHashCode();
        }

        /// <summary>
        /// Compares this Unit with the specified object.
        /// </summary>
        /// <param name="obj">The object to compare.</param>
        /// <returns>Returns whether they are equal.</returns>
        public override bool Equals(object obj)
        {
            if (obj == null || !(obj is Unit))
            {
                return false;
            }

            Unit u = (Unit)obj;
            return (u._type == _type && u._value == _value);
        }

        /// <summary>
        /// Compares two units to find out if they have the same value and type.
        /// </summary>
        /// <param name="left">The left Unit.</param>
        /// <param name="right">The right Unit.</param>
        /// <returns>Returns whether they are equal.</returns>
        public static bool operator ==(Unit left, Unit right)
        {
            return (left._type == right._type && left._value == right._value);
        }

        /// <summary>
        /// Compares two units to find out if they have different values and/or 
        /// types.
        /// </summary>
        /// <param name="left">The left Unit.</param>
        /// <param name="right">The right Unit.</param>
        /// <returns>Returns whether they are not equal.</returns>
        public static bool operator !=(Unit left, Unit right)
        {
            return (left._type != right._type || left._value != right._value);
        }

        /// <summary>
        /// Converts UnitType to persistence string.
        /// </summary>
        /// <param name="type">The type of unit.</param>
        /// <returns>Returns the string value for the UnitType.</returns>
        private static string GetStringFromType(UnitType type)
        {
            switch (type)
            {
                case UnitType.Pixel:
                    return "px";
                case UnitType.Point:
                    return "pt";
                case UnitType.Pica:
                    return "pc";
                case UnitType.Inch:
                    return "in";
                case UnitType.Mm:
                    return "mm";
                case UnitType.Cm:
                    return "cm";
                case UnitType.Percentage:
                    return "%";
                case UnitType.Em:
                    return "em";
                case UnitType.Ex:
                    return "ex";
            }
            return String.Empty;
        }

        /// <summary>
        /// Converts persistence string to UnitType.
        /// </summary>
        /// <param name="value">The string value to convert.</param>
        /// <returns>Returns a UnitType object.</returns>
        private static UnitType GetTypeFromString(string value)
        {
            if (value != null && value.Length > 0)
            {
                if (value.Equals("px"))
                {
                    return UnitType.Pixel;
                }
                else if (value.Equals("pt"))
                {
                    return UnitType.Point;
                }
                else if (value.Equals("%"))
                {
                    return UnitType.Percentage;
                }
                else if (value.Equals("pc"))
                {
                    return UnitType.Pica;
                }
                else if (value.Equals("in"))
                {
                    return UnitType.Inch;
                }
                else if (value.Equals("mm"))
                {
                    return UnitType.Mm;
                }
                else if (value.Equals("cm"))
                {
                    return UnitType.Cm;
                }
                else if (value.Equals("em"))
                {
                    return UnitType.Em;
                }
                else if (value.Equals("ex"))
                {
                    return UnitType.Ex;
                }
                else
                {
                    throw new ArgumentOutOfRangeException("value");
                }
            }
            return UnitType.Pixel;
        }

        /// <summary>
        /// Parse a string into a new Unit instance.
        /// </summary>
        /// <param name="value">The string value.</param>
        /// <returns>Returns a new Unit struct.</returns>
        public static Unit Parse(string value)
        {
            return new Unit(value, CultureInfo.InvariantCulture);
        }

        /// <summary>
        /// Parse a string into a new Unit instance.
        /// </summary>
        /// <param name="value">The string value.</param>
        /// <param name="culture">The culture to use.</param>
        /// <returns>Returns a new Unit struct.</returns>
        public static Unit Parse(string value, CultureInfo culture)
        {
            return new Unit(value, culture);
        }

        /// <summary>
        /// Creates a Unit of type Percentage from the specified integer.
        /// </summary>
        /// <param name="number">The number value of the unit.</param>
        /// <returns>Returns a new Unit of a percentage value.</returns>
        public static Unit Percentage(double number)
        {
            return new Unit(number, UnitType.Percentage);
        }

        /// <summary>
        /// Creates a Unit of type Pixel from the specified integer.
        /// </summary>
        /// <param name="number">The number value of the unit.</param>
        /// <returns>Returns a new Unit of a pixel value.</returns>
        public static Unit Pixel(int number)
        {
            return new Unit(number);
        }

        /// <summary>
        /// Creates a Unit of type Point from the specified integer.
        /// </summary>
        /// <param name="number">The number value of the unit.</param>
        /// <returns>Returns a Unit of the point value.</returns>
        public static Unit Point(int number)
        {
            return new Unit(number, UnitType.Point);
        }

        /// <summary>
        /// Converts a Unit to a string.
        /// </summary>
        /// <returns>Returns a string version of the Unit.</returns>
        public override string ToString()
        {
            return ToString((IFormatProvider)CultureInfo.CurrentCulture);
        }

        /// <summary>
        /// Converts a Unit to a string.
        /// </summary>
        /// <param name="culture">The culture to use.</param>
        /// <returns>Returns a string version of the Unit.</returns>
        [SuppressMessage("Microsoft.Design", "CA1011:ConsiderPassingBaseTypesAsParameters", Justification = "This type designation clarifies the API's purpose to a developer.")]
        public string ToString(CultureInfo culture)
        {
            return ToString((IFormatProvider)culture);
        }

        /// <summary>
        /// Converts a Unit to a string.
        /// </summary>
        /// <param name="formatProvider">The format provider.</param>
        /// <returns>Returns a string version of the Unit.</returns>
        public string ToString(IFormatProvider formatProvider)
        {
            if (IsEmpty)
            {
                return String.Empty;
            }

            string valuePart;
            if (_type == UnitType.Pixel)
            {
                valuePart = ((int)_value).ToString(formatProvider);
            }
            else
            {
                // There are issues using double.ToString
                valuePart = ((float)_value).ToString(formatProvider);
            }
            return valuePart + Unit.GetStringFromType(_type);
        }

        /// <summary>
        /// Implicitly creates a Unit of type Pixel from the specified integer.
        /// </summary>
        /// <param name="number">The number to implicitly convert.</param>
        /// <returns>Returns a new Unit type from the number.</returns>
        public static implicit operator Unit(int number)
        {
            return Unit.Pixel(number);
        }
    }
}