
#ifndef GETLOGLISTTASK_H_
#define GETLOGLISTTASK_H_

#include "../task_request.h"
#include "../../pool/request.h"
#include "../ITask.h"
#include "../../crypto/AESalgorithm.h"
#include "../../server/client.h"

class GetLogListTask : public ITask
{
private:
	Request* req;
	char* msg;

public:
	GetLogListTask(Request* req, char* msg);
	virtual ~GetLogListTask();
	void solveTask(void);
	void sendReady(int fd);
};

#endif /* GETLOGLISTTASK_H_ */
