
/* includes. */
#include "client.h"

void printHash256(uint32_t* hash)
{
	for (int i = 0; i < 8; ++i)
	{
		std::cout << std::hex << *((uint32_t*)(hash + i)) << " ";
	}
	std::cout << std::endl;
}

ClientTCP::ClientTCP()
{
	this->sock = nullptr;
	this->is_connected = false;
	this->is_auth = false;
}

ClientTCP::ClientTCP(std::string name, std::string pwd, std::string addr, int port) : ClientTCP()
{
	this->name = name;
	this->pass = pwd;
	this->address = addr;
	this->port = port;
	/* generate key. */
	uint64_t size = strlen(this->name.c_str()) + strlen(this->pass.c_str()) + 1;
	unsigned char* uname = new unsigned char[strlen(this->name.c_str()) + strlen(this->pass.c_str()) + 1];
	memset(uname, '\0', size);
	strcpy((char*)uname, this->name.c_str());
	strcat((char*)uname, this->pass.c_str());
	this->key = (uint8_t*)SHA256::computeDigest(uname, size);
	/* afisam cheia. */
	for (int i = 0; i < 64; ++i)
	{
		std::cout << (unsigned int)this->key[i] << " ";
	}
	std::cout << std::endl;

	printHash256((uint32_t*)this->key);
}

ClientTCP::~ClientTCP()
{
	if (this->is_connected == true)
	{
		close(this->client_fd);
	}
	delete this->key;
	delete this->sock;
}

int ClientTCP::init()
{
	/* create socket. */
	this->client_fd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL);
	if (this->client_fd == -1) {
		std::cout << "Error: " << errno << std::endl;
		return -1;
	}

	/* configure socket. */
	if (this->sock == nullptr)
	{
		this->sock = new sockaddr_in;
	}
	this->sock->sin_family = AF_INET;      /* host byte order */
	this->sock->sin_port = htons(this->port);    /* short, network byte order */
	int result = inet_aton(this->address.c_str(), &this->sock->sin_addr);
	if (result == 0)
	{	/* address string error. */
		throw "Invalid address string.";
	}
	return 0;
}

int ClientTCP::connectToServer(void)
{
	if (this->is_connected == false)
	{
		int result = connect(this->client_fd, (sockaddr*)this->sock, sizeof(sockaddr_in));
		if (result == -1)
		{
			return errno;
		}
		this->is_connected = true;
		return result;
	}
	else
	{
		return 0;
	}
}

int ClientTCP::disconnectFromServer(void)
{
	int res = 1;
	if (this->is_connected == true)
	{
		int result = close(this->client_fd);
		if (result == -1)
		{
			throw "Error closing connection to server: " + errno;
		}
		res = 0;
	}
	this->is_connected = false;
	this->is_auth = false;
	return res;
}

int ClientTCP::sendToServer(uint8_t* msg, int size, bool crypt)
{
	if (this->is_connected == true)
	{
		void* buffer = (void*)msg;
		if (crypt == true)
		{
			AESalgorithm* aes = new AESalgorithm(256);
			uint8_t* encr_msg = nullptr;
			try
			{
				encr_msg = aes->encrypt((const uint8_t*)msg, size, this->key);
			}
			catch (std::exception e)
			{
				std::cout << strerror(errno) << std::endl;
			}
			buffer = (void*)encr_msg;
			delete aes;
		}
		/* returns the number of bytes sent. */
		int bytes = send(this->client_fd, buffer, size, 0); /* s-ar putea sa trebuiasca sa folosesti un flag aici pentru cand nu se poate trimite tot mesajul o data. */
		if (bytes == -1)
		{
			throw "Client send to server error: " + errno;
		}
		else if (bytes != size)
		{
			std::cout << "Message partially sent!" << std::endl;
		}
		if (crypt == true)
		{
			delete (uint8_t*)buffer;
		}
		return bytes;
	}
	else
	{
		return -1;
	}
}

int ClientTCP::receiveFromServer(uint8_t* buffer, size_t max_size, bool decrypt)
{
	int len = recv(this->client_fd, (void*)buffer, max_size, 0);
	if (len != -1 && decrypt == true)
	{
		/* decrypt message. */
		AESalgorithm* aes = new AESalgorithm(256);
		uint8_t* decrypted = aes->decrypt(buffer, len, this->key);
		memcpy(buffer, decrypted, len);
		buffer[len] = '\0';
		delete decrypted;
		delete aes;
	}
	return len;
}

bool ClientTCP::authenticate(void)
{
	/* authenticate. */
	uint8_t msg_buf[100];
	memset(msg_buf, 0, 100);
	msg_buf[0] = TASK_AUTHENTICATE;
	msg_buf[1] = ':';
	memcpy(msg_buf+2, this->key, 32);
	std::cout << msg_buf << std::endl;
	this->sendToServer(msg_buf, 2 + 32, false);
	/* receive response. */
	int lenret = this->receiveFromServer(msg_buf, 100, false);
	std::cout << msg_buf << std::endl;
	if ((lenret == 2) && (msg_buf[0] == RESP_SUCCESS))
	{
		this->is_auth = true;
		return true;
	}
	return false;
}

int ClientTCP::shortWaitCommand(void)
{
	if (this->is_connected == true)
	{
		timeval* tv = new timeval;
		fd_set* temp_fdset = new fd_set;
		/* set fd set. */
		FD_SET(this->client_fd, temp_fdset);
		/* set timeval. */
		tv->tv_sec = WAIT_SEC;
		tv->tv_usec = WAIT_USEC;

		/* execute select. */
		int result =  select(1 + this->client_fd, temp_fdset, NULL, NULL, tv);
		delete tv;
		delete temp_fdset;
		return result;
	}
	else
	{
		return 0;
	}
}

void ClientTCP::sendName(char* buff, int size)
{
	memset(buff, '\0', size);
	sprintf(buff, "%d:%s_%s", TASK_SET_NAME, this->name.c_str(), this->pass.c_str());
	/* send to server. */
	this->sendToServer((uint8_t*)buff, 1 + strlen(buff), true);
}

bool ClientTCP::isConnected(void)
{
	return this->is_connected;
}
