// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.ComponentModel;

namespace System.Windows.Controls
{
    // These aren't flags
    [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1027:MarkEnumsWithFlags")]
    internal enum DataGridLengthUnitType
    {
        Auto = 0,
        Pixel = 1,
        SizeToCells = 2,
        SizeToHeader = 3,
        // 
    }

    [TypeConverter(typeof(DataGridLengthConverter))]
    public struct DataGridLength : IEquatable<DataGridLength>
    {
        #region Data

        private double _unitValue;      //  unit value storage
        private DataGridLengthUnitType _unitType; //  unit type storage

        //  static instances of value invariant DataGridLengths
        private static readonly DataGridLength _auto = new DataGridLength(DATAGRIDLENGTH_DefaultValue, DataGridLengthUnitType.Auto);
        private static readonly DataGridLength _sizeToCells = new DataGridLength(DATAGRIDLENGTH_DefaultValue, DataGridLengthUnitType.SizeToCells);
        private static readonly DataGridLength _sizeToHeader = new DataGridLength(DATAGRIDLENGTH_DefaultValue, DataGridLengthUnitType.SizeToHeader);

        // WPF uses 1.0 as the default value as well
        internal const double DATAGRIDLENGTH_DefaultValue = 1.0;

        #endregion Data

        public DataGridLength(double value)
            : this(value, DataGridLengthUnitType.Pixel)
        {
        }

        private DataGridLength(double value, DataGridLengthUnitType type)
        {
            if (double.IsNaN(value))
            {
                throw DataGridError.DataGrid.ValueCannotBeSetToNAN("value");
            }
            if (double.IsInfinity(value))
            {
                throw DataGridError.DataGrid.ValueCannotBeSetToInfinity("value");
            }
            if (value < 0)
            {
                throw DataGridError.DataGrid.ValueMustBeGreaterThanOrEqualTo("value", "value", 0);
            }

            if (type != DataGridLengthUnitType.Auto &&
                type != DataGridLengthUnitType.SizeToCells &&
                type != DataGridLengthUnitType.SizeToHeader &&
                // 
                type != DataGridLengthUnitType.Pixel)
            {
                throw DataGridError.DataGridLength.InvalidUnitType("type");
            }

            _unitValue = (type == DataGridLengthUnitType.Auto) ? DATAGRIDLENGTH_DefaultValue : value;
            _unitType = type;
        }

        #region Properties

        public static DataGridLength Auto
        {
            get
            {
                return _auto;
            }
        }

        internal bool IsAbsolute 
        { 
            get 
            {
                return _unitType == DataGridLengthUnitType.Pixel;
            }
        }

        internal bool IsAuto 
        {
            get
            {
                return _unitType == DataGridLengthUnitType.Auto;
            }
        }

        // 








        internal bool IsSizeToCells 
        {
            get
            {
                return _unitType == DataGridLengthUnitType.SizeToCells;
            }
        }

        internal bool IsSizeToHeader 
        {
            get
            {
                return _unitType == DataGridLengthUnitType.SizeToHeader;
            }
        }

        public static DataGridLength SizeToCells
        {
            get
            {
                return _sizeToCells;
            }
        }

        public static DataGridLength SizeToHeader
        {
            get
            {
                return _sizeToHeader;
            }
        }

        internal DataGridLengthUnitType UnitType
        {
            get
            {
                return _unitType;
            }
        }

        public double Value 
        {
            get
            {
                return _unitValue;
            }
        }

        #endregion Properties

        #region Methods

        /// <summary>
        /// Overloaded operator, compares 2 GridLength's.
        /// </summary>
        /// <param name="gl1">first DataGridLength to compare.</param>
        /// <param name="gl2">second DataGridLength to compare.</param>
        /// <returns>true if specified DataGridLength have same value 
        /// and unit type.</returns>
        public static bool operator ==(DataGridLength gl1, DataGridLength gl2)
        {
            return (gl1.UnitType == gl2.UnitType
                    && gl1.Value == gl2.Value);
        }

        /// <summary>
        /// Overloaded operator, compares 2 GridLength's.
        /// </summary>
        /// <param name="gl1">first DataGridLength to compare.</param>
        /// <param name="gl2">second DataGridLength to compare.</param>
        /// <returns>true if specified DataGridLength have either different value or 
        /// unit type.</returns>
        public static bool operator !=(DataGridLength gl1, DataGridLength gl2)
        {
            return (gl1.UnitType != gl2.UnitType
                    || gl1.Value != gl2.Value);
        }

        /// <summary>
        /// Compares this instance of DataGridLength with another instance.
        /// </summary>
        /// <param name="other">DataGridLength length instance to compare.</param>
        /// <returns><c>true</c>if this DataGridLength instance has the same value 
        /// and unit type as gridLength.</returns>
        public bool Equals(DataGridLength other)
        {
            return (this == other);
        }

        /// <summary>
        /// Compares this instance of GridLength with another object.
        /// </summary>
        /// <param name="obj">Reference to an object for comparison.</param>
        /// <returns><c>true</c>if this DataGridLength instance has the same value 
        /// and unit type as oCompare.</returns>
        public override bool Equals(object obj)
        {
            DataGridLength? dataGridLength = obj as DataGridLength?;
            if (dataGridLength.HasValue)
            {
                return (this == dataGridLength);
            }
            return false;
        }

        /// <summary>
        /// Returns a unique HashCode for this DataGridLength
        /// </summary>
        /// <returns>hashcode</returns>
        public override int GetHashCode()
        {
            return ((int)_unitValue + (int)_unitType);
        }

        #endregion Methods
    }
}
