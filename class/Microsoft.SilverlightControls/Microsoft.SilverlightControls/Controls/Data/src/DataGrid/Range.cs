// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

namespace System.Windows.Controlsb1
{ 
    internal class Range<T> 
    {
        #region Data 
        #endregion Data

        public Range(int lowerBound, int upperBound, T value) 
        {
            LowerBound = lowerBound;
            UpperBound = upperBound; 
            Value = value; 
        }
 
        #region Public Properties

        public int Count 
        {
            get
            { 
                return UpperBound - LowerBound + 1; 
            }
        } 

        public int LowerBound
        { 
            get;
            set;
        } 
 
        public int UpperBound
        { 
            get;
            set;
        } 

        public T Value
        { 
            get; 
            set;
        } 

        #endregion Public Properties
 
        #region Public Methods

        public bool ContainsIndex(int index) 
        { 
            return (LowerBound <= index) && (UpperBound >= index);
        } 

        #endregion Public Methods
    } 
}
