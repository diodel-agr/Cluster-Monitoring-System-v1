
#include "AuthenticateTask.h"

AuthenticateTask::AuthenticateTask(Request* req, unsigned char* key)
{
	this->client = req->client;
	this->key = key;
}

AuthenticateTask::~AuthenticateTask() { }

void AuthenticateTask::solveTask()
{
	Authenticator* auth = Authenticator::getInstance();
	if (auth->checkKey(this->key) == true)
	{   /* set client key and set the authentication flag. */
		uint8_t* clientKey = new uint8_t[32];
		memset(clientKey, 0, 32);
		memcpy(clientKey, this->key, 32);
		this->client->setKey(clientKey);
		this->client->setAuth(true);
		std::cout << "Client " << this->client->getFD() << " authenticated successfully!" << std::endl;
	}
	else
	{
		std::cout << "Client " << this->client->getFD() << " authentication error!" << std::endl;
	}
}
