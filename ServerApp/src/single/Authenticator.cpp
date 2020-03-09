
#include "Authenticator.h"

Authenticator* Authenticator::instance;

Authenticator::Authenticator()
{
	this->keyFile = nullptr;
}

Authenticator::~Authenticator()
{
	if (this->keyFile != nullptr)
	{
		if (this->keyFile->is_open() == true)
		{
			this->keyFile->close();
		}
		delete keyFile;
	}
}

Authenticator* Authenticator::getInstance(void)
{
	static std::mutex instanceMutex;
	instanceMutex.lock();
	if (instance == nullptr)
	{
		std::cout << "Authenticator instance created." << std::endl;
		instance = new Authenticator();
	}
	instanceMutex.unlock();
	return instance;
}

void Authenticator::deleteInstance()
{
	static std::mutex instanceMutex;
	instanceMutex.lock();
	if (instance != nullptr)
	{
		delete instance;
	}
	instanceMutex.unlock();
}

bool Authenticator::checkKey(uint8_t* key)
{
	static std::mutex instanceMutex;
	instanceMutex.lock();
	if (this->keyFile == nullptr)
	{
		this->keyFile = new std::fstream;
	}
	this->keyFile->open(KEY_FILE);
	/* check if file is opened. */
	if (this->keyFile->is_open() == false)
	{
		std::cout << "Key file could not be opened!" << errno << std::endl;
		return false;
	}
	/* search the key in the file. */
	bool result = false;
	unsigned char buf[1 + (KEY_SIZE << 1)]; /* buffer to hold one key. */
	while (this->keyFile->getline((char*)buf, 1 + (KEY_SIZE << 1)))
	{
		if (this->memcmp((uint8_t*)buf, key, KEY_SIZE) == 0)
		{
			result = true;
			break;
		}
	}
	this->keyFile->close();
	instanceMutex.unlock();
	return result;
}

uint8_t Authenticator::mapValue(uint8_t chr)
{
	static uint8_t map[6] = { 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	if (chr <= '9')
	{
		return chr - '0';
	}
	else
	{
		return map[chr - 'a'];
	}
}

int Authenticator::memcmp(uint8_t* key0, uint8_t* key1, int size)
{
	int n = size / sizeof(int);
	for (int i = 0; i < n; ++i)
	{
		int j = i << 3;
		int k = i << 2;
		/* a[3:0]. */
		uint8_t a3 = (this->mapValue(key0[j]) << 4) | this->mapValue(key0[j + 1]);
		uint8_t a2 = (this->mapValue(key0[j + 2]) << 4) | this->mapValue(key0[j + 3]);
		uint8_t a1 = (this->mapValue(key0[j + 4]) << 4) | this->mapValue(key0[j + 5]);
		uint8_t a0 = (this->mapValue(key0[j + 6]) << 4) | this->mapValue(key0[j + 7]);
		/* b[0:3]. */
		uint8_t b0 = key1[k];
		uint8_t b1 = key1[k + 1];
		uint8_t b2 = key1[k + 2];
		uint8_t b3 = key1[k + 3];
		/* print. */
//		std::cout << (int)a3 << " " << (int)a2 << " " << (int)a1 << " " << (int)a0 << std::endl;
//		std::cout << (int)b3 << " " << (int)b2 << " " << (int)b1 << " " << (int)b0 << std::endl;
//		std::cout << ((a3 != b3) && (a2 != b2) && (a1 != b1) && (a0 != b0)) << std::endl;
		/* check. */
		if ((a3 != b3) && (a2 != b2) && (a1 != b1) && (a0 != b0))
		{
			return 1;
		}
	}
	return 0;
}
