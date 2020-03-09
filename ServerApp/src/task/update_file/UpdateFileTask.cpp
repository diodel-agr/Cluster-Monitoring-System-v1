/*
 * UpdateFileTask.cpp
 *
 *  Created on: Jul 10, 2019
 *      Author: razvan
 */

#include "UpdateFileTask.h"

UpdateFileTask::UpdateFileTask(Request* req, char* msg) {
	this->req = req;
	this->msg = msg;

}

UpdateFileTask::~UpdateFileTask() {
	// TODO Auto-generated destructor stub
}

void UpdateFileTask::solveTask(void)
{
	char buf[100], buf_crypt[100];
	memset(buf, '\0', 100);
	char path[] = "src/files/";
	char* filename = strtok(this->msg, ":");
	sprintf(filename, "%s%s", path, filename);
	std::ofstream* file = new std::ofstream(filename);
	if (file->is_open() == false)
	{
		std::cout << "Nu s-a deschis fisierul " << filename << std::endl;
		delete file;
		return;
	}
	char* contents = strtok(NULL, ":");
	file->write(msg, 1 + strlen(contents));
	file->close();
	delete file;
	/* notify clients. */
	AESalgorithm* aes = new AESalgorithm();
	sprintf(buf, "%d:%s", TASK_UPDATE_FILE, filename);
	std::vector<Client*>* list = this->req->server->getClients();
	for (int i = 0; i < list->size(); ++i)
	{
		sprintf(buf, "%d:%s", TASK_UPDATE_FILE, filename);
		Client* cl = list->at(i);
		int len = 1 + strlen(buf);
		uint8_t* encrypted = aes->encrypt((uint8_t*)buf, len, cl->getKey());
		send(cl->getFD(), encrypted, len, MSG_WAITALL);
		delete encrypted;
	}
}
