
#pragma once

#include <sys/select.h>
#include <cstring>
#include <iostream>

#define BUFFER_SIZE		1024
#define USEC			50 * 1000 /* milliseconds delay. */
/*
 * ITask interface. Define the operation used to execute all client requests.
 */
class ITask
{
public:
	virtual ~ITask() { }
	virtual void solveTask(void) = 0;

	/*
	 * waitForDataOnSocket - method used to wait for some data to be ready to read from the @sock socket.
	 *
	 * @sock: socket to sniff.
	 */
	int waitForDataOnSocket(int sock)
	{
		int result, ok = 1;
		fd_set* fdset = new fd_set;
		timeval* tv = new timeval;
		tv->tv_sec = 0;
		/* loop until some message has arrived on @sock socket. */
		std::cout << "Wait for message on sock " << sock << std::endl;
		while (ok == 1)
		{
			/* set-up. */
			FD_ZERO(fdset);
			FD_SET(sock, fdset);
			tv->tv_usec = USEC;
			/* wait. */
			result = select(1 + sock, fdset, NULL, NULL, tv);
			if (result == -1)
			{   /* error. */
				std::cout << "Error ITask select." << std::endl;
				ok = 0;
			}
			else if (result == 0) {
				std::cout << "???" << std::endl;
				ok = 0;
			}
			else if (FD_ISSET(sock, fdset))
			{
				/* message ready to read. */
				ok = 0;
			}
			else
			{
				std::cout << "Don't know what is happening!" << std::endl;
				ok = 0;
			}
		}
		std::cout << "Ready to read socket " << sock << std::endl;
		/* release resources. */
		delete fdset;
		delete tv;
		return result;
	}
};
