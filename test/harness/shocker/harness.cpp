/*
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Author:
 *   Rolf Bjarne Kvinge
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 *
 * harness.h: Interface with our test harness
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

bool send_harness_message (const char *msg, int *output)
{
	int sockfd;
	int result;
	sockaddr_in addr;
	char *strport;
	int port;

	*output = 0;

	// get and validate port
	strport = getenv ("MOONLIGHT_HARNESS_LISTENER_PORT");
	if (strport == NULL || strport [0] == 0) {
		printf ("[Shocker]: MOONLIGHT_HARNESS_LISTENER_PORT is not set.\n");
		return false;
	}

	port = atoi (strport);
	if (port < 1024) {
		printf ("[Shocker]: The port MOONLIGHT_HARNESS_LISTENER_PORT (%s) is invalid, it must be >= 1024.\n", strport);
		return false;
	}

	// create the socket
	sockfd = socket (PF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		printf ("[Shocker]: Failed to open socket: %i (%s)\n", errno, strerror (errno));
		return false;
	}

	// connect
	addr.sin_family = AF_INET;
	addr.sin_port = htons (1234);
	memset (addr.sin_zero, 0, sizeof (addr.sin_zero));
	result = inet_pton (AF_INET, "127.0.0.1", &addr.sin_addr);
	result = connect (sockfd, (struct sockaddr *) &addr, sizeof (addr));

	if (result == -1) {
		printf ("[Shocker]: Could not connect to localhost:%i (%i %s)\n", port, errno, strerror (errno));
	} else {
		result = send (sockfd, msg, strlen (msg), MSG_NOSIGNAL);
		if (result > 0) {
			char out; 
			result = recv (sockfd, &out, 1, 0);
			if (result > 0) {
				*output = out;
			} else {
				printf ("[Shocker]: receive failed, returned %i (%i %s)\n", result, errno, strerror (errno));
			}
		} else {
			printf ("[Shocker]: send failed, returned %i (%i %s)\n", result, errno, strerror (errno));
		}
	}

	close (sockfd);;

	return result != -1;
}