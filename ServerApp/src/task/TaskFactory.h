
#pragma once

#include "ITask.h"
#include "task_request.h"
#include "authenticate/AuthenticateTask.h"
#include "send_msg/SendMessageTask.h"
#include "send_file/SendFileTask.h"
#include "pass_file/PassFileTask.h"
#include "set_name/SetNameTask.h"
#include "send_client_list/SendClientListTask.h"
#include "get_log_list/GetLogListTask.h"
#include "update_file/UpdateFileTask.h"

class SendMessageTask;

class TaskFactory
{
public:
	static ITask* createTask(int task, Request* req, unsigned char* msg);
};
