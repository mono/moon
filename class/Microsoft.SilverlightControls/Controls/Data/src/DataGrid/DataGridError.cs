// Copyright © Microsoft Corporation.
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved.

using System.Globalization;

namespace System.Windows.Controlsb1
{
    internal static class DataGridError
    {
        public static class DataGrid
        {
            public static InvalidOperationException CannotAddFrozenColumn()
            {
                return new InvalidOperationException(Resource.DataGrid_CannotAddFrozenColumn);
            }

            public static InvalidOperationException CannotAddNonFrozenColumn()
            {
                return new InvalidOperationException(Resource.DataGrid_CannotAddNonFrozenColumn);
            }

            public static InvalidOperationException CannotChangeColumnCollectionWhileAdjustingDisplayIndexes()
            {
                return new InvalidOperationException(Resource.DataGrid_CannotChangeColumnCollectionWhileAdjustingDisplayIndexes);
            }

            public static InvalidOperationException CannotMoveFrozenColumn()
            {
                return new InvalidOperationException(Resource.DataGrid_CannotMoveFrozenColumn);
            }

            public static InvalidOperationException CannotMoveNonFrozenColumn()
            {
                return new InvalidOperationException(Resource.DataGrid_CannotMoveNonFrozenColumn);
            }

            public static InvalidOperationException ColumnCannotBeCollapsed()
            {
                return new InvalidOperationException(Resource.DataGrid_ColumnCannotBeCollapsed);
            }

            public static InvalidOperationException ColumnCannotBeReassignedToDifferentDataGrid()
            {
                return new InvalidOperationException(Resource.DataGrid_ColumnCannotBeReassignedToDifferentDataGrid);
            }

            public static ArgumentException ColumnNotInThisDataGrid()
            {
                return new ArgumentException(Resource.DataGrid_ColumnNotInThisDataGrid);
            }

            public static InvalidOperationException CommitFailedCannotCompleteOperation()
            {
                return new InvalidOperationException(Resource.DataGrid_CommitFailedCannotCompleteOperation);
            }

            public static ArgumentException InvalidEnumValue(string value, string enumTypeName, string paramName)
            {
                return new ArgumentException(Format(Resource.DataGrid_InvalidEnumValue, value, enumTypeName), paramName);
            }

            public static ArgumentException InvalidRowElement(string paramName)
            {
                return new ArgumentException(Resource.DataGrid_InvalidRowElement, paramName);
            }

            public static ArgumentException ItemIsNotContainedInTheItemsSource(string paramName)
            {
                return new ArgumentException(Resource.DataGrid_ItemIsNotContainedInTheItemsSource, paramName);
            }

            public static InvalidOperationException ItemsSourceNullCannotCompleteOperation()
            {
                return new InvalidOperationException(Resource.DataGrid_ItemsSourceNullCannotCompleteOperation);
            }

            public static TypeInitializationException MissingTemplateForType(Type type)
            {
                return new TypeInitializationException(Format(Resource.DataGrid_MissingTemplateForType, type.FullName), null);
            }

            public static InvalidOperationException NoCurrentRow()
            {
                return new InvalidOperationException(Resource.DataGrid_NoCurrentRow);
            }

            public static InvalidOperationException NoOwningGrid(Type type)
            {
                return new InvalidOperationException(Format(Resource.DataGrid_NoOwningGrid, type.FullName));
            }

            public static InvalidOperationException UnderlyingPropertyIsReadOnly()
            {
                return new InvalidOperationException(Resource.DataGrid_UnderlyingPropertyIsReadOnly);
            }

            public static ArgumentNullException ValueCannotBeSetToNull(string paramName, string valueName)
            {
                return new ArgumentNullException(paramName, Format(Resource.DataGrid_ValueCannotBeSetToNull, valueName));
            }

            public static ArgumentException ValueIsNotAnInstanceOf(string paramName, Type type)
            {
                return new ArgumentException(paramName, Format(Resource.DataGrid_ValueIsNotAnInstanceOf, type.FullName));
            }

            public static ArgumentException ValueIsNotAnInstanceOfEitherOr(string paramName, Type type1, Type type2)
            {
                return new ArgumentException(paramName, Format(Resource.DataGrid_ValueIsNotAnInstanceOfEitherOr, type1.FullName, type2.FullName));
            }

            public static InvalidOperationException ValueIsReadOnly(string paramName)
            {
                return new InvalidOperationException(Format(Resource.DataGrid_ValueIsReadOnly, paramName));
            }

            public static ArgumentOutOfRangeException ValueMustBeBetween(string paramName, string valueName, object lowValue, bool lowInclusive, object highValue, bool highInclusive)
            {
                string message = null;

                if (lowInclusive && highInclusive)
                {
                    message = Resource.DataGrid_ValueMustBeGTEandLTE;
                }
                else if (lowInclusive && !highInclusive)
                {
                    message = Resource.DataGrid_ValueMustBeGTEandLT;
                }
                else if (!lowInclusive && highInclusive)
                {
                    message = Resource.DataGrid_ValueMustBeGTandLTE;
                }
                else
                {
                    message = Resource.DataGrid_ValueMustBeGTandLT;
                }

                return new ArgumentOutOfRangeException(paramName, Format(message, valueName, lowValue, highValue));
            }

            public static ArgumentOutOfRangeException ValueMustBeGreaterThanOrEqualTo(string paramName, string valueName, object value)
            {
                return new ArgumentOutOfRangeException(paramName, Format(Resource.DataGrid_ValueMustBeGreaterThanOrEqualTo, valueName, value));
            }

            public static ArgumentOutOfRangeException ValueMustBeLessThanOrEqualTo(string paramName, string valueName, object value)
            {
                return new ArgumentOutOfRangeException(paramName, Format(Resource.DataGrid_ValueMustBeLessThanOrEqualTo, valueName, value));
            }

            public static ArgumentOutOfRangeException ValueMustBeGreaterThan(string paramName, string valueName, object value)
            {
                return new ArgumentOutOfRangeException(paramName, Format(Resource.DataGrid_ValueMustBeGreaterThan, valueName, value));
            }

            public static ArgumentOutOfRangeException ValueMustBeLessThan(string paramName, string valueName, object value)
            {
                return new ArgumentOutOfRangeException(paramName, Format(Resource.DataGrid_ValueMustBeLessThan, valueName, value));
            }

        }

        public static class DataGridDataErrorEventArgs
        {
            public static ArgumentException CannotThrowNullException()
            {
                return new ArgumentException(Resource.DataGridDataErrorEventArgs_NullException);
            }
        }

        public static class DataGridRow
        {
            public static InvalidOperationException InvalidRowIndexCannotCompleteOperation()
            {
                return new InvalidOperationException(Resource.DataGridRow_InvalidRowIndexCannotCompleteOperation);
            }
        }

        public static class DataGridSelectedItemsCollection
        {
            public static InvalidOperationException CannotChangeSelectedItemsCollectionInSingleMode()
            {
                return new InvalidOperationException(Resource.DataGridSelectedItemsCollection_CannotChangeSelectedItemsCollectionInSingleMode);
            }
        }

        private static string Format(string formatString, params object[] args)
        {
            return String.Format(CultureInfo.CurrentCulture, formatString, args);
        }
    }
}
