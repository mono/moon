// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Net;
using System.Xml.Linq;

namespace Microsoft.Silverlight.Testing.Harness.Service
{
    /// <summary>
    /// Result object for asynchronous test service response that uses a simple 
    /// web service / POX call.
    /// </summary>
    public class WebServiceResult : ServiceResult
    {
        /// <summary>
        /// Gets the web request associated with the service call.
        /// </summary>
        public WebRequest Request { get; private set; }

        /// <summary>
        /// Gets the web response associated with the service call.
        /// </summary>
        public WebResponse Response { get; private set; }

        /// <summary>
        /// Gets the details of the request, used for interactive debugging 
        /// sessions only.
        /// </summary>
        public object RequestDetails { get; private set; }

        /// <summary>
        /// Gets the string retrieved from the response.
        /// </summary>
        protected string ResponseString { get; private set; }

        /// <summary>
        /// Initializes a new web service result.
        /// </summary>
        /// <param name="request">The request object.</param>
        /// <param name="response">The response object.</param>
        public WebServiceResult(WebRequest request, WebResponse response) : base()
        {
            Request = request;
            Response = response;
        }

        /// <summary>
        /// Initializes a new web service result.
        /// </summary>
        /// <param name="request">The request object.</param>
        /// <param name="response">The response object.</param>
        /// <param name="details">The details to associate for debugging 
        /// purposes.</param>
        public WebServiceResult(WebRequest request, WebResponse response, object details)
            : this(request, response)
        {
            RequestDetails = details;
        }

        /// <summary>
        /// Reads the web response, if successful, and parses out the string 
        /// content.
        /// </summary>
        public void ReadHttpWebResponse()
        {
            HttpWebResponse response = (HttpWebResponse)Response;
            if (response.StatusCode == HttpStatusCode.OK)
            {
                using (Stream stream = response.GetResponseStream())
                {
                    StreamReader sr = new StreamReader(stream);
                    ResponseString = sr.ReadToEnd();
                    sr.Close();
                }
            }
            else
            {
                // TODO: Use resources
                Exception = new InvalidOperationException("The status code returned was not OK.");
            }
        }

        /// <summary>
        /// Process the response text.
        /// </summary>
        [SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes", Justification = "By storing the Exception, it is effectively re-thrown in the appropriate UI thread.")]
        protected override void ProcessResponse()
        {
            if (Exception == null)
            {
                // use ResponseString...
                // use SetResult...

                try
                {
                    SetResult(XElement.Parse(ResponseString));
                }
                catch (Exception exception)
                {
                    Exception = exception;
                }
            }

            base.ProcessResponse();
        }
    }
}
