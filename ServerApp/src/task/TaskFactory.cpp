
#include "TaskFactory.h"

ITask* TaskFactory::createTask(int task, Request* req, unsigned char* msg)
{
	ITask* instance = nullptr;
	switch (task)
	{
	case TASK_AUTHENTICATE:
		instance = new AuthenticateTask(req, msg);
		break;
	case TASK_SEND_MESSAGE:
		instance = new SendMessageTask(req, (char*)msg);
		break;
	case TASK_SEND_FILE:
		instance = new SendFileTask(req, (char*)msg);
		break;
	case TASK_GET_CLIENT_LIST:
		instance = new SendClientListTask(req, (char*)msg);
		break;
	case TASK_GET_LOG:
		instance = new PassFileTask(req, (char*)msg);
		break;
	case TASK_SET_NAME:
		instance = new SetNameTask(req, (char*)msg);
		break;
	case TASK_GET_LOG_LIST:
		instance = new GetLogListTask(req, (char*)msg);
		break;
	case TASK_UPDATE_FILE:
		instance = new UpdateFileTask(req, (char*)msg);
		break;
	default:
		std::cout << "Unknown task: " << task << std::endl;
		return nullptr;
		break;
	}
	return instance;
}
