// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Net;
using System.Text;
using System.IO;
using System.Windows.Browser;
using System.Threading;

namespace Microsoft.Silverlight.Testing.Harness.Service
{
    /// <summary>
    /// Provides out-of-process access to operating system functions and other 
    /// services such as visual verification, if present.
    /// </summary>
    public partial class WebTestService
    {
        /// <summary>
        /// The service address.
        /// </summary>
        private Uri _serviceUri;
        
        /// <summary>
        /// The synchronization context.
        /// </summary>
        private SynchronizationContext _sync;

        /// <summary>
        /// Initializes a new plain-old-XML test service.  This assumes that 
        /// the caller has already verifier that a service is present and 
        /// responding at the service address.
        /// </summary>
        /// <param name="serviceUri">The base service URI, such as 
        /// "scheme://hostname:port/servicePath/".</param>
        public WebTestService(Uri serviceUri)
        {
            _sync = SynchronizationContext.Current;
            _serviceUri = serviceUri;
        }

        /// <summary>
        /// Creates a simple REST-style Uri given the method/service name and 
        /// a dictionary of key/value pairs to send as arguments.
        /// </summary>
        /// <param name="service">The method/service name.</param>
        /// <param name="arguments">A set of key/value pairs.</param>
        /// <returns>Returns a new Uri.</returns>
        protected Uri CreateUri(string service, Dictionary<string, string> arguments)
        {
            return CreateUri(service, arguments, String.Empty);
        }

        /// <summary>
        /// Creates a simple REST-style Uri given the method/service name and 
        /// a dictionary of key/value pairs to send as arguments.
        /// </summary>
        /// <param name="service">The method/service name.</param>
        /// <param name="arguments">A set of key/value pairs.</param>
        /// <param name="queryString">Optional query string.</param>
        /// <returns>Returns a new Uri.</returns>
        protected Uri CreateUri(string service, Dictionary<string, string> arguments, string queryString)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append(_serviceUri);
            sb.Append(service);
            sb.Append("/");
            if (arguments != null)
            {
                foreach (string key in arguments.Keys)
                {
                    sb.Append(HttpUtility.UrlEncode(key));
                    sb.Append("/");
                    sb.Append(HttpUtility.UrlEncode(arguments[key]));
                    sb.Append("/");
                }
            }
            if (!string.IsNullOrEmpty(queryString))
            {
                sb.Append("?");
                sb.Append(queryString);
            }
            return new Uri(sb.ToString());
        }

        /// <summary>
        /// Builds a simple dictionary from parameters.  The value follows the 
        /// key parameter.  {[key, value], } ...
        /// 
        /// The ToString() method is called on every object.
        /// </summary>
        /// <param name="parameters">The parameters.</param>
        /// <returns>Returns a key/value dictionary from the parameters.</returns>
        public static Dictionary<string, string> Dictionary(params object[] parameters)
        {
            if (parameters.Length % 2 != 0)
            {
                throw new InvalidOperationException("There must be an even number of parameters.");
            }

            Dictionary<string, string> dictionary = new Dictionary<string, string>();
            for (int i = 0; i < parameters.Length; i += 2)
            {
                string key = parameters[i].ToString();
                string value = parameters[i + 1] == null ? string.Empty : parameters[i + 1].ToString();
                dictionary[key] = value;
            }
            return dictionary;
        }

        /// <summary>
        /// Begin a POX method call.  The callback is used with the result when 
        /// it becomes available.
        /// </summary>
        /// <param name="method">The method name.</param>
        /// <param name="callback">The callback action.</param>
        public void CallMethod(string method, Action<ServiceResult> callback)
        {
            CallMethod(method, null, callback);
        }

        /// <summary>
        /// Begin a POX method call.  The callback is used with the result when 
        /// it becomes available.
        /// </summary>
        /// <param name="method">The method name.</param>
        /// <param name="arguments">Dictionary of key/value pairs.</param>
        /// <param name="callback">The callback action.</param>
        public void CallMethod(string method, Dictionary<string, string> arguments, Action<ServiceResult> callback)
        {
            CallMethod(method, arguments, String.Empty, callback);
        }

        /// <summary>
        /// Begin a POX method call.  The callback is used with the result when 
        /// it becomes available.
        /// </summary>
        /// <param name="method">The method name.</param>
        /// <param name="arguments">Dictionary of key/value pairs.</param>
        /// <param name="postData">Optional string that will transform the 
        /// request to a POST request.</param>
        /// <param name="callback">The callback action.</param>
        public void CallMethod(string method, Dictionary<string, string> arguments, string postData, Action<ServiceResult> callback)
        {
            // Always prevent caching results in our stack
            string unique = "_unique_=" + DateTime.Now.Ticks.ToString(CultureInfo.InvariantCulture);
            string queryString = unique;
            Uri uri = CreateUri(method, arguments, queryString);
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(uri);
            
            RequestData data = new RequestData(uri, request, callback);

            if (!string.IsNullOrEmpty(postData))
            {
                // POST request
                data.PostData = postData;
                request.Method = "POST";
                request.ContentType = "application/x-www-form-urlencoded";
                // ? ContentType needed ?
                request.BeginGetRequestStream(new AsyncCallback(CallMethodPostContinue), data);
            } 
            else 
            {
                // GET request
                request.BeginGetResponse(new AsyncCallback(ResponseCallback), data);
            }
        }

        /// <summary>
        /// The intermediate step that writes the POST data and then continues 
        /// the web request.
        /// </summary>
        /// <param name="ar">The async result object.</param>
        private void CallMethodPostContinue(IAsyncResult ar)
        {
            RequestData rd = (RequestData)ar.AsyncState;
            HttpWebRequest request = rd.Request;
            StreamWriter sw = new StreamWriter(request.EndGetRequestStream(ar));
            sw.Write(rd.PostData);
            sw.Close();

            // Continue like a GET request now
            request.BeginGetResponse(new AsyncCallback(ResponseCallback), rd);
        }

        /// <summary>
        /// Process the response callback from a POX method call.
        /// </summary>
        /// <param name="ar">The async result object.</param>
        private void ResponseCallback(IAsyncResult ar)
        {
            RequestData data = (RequestData)ar.AsyncState;
            HttpWebRequest request = data.Request;
            HttpWebResponse response = (HttpWebResponse)request.EndGetResponse(ar);
            Action<ServiceResult> callback = data.Callback;
            WebServiceResult wsr = data.ConvertToResult(response);
            wsr.ReadHttpWebResponse();

            _sync.Post(UserInterfaceThreadCallback, new CrossThreadState(callback, wsr));
        }

        /// <summary>
        /// On the UI thread, invoke the callback action with the result.
        /// </summary>
        /// <param name="state">The temporary state object.</param>
        private void UserInterfaceThreadCallback(object state)
        {
            CrossThreadState temp = (CrossThreadState)state;
            if (temp != null && temp.Callback != null)
            {
                temp.Callback(temp.Result);
            }
        }
    }
}