//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System;
using System.Collections.Generic;

namespace System.Windows.Messaging {

	public sealed class LocalMessageReceiver : IDisposable
	{
		public static readonly IEnumerable<string> AnyDomain;


		public LocalMessageReceiver (string receiverName)
		{
			this.receiverName = receiverName;
			this.nameScope = ReceiverNameScope.Global;
			this.allowedSenderDomains = AnyDomain;
			Console.WriteLine ("LocalMessageReceiver.ctor1 ({0})", receiverName);
		}

		public LocalMessageReceiver (string receiverName,
					     ReceiverNameScope nameScope,
					     IEnumerable<string> allowedSenderDomains)
		{
			this.receiverName = receiverName;
			this.nameScope = nameScope;
			this.allowedSenderDomains = allowedSenderDomains;
			Console.WriteLine ("LocalMessageReceiver.ctor2 ({0}, {1})", receiverName, nameScope);
		}

		public void Listen()
		{
			Console.WriteLine ("LocalMessageReceiver.Listen");
		}

		public void Dispose ()
		{
			Console.WriteLine ("LocalMessageReceiver.Dispose");
		}

		public IEnumerable<string> AllowedSenderDomains {
			get { return allowedSenderDomains; }
		}

		public bool DisableSenderTrustCheck {
			get { return disableSenderTrustCheck; }
			set { disableSenderTrustCheck = value; }
		}

		public ReceiverNameScope NameScope {
			get { return nameScope; }
		}

		public string ReceiverName {
			get { return receiverName; }
		}

		public event EventHandler<MessageReceivedEventArgs> MessageReceived;

		string receiverName;
		ReceiverNameScope nameScope;
		IEnumerable<string> allowedSenderDomains;
		bool disableSenderTrustCheck;
	}
}
