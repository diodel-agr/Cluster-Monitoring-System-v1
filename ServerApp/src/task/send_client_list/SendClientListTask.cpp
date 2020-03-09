
#include "SendClientListTask.h"

SendClientListTask::SendClientListTask(Request* req, char* msg)
{
	this->req = req;
}

SendClientListTask::~SendClientListTask() { }

void SendClientListTask::solveTask(void)
{
	std::string* clientList = this->req->server->getClientList();
	char* buffer = new char[3 + clientList->length()];
	sprintf(buffer, "%d:%s", RESP_CLIENT_LIST, clientList->c_str());
	/* encrypt message. */
	AESalgorithm* aes = new AESalgorithm(256);
	int len = 1 + strlen(buffer);
	uint8_t* encrypted = aes->encrypt((uint8_t*)buffer, len, this->req->client->getKey());
	/* send response. */
	int sent = send(this->req->client->getFD(), encrypted, len, 0);
	if (sent != len)
	{
		std::cout << "Client List partially sent!" << std::endl;
	}
	delete clientList;
	delete buffer;
	delete encrypted;
	delete aes;
}
