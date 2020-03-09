
#pragma once

/* includes. */
#include <string>
#include <ctime>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <dirent.h>

#include "../client/client.h"

/*
 * DirectoryParser - class used to manage the log files: write, read.
 *
 */
class DirectoryParser
{
private:
	/* char device path + name. */
	std::string charFile;
public:
	/*
	 * Constructor.
	 *
	 * @path: path to the char device file.
	 * @charFilename: name of the char device to read.
	 */
	DirectoryParser(std::string path, std::string charFilename);

	/*
	 * Destructor.
	 */
	~DirectoryParser();

	/*
	 * UpdateLogFiles - read the char device file and save the contents into the corresponding file.
	 */
	void UpdateLogFiles();

	/*
	 * getLogList - read the log list and compose the response for server.
	 *
	 * @buffer: the buffer to store the log list.
	 * @max_size: buffer max size;
	 * @return: the buffer length.
	 */
	int getLogList(char* buffer, int max_size);

	/*
	 * getLogFile - method used to send a log file to the server.
	 *
	 * @client: client socket to send.
	 * @buff: contains the file path (input).
	 * @max_size: buffer max size.
	 * @return: 0 for success, 1 for error.
	 */
	void getLogFile(ClientTCP* client, char* buff, int buf_size);

private:
	/*
	 * UpdateDirectories - create the directory structure of the logging system.
	 * The directories structure is the following:
	 * logs/
	 * 		year/
	 * 			month/
	 * 				day.txt
	 *
	 * 	@date: current date.
	 * 	@dirname: return value. The pth to the corresponding directory.
	 */
	void UpdateDirectories(tm* date, char* dirname);

	/*
	 * ReadLogsFromModule - method used to read the char device file and obtain the latest user logs.
	 *
	 * @buffer: buffer to store the logs.
	 * @size: buffer size.
	 */
	void ReadLogsFromModule(char* buffer, int size);
};
