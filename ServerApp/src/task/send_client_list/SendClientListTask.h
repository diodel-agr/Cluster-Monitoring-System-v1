
#pragma once

#include "../ITask.h"
#include "../../server/client.h"
#include "../../pool/request.h"

class SendClientListTask : public ITask
{
private:
	Request* req;
public:
	SendClientListTask(Request* req, char* msg);
	virtual ~SendClientListTask();
	void solveTask(void);
};
