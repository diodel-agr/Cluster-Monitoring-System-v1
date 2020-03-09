
#pragma once

#include <mutex>
#include <fstream>
#include <iostream>
#include <cstring>

#define KEY_FILE 		"src/files/keys.txt"
#define KEY_SIZE 		32 						/* key size in bytes. */

/*
 * Authenticator: class used to check if the client's key is recognized by the server.
 * This is a singleton class, we want to have only one instance with only one opened file.
 */
class Authenticator
{
/* fields. */
private:
	static Authenticator* instance;
	std::fstream* keyFile;
/* methods. */
protected:
	/*
	 * Void constructor.
	 */
	Authenticator();
public:
	/*
	 * Destructor.
	 */
	~Authenticator();

	/*
	 * getInstance - method used to obtain the instance of this class.
	 */
	static Authenticator* getInstance(void);

	/*
	 * deleteInstance - deletes the memory occupied by the Authenticator instance and the fsteam file.
	 */
	static void deleteInstance();

	/*
	 * checkKey - method used to check if the @key is found in the server's keyFile.
	 */
	bool checkKey(uint8_t* key);

	/*
	 * mapValue - method used to obtain the value of the @chr character.
	 * @chr: character to map.
	 * @return: mapped value.
	 */
	uint8_t mapValue(uint8_t chr);
private:
	/*
	 * memcmp: used to compare the memory values for the two keys.
	 * If the memory locations have the same values, the function return 0, positive number otherwise.
	 * @key0: first key to compare.
	 * @key1: second key to compare.
	 * @size: key size.
	 */
	int memcmp(uint8_t* key0, uint8_t* key1, int size);
};
