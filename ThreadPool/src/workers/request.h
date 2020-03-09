#pragma once

#include <mutex>
#include <condition_variable>
#include "../server/client.h"

struct Request
{
	Client* client;
	uint8_t* msg;
	int size;

	Request(Client* cl, uint8_t* mesg, int sz)
	{
		this->client = cl;
		this->msg = mesg;
		this->size = sz;
	}
};
