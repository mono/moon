/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * messaging.h: client side messaging
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


#ifndef __MOON_MESSAGING_H__
#define __MOON_MESSAGING_H__

#include "dependencyobject.h"
#include "pal.h"

namespace Moonlight {

/* @Namespace=None,ManagedEvents=Manual */
class LocalMessageReceiver : public DependencyObject {
public:
	/* @GeneratePInvoke */
	LocalMessageReceiver (const char *receiverName,
			      ReceiverNameScope namescope);

	/* @GeneratePInvoke */
	const char *GetReceiverName () { return receiverName; }

	/* @GeneratePInvoke */
	ReceiverNameScope GetReceiverNameScope () { return namescope; }

	/* @GeneratePInvoke */
	void SetAllowedSenderDomains (char **allowedSenderDomains, int count);

	/* @GeneratePInvoke */
	void ListenWithError (MoonError *error);

	/* @GeneratePInvoke */
	void DisposeWithError (MoonError *error);

	/* @DelegateType=EventHandler<MessageReceivedEventArgs> */
	const static int MessageReceivedEvent;

protected:
	virtual ~LocalMessageReceiver();

private:
	static char* MessageReceivedHandler (const char *msg, gpointer data);
	char* MessageReceived (const char *msg);

	char *receiverName;
	char *receiverDomain;

	char **allowedSenderDomains;
	int allowedSenderDomainsCount;

	ReceiverNameScope namescope;
	MoonMessageListener *listener;
	bool unref_in_dispose;
};

/* @Namespace=None,ManagedEvents=Manual */
class LocalMessageSender : public DependencyObject {
public:
	/* @GeneratePInvoke */
	LocalMessageSender (const char *receiverName, const char *receiverDomain);

	/* @GeneratePInvoke */
	void SendAsyncWithError (const char *msg, GCHandle managedUserState, MoonError *error);

	/* @DelegateType=EventHandler<SendCompletedEventArgs> */
	const static int SendCompletedEvent;

protected:
	virtual ~LocalMessageSender ();

private:
	static void MessageSentHandler (MoonError *error, const char *message, const char *response, GCHandle managedUserState, gpointer data);
	void MessageSent (MoonError *error, const char *message, const char *msg, GCHandle managedUserState);

	MoonMessageSender *sender;

	char *receiverName;
	char *receiverDomain;
	char *senderDomain;
};

};
#endif /* __MOON_MESSAGING_H__ */
