
#include "ModuleConfigurer.h"

ModuleConfigurer::ModuleConfigurer() { }

ModuleConfigurer::~ModuleConfigurer() { }

void ModuleConfigurer::UpdateTable(char* filename, char* contents)
{
	/* open file. */
	FILE* file = nullptr;
	char* fname = new char[50];
	if (strcmp(filename, PROCESS_TABLE) == 0)
	{
		strcpy(fname, PARSER_PATH);
	}
	else if (strcmp(filename, WORDS_TABLE) == 0)
	{
		strcpy(fname, ANALYSER_PATH);
	}
	else
	{
		std::cout << "Unknown file name: " << filename << std::endl;
		delete fname;
		return;
	}
	/* write to file. */
	char* line = new char[LINE_SIZE];
	memset(line, '\0', LINE_SIZE);
	/* delete command. */
	file = fopen(fname, "w");
	if (file == nullptr)
	{
		std::cout << "File " << fname << " could not be opened" << std::endl;
		delete fname;
		return;
	}
	sprintf(line, "%d:", HT_ERASE);
	fwrite(line, 1, strlen(line), file);
	fclose(file);
	/* insert into hash. */
	char* word = strtok(contents, "\n");
	while (word != NULL)
	{
		file = fopen(fname, "w");
		memset(line, '\0', LINE_SIZE);
		sprintf(line, "%d:%s", HT_INSERT, word);
		fwrite(line, 1, 1 + strlen(line), file);
		word = strtok(NULL, "\n");
		fclose(file);
	}
	/* print command. */
	file = fopen(fname, "w");
	memset(line, '\0', LINE_SIZE);
	sprintf(line, "%d:", HT_PRINT);
	fwrite(line, 1, strlen(line), file);
	/* close file. */
	fclose(file);
	/* free resources. */
	delete fname;
	std::cout << "File " << filename << " was updated!" << std::endl;
}
