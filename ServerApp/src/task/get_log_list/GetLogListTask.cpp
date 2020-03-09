
#include "GetLogListTask.h"

GetLogListTask::GetLogListTask(Request* req, char* msg)
{

	this->req = req;
	this->msg = new char[1 + strlen(msg)];
	strcpy(this->msg, msg);
}

GetLogListTask::~GetLogListTask()
{
	delete this->msg;
}

void GetLogListTask::solveTask()
{
	int len = 0, code;
	AESalgorithm* aes = new AESalgorithm(256);
	uint8_t* encrypted, *decrypted;
	char* buffer = new char[BUFFER_SIZE];
	std::string* response = new std::string;
	*response = "";
	/* get source client. */
	Client* srcClient = this->req->server->getClientByName(this->msg);
	/* request log list. */
	memset(buffer, '\0', BUFFER_SIZE);
	sprintf(buffer, "%d:", TASK_GET_LOG_LIST);
	len = 1 + strlen(buffer);
	encrypted = aes->encrypt((uint8_t*)buffer, len, srcClient->getKey());
	send(srcClient->getFD(), encrypted, len, 0);
	delete encrypted;
	/* wait for response. */
	do
	{
		len = -1;
		memset (buffer, '\0', BUFFER_SIZE);
		while (len == -1) {
			//this->waitForDataOnSocket(srcClient->getFD());
			len = recv(srcClient->getFD(), buffer, BUFFER_SIZE, MSG_DONTWAIT);
			std::cout << "plm\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DELAY));
		}
		decrypted = aes->decrypt((uint8_t*)buffer, len, srcClient->getKey());
		code = (int)decrypted[0];
		if (code == RESP_LOG_LIST)
		{
			*response += (char*)(decrypted + 2);
		}
		delete decrypted;
	} while (len != BUFFER_SIZE && code == RESP_LOG_LIST);
	/* send response to administrator. */
	char* respptr = (char*)response->c_str();
	int size = 1 + response->length();
	int sent = 0;
	int amount;
	memset(buffer, '\0', BUFFER_SIZE);
	buffer[0] = RESP_LOG_LIST;
	buffer[1] = ':';
	do
	{
		if (size - sent > 1021)
		{
			amount = 1021;
		}
		else
		{
			amount = size - sent;
		}
		memcpy((buffer + 2), respptr, amount);
		buffer[amount + 2] = '\0';
		respptr += amount;
		sent += amount;
		/* send. */
		encrypted = aes->encrypt((uint8_t*)buffer, amount, this->req->client->getKey());
		send(this->req->client->getFD(), encrypted, amount, MSG_WAITALL);
		delete encrypted;
	} while (sent != size);
	/* add sender to server fdset. */
	this->req->server->addFD(srcClient);
	/* release memory. */
	delete response;
	delete buffer;
	delete aes;
}

void GetLogListTask::sendReady(int fd)
{
	char msg[5];
	memset(msg, '\0', 5);
	sprintf(msg, "%d:", TASK_READY);
	send(fd, msg, 1 + strlen(msg), 0);
}
