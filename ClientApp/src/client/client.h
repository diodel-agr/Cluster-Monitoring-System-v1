
#pragma once

/* includes */
#include <iostream>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <exception>
#include <cerrno>
#include <unistd.h>
#include "../crypto/AESalgorithm.h"
#include "../hashing/SHA256.h"
#include "task_request.h"

/* defines. */
#define DEFAULT_PROTOCOL 		0
#define DEFAULT_ADDRESS 		"127.0.0.1" 	/* default server address. */
#define DEFAULT_PORT 			2222 			/* default server port. */
#define BUFFER_SIZE 			1024 			/* receive / transmit buffer size. */
#define CLIENT_NAME				"Client"
#define CLIENT_PASS				"Pass"
#define WAIT_SEC				0	/* seconds to wait for a request from server. */
#define WAIT_USEC				100 /* microseconds to wait for a request from server. */

/*
 * Client class that will communicate with the server.
 */
class ClientTCP
{
private:
	std::string name, pass; 		/* client name and password used to authenticate to server. */
	std::string address; 			/* server address. */
	int port; 						/* server port. */
	int client_fd; 					/* socket file descriptor. */
	sockaddr_in* sock; 				/* client address information. */
	bool is_connected;				/* flag to indicate if the client is connected to the server. */
	bool is_auth;					/* flag to indicate if the client is authenticated at the server. */
	uint8_t* key;					/* 256 bit key. */
public:
	/*
	 * Void constructor.
	 */
	ClientTCP();

	/*
	 * Parameterized constructor.
	 * @addr: server address represented as a string of dotted values.
	 * @port: server port number.
	 */
	ClientTCP(std::string name, std::string pwd, std::string addr, int port);

	/*
	 * Destructor.
	 */
	virtual ~ClientTCP();

	/*
	 * init - method used to initialize the client socket for connection.
	 * @return: 0 - success, throw exception for error.
	 */
	int init(void);

	/*
	 * connect - method that establishes the connection to the server.
	 * @ return: 0 for success, negative numer for error.
	 */
	int connectToServer(void);

	/*
	 * disconnectFromServer - method used to disconnect from the server.
	 * @return: 0-success, 1-not connected, throw exception otherwise.
	 */
	int disconnectFromServer(void);

	/*
	 * sendToServer - method used to send a message to the connected server.
	 * @msg: the message to send.
	 * @size: the message length in bytes.
	 * @crypt: boolean value used to specify if the message needs encryption (true) or not (false).
	 * @return: the number of bytes sent.
	 */
	int sendToServer(uint8_t* msg, int size, bool crypt);

	/*
	 * receiveFromServer - method used to wait a message from the server and write the message to the @buffer.
	 * @buffer: buffer to store the received message.
	 * @max_size: bffer max size.
	 * @return: message length.
	 */
	int receiveFromServer(uint8_t* buffer, size_t max_size, bool decrypt);

	/*
	 * authenticate - method used to send the client's key to the server to authenticate.
	 * @return: true if the authentication is successful, false otherwise.
	 */
	bool authenticate(void);

	/*
	 * shortWaitCommand - method used to wait a specified amount of time for a request from server.
	 *
	 * @return: 1 if the socket has some data received, 0 otherwise.
	 */
	int shortWaitCommand(void);

	/*
	 * sendName-  method used to send the client name to the server.
	 *
	 * @buff: buffer to compose the message.
	 * @max_size: maximum buffer size.
	 */
	void sendName(char* buff, int max_size);

	bool isConnected(void);
};
