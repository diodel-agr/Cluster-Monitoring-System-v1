/*
 * UpdateFileTask.h
 *
 *  Created on: Jul 10, 2019
 *      Author: razvan
 */

#ifndef UPDATEFILETASK_H_
#define UPDATEFILETASK_H_

#include "../ITask.h"
#include "../../pool/request.h"
#include <fstream>

class UpdateFileTask : public ITask {
private:
	Request* req;
	char* msg;
public:
	UpdateFileTask(Request* req, char* msg);
	~UpdateFileTask();

	void solveTask(void);
};

#endif /* UPDATEFILETASK_H_ */
