
#pragma once

/* includes. */
#include <thread>
#include "request.h"
#include "WorkerPool.h"
#include "../crypto/AESalgorithm.h"
#include "../task/task_request.h"
#include "../task/TaskFactory.h"

class WorkerPool;

/*
 * Worker - class that will resolve clients requests.
 */
class Worker
{
/* fields. */
public:
	bool runFlag;
	std::mutex* queueMutex;
	std::condition_variable* cv;
	WorkerPool* master;
	std::thread* workerThread;
/* methods. */
public:
	/*
	 * Void constructor.
	 */
	Worker();

	/*
	 * Parameterized constructor.
	 */
	Worker(WorkerPool* master, std::mutex* queueMutex, std::condition_variable* cv);

	/* Destructor. */
	~Worker();

	/*
	 * start - method used to start the worker thread.
	 */
	void start(void);

	/*
	 * stop - method used to set the runFlag to false.
	 */
	void stop(void);

	/*
	 * wait - method used to wait for the worker thread to finish executing.
	 */
	void wait(void);

	/*
	 * Override operator() to avoid std::thread exceptions.
	 */
	void operator()();

//public:
	/*
	 * consume - the function that will treat the client request.
	 */
	//void consume(void);
	//static void consume(Worker* w);
};

void consume(Worker* w);

void consumeRequest(Request* req);



