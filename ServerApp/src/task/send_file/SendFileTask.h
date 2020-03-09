
#pragma once

#include <iostream>
#include <fstream>
#include "../ITask.h"
#include "../../crypto/AESalgorithm.h"
#include "../../pool/request.h"

class Request;

class SendFileTask : public ITask {
private:
	Client* client;
	char* filename;
public:
	/*
	 * Parameterized constructor.
	 */
	SendFileTask(Request* request, char* filename);

	/*
	 * Destructor.
	 */
	~SendFileTask();

	/*
	 * solveTask: resolve the client request.
	 * Steps:
	 * 1. Read file from computer.
	 * 2. Encrypt file using client's key.
	 * 3. Send file to client.
	 */
	void solveTask(void);
};
