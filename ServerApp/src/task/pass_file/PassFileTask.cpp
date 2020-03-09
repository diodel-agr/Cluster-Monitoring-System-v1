
#include "PassFileTask.h"

PassFileTask::PassFileTask(Request* req, char* msg)
{
	this->req = req;
	this->cmd = new char[1 + strlen(msg)];
	strcpy(this->cmd, msg);
}

PassFileTask::~PassFileTask()
{
	delete this->cmd;
}

void PassFileTask::solveTask()
{
	char* buffer = new char[BUFFER_SIZE];
	AESalgorithm* aes = new AESalgorithm(256);
	uint8_t* encrypted, *decrypted;
	int len;
	/* obtain sender. */
	char* name = strtok(this->cmd, ":");
	if (name == NULL)
	{
		std::cout << "PassFileTask could not find the name separator." << std::endl;
	}
	Client* sender = req->server->getClientByName(name);
	if (sender == nullptr)
	{
		std::cout << "PassFileTask could not obtain reference to sender " << name << "!" << std::endl;
		sprintf(buffer, "%d:%s%s", ERR_MESSAGE, "Could not obtain reference to sender ", name);
		len = 1 + strlen(buffer);
		encrypted = aes->encrypt((uint8_t*)buffer, len, this->req->client->getKey());
		send(this->req->client->getFD(), encrypted, len, 0);
		delete encrypted;
		delete buffer;
		delete aes;
		/* add sender to server fdset. */
		this->req->server->addFD(sender);
		return;
	}
	/* request file. */
	name = strtok(NULL, ":");
	if (name == NULL)
	{
		std::cout << "PassFileTask message error." << std::endl;
	}
	memset(buffer, '\0', BUFFER_SIZE);
	sprintf(buffer, "%d:%s", TASK_GET_LOG, name);
	len = 1 + strlen(buffer);
	encrypted = aes->encrypt((uint8_t*)buffer, len, sender->getKey());
	send(sender->getFD(), encrypted, len, 0);
	/* pass file. */
	while (true) {
		std::cout << "Asteapta de la sender." << std::endl;
		/* receive file part. */
		this->waitForDataOnSocket(sender->getFD());
		do
		{
			len = recv(sender->getFD(), buffer, BUFFER_SIZE, MSG_DONTWAIT); // MSG_WAITALL
			std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DELAY));
		} while (len == -1);
		/* decrypt received file part. */
		decrypted = aes->decrypt((uint8_t*)buffer, len, sender->getKey());
		std::cout << "A primit de la sender: " << decrypted << std::endl;
		if (decrypted[0] == RESP_LOG || decrypted[0] == RESP_LOG_END)
		{
			/* print result. */
			//std::cout << "Recv: " << (char*)decrypted << std::endl;
			/* encrypt buffer. */
			encrypted = aes->encrypt(decrypted, len, this->req->client->getKey());
			/* send to receiver. */
			std::cout << "Trimite la receiver." << std::endl;
			int sent = send(this->req->client->getFD(), encrypted, len, 0);
			if (sent != len)
			{
				std::cout << "File partially sent to client " << this->req->client->getFD() << std::endl;
			}
		}
		else if (decrypted[0] != '\0')/* error occured. */
		{
			char value;
			sprintf(&value, "%d", decrypted[0]);
			std::cout << "PassFileTask invalid response message: " << value << std::endl;
			/* send error message to receiver. */
			len = 1 + strlen((char*)decrypted);
			encrypted = aes->encrypt(decrypted, len, this->req->client->getKey());
			int sent = send(this->req->client->getFD(), encrypted, len, 0);
			if (sent != len)
			{
				std::cout << "File partially sent to client " << this->req->client->getFD() << std::endl;
			}
			delete encrypted;
			delete decrypted;
			break;
		}
		else
		{
			delete decrypted;
			continue;
		}
		if (decrypted[0] == RESP_LOG_END)
		{
			std::cout << "A primit mesajul de final." << std::endl;
			delete encrypted;
			delete decrypted;
			break;
		}
		else
		{
			delete decrypted;
			delete encrypted;
		}
	}
	std::cout << "A terminat." << std::endl;
	/* delete resources. */
	delete buffer;
	delete aes;
	/* add sender to server fdset. */
	this->req->server->addFD(sender);
}
