

namespace System.Net.Browser {
	public static class WebRequestCreator {

		private static BrowserWebRequestCreate browser_http;
		private static ClientWebRequestCreate client_http;

		public static IWebRequestCreate BrowserHttp {
			get {
				if (browser_http == null)
					browser_http = new BrowserWebRequestCreate ();
				return browser_http;
			}
		}

		

		public static IWebRequestCreate ClientHttp {
			get {
				if (client_http == null)
					client_http = new ClientWebRequestCreate ();
				return client_http;
			}
		}
	}
}


