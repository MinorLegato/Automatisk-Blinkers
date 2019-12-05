#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define LOCAL_HOST  ("127.0.0.1")

struct Server {
	int          	socket;
	sockaddr_in  	addr;
};
        
struct Client {
	int          	socket;
	sockaddr_in  	addr;
};

static void netServerInit(Server *server, int port) {
	memset(server, 0, sizeof server);

	server->socket = socket(AF_INET, SOCK_DGRAM, 0);

	server->addr.sin_family         = AF_INET;
	server->addr.sin_addr.s_addr    = INADDR_ANY;
	server->addr.sin_port           = htons(port);

	bind(server->socket, (sockaddr *)&server->addr, sizeof server->addr);
}

static int netServerRecv(Server *server, void *data, size_t data_size) {
	int len = recvfrom(server->socket, data, data_size, 0, NULL, 0);
	return len;
}

#if 0
static void netClientInit(Client *client, const char *ip, int port) {
	memset(client, 0, sizeof client);

	client->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	client->addr.sin_family              = AF_INET;
	client->addr.sin_port                = htons(port);
	client->addr.sin_addr.S_un.S_addr    = inet_addr(ip);
}

static int netClientSend(Client *client, const void *data, size_t data_size) {
	sendto(client->socket, data, data_size, 0, (sockaddr *)&client->addr, sizeof client->addr);
}
#endif

