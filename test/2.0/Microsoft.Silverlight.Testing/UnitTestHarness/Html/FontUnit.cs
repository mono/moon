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
    /// Represents the font unit.
    /// </summary>
    [TypeConverterAttribute(typeof(FontUnitConverter))]
    public struct FontUnit 
    {
        /// <summary>
        /// Specifies an empty FontUnit.  This field is read only. 
        /// </summary>
        public static readonly FontUnit Empty = new FontUnit();

        /// <summary>
        /// Specifies a FontUnit with FontSize.Smaller font. This field is 
        /// read only. 
        /// </summary>
        public static readonly FontUnit Smaller = new FontUnit(FontSize.Smaller);

        /// <summary>
        /// Specifies a  with FontSize.Larger font. This field is read only.
        /// </summary>
        public static readonly FontUnit Larger = new FontUnit(FontSize.Larger);

        /// <summary>
        /// Specifies a  with FontSize.XXSmall font. This field is read only.
        /// </summary>
        public static readonly FontUnit XXSmall = new FontUnit(FontSize.XXSmall);
        
        /// <summary>
        /// Specifies a  with FontSize.XSmall font. This field is read only.
        /// </summary>
        public static readonly FontUnit XSmall = new FontUnit(FontSize.XSmall);
        
        /// <summary>
        /// Specifies a  with FontSize.Small font. This field is read only.
        /// </summary>
        public static readonly FontUnit Small = new FontUnit(FontSize.Small);
        
        /// <summary>
        /// Specifies a  with FontSize.Medium font. This field is read only.
        /// </summary>
        public static readonly FontUnit Medium = new FontUnit(FontSize.Medium);
        
        /// <summary>
        /// Specifies a  with FontSize.Large font. This field is read only.
        /// </summary>
        public static readonly FontUnit Large = new FontUnit(FontSize.Large);

        /// <summary>
        /// Specifies a  with FontSize.XLarge font. This field is read only.
        /// </summary>
        public static readonly FontUnit XLarge = new FontUnit(FontSize.XLarge);
        
        /// <summary>
        /// Specifies a  with FontSize.XXLarge font. This field is read only.
        /// </summary>
        public static readonly FontUnit XXLarge = new FontUnit(FontSize.XXLarge);

        /// <summary>
        /// The FontSize type.
        /// </summary>
        private readonly FontSize _type;

        /// <summary>
        /// The Unit value.
        /// </summary>
        private readonly Unit _value;

        /// <summary>
        /// Initializes a new instance of the  class with a FontSize.
        /// </summary>
        /// <param name="type">The font size.</param>
        public FontUnit(FontSize type) 
        {
            if (type < FontSize.NotSet || type > FontSize.XXLarge) 
            {
                throw new ArgumentOutOfRangeException("type");
            }
            _type = type;
            if (_type == FontSize.AsUnit) 
            {
                _value = Unit.Point(10);
            }
            else 
            {
                _value = Unit.Empty;
            }
        }

        /// <summary>
        /// Initializes a new instance of the  class with a Unit.
        /// </summary>
        /// <param name="value">The integer value.</param>
        public FontUnit(Unit value) 
        {
            _type = FontSize.NotSet;
            if (value.IsEmpty == false) 
            {
                _type = FontSize.AsUnit;
                _value = value;
            }
            else 
            {
                _value = Unit.Empty;
            }
        }

        /// <summary>
        /// Initializes a new instance of the  class with an integer value.
        /// </summary>
        /// <param name="value">The integer value.</param>
        public FontUnit(int value) 
        {
            _type = FontSize.AsUnit;
            _value = Unit.Point(value);
        }

        /// <summary>
        /// Initializes a new instance of the  class with a double value.
        /// </summary>
        /// <param name="value">The unit value.</param>
        public FontUnit(double value) : this(new Unit(value, UnitType.Point)) { }

        /// <summary>
        /// Initializes a new instance of the  class with a double value.
        /// </summary>
        /// <param name="value">The unit value.</param>
        /// <param name="type">The type of unit.</param>
        public FontUnit(double value, UnitType type) : this(new Unit(value, type)) { }

        /// <summary>
        /// Initializes a new instance of the  class with a string.
        /// </summary>
        /// <param name="value">The unit value.</param>
        public FontUnit(string value) : this(value, CultureInfo.CurrentCulture) { }

        /// <summary>
        /// Initializes a new FontUnit type with a value and culture info.
        /// </summary>
        /// <param name="value">The string value.</param>
        /// <param name="culture">Font culture.</param>
        [SuppressMessage("Microsoft.Globalization", "CA1308:NormalizeStringsToUppercase", Justification = "Ported code, tested with code as is.")]
        public FontUnit(string value, CultureInfo culture) 
        {
            _type = FontSize.NotSet;
            _value = Unit.Empty;

            if ((value != null) && (value.Length > 0)) 
            {
                // This is invariant because it acts like an enum with a number 
                // together. The enum part is invariant, but the number uses 
                // current culture. 
                char firstChar = Char.ToLower(value[0], CultureInfo.InvariantCulture);
                if (firstChar == 'x') 
                {
                    string lcaseValue = value.ToLower(CultureInfo.InvariantCulture);

                    if (lcaseValue.Equals("xx-small") || lcaseValue.Equals("xxsmall")) 
                    {
                        _type = FontSize.XXSmall;
                        return;
                    }
                    else if (lcaseValue.Equals("x-small") || lcaseValue.Equals("xsmall")) 
                    {
                        _type = FontSize.XSmall;
                        return;
                    }
                    else if (lcaseValue.Equals("x-large") || lcaseValue.Equals("xlarge")) 
                    {
                        _type = FontSize.XLarge;
                        return;
                    }
                    else if (lcaseValue.Equals("xx-large") || lcaseValue.Equals("xxlarge")) 
                    {
                        _type = FontSize.XXLarge;
                        return;
                    }
                }
                else if (firstChar == 's') 
                {
                    string lcaseValue = value.ToLower(CultureInfo.InvariantCulture);
                    if (lcaseValue.Equals("small")) 
                    {
                        _type = FontSize.Small;
                        return;
                    }
                    else if (lcaseValue.Equals("smaller")) 
                    {
                        _type = FontSize.Smaller;
                        return;
                    }
                }
                else if (firstChar == 'l') 
                {
                    string lcaseValue = value.ToLower(CultureInfo.InvariantCulture);
                    if (lcaseValue.Equals("large")) 
                    {
                        _type = FontSize.Large;
                        return;
                    }
                    if (lcaseValue.Equals("larger")) 
                    {
                        _type = FontSize.Larger;
                        return;
                    }
                }
                else if ((firstChar == 'm') && (value.ToLower(CultureInfo.InvariantCulture).Equals("medium"))) 
                {
                    _type = FontSize.Medium;
                    return;
                }

                _value = new Unit(value, culture, UnitType.Point);
                _type = FontSize.AsUnit;
            }
        }

        /// <summary>
        /// Gets a value indicating whether the font size has been set.
        /// </summary>
        public bool IsEmpty 
        {
            get 
            {
                return _type == FontSize.NotSet;
            }
        }

        /// <summary>
        /// Gets the font size by type.
        /// </summary>
        [SuppressMessage("Microsoft.Naming", "CA1721:PropertyNamesShouldNotMatchGetMethods", Justification = "The property name maintains compatibility with the desktop framework.")]
        public FontSize Type 
        {
            get 
            {
                return _type;
            }
        }
        
        /// <summary>
        /// Gets the font size by Unit.
        /// </summary>
        public Unit Unit 
        {
            get 
            {
                return _value;
            }
        }

        /// <summary>
        /// Get the hash code for the object.
        /// </summary>
        /// <returns>Returns the hash code.</returns>
        public override int GetHashCode() 
        {
            return _type.GetHashCode() << 2 ^ _value.GetHashCode();
        }

        /// <summary>
        /// Determines if the specified object is equal to the represented by 
        /// this instance.
        /// </summary>
        /// <param name="obj">Object to check.</param>
        /// <returns>Returns a value indicating whether the object is 
        /// equivalent.</returns>
        public override bool Equals(object obj) 
        {
            if (obj == null || !(obj is FontUnit))
            {
                return false;
            }
            FontUnit f = (FontUnit)obj;
            if ((f._type == _type) && (f._value == _value)) 
            {
                return true;
            }
            return false;
        }

        /// <summary>
        /// Compares two  objects for equality.
        /// </summary>
        /// <param name="left">Left object.</param>
        /// <param name="right">Right object.</param>
        /// <returns>Returns whether the object are identical.</returns>
        public static bool operator ==(FontUnit left, FontUnit right) 
        {
            return ((left._type == right._type) && (left._value == right._value));
        }
        
        /// <summary>
        /// Compares two  objects for inequality.
        /// </summary>
        /// <param name="left">Left object.</param>
        /// <param name="right">Right object.</param>
        /// <returns>Returns whether the objects are not identical.</returns>
        public static bool operator !=(FontUnit left, FontUnit right) 
        {
            return ((left._type != right._type) || (left._value != right._value));
        }

        /// <summary>
        /// Parse a string into a FontUnit object.
        /// </summary>
        /// <param name="value">The string value.</param>
        /// <returns>Returns a new FontUnit.</returns>
        public static FontUnit Parse(string value) 
        {
            return new FontUnit(value, CultureInfo.InvariantCulture);
        }

        /// <summary>
        /// Parse a string into a FontUnit object.
        /// </summary>
        /// <param name="value">The string value.</param>
        /// <param name="culture">The culture info object.</param>
        /// <returns>Returns a new FontUnit.</returns>
        public static FontUnit Parse(string value, CultureInfo culture) 
        {
            return new FontUnit(value, culture);
        }
        
        /// <summary>
        /// Creates a  of type Point from an integer value.
        /// </summary>
        /// <param name="number">The point data.</param>
        /// <returns>Returns a new FontUnit.</returns>
        public static FontUnit Point(int number) 
        {
            return new FontUnit(number);
        }

        /// <summary>
        /// Convert a FontUnit to a string.
        /// </summary>
        /// <returns>Returns the string value.</returns>
        public override string ToString() 
        {
            return ToString((IFormatProvider)CultureInfo.CurrentCulture);
        }

        /// <summary>
        /// String vale of the FontUnit.
        /// </summary>
        /// <param name="culture">The culture info object.</param>
        /// <returns>Returns the string value.</returns>
        [SuppressMessage("Microsoft.Design", "CA1011:ConsiderPassingBaseTypesAsParameters", Justification = "The type clarifies the purpose of the parameter.")]
        public string ToString(CultureInfo culture) 
        {
            return ToString((IFormatProvider)culture);
        }

        /// <summary>
        /// String value of the FontUnit.
        /// </summary>
        /// <param name="formatProvider">The format provider.</param>
        /// <returns>Returns the string version of FontUnit.</returns>
        public string ToString(IFormatProvider formatProvider) 
        {
            string s = String.Empty;
            if (IsEmpty)
            {
                return s;
            }

            switch (_type) 
            {
                case FontSize.AsUnit:
                    s = _value.ToString(formatProvider);
                    break;
                case FontSize.XXSmall:
                    s = "XX-Small";
                    break;
                case FontSize.XSmall:
                    s = "X-Small";
                    break;
                case FontSize.XLarge:
                    s = "X-Large";
                    break;
                case FontSize.XXLarge:
                    s = "XX-Large";
                    break;
                default:
                    s = _type.ToString();
                    break;
            }
            return s;
        }
        
        /// <summary>
        /// Implicitly creates a  of type Point from an integer value.
        /// </summary>
        /// <param name="points">The unit to implicitly convert.</param>
        public static implicit operator FontUnit(int points) 
        {
            return FontUnit.Point(points);
        }
    }
}