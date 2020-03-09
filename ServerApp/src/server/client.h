
#pragma once

#include <sys/socket.h>
#include <cstdint>
#include <cstring>

class Client
{
/* fields. */
private:
	int fd;							/* client file descriptor. */
	sockaddr_storage* addr_info;	/* client address information. */
	unsigned char* key;				/* client cryptographic key. */
	bool auth;						/* boolean value specifying if the client is authentified or not. */
	char* name;						/* the name of the client. */
/* methods. */
public:
	/*
	 * Parameterized constructor.
	 */
	Client(int fd, sockaddr_storage* addr)
	{
		this->fd = fd;
		this->addr_info = addr;
		this->key = nullptr;
		this->auth = false;
		this->name = nullptr;
	}

	/*
	 * Destructor.
	 */
	~Client()
	{
		delete addr_info;
		if (this->key != nullptr)
		{
			memset((char*)key, 0, strlen((char*)key));
			delete key;
		}
		if (this->name != nullptr)
		{
			delete this->name;
		}
	}

	/*
	 * getFD - return the client file descriptor.
	 * @return: client fd.
	 */
	int getFD(void)
	{
		return this->fd;
	}

	/*
	 * getKey - method used to obtain a copy of the client cryptographic key.
	 * @return: client key.
	 */
	uint8_t* getKey(void)
	{
		if (this->key != nullptr)
		{
			/*
			int len = strlen((char*)this->key);
			unsigned char* key_copy = new unsigned char[1 + len];
			memcpy(key_copy, this->key, len);
			return key_copy;
			*/
			return this->key;
		}
		return nullptr;
	}

	/*
	 * setKey - method used to set the client's key.
	 */
	void setKey(unsigned char* key)
	{
		this->key = key;
	}

	/*
	 * setAuth - set the authenticity of the client: true - client successfully authentified, false - client non-authentified.
	 * @value: boolean value.
	 */
	void setAuth(bool value)
	{
		this->auth = value;
	}

	/*
	 * getAuth - get the authenticity of the client.
	 * @return: the value of auth field.
	 */
	bool getAuth(void)
	{
		return this->auth;
	}

	/*
	 * setName - method used to set the clients name.
	 *
	 * @name: clients name.
	 */
	void setName(char* name)
	{
		if (this->name != nullptr)
		{
			delete this->name;
		}
		this->name = new char[1 + strlen(name)];
		strcpy(this->name, name);
	}

	/*
	 * getName - method used to return the name of the client.
	 */
	char* getName(void)
	{
		return this->name;
	}
};
