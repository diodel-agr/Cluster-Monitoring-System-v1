
#include "SendFileTask.h"

SendFileTask::SendFileTask(Request* req, char* filename)
{
	this->client = req->client;
	this->filename = filename;
}

SendFileTask::~SendFileTask() { }

void SendFileTask::solveTask()
{
	/* open file. */
	char path[] = "src/files/";
	char* filepath = new char[strlen(path) + strlen(this->filename) + 1];
	strcpy(filepath, path);
	strcat(filepath, this->filename);
	std::ifstream inf(filepath, std::ifstream::in);
	if (inf.is_open() == false)
	{
		std::cout << "Worker could not open file for reading." << std::endl;
		delete filepath;
		return;
	}
	/* read file. */
	std::string contents = "";
	char buf[1024]; /* file content buffer. */
	do {
		memset(buf, 0, 1024);
		inf.read(buf, 1023);
		buf[inf.gcount()] = '\0';
		std::cout << inf.gcount() << std::endl;
		std::cout << buf << std::endl;
		std::string* s = new std::string(buf);
		contents += *s;
		delete s;
		std::cout << contents.size() << std::endl;
	} while (inf.gcount() == 1023);
	inf.close();
	/* encrypt file contents. */
	int size = contents.size();
	AESalgorithm* aes = new AESalgorithm(256);
	uint8_t* encrypted = aes->encrypt((uint8_t*)contents.c_str(), size, client->getKey());
	/* send response. */
	int sent = send(client->getFD(), encrypted, size, 0);
	if (sent != size)
	{
		std::cout << "File partially sent to client " << client->getFD() << std::endl;
	}
	else
	{
		std::cout << "File sent to client " << client->getFD() << std::endl;
	}
	/* free memory. */
	delete filepath;
	delete aes;
	delete encrypted;
}
