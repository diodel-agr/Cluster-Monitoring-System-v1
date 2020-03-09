#pragma once

/* task defines. */
#define TASK_AUTHENTICATE 		0x00 	/* request to authenticate to server. */
#define TASK_SEND_MESSAGE 		0x01 	/* request to send a message to a client. */
#define TASK_SEND_FILE			0x02	/* request to send a crypted file to the client. */
#define TASK_NEW_FILE			0x03	/* request to update the modules hash tables. */
#define TASK_GET_LOG_LIST		0x04	/* request to send the log list. */
#define TASK_GET_LOG			0x05	/* request to send the contents of some log file. */
#define TASK_SET_NAME			0x06	/* request to the the client's name. */
#define TASK_GET_CLIENT_LIST	0x07	/* request to obtain the client list and send to administrator. */

/* response defines. */
#define RESP_SUCCESS			0x10	/* request success response. */
#define RESP_LOG_LIST			0x11
#define RESP_LOG				0x12
#define RESP_LOG_END			0x13
#define RESP_CLIENT_LIST		0x14

/* error defines. */
#define ERR_MESSAGE 			0x20 	/* message format error. */
#define ERR_AUTHENTICATE 		0x21 	/* authentication error. */
