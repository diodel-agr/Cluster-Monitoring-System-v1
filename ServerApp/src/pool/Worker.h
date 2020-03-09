
# pragma once

#include "request.h"
#include "../task/ITask.h"
#include "../task/TaskFactory.h"
#include "../task/task_request.h"

struct Request;

/*
 * consumeRequest - executed by the worker threads to resolve the clients requests.
 */
void consumeRequest(Request* req);
