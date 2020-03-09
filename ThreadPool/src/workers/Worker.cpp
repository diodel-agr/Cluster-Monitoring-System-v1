
#include "Worker.h"

Worker::Worker() { }

Worker::Worker(WorkerPool* master, std::mutex* queueMutex, std::condition_variable* cv)
{
	this->master = master;
	this->queueMutex = queueMutex;
	this->cv = cv;
	this->runFlag = true;
	this->workerThread = nullptr;
}

Worker::~Worker()
{
	delete this->workerThread;
}

void Worker::start(void)
{
	this->workerThread = new std::thread(*this);
}

void Worker::stop(void)
{
	this->runFlag = false;
}

void Worker::wait(void)
{
	this->workerThread->join();
}

void Worker::operator()()
{
	consume(this);
}

void consume(Worker* w)
{
	std::cout << "Thread " << w->workerThread->get_id() << " running!" << std::endl;
	std::unique_lock<std::mutex> ulock(*w->queueMutex);
	while (w->runFlag == true)
	{
		w->cv->wait(ulock);
		std::cout << "Thread " << w->workerThread->get_id() << " has waken up!" << std::endl;
		/* extract request. */
		Request* req = w->master->removeRequest();
		if (w->runFlag == true && req != nullptr)
		{
			/* check if client is authenticated. */
			if (req->client->getAuth() == false)
			{   /* authenticate task. */
				/* check key. */
				if (req->msg[0] == TASK_AUTHENTICATE && req->msg[1] == ':')
				{
					/* found separator, authenticate. */
					ITask* authTask = TaskFactory::createTask(TASK_AUTHENTICATE, req->client, req->msg + (2 * sizeof(unsigned char)));
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
					responseTask = TaskFactory::createTask(TASK_SEND_MESSAGE, req->client, (unsigned char*)msg);
					responseTask->solveTask();
					delete responseTask;
					delete authTask;
					if (req->client->getAuth() == false)
					{
						/* disconnect client. */
						close(req->client->getFD());
						continue;
					}
				}
				else
				{   /* send error message. */
					char msg[100];
					memset(msg, 0, 100);
					msg[0] = ERR_MESSAGE;
					strcat(msg, ":Request format error.");
					ITask* msgTask = TaskFactory::createTask(TASK_SEND_MESSAGE, req->client, (unsigned char*)msg);
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
				ITask* task = TaskFactory::createTask(decrypted[0], req->client, req->msg);
				/* resolve task. */
				task->solveTask();
				/* free memory. */
				delete aes;
				delete task;
				delete decrypted;
			}
			/* append client fd to server fdset. */
			w->master->addFD(req->client);
			/* free memory. */
			delete req->msg;
			delete req;
		}
	}
	std::cout << "Thread " << w->workerThread->get_id() << " stopped!" << std::endl;
}


void consumeRequest(Request* req)
{
	std::cout << "Merge plm." << std::endl;
	/* check if client is authenticated. */
	if (req->client->getAuth() == false)
	{   /* authenticate task. */
		/* check key. */
		if (req->msg[0] == TASK_AUTHENTICATE && req->msg[1] == ':')
		{
			/* found separator, authenticate. */
			ITask* authTask = TaskFactory::createTask(TASK_AUTHENTICATE, req->client, req->msg + (2 * sizeof(unsigned char)));
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
			responseTask = TaskFactory::createTask(TASK_SEND_MESSAGE, req->client, (unsigned char*)msg);
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
			ITask* msgTask = TaskFactory::createTask(TASK_SEND_MESSAGE, req->client, (unsigned char*)msg);
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
		ITask* task = TaskFactory::createTask(decrypted[0], req->client, req->msg);
		/* resolve task. */
		task->solveTask();
		/* free memory. */
		delete aes;
		delete task;
		delete decrypted;
	}
	/* append client fd to server fdset. */
	//w->master->addFD(req->client);  /////////////////// Nu uita de pasul asta.
	/* free memory. */
	delete req->msg;
	delete req;
}
