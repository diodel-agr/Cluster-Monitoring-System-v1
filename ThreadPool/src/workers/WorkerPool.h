
#pragma once

/* includes. */
#include <vector>
#include <queue>
#include "request.h"
#include "Worker.h"
#include "../server/server.h"

/* defines. */
#define WORKER_NO 		5 /* default number of workers to instantiate. */

class Worker;
class ServerTCP;
/*
 * WorkerPool - this class will manage the workers that will process clients requests.
 */
class WorkerPool
{
/* fields. */
private:
	ServerTCP* server;
	std::vector<Worker*>* workers;			/* vector of workers. */
	std::queue<Request*>* requests;			/* queue of requests. */
	std::mutex* requestMutex;				/* mutex to synchronize queue access. */
	std::condition_variable* cv;			/* condition variable used to notify workers. */
/* methods. */
public:
	/*
	 * Void constructor.
	 */
	WorkerPool();

	/*
	 * Parameterized constructor.
	 * @server: server reference.
	 * @worker_no: number of workers to instantiate.
	 */
	WorkerPool(ServerTCP* server);

	/*
	 * Parameterized constructor.
	 * @server: server reference.
	 * @worker_no: number of workers to instantiate.
	 */
	WorkerPool(ServerTCP* server, int worker_no);

	/* destructor. */
	~WorkerPool();

	/*
	 * removeRequest - method called by the workers to pop one request from the queue.
	 * @return: a request or nullptr if the queue is empty.
	 */
	Request* removeRequest();

	/*
	 * appendRequest - used to append a request to the queue, waiting for a worker thread to consume it.
	 * @client: client information.
	 * @msg: client message. Needs decryption.
	 * @size: message size.
	 */
	void appendRequest(Client* client, uint8_t* msg, int size);

	/*
	 * addFD - request to add a file descriptor to the server socket_fdlset.
	 * @client: client structure.
	 */
	void addFD(Client* client);
private:
	/*
	 * createAndStartWorkers - instantiates @worker_no workers.
	 * @worker_no: number of workers to create.
	 */
	void createAndStartWorkers(int worker_no);
};
