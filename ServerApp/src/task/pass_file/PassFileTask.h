
#pragma once

#include "../task_request.h"
#include "../../pool/request.h"
#include "../ITask.h"
#include "../../crypto/AESalgorithm.h"

#include <iostream>

#define SLEEP_DELAY		2000

/*
 * PassFileTask - class used to receive a file from a client and send it to another file.
 */
class PassFileTask : public ITask
{
private:
	Request* req;		/* request object. */
	char* cmd;			/* command from receiver. */
public:
	/*
	 * Constructor.
	 *
	 * @request: request object.
	 * @msg: Message used to identify the sender.
	 */
	PassFileTask(Request* req, char* msg);

	/*
	 * Destructor.
	 */
	~PassFileTask();

	/*
	 * solveTask - 	method used to receive a file from a client and
	 * 				send it to another.
	 */
	void solveTask(void);
};
