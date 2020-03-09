
#include "server.h"

ServerTCP::ServerTCP(void)
{
	this->port = DEFAULT_PORT;
	this->server_fd = 0;
	this->server_address = nullptr;
	this->socket_fdset = nullptr;
	this->clients = nullptr;
	this->fdset_mutex = new std::mutex;
	this->clients_mutex = new std::mutex;
	this->tp = nullptr;
}

ServerTCP::ServerTCP(uint port) : ServerTCP()
{
	this->port = port;
}

ServerTCP::~ServerTCP(void)
{
	if (this->server_fd != 0)
	{
		this->clients_mutex->lock();
		this->shutdown();
		this->clients_mutex->unlock();
		/* disconnect clients. */
		std::cout << "Disconnect clients." << std::endl;
		/* close server socket. */
		std::cout << "Close server socket." << std::endl;
	}
	delete this->server_address;
	delete this->socket_fdset;
	if (this->tp != nullptr)
	{
		this->tp->shutdown();
		delete this->tp;
	}
	delete this->fdset_mutex;
	delete this->clients_mutex;
}

uint ServerTCP::open(void)
{
	/* create unbound socket. */
	this->server_fd = socket(AF_INET, SOCK_STREAM, DEFAULT_PROTOCOL); /* Internet addresses, TCP */
	if (this->server_fd == ERROR)
	{
		throw new ServerException(errno);
	}

	/* init fdset. */
	this->socket_fdset = new fd_set;
	FD_ZERO(this->socket_fdset);
	FD_SET(this->server_fd, this->socket_fdset);
	this->clients = new std::vector<Client*>();
	this->maxfd = this->server_fd;

	/* configure socket. */
	int value = 1;
	int result = setsockopt(this->server_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &value, sizeof (int)); /* bind() should allow reuse of local addresses. */
	if (result < 0)
	{
		throw new ServerException("Set socket option error.");
	}
	/* configure server address. */
	if (this->server_address == nullptr)
	{
		this->server_address = new sockaddr_in;
	}
	this->server_address->sin_family = AF_INET;
	this->server_address->sin_port = htons(this->port);
	result = inet_aton(DEFAULT_ADDRESS, &this->server_address->sin_addr);
	if (result == 0)
	{	/* address string error. */
		throw new ServerException("Invalid address string.");
	}

	/* bind socket to address. */
	result = bind(this->server_fd, (sockaddr*)this->server_address, sizeof(sockaddr_in));
	if (result < 0)
	{
		throw new ServerException(errno);
	}

	std::cout << "Server listens on port: " << this->server_address->sin_port << std::endl;
	return 0;
}

int ServerTCP::shutdown(void)
{
	/* close connections with all clients. */
	int n = this->clients->size();
	while (n != 0)
	{
		Client* client = this->clients->at(n - 1);
		this->clients->pop_back();
		close(client->getFD());
		delete client;
		n = this->clients->size();
	}
	/* close server. */
	int result = close(this->server_fd);
	if (result == ERROR)
	{
		throw new ServerException(errno);
	}
	return result;
}

