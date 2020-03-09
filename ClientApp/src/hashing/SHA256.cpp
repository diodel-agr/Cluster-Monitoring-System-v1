#include "SHA256.h"

constexpr uint32_t SHA256::K[];

void printBytes(unsigned char* mesg, int len)
{
	for (int i = 0; i < len; ++i)
	{
		std::cout << (int)mesg[i] << " ";
	}
	std::cout << std::endl;
}


uint32_t* SHA256::computeDigest(unsigned char* msg, uint64_t& length)
{
	/* step 1. Append padding bits and length. */
	unsigned char* padded = padString(msg, length);
	//printBytes(padded, length);
	/* step 2. Initialize hash buffer. */
	uint32_t* hash = initHashBuffer();
	uint32_t* registers = initHashBuffer();
	/* step 3. Take each 512bit block and apply SHA256 algorithm. */
	int N = length >> 6;
	/* for each message block. */
	for (int i = 0; i < N; ++i)
	{	/* block decomposition. */
		std::vector<uint32_t>* words = nullptr;
		for (int t = 0; t < 64; ++t)
		{   /* apply the 80 rounds. */
			uint32_t word = extractWord(padded + (i << 9), words, i);
			uint32_t k = K[t];
			Round(word, registers, k);
		}
		delete words;
		/* add buffers. */
		for (int j = 0; j < 8; ++j)
		{
			hash[i] += registers[i];
			registers[i] = hash[i];
		}
	}
	delete registers;
	return hash;
}

unsigned char* SHA256::padString(unsigned char* msg, uint64_t& length)
{
	int l = length << 3; /* message number of bits. */
	/* determine k. */
	int bitlen = (l + 1) % 512;
	int k = 0; /* k is the number of zeros we have to append. */
	if (bitlen < 448)
	{
		k = 448 - bitlen;
	}
	else
	{
		k = 512 - bitlen + 448;
	}
	int pad = (k + 1) >> 3; /* number of bytes to pad. */
	int padLength = length + pad + 8 + 1;
	unsigned char* paddedMsg = new unsigned char[padLength];
	paddedMsg[padLength] = '\0';
	/* copy message. */
	memcpy(paddedMsg, msg, length);
	/* add padding. */
	int i = length;
	paddedMsg[i++] = 0x80;
	while (pad > 1)
	{
		paddedMsg[i++] = 0x00;
		pad--;
	}
	/* add length. */
	memcpy(paddedMsg + i, &length, sizeof(length));
	length = padLength;
	return paddedMsg;
}

uint32_t* SHA256::initHashBuffer(void)
{
	uint32_t* registers = new uint32_t[8];
	registers[0] = 0x6A09E667; // a.
	registers[1] = 0xBB67AE85; // b.
	registers[2] = 0x3C6EF372; // c.
	registers[3] = 0xA54FF53A; // d.
	registers[4] = 0x510E527F; // e.
	registers[5] = 0x9B05688C; // f.
	registers[6] = 0x1F83D9AB; // g.
	registers[7] = 0x5BE0CD19; // h.
	return registers;
}

uint32_t SHA256::getRegister(uint32_t* reg, char pos)
{
	return reg[pos - 'a'];
}

void SHA256::setRegister(uint32_t* reg, char pos, uint32_t val)
{
	reg[pos - 'a'] = val;
}

void SHA256::Round(uint32_t word, uint32_t* registers, uint32_t constant)
{
	/* extract all registers. */
	uint32_t a = getRegister(registers, 'a');
	uint32_t b = getRegister(registers, 'b');
	uint32_t c = getRegister(registers, 'c');
	uint32_t d = getRegister(registers, 'd');
	uint32_t e = getRegister(registers, 'e');
	uint32_t f = getRegister(registers, 'f');
	uint32_t g = getRegister(registers, 'g');
	uint32_t h = getRegister(registers, 'h');
	/* compute next register. */
	uint32_t T1 = h + ch(e, f, g) + SIGMA1(e) + word + constant;
	uint32_t T2 = SIGMA0(a) + maj(a, b, c);
	setRegister(registers, 'h', g);
	setRegister(registers, 'g', f);
	setRegister(registers, 'f', e);
	setRegister(registers, 'e', d + T1);
	setRegister(registers, 'd', c);
	setRegister(registers, 'c', b);
	setRegister(registers, 'b', a);
	setRegister(registers, 'a', T1 + T2);
}

uint32_t SHA256::extractWord(unsigned char* block, std::vector<uint32_t>* words, int i)
{
	/* init vector, if necessary. */
	if (words == nullptr)
	{
		words = new std::vector<uint32_t>();
		uint32_t* ptr = (uint32_t*)block;
		for (int i = 0; i < 16; ++i)
		{
			words->push_back(*ptr);
			ptr++;
		}
		for (int i = 16; i < 80; ++i)
		{
			words->push_back(0);
		}
	}
	/* extract or compute the word. */
	if (i >= 16 && words->at(i) == 0)
	{   /* compute. */
		uint32_t w16 = extractWord(block, words, i - 16);
		uint32_t w15 = extractWord(block, words, i - 15);
		uint32_t w7 = extractWord(block, words, i - 7);
		uint32_t w2 = extractWord(block, words, i - 2);
		std::vector<uint32_t>::iterator it = words->begin();
		it += i;
		*it = w16 + sigma0(w15) + w7 + sigma1(w2);
	}
	return words->at(i);
}

uint32_t SHA256::ROTR32(uint32_t x, unsigned int n)
{
	const unsigned int mask = (CHAR_BIT * sizeof(x) - 1);

	n &= mask;
	return (x >> n) | (x << ((-n) & mask));
}

uint32_t SHA256::SHR32(uint32_t x, unsigned int n)
{
	return (x >> n);
}

uint32_t SHA256::sigma0(uint32_t x)
{
	return ROTR32(x, 7) ^ ROTR32(x, 18) ^ SHR32(x, 3);
}

uint32_t SHA256::sigma1(uint32_t x)
{
	return ROTR32(x, 17) ^ ROTR32(x, 19) ^ SHR32(x, 10);
}

uint32_t SHA256::ch(uint32_t e, uint32_t f, uint32_t g)
{
	return (e & f) ^ ((~e) & g);
}

uint32_t SHA256::maj(uint32_t a, uint32_t b, uint32_t c)
{
	return (a & b) ^ (a & c) ^ (b & c);
}

uint32_t SHA256::SIGMA0(uint32_t x)
{
	return ROTR32(x, 2) ^ ROTR32(x, 13) ^ ROTR32(x, 22);
}

uint32_t SHA256::SIGMA1(uint32_t x)
{
	return ROTR32(x, 6) ^ ROTR32(x, 11) ^ ROTR32(x, 25);
}
