
#include "WorkerPool.h"

WorkerPool::WorkerPool()
{
	this->server = nullptr;
	this->requests = new std::queue<Request*>();
	this->workers = new std::vector<Worker*>(WORKER_NO);
	this->requestMutex = new std::mutex;
	this->cv = new std::condition_variable;
	/* instantiate and start each worker. */
	this->createAndStartWorkers(WORKER_NO);
}

WorkerPool::WorkerPool(ServerTCP* server)
{
	this->server = server;
	this->requests = new std::queue<Request*>();
	this->workers = new std::vector<Worker*>(WORKER_NO);
	this->requestMutex = new std::mutex;
	this->cv = new std::condition_variable;
	/* instantiate and start each worker. */
	this->createAndStartWorkers(WORKER_NO);
}

WorkerPool::WorkerPool(ServerTCP* server, int worker_no)
{
	this->server = server;
	this->requests = new std::queue<Request*>();
	this->workers = new std::vector<Worker*>(worker_no);
	this->requestMutex = new std::mutex;
	this->cv = new std::condition_variable;
	/* instantiate and start each worker. */
	this->createAndStartWorkers(worker_no);
}

WorkerPool::~WorkerPool()
{
	/* stop workers. */
	for (unsigned int i = 0; i < this->workers->size(); ++i)
	{
		this->workers->at(i)->stop();
	}
	this->cv->notify_all();
	for (unsigned int i = 0; i < this->workers->size(); ++i)
	{
		this->workers->at(i)->wait();
		delete this->workers->at(i);
	}
	while (this->requests->empty() == false)
	{
		Request* req = this->requests->front();
		this->requests->pop();
		delete req;
	}
	/* free memory. */
	delete this->workers;
	delete this->requests;
	delete this->requestMutex;
	delete this->cv;
}

void WorkerPool::createAndStartWorkers(int worker_no)
{
	for (int i = 0; i < worker_no; ++i)
	{
		try
		{
			///Worker* worker = new Worker(this, this->requestMutex, this->cv);
			//worker->start();
			std::thread thr(Worker(this, this->requestMutex, this->cv));
			//this->workers->push_back(worker);
		}
		catch (std::exception e)
		{
			std::cout << errno << std::endl;
		}
	}
}

void WorkerPool::appendRequest(Client* client, uint8_t* msg, int size)
{
	/* insert request into the queue. */
	Request* request = new Request(client, msg, size);
	this->requests->push(request);
	/* notify workers. */
	std::unique_lock<std::mutex> ulock(*this->requestMutex);
	this->cv->notify_one();
}

Request* WorkerPool::removeRequest()
{
	Request* result = nullptr;
	if (this->requests->empty() == false)
	{
		result = this->requests->front();
		this->requests->pop();
	}
	return result;
}

void WorkerPool::addFD(Client* client)
{
	this->server->addFD(client);
}
