
#include "SetNameTask.h"

SetNameTask::SetNameTask(Request* req, char* msg)
{
	this->client = req->client;
	this->name = new char[1 + strlen(msg)];
	strcpy(this->name, msg);
}

SetNameTask::~SetNameTask()
{
	delete this->name;
}

void SetNameTask::solveTask(void)
{
	this->client->setName(this->name);
}
