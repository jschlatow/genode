/*
 * \brief  Libc udp send and recv test
 * \author Johannes Schlatow
 * \date   2022-06-28
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Libc includes */
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int _send_packet(int sock, struct sockaddr_in *addr)
{
	char buf[1600];
	size_t len = 1450;
	ssize_t snd_sz = sendto(sock, buf, len, 0, (struct sockaddr*)addr, sizeof(struct sockaddr_in));
	if (snd_sz > 0)
		return 0;

	return -1;
}


int _recv_packet(int sock)
{
	char buf[1600];
	ssize_t rcv_sz = recvfrom(sock, buf, sizeof(buf), 0, 0, 0);
	if (rcv_sz > 0)
		return 0;

	return -1;
}


int test_send_and_recv(char const *dst_ip, unsigned port)
{
	struct sockaddr_in addr;
	addr.sin_family      = AF_INET;
	addr.sin_addr.s_addr = inet_addr(dst_ip);
	addr.sin_port        = htons(2);

	/* create a blocking socket */
	int bsock = socket(AF_INET, SOCK_DGRAM, 0);
	if (bsock < 0) {
		perror("`socket` failed");
		return bsock;
	}

	/* create a non-blocking socket */
	int sock = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
	if (sock < 0) {
		perror("`socket` failed");
		return sock;
	}

	/* bind socket for recv */
	struct sockaddr_in bind_addr;
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_port = htons(port);
	bind_addr.sin_addr.s_addr = INADDR_ANY;

	int result = bind(sock, (struct sockaddr*)&bind_addr, sizeof(bind_addr));

	while (!result) {
		while (!_recv_packet(sock));
		while (!_send_packet(sock, &addr));

		/* block until we can send again */
		result = _send_packet(bsock, &addr);
	}

	return result;
}


int test_recv(unsigned port)
{
	int result = 0;

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("`socket` failed");
		return sock;
	}

	/* bind socket for recv */
	struct sockaddr_in bind_addr;
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_port = htons(port);
	bind_addr.sin_addr.s_addr = INADDR_ANY;

	result = bind(sock, (struct sockaddr*)&bind_addr, sizeof(bind_addr));

	while (!result) {
		result = _recv_packet(sock); }

	return result;
}


int main(int argc, char **argv)
{
	enum { PORT = 12345 };

	if (argc < 1) {
		fprintf(stderr, "Usage: (recv|send <dst-ip>)\n");
		return ~0;
	}
	if (argc >= 2) {
		if (strcmp(argv[0], "send") == 0)
			return test_send_and_recv(argv[1], PORT);
	}

	return test_recv(PORT);
}
