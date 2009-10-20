
using System.Net;
using System.Reflection;

namespace System.Net.Browser {

	public class ClientWebRequestCreate : IWebRequestCreate {

		//
		// This should create a 'real' web request, not one that uses the browser
		// For now we will just use the browser web request
		//
		public WebRequest Create (Uri uri)
		{
			BrowserWebRequestCreate creator = new BrowserWebRequestCreate ();
			return creator.Create (uri);
		}
	}
}



