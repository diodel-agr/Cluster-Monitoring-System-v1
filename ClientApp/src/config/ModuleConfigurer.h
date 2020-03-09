
#pragma once

#include <iostream>
#include <cstring>
#include <errno.h>

#define LINE_SIZE	50
#define PROCESS_TABLE	"processes"
#define WORDS_TABLE		"words"
#define ANALYSER_PATH	"../modules/keylogger/analyser/analyser"
#define PARSER_PATH		"../modules/keylogger/parser/ps_parser"
#define HT_ERASE		0
#define HT_INSERT		1
#define HT_PRINT		2

/*
 * ModuleConfigurer - class used to configure the 'analyser' and 'parser' hash table contents.
 */
class ModuleConfigurer {
public:
	/*
	 * Constructor.
	 */
	ModuleConfigurer();
	/*
	 * Destructor.
	 */
	~ModuleConfigurer();

	/*
	 * UpdateTable - used to update the contents of a hash table.
	 *
	 * @filename: name of the char device.
	 * @contents: the contents of the hash table.
	 */
	void UpdateTable(char* filename, char* contents);
};
