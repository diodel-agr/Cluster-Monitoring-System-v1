
#pragma once

#include <iostream>
#include "../ITask.h"
#include "../../pool/request.h"

class Request;

class SendMessageTask : public ITask
{
private:
	Client* client;
	char* message;
public:
	/*
	 * Parameterized constructor.
	 */
	SendMessageTask(Request* req, char* msg);

	/*
	 * Destructor.
	 */
	~SendMessageTask();

	/*
	 * solveTask - send a message to the corresponding client.
	 */
	void solveTask(void);
};
