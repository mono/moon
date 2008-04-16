using System;
using System.Net;

namespace System.Windows.Browser.Net
{
	public abstract class SoapHttpClientProtocol
	{
		string url;

		public string Url {
			get { return url; }
			set { url = value; }
		}

		[MonoTODO]
		protected object [] Invoke (string methodName, ServiceParameter [] parameters, Type returnType)
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		protected IAsyncResult BeginInvoke (string methodName, ServiceParameter [] parameters, Type returnType, AsyncCallback callback, object asyncState)
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		protected object [] EndInvoke (IAsyncResult result)
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		protected void CancelAsync (object userState)
		{
			throw new NotImplementedException ();
		}

		public class ServiceParameter
		{
			string name;
			object value;

			public ServiceParameter (string name, object value)
			{
				if (name == null)
					throw new ArgumentNullException ("name");
				this.name = name;
				this.value = value;
			}
		}
	}
}
