
#pragma once

#include <mutex>
#include "../ITask.h"
#include "../../single/Authenticator.h"
#include "../../pool/request.h"

struct Request;

class AuthenticateTask : public ITask
{
private:
	Client* client;
	unsigned char* key;
public:
	/*
	 * Parameterized constructor.
	 * @client: client reference.
	 * @key: client's key.
	 */
	AuthenticateTask(Request* req, unsigned char* key);

	/*
	 * Destructor.
	 */
	~AuthenticateTask();

	/*
	 * solveTask - check if the client's key is known.
	 */
	void solveTask(void);
};
