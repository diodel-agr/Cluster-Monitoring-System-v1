#pragma once

#include <limits.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <cstdint>

class SHA256 {
/* private variables. */
private:
	constexpr static uint32_t K[64] = {
			0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
			0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
			0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
			0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
			0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
			0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
			0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
			0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};
/* public methods. */
public:
	SHA256() { }

	~SHA256() { }

	/*
	 * computeDigest - compute the hash value for an arbitrary length message.
	 * @msg: the message.
	 * @return: the hash key.
	 */
	static uint32_t* computeDigest(unsigned char* msg, uint64_t& length);

/* private methods. */
private:
	/*
	 * padString - method used to pad the input string to make it congruent to 896 (modulus 1024).
	 * @msg: pointer to the message.
	 * @length: message length.
	 * @return: the padded message.
	 */
	static unsigned char* padString(unsigned char* msg, uint64_t& length);

	/*
	 * initHashBuffer - create the buffer used to hold the intermediate and final results of the hash function.
	 * @return: long int array holding the initial values for the 8 registers.
	 */
	static uint32_t* initHashBuffer(void);

	/*
	 * getRegister - extracts the specified register.
	 * @reg: the array of registers.
	 * @pos: the register name.
	 * @return: the register value.
	 */
	static uint32_t getRegister(uint32_t* reg, char pos);

	/*
	 * setRegister - sets the value for the specified register.
	 * @reg: register array.
	 * @pos: the register name.
	 * @val: new register value.
	 */
	static void setRegister(uint32_t* reg, char pos, uint32_t val);

	/*
	 * applyRound - process the 1024bit block.
	 * @word: the message.
	 * @registers: the state of the 8 registers.
	 * @constant: additive constant.
	 */
	static void Round(uint32_t word, uint32_t* registers, uint32_t constant);

	/*
	 * extractWord - used to extract or compute the value of the word for the next processing.
	 * 				 The word is one of the 80 words used in each round.
	 * @block: message block.
	 * @words: vector of pre-computed words.
	 * @i: the index of the desired word.
	 * @return: the value of the word.
	 */
	static uint32_t extractWord(unsigned char* block, std::vector<uint32_t>* words, int i);

	/*
	 * ROTR64 - rotates a variable with a specified number of bits.
	 * @x: variable to rotate.
	 * @n: number of bits.
	 * @return: the rotated number.
	 */
	static uint32_t ROTR32(uint32_t x, unsigned int n);

	/*
	 * SHR64 - shifts to the right the number @x with a specified number of bits.
	 * @x: variable to shift.
	 * @n: number of bits.
	 * @return: the shifted number.
	 */
	static uint32_t SHR32(uint32_t x, unsigned int n);

	/*
	 * sigma0 - sigma function specific to SHA256 algorithm.
	 * @x: variable to apply the sigma function.
	 */
	static uint32_t sigma0(uint32_t x);

	/*
	 * sigma1 - sigma function specific to SHA256 algorithm.
	 * @x: variable to apply the sigma function.
	 */
	static uint32_t sigma1(uint32_t x);

	/*
	 * ch - the conditional function: if e then f else g.
	 * @e: the e register.
	 * @f: the f register.
	 * @g: the g register.
	 */
	static uint32_t ch(uint32_t e, uint32_t f, uint32_t g);

	/*
	 * maj - function is true only of the majority (two or three) of the arguments are true.
	 * @a: the a register.
	 * @b: the b register.
	 * @c: the c register.
	 */
	static uint32_t maj(uint32_t a, uint32_t b, uint32_t c);

	/*
	 * SIGMA0 - sigma function specific to SHA256 algorithm.
	 * @x: variable to apply the sigma function.
	 */
	static uint32_t SIGMA0(uint32_t x);

	/*
	 * SIGMA1 - sigma function specific to SHA256 algorithm.
	 * @x: variable to apply the sigma function.
	 */
	static uint32_t SIGMA1(uint32_t x);
};

