// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 


namespace System.Windows.Controls 
{ 
    /// <summary>
    /// Provides data for the TextParseError event. 
    /// </summary>
    public class DatePickerTextParseErrorEventArgs : EventArgs
    { 
        private bool _throwException;

        /// <summary> 
        /// Initializes a new instance of the DatePickerTextParseErrorEventArgs class. 
        /// </summary>
        /// <param name="exception">The exception that initially triggered the TextParseError event.</param> 
        /// <param name="text">The text being parsed.</param>
        public DatePickerTextParseErrorEventArgs(Exception exception,
                                          string text) 
        {
            this.Text = text;
            this.Exception = exception; 
        } 

        /// <summary> 
        /// Gets the exception that initially triggered the TextParseError event.
        /// </summary>
        public Exception Exception 
        {
            get;
            private set; 
        } 

        /// <summary> 
        /// Gets the text being parsed when the TextParseError event was raised.
        /// </summary>
        public string Text 
        {
            get;
            private set; 
        } 

        /// <summary> 
        /// Gets or sets a value that indicates whether Exception should be thrown.
        /// </summary>
        public bool ThrowException 
        {
            get
            { 
                return this._throwException; 
            }
            set 
            {
                if (value && this.Exception == null)
                { 
                    throw new ArgumentException("Cannot Throw Null Exception");
                }
                this._throwException = value; 
            } 
        }
    } 
}
