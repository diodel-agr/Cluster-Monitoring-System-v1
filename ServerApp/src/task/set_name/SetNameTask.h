
#pragma once

#include "../ITask.h"
#include "../../server/client.h"
#include "../../pool/request.h"

class SetNameTask : public ITask
{
private:
	Client* client;
	char* name;
public:
	/*
	 * Constructor.
	 */
	SetNameTask(Request* req, char* msg);

	/*
	 * Destructor.
	 */
	~SetNameTask();

	/*
	 * solveTask - method used to set the name of the client.
	 */
	void solveTask(void);
};