void ServerTCP::loop(void)
{
	/* start worker pool. */
	this->tp = new ThreadPool(WORKER_NO);
	this->tp->init();
	/* listen. */
	int result = listen(this->server_fd, BACKLOG);
	if (result == ERROR)
	{
		throw new ServerException(errno);
	}
	/* server main while. */
	uint8_t buffer[BUFFER_SIZE];
	timeval* tv = new timeval;
	fd_set* temp_fdset = new fd_set;

	this->run_flag = true;
	int i = 0;
	while (this->run_flag == true)
	{
		this->setTimeval(tv, SERVER_WAIT_SEC, SERVER_WAIT_USEC);
		this->fdset_mutex->lock();
		memcpy(temp_fdset, this->socket_fdset, sizeof(fd_set));
		this->fdset_mutex->unlock();
		/* sniff socket_fdset for connections or messages. */
		int result = select(1 + this->maxfd, temp_fdset, NULL, NULL, tv);
		if (result == ERROR) /* error. */
		{
			std::cout << "Server error when calling 'select'!" << std::endl;
			delete tv;
			delete temp_fdset;
			throw new ServerException(errno);
		}
		else if (result != 0) /* data available. */
		{
			if (FD_ISSET(this->server_fd, temp_fdset))
			{	/* new connection. */
				std::cout << "New connection!" << std::endl;
				this->acceptConnection();
				continue;
			}
			/* check fd on which the event occur. */
			for (auto it = this->clients->cbegin(); it != this->clients->cend(); ++it)
			{
				if (FD_ISSET((*it)->getFD(), temp_fdset))
				{	/* read buffer. */
					memset(buffer, '\0', BUFFER_SIZE);
					int len = recv((*it)->getFD(), buffer, BUFFER_SIZE, 0);
					if (len == 0)
					{	/* disconnect. */
						std::cout << "Bye bye, " << (*it)->getFD() << std::endl;
						close((*it)->getFD());
						FD_CLR((*it)->getFD(), this->socket_fdset);
						delete (*it); /* delete client. */
						this->clients_mutex->lock();
						this->clients->erase(it);
						this->clients_mutex->unlock();
					}
					else
					{   /* handle request: send request to the worker pool. */
						std::cout << (*it)->getFD() << ": len = " << len << std::endl;
						/* extract client. */
						FD_CLR((*it)->getFD(), this->socket_fdset);
						Client* client = *it;
						/* prepare message. */
						uint8_t* buf = new uint8_t[BUFFER_SIZE];
						memset(buf, '\0', BUFFER_SIZE);
						memcpy(buf, buffer, len);
						/* append request. */
						Request* req = new Request(this, client, buf, len);
						this->tp->submit(consumeRequest, req);
					}
					break;
				}
			}
		}
		/* else, elapsed time. */
		if (i++ == SERVER_RUN)
		{
			this->run_flag = false;
		}
	}
	std::cout << "Server loop exit." << std::endl;
	/* release local memory. */
	delete tv;
	delete temp_fdset;
	return;
}

void ServerTCP::addFD(Client* client)
{
	this->fdset_mutex->lock();
	FD_SET(client->getFD(), this->socket_fdset);
	this->fdset_mutex->unlock();
}

void ServerTCP::setTimeval(timeval* tv, time_t sec, suseconds_t usec)
{
	tv->tv_sec = sec;
	tv->tv_usec = usec;
}

void ServerTCP::acceptConnection(void)
{
	sockaddr_storage* client_addr = new sockaddr_storage;
	socklen_t addr_len = sizeof(*client_addr);
	int client_fd = accept(this->server_fd, (sockaddr*)client_addr, &addr_len);
	if (client_fd == ERROR)
	{
		throw new ServerException(errno);
	}
	/* init Client */
	Client* newClient = new Client(client_fd, client_addr);
	/* handle client information. */
	FD_SET(client_fd, this->socket_fdset);
	this->clients_mutex->lock();
	this->clients->push_back(newClient);
	this->clients_mutex->unlock();
	if (client_fd > this->maxfd)
	{
		this->maxfd = client_fd;
	}
	std::cout << "New client connected on fd: " << client_fd << std::endl;
}

Client* ServerTCP::getClient(int clientfd)
{
	Client* result = nullptr;
	for (auto it = this->clients->cbegin(); it != this->clients->cend(); ++it)
	{
		if ((*it)->getFD() == clientfd)
		{
			/* extract client. */
			this->fdset_mutex->lock();
			FD_CLR((*it)->getFD(), this->socket_fdset);
			this->fdset_mutex->unlock();
			result = *it;
			break;
		}
	}
	return result;
}

Client* ServerTCP::getClientByName(char* name)
{
	Client* result = nullptr;
	for (auto it = this->clients->cbegin(); it != this->clients->cend(); ++it)
	{
		char* cl_name = (*it)->getName();
		if (cl_name != nullptr && strcmp(cl_name, name) == 0)
		{
			/* extract client. */
			this->fdset_mutex->lock();
			FD_CLR((*it)->getFD(), this->socket_fdset);
			this->fdset_mutex->unlock();
			result = *it;
			break;
		}
	}
	return result;
}

std::string* ServerTCP::getClientList(void)
{
	std::string* result = new std::string();
	this->clients_mutex->lock();
	for (auto it = this->clients->cbegin(); it != this->clients->cend(); ++it)
	{
		*result += ":";
		if ((*it != nullptr) && ((*it)->getName() != nullptr))
		{
			*result += (*it)->getName();
		}
	}
	this->clients_mutex->unlock();
	return result;
}
