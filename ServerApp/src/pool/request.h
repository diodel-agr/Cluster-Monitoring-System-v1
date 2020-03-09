#pragma once

#include <mutex>
#include <condition_variable>

#include "../server/client.h"
#include "../server/server.h"

class ServerTCP;

struct Request
{
	ServerTCP* server;
	Client* client;
	uint8_t* msg;
	int size;

	Request(ServerTCP* server, Client* cl, uint8_t* mesg, int sz)
	{
		this->server = server;
		this->client = cl;
		this->msg = mesg;
		this->size = sz;
	}
};
