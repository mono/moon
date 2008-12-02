// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Xml.Linq;

namespace Microsoft.Silverlight.Testing.Harness.Service
{
    /// <summary>
    /// Result object for asynchronous test service response.
    /// </summary>
    public class ServiceResult
    {
        /// <summary>
        /// Creates a new ServiceResult object for a failed result, the 
        /// sets the exception.
        /// </summary>
        /// <param name="except">The Exception object.</param>
        /// <returns>Returns a new ServiceResult with the Exception set.</returns>
        public static ServiceResult CreateExceptionalResult(Exception except)
        {
            return new ServiceResult()
            {
                Exception = except
            };
        }

        /// <summary>
        /// The result LINQ element.
        /// </summary>
        private XElement _result;

        /// <summary>
        /// Initializes a new instance of the ServiceResult class.
        /// </summary>
        public ServiceResult()
        {
        }

        /// <summary>
        /// Gets or sets a value indicating whether the result has been 
        /// processed.
        /// </summary>
        protected bool Processed { get; set; }

        /// <summary>
        /// Gets or sets the exception intercepted or generated during the 
        /// request or 
        /// processing timeframe.
        /// </summary>
        public Exception Exception { get; set; }

        /// <summary>
        /// Gets the root XElement of the test service result.
        /// </summary>
        public XElement Element
        {
            get
            {
                if (!Processed)
                {
                    ProcessResponse();
                }
                if (Exception != null)
                {
                    throw Exception;
                }
                return _result;
            }
        }

        /// <summary>
        /// Gets a value indicating whether the request was successful.
        /// </summary>
        public bool Successful
        {
            get
            {
                if (!Processed)
                {
                    ProcessResponse();
                }
                return (Exception == null);
            }
        }

        /// <summary>
        /// Process the response text.
        /// </summary>
        protected virtual void ProcessResponse()
        {
            Processed = true;
        }

        /// <summary>
        /// Attempt to process and return the root element of a successful 
        /// request. Returns null if there was an Exception.
        /// </summary>
        /// <returns>The root XML element of the response.</returns>
        public XElement TryGetElement()
        {
            if (!Processed)
            {
                ProcessResponse();
            }
            if (Exception != null)
            {
                return null;
            }
            return Element;
        }

        /// <summary>
        /// Sets the result.
        /// </summary>
        /// <param name="result">The LINQ element for the result.</param>
        protected void SetResult(XElement result)
        {
            _result = result;
        }
    }
}