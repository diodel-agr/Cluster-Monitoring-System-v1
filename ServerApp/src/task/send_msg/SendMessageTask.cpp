
#include "SendMessageTask.h"

SendMessageTask::SendMessageTask(Request* req, char* msg)
{
	this->client = req->client;
	this->message = msg;
}

SendMessageTask::~SendMessageTask() { }

void SendMessageTask::solveTask(void)
{
	int len = strlen(this->message);
	int ret = send(this->client->getFD(), this->message, 1 + len, 0);
	if (ret != 1 + len)
	{
		std::cout << "SendMessageTask: message partially sent!" <<  std::endl;
	}
	else
	{
		std::cout << "Message sent to client " << this->client->getFD() << std::endl;
	}
}
