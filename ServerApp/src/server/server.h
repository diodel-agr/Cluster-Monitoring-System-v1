
#pragma once

/**
 * includes.
 */
#include <iostream>
#include <string>
#include <cerrno>
#include <vector>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../exception/server_exception.h"
#include "../pool/ThreadPool.h"
#include "client.h"

/*
 * defines.
 */
#define DEFAULT_PORT 		2222
#define DEFAULT_PROTOCOL 	0
#define DEFAULT_ADDRESS		"127.0.0.1"
#define ERROR				-1
#define BACKLOG				10
#define SERVER_WAIT_SEC		1 			/* number of seconds to wait for a new connection or message. */
#define SERVER_WAIT_USEC	0 			/* number of microseconds to wait for a new connection or message. */
#define SERVER_RUN			10000 		/* ~( SERVER_WAIT_SEC * SERVER_RUN ) seconds. */
#define BUFFER_SIZE			1024 		/* buffer size in bytes. */
#define DEFAULT_WORKERS		1			/* default number of workers to set. */
#define WORKER_NO			5

class WorkerPool;

/*
 * ServerTCP - class that will handle client connection and communication.
 */
class ServerTCP {

private:
	uint port; 									/* port used by the server. Default value 2222. */
	int server_fd; 								/* file descriptor for the server. */
	sockaddr_in* server_address; 				/* structure that holds the server's address and port. */
	fd_set* socket_fdset; 						/* fd set to manage socket file descriptors. */
	std::mutex* fdset_mutex;					/* mutex used to synchronize access to the fdset. */
	std::mutex* clients_mutex;					/* mutex used to synchronize access to the client vector. */
	std::vector<Client*>* clients;				/* vector of Clients. */
	ThreadPool* tp;								/* thread pool used to treat client requests. */
	int maxfd;									/* used by the 'select' function. */
	bool run_flag;								/* used to stop the server. */
public:
	/*
	 * Void constructor.
	 */
	ServerTCP(void);

	/*
	 * Parameterized constructor.
	 */
	ServerTCP(uint);

	/*
	 * Destructor.
	 */
	~ServerTCP(void);

	/*
	 * init() - initialization function.
	 * @return: 0 for success, raise exception otherwise.
	 *
	 * Attempts to open a socket on the port specified by the @port field on local host.
	 */
	uint open(void);

	/*
	 * loop() - the main loop of the server.
	 * @return: the error code for this operation: 0 - success, raise exception otherwise.
	 *
	 * Listens to all ports and handle clients connection.
	 */
	void loop(void);

	/*
	 * shutdown() - stops the server.
	 * @return: the error code for this operation: 0 - success, raise exception otherwise.
	 *
	 * Close all open connections and stops the server.
	 */
	int shutdown(void);

	/*
	 * addFD - method used to add a file descriptor to the server fd_set.
	 * @client: client structure containing the fd.
	 */
	void addFD(Client* client);

	/*
	 * getClient - method used to obtain a client based on his file descriptor.
	 *
	 * @clientfd: client file descriptor.
	 * @return: client object.
	 */
	Client* getClient(int clientfd);

	/*
	 * getClientByName - method used to return the client based on its name.
	 *
	 * @name: client's name.
	 * @return: reference to the client or null.
	 */
	Client* getClientByName(char* name);

	/*
	 * getClientList - method used to obtain the list of connected clients.
	 *
	 * @return: string representing the clients name.
	 */
	std::string* getClientList(void);

	std::vector<Client*>* getClients(void)
		{
		return this->clients;
		}

private:
	/*
	 * setTimeVal() - set the timeval variable with the specified parameters.
	 * @tv: timeval variable to set.
	 * @sec: number of seconds.
	 * @usec: number of microseconds.
	 */
	void setTimeval(timeval* tv, time_t sec, suseconds_t usec);

	/*
	 * acceptConnection - accept connection from a client.
	 */
	void acceptConnection(void);

};
