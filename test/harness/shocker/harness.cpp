/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * harness.cpp: Interface with our managed test harness
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include "harness.h"

bool
send_all (int sockfd, const char *buffer, guint32 length)
{
	guint32 written = 0;
	ssize_t result;
	
	do {
		do {
			result = send (sockfd, buffer + written, length - written, MSG_NOSIGNAL);
		} while (result == -1 && errno == EINTR);
		
		if (result == -1) {
			printf ("[Shocker]: send failed, returned %i (%i %s)\n", (int) result, errno, strerror (errno));
			return false;
		}
		
		written += result;
	} while (written < length);
	
	return true;
}

bool
recv_all (int sockfd, guint8 *buffer, guint32 length)
{
	guint32 read = 0;
	ssize_t result;

	do {
		do {
			result = recv (sockfd, buffer + read, length - read, MSG_WAITALL);
		} while (result == -1 && errno == EINTR);

		if (result == -1) {
			printf ("[Shocker]: receive failed, returned %i (%i %s)\n", (int) result, errno, strerror (errno));
			return false;
		}

		read += result;
	} while (read < length);

	return true;
}

bool
send_harness_message (const char *msg, guint8 **buffer, guint32 *output_length)
{
	char *tmp;
	int sockfd;
	int result;
	sockaddr_in addr;
	char *strport;
	int port;

	*output_length = 0;
	*buffer = NULL;

	// get and validate port
	strport = getenv ("MOONLIGHT_HARNESS_LISTENER_PORT");
	if (strport == NULL || strport [0] == 0) {
		printf ("[Shocker]: MOONLIGHT_HARNESS_LISTENER_PORT is not set, assuming a default value of 1234\n");
		port = 1234;
	} else {
		port = atoi (strport);
		if (port < 1024) {
			printf ("[Shocker]: The port MOONLIGHT_HARNESS_LISTENER_PORT (%s) is probably invalid, it should be >= 1024.\n", strport);
		}
	}

	// create the socket
	sockfd = socket (PF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		printf ("[Shocker]: Failed to open socket: %i (%s)\n", errno, strerror (errno));
		return false;
	}

	// connect
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);
	memset (addr.sin_zero, 0, sizeof (addr.sin_zero));
	result = inet_pton (AF_INET, "127.0.0.1", &addr.sin_addr);
	result = connect (sockfd, (struct sockaddr *) &addr, sizeof (addr));

	if (result == -1) {
		printf ("[Shocker]: Could not connect to localhost:%i (%i %s)\n", port, errno, strerror (errno));
		goto cleanup;
	} 

	// send request
	tmp = g_strdup_printf ("v2|%s", msg);
	if (!send_all (sockfd, tmp, strlen (tmp))) {
		g_free (tmp);
		result = -1;
		goto cleanup;
	}
	g_free (tmp);

	// First 4 bytes is the size
	if (!recv_all (sockfd, (guint8 *) output_length, 4)) {
		result = -1;
		goto cleanup;
	}

	// printf ("[shocker] receiving %i bytes...\n", *output_length);

	// Then the response
	*buffer = (guint8 *) g_malloc0 (*output_length + 1 /* any missing null terminator */);
	if (!recv_all (sockfd, *buffer, *output_length)) {
		g_free (*buffer);
		*buffer = NULL;
		result = -1;
		goto cleanup;
	}

cleanup:
	close (sockfd);

	return result != -1;
}
