// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.Specialized;
using System.Globalization;

namespace System.Windows.Controls
{
    internal static class CollectionViewError
    {
        public static class ListCollectionView
        {
            public static InvalidOperationException AddedItemNotAtIndex(int index)
            {
                return new InvalidOperationException(Format(Resource.ListCollectionView_AddedItemNotAtIndex, index));
            }

            public static InvalidOperationException AddedItemNotInCollection()
            {
                return new InvalidOperationException(Resource.ListCollectionView_AddedItemNotInCollection);
            }

            public static InvalidOperationException CollectionChangeIndexOutOfRange(int index, int size)
            {
                return new InvalidOperationException(Format(Resource.ListCollectionView_CollectionChangeIndexOutOfRange, index, size));
            }

            public static InvalidOperationException FailedToCompareElements()
            {
                return new InvalidOperationException(Resource.ListCollectionView_FailedToCompareElements);
            }

            public static InvalidOperationException InvalidPropertyName(string paramName)
            {
                return new InvalidOperationException(Format(Resource.ListCollectionView_InvalidPropertyName, paramName));
            }

            public static NotSupportedException MultiThreadedCollectionChangeNotSupported()
            {
                return new NotSupportedException(Resource.ListCollectionView_MultiThreadedCollectionChangeNotSupported);
            }

            public static NotSupportedException RangeActionsNotSupported()
            {
                return new NotSupportedException(Resource.ListCollectionView_RangeActionsNotSupported);
            }

            public static NotSupportedException UnexpectedCollectionChangeAction(NotifyCollectionChangedAction action)
            {
                return new NotSupportedException(Format(Resource.ListCollectionView_UnexpectedCollectionChangeAction, action));
            }
        }
            
        private static string Format(string formatString, params object[] args)
        {
            return String.Format(CultureInfo.CurrentCulture, formatString, args);
        }
    }
}
