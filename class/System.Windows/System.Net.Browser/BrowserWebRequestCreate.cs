
using System.Net;

namespace System.Net.Browser {

	public class BrowserWebRequestCreate : IWebRequestCreate {

		public WebRequest Create (Uri uri)
		{
			// The 2.1 profile will create a WebRequest using System.Windows.Browser.dll's
			// BrowserHttpWebRequest class via reflection.

			WebRequest wr = WebRequest.Create (uri);

			return wr;
		}
	}
}


