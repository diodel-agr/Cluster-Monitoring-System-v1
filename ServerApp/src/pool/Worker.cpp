
#include "Worker.h"

void consumeRequest(Request* req)
{
	/* check if client is authenticated. */
	if (req->client->getAuth() == false)
	{   /* authenticate task. */
		/* check key. */
		if (req->msg[0] == TASK_AUTHENTICATE && req->msg[1] == ':')
		{
			/* found separator, authenticate. */
			ITask* authTask = TaskFactory::createTask(TASK_AUTHENTICATE, req, req->msg + (2 * sizeof(unsigned char)));
			authTask->solveTask();
			char msg[2];
			memset(msg, 0, 2);
			ITask* responseTask = nullptr;
			if (req->client->getAuth() == false)
			{   /* send error message. */
				msg[0] = ERR_AUTHENTICATE;
			}
			else
			{
				msg[0] = RESP_SUCCESS;
			}
			msg[1] = '\0';
			responseTask = TaskFactory::createTask(TASK_SEND_MESSAGE, req, (unsigned char*)msg);
			responseTask->solveTask();
			delete responseTask;
			delete authTask;
			if (req->client->getAuth() == false)
			{
				/* disconnect client. */
				close(req->client->getFD());
				return;
			}
		}
		else
		{   /* send error message. */
			char msg[100];
			memset(msg, 0, 100);
			msg[0] = ERR_MESSAGE;
			strcat(msg, ":Request format error.");
			ITask* msgTask = TaskFactory::createTask(TASK_SEND_MESSAGE, req, (unsigned char*)msg);
			msgTask->solveTask();
			delete msgTask;
		}
	}
	else /* respond to request. */
	{
		/* decrypt message. */
		AESalgorithm* aes = new AESalgorithm(256);
		uint8_t* decrypted = aes->decrypt(req->msg, req->size, req->client->getKey());
		/* instantiate task. */
		ITask* task = TaskFactory::createTask(decrypted[0] - '0', req, decrypted + (2 * sizeof(char)));
		/* resolve task. */
		if (task == nullptr)
		{
			std::cout << "Worker task is null! Task: " << decrypted[0] << std::endl;
		}
		else
		{
			task->solveTask();
			delete task;
		}
		/* free memory. */
		delete aes;
		delete decrypted;
	}
	/* append client fd to server fdset. */
	req->server->addFD(req->client);
	/* free memory. */
	delete req->msg;
	delete req;
}
