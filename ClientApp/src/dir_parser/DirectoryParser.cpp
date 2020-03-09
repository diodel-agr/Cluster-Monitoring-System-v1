
#include "DirectoryParser.h"

/* defines. */
#define DIR_NAME_SIZE 		50
#define BUF_SIZE			11
#define MODE				S_IRWXU
#define LOG_BUFFER_SIZE		1024

/* constants. */
const char* months[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
const char* days[] = { "SUN", "MON", "TUE", "WEN", "THU", "FRI", "SAT" };

/* definitions. */
DirectoryParser::DirectoryParser(std::string path, std::string charFilename)
{
	this->charFile = path + charFilename;
}

DirectoryParser::~DirectoryParser() { }

void DirectoryParser::UpdateLogFiles()
{
	/* get the current date. */
	time_t now = time(0);
	tm* date = localtime(&now);
	/* obtain logs. */
	char logs[LOG_BUFFER_SIZE];
	memset(logs, '\0', LOG_BUFFER_SIZE);
	this->ReadLogsFromModule(logs, LOG_BUFFER_SIZE);
	std::cout << logs << std::endl;
	/* update directory structure. */
	char* dirname = new char[DIR_NAME_SIZE];
	this->UpdateDirectories(date, dirname);
	if (dirname != nullptr && strlen(logs) > 0)
	{
		/* prepare filename. */
		char buf[BUF_SIZE];
		sprintf(buf, "%d_%s.txt", date->tm_mday, days[date->tm_wday]);
		strcat(dirname, buf);
		/* open file. */
		std::ofstream fout;
		fout.open(dirname, std::ofstream::out | std::ofstream::app);
		if (fout.is_open() == false)
		{
			std::cout << "Could not open file: " << dirname << std::endl;
		}
		else
		{
			/* write log. */
			fout << logs;
			fout.close();
		}
	}
}

void DirectoryParser::UpdateDirectories(tm* date, char* dirname)
{
	memset(dirname, '\0', DIR_NAME_SIZE);
	/* create year directory. */
	char buf[BUF_SIZE];
	memset(buf, '\0', BUF_SIZE);
	sprintf(buf, "%d/", (1900 + date->tm_year));
	strcpy(dirname, "logs/");
	strcat(dirname, buf);
	/* create directory. */
	int ok = mkdir(dirname, MODE);
	if (ok < -1) /* -1: directory already exists. */
	{
		std::cout << "Could not create 'year' directory.\n" << std::endl;
		delete dirname;
		dirname = nullptr;
		return;
	}
	/* create month directory. */
	memset(buf, '\0', BUF_SIZE);
	sprintf(buf, "%s/", months[date->tm_mon]);
	strcat(dirname, buf);
	/* create directory. */
	ok = mkdir(dirname, MODE);
	if (ok < -1)
	{
		std::cout << "Could not create 'month' directory.\n" << std::endl;
		delete dirname;
		dirname = nullptr;
	}
}

void DirectoryParser::ReadLogsFromModule(char* buffer, int size)
{
	FILE* file = fopen(this->charFile.c_str(), "r");
	if (file != NULL)
	{
		fread(buffer, 1, size, file);
		fclose(file);
	}
	else
	{
		std::cout << "Char device file could not be opened." << std::endl;
	}
}

int DirectoryParser::getLogList(char* buffer, int max_size)
{
	int size = 2; /* default size. */
	DIR *log_dir; /* log directory. */
	struct dirent *year_, *month_, *day_;
	/* read 'log/' directory. */
	if((log_dir = opendir("logs/")) != NULL)
	{
		char* path = new char[14];
		char* line = new char[360];
		/* read 'logs/year' directories. */
		while ((year_ = readdir(log_dir)) != NULL)
		{
			/* don't process ./ and ../ directories. */
			if ((strcmp(".", year_->d_name) == 0) || (strcmp("..", year_->d_name) == 0))
			{
				continue;
			}
			memset(path, '\0', 14);
			sprintf(path, "%s/%s/", "logs", year_->d_name);
			DIR* year_dir = opendir(path);
			if (year_dir != NULL)
			{
				/* read 'logs/year/months/' directories. */
				while ((month_ = readdir(year_dir)) != NULL)
				{
					/* don't process ./ and ../ directories. */
					if ((strcmp(".", month_->d_name) == 0) || (strcmp("..", month_->d_name) == 0))
					{
						continue;
					}
					sprintf(path, "logs/%s/%s/", year_->d_name, month_->d_name);
					DIR* month_dir = opendir(path);
					if (month_dir != NULL)
					{
						memset(line, '\0', 360);
						sprintf(line, "%s/%s/{", year_->d_name, month_->d_name);
						/* read files. */
						while ((day_ = readdir(month_dir)) != NULL)
						{
							/* don't process ./ and ../ directories. */
							if ((strcmp(".", day_->d_name) == 0) || (strcmp("..", day_->d_name) == 0))
							{
								continue;
							}
							strcat(line, day_->d_name);
							strcat(line, ",");
						}
						int len = strlen(line);
						line[len - 1] = '}';
						strcat(line, "\n");
						/* append to result. */
						size += ++len;
						strcat(buffer, line);
					}
					closedir(month_dir);
				}
			}
			closedir(year_dir);
		}
		/* close 'log/' directory. */
		closedir(log_dir);
		/* free memory. */
		delete path;
		delete line;
	}
	else
	{
		std::cout << "Could not read the contents of the 'log/' directory! Error: " << strerror(errno) << std::endl;
	}
	return size;
}

void DirectoryParser::getLogFile(ClientTCP* client, char* buff, int buf_size)
{
	buff[0] = RESP_LOG;
	buff[1] = ':';
	/* open file. */
	int len = 6 + strlen((buff + 2));
	char* filename = new char[len];
	memset(filename, '\0', len);
	sprintf(filename, "%s/%s", "logs", (buff + 2));
	std::ifstream* fin = new std::ifstream(filename);
	delete filename;
	if (fin->is_open() == true)
	{
		/* read file and send its contents. */
		do {
			/* read block. */
			fin->read(buff + 2, (buf_size-3));
			len = fin->gcount();
			buff[2 + len] = '\0';
			/* send file piece. */
			std::cout << "Trimite la server: "  << buff << std::endl;
			client->sendToServer((uint8_t*)buff, (3 + len), true);
		} while (len == (buf_size - 3));
		/* send file end. */
		std::cout << "Trimite mesaj final." << std::endl;
		memset(buff, '\0', buf_size);
		buff[0] = RESP_LOG_END;
		buff[1] = ':';
		client->sendToServer((uint8_t*)buff, buf_size, true);
		/* close file. */
		fin->close();
		std::cout << "A terminat." << std::endl;
	}
	else
	{
		/* send error message to server. */
		memset(buff, '\0', buf_size);
		sprintf(buff, "%d:%s", ERR_FILE_NOT_FOUND, (buff + 2));
		len = 1 + strlen(buff);
		client->sendToServer((uint8_t*)buff, len, true);
		std::cout << "Could not open file " << (buff+2) << "." << std::endl;
	}
	/* delete resources. */
	delete fin;
}
