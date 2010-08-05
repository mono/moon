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

using Mono;
using System;
using System.Threading;
using System.Collections;
using System.Collections.Generic;

namespace System.Windows.Messaging {

	public sealed partial class LocalMessageReceiver : INativeEventObjectWrapper, IDisposable
	{
		public static readonly IEnumerable<string> AnyDomain = new List<string> { "*" };

		public LocalMessageReceiver (string receiverName)
			: this (receiverName,
				ReceiverNameScope.Domain,
				AnyDomain)
		{
		}

		public LocalMessageReceiver (string receiverName,
					     ReceiverNameScope nameScope,
					     IEnumerable<string> allowedSenderDomains)
			: this (NativeMethods.local_message_receiver_new (receiverName,
									  (int)nameScope), true)
		{
			int i = 0;
			foreach (string s in allowedSenderDomains)
				i ++;

			string[] sender_domains = new string[i];
			i = 0;

			foreach (string s in allowedSenderDomains)
				sender_domains[i++] = s;

			NativeMethods.local_message_receiver_set_allowed_sender_domains (NativeHandle, sender_domains, sender_domains.Length);
		}

		internal LocalMessageReceiver (IntPtr raw, bool dropref)
		{
			NativeHandle = raw;
			if (dropref)
				NativeMethods.event_object_unref (raw);
		}

		~LocalMessageReceiver ()
		{
			Free ();
		}

		void Free ()
		{
			if (free_mapping) {
				free_mapping = false;
				NativeDependencyObjectHelper.FreeNativeMapping (this);
			}
		}

		public void Listen()
		{
			NativeMethods.local_message_receiver_listen (NativeHandle);
		}

		public void Dispose ()
		{
			NativeMethods.local_message_receiver_dispose (NativeHandle);
		}

		public IEnumerable<string> AllowedSenderDomains {
			get { return allowedSenderDomains; }
		}

		[MonoTODO ("is this IE specific?")]
		public bool DisableSenderTrustCheck {
			get { return disableSenderTrustCheck; }
			set { disableSenderTrustCheck = value; }
		}

		public ReceiverNameScope NameScope {
			get { return (ReceiverNameScope)NativeMethods.local_message_receiver_get_receiver_name_scope (NativeHandle); }
		}

		public string ReceiverName {
			get { return NativeMethods.local_message_receiver_get_receiver_name (NativeHandle); }
		}

		public event EventHandler<MessageReceivedEventArgs> MessageReceived {
			add { RegisterEvent (EventIds.LocalMessageReceiver_MessageReceivedEvent, value, Events.CreateMessageReceivedEventArgsEventHandlerDispatcher (value)); }
			remove { UnregisterEvent (EventIds.LocalMessageReceiver_MessageReceivedEvent, value); }
		}

		IEnumerable<string> allowedSenderDomains;
		bool disableSenderTrustCheck;

		bool free_mapping;

		IntPtr _native;

		internal IntPtr NativeHandle {
			get { return _native; }
			set {
				if (_native != IntPtr.Zero) {
					throw new InvalidOperationException ("native handle is already set");
				}

				_native = value;

				free_mapping = NativeDependencyObjectHelper.AddNativeMapping (value, this);
			}
		}

		IntPtr INativeEventObjectWrapper.NativeHandle {
			get { return NativeHandle; }
			set { NativeHandle = value; }
		}

		void INativeEventObjectWrapper.MentorChanged (IntPtr mentor_ptr)
		{
		}

		void INativeEventObjectWrapper.OnAttached ()
		{
		}

		void INativeEventObjectWrapper.OnDetached ()
		{
		}

		Kind INativeEventObjectWrapper.GetKind ()
		{
			return Kind.LOCALMESSAGERECEIVER;
		}

		private EventHandlerList EventList = new EventHandlerList ();

		private void RegisterEvent (int eventId, Delegate managedHandler, UnmanagedEventHandler nativeHandler)
		{
			if (managedHandler == null)
				return;

			int token = Events.AddHandler (this, eventId, nativeHandler);
			EventList.AddHandler (eventId, token, managedHandler, nativeHandler);
		}

		private void UnregisterEvent (int eventId, Delegate managedHandler)
		{
			UnmanagedEventHandler nativeHandler = EventList.RemoveHandler (eventId, managedHandler);

			if (nativeHandler == null)
				return;

			Events.RemoveHandler (this, eventId, nativeHandler);
		}

	}
}
