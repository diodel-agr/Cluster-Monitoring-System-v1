#include "AESalgorithm.h"

void printHex(uint8_t* arr, int size)
{
    size = size >> 2;
    for (int i = 0; i < size; ++i)
    {
        std::cout << std::hex << *((uint32_t*)(arr + (i << 2))) << std::endl;
    }
    std::cout << std::endl;
}


AESalgorithm::AESalgorithm() { }

AESalgorithm::AESalgorithm(int k)
{
    this->NB = 4; /* default size for the 256bit key. [ block_len / 32 = 128 / 32 = 4] */
    switch (k)
    {
    case 128:
        this->NK = 4;
        this->NR = 10;
        break;
    case 192:
        this->NK = 6;
        this->NR = 12;
        break;
    case 256:
        this->NK = 8;
        this->NR = 14;
        break;
    default:
        throw "Incorrect key length";
    }
}

AESalgorithm::~AESalgorithm() { }

uint8_t* AESalgorithm::formatInputMsg(const uint8_t* input, int& size)
{
    int blockLen = 4 * this->NB;
    int newLen = (size >> 4); // number of 16 bytes blocks to process.
    if (size % 16 != 0)
    {
        newLen++;
    }
    newLen *= blockLen;
    //int newLen = (size / blockLen + 1) * blockLen;
    uint8_t* result = new uint8_t[newLen];
    memset(result, '\0', newLen);
    memcpy(result, input, size);
    size = newLen;
    return result;
}

uint8_t* AESalgorithm::encrypt(const uint8_t* msg, int& size, const uint8_t* key)
{
    uint8_t* input = formatInputMsg(msg, size);
    uint8_t* output = new uint8_t[size];
    uint8_t* exp_key = keyExpansion(key);
    /* encrypt. */
    int n = size >> 4;
    for (int i = 0; i < n; ++i)
    {   /* take 128bit block and encrypt it. */
        uint8_t* result = cipher(input + i * 16, exp_key);
        memcpy(output + i * 16, result, 16); /* copy 16 bytes from result to the output. */
        delete result;
    }
    return output;
}

uint8_t* AESalgorithm::decrypt(const uint8_t* msg, int size, const uint8_t* key)
{
    uint8_t* input = (uint8_t*)msg; //formatInputMsg(msg, size);
    uint8_t* output = new uint8_t[size];
    uint8_t* exp_key = keyExpansion(key);
    /* decrypt. */
    int n = size >> 4;
    for (int i = 0; i < n; ++i)
    {   /* take 128bit block and decrypt it. */
        uint8_t* result = invCipher(input + i * 16, exp_key);
        memcpy(output + i * 16, result, 16); /* copy 16 bytes from result to the output. */
        delete result;
    }
    return output;
}

/*************************************** ENCRYPT ***************************************/
void printState(uint8_t** state)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            std::cout << std::hex << (state[i][j] | 0x00000000) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

uint8_t* AESalgorithm::cipher(const uint8_t* input, const uint8_t* key)
{
    /* set state. */
    uint8_t** state = new uint8_t*[4]; /* the state has 4 lines by default. */
    for (int i = 0; i < 4; ++i)
    {
        state[i] = new uint8_t[this->NB];
        memcpy(state[i], input + (i * this->NB), this->NB);
    }
    /* transpose matrix. */
    transposeState(state);
    //printState(state);
    /* start encrypting. */
    addRoundKey(state, (uint8_t*)key);
    //printState(state);

    for (int round = 1; round < this->NR; ++round)
    {
        subBytes(state);
        //printState(state);
        shiftRows(state);
        //printState(state);
        mixColumns(state);
        //printState(state);
        addRoundKey(state, (uint8_t*)(key + ((round * this->NB) << 2)));
        //printState(state);
    }

    subBytes(state);
    //printState(state);
    shiftRows(state);
    //printState(state);
    addRoundKey(state, (uint8_t*)(key + ((this->NR * this->NB) << 2)));
    //printState(state);

    /* create result. */
    uint8_t* result = new uint8_t[16]; /* the encrypted block is always 16 bytes long. */
    for (int i = 0; i < 4; ++i)
    {
        memcpy(result + (i << 2), state[i], 4);
        delete state[i];
    }
    /* free state. */
    delete state;
    /* return. */
    return result;
}

void AESalgorithm::transposeState(uint8_t** state)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = i + 1; j < this->NB; ++j)
        {
            uint8_t aux = state[i][j];
            state[i][j] = state[j][i];
            state[j][i] = aux;
        }
    }
}

void AESalgorithm::subBytes(uint8_t** state)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < this->NB; j++)
        {
            int x = state[i][j] >> 4;
            int y = state[i][j] & 0x0f;
            state[i][j] = sbox[x][y];
        }
    }
}

void AESalgorithm::shiftRows(uint8_t** state)
{
    /* shift row 1. */
    uint8_t aux1 = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = aux1;
    /* shift row 2. */
    aux1 = state[2][0];
    uint8_t aux2 =  state[2][1];
    state[2][0] = state[2][2];
    state[2][1] = state[2][3];
    state[2][2] = aux1;
    state[2][3] = aux2;
    /* shift row 3. */
    aux1 = state[3][0];
    aux2 = state[3][1];
    uint8_t aux3 = state[3][2];
    state[3][0] = state[3][3];
    state[3][1] = aux1;
    state[3][2] = aux2;
    state[3][3] = aux3;
}

void AESalgorithm::mixColumns(uint8_t** state)
{
    uint8_t temp[4], aux[4];
    /* for each block. */
    for (int j = 0; j < this->NB; ++j)
    {
        /* copy column i. */
        for (int i = 0; i < 4; ++i)
        {
            temp[i] = state[i][j];
        }
        /* compute product. */
        aux[0] = mulBytes(0x02, temp[0]) ^ mulBytes(0x03, temp[1]) ^ temp[2] ^ temp[3];
        aux[1] = temp[0] ^ mulBytes(0x02, temp[1]) ^ mulBytes(0x03, temp[2]) ^ temp[3];
        aux[2] = temp[0] ^ temp[1] ^ mulBytes(0x02, temp[2]) ^ mulBytes(0x03, temp[3]);
        aux[3] = mulBytes(0x03, temp[0]) ^ temp[1] ^ temp[2] ^ mulBytes(0x02, temp[3]);
        /* copy result. */
        for (int i = 0; i < 4; ++i)
        {
            state[i][j] = aux[i];
        }
    }
}

uint8_t AESalgorithm::mulBytes(uint8_t a, uint8_t b)
{
    uint8_t c = 0, mask = 0x01;
    for (int i = 0; i < 8; i++)
    {
        if (b & mask)
        {
            uint8_t d = a;
            for (int j = 0; j < i; j++)
            {
                d = *xtime(&d);
            }
            c = c ^ d;
        }
        b = b >> 1;
    }
    return c;
}

void AESalgorithm::addRoundKey(uint8_t** state, uint8_t* key)
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < this->NB; ++j)
        {
            state[i][j] = state[i][j] ^ key[i + (j << 2)];
        }
    }
}
/*************************************** KEY EXPANSION ***************************************/
void AESalgorithm::subWord(uint8_t* word)
{
    for (int i = 0; i < 4; ++i)
    {
        int x = word[i] >> 4;
        int y = word[i] & 0x0f;
        word[i] = sbox[x][y];
    }
}

void AESalgorithm::rotWord(uint8_t* word)
{
    uint8_t aux = word[0];
    word[0] = word[1];
    word[1] = word[2];
    word[2] = word[3];
    word[3] = aux;
}

uint8_t* AESalgorithm::xorWords(uint8_t* w1, uint8_t* w2)
{
    uint8_t* result = new uint8_t[4];
    for (int i = 0; i < 4; ++i)
    {
        result[i] = w1[i] ^ w2[i];
    }
    return result;
}

uint8_t* AESalgorithm::xtime(uint8_t* byte)
{
    uint8_t mask = 0x80;
    uint8_t m = 0x1b; /* the prime 283. */
    uint8_t msb = *byte & mask;
    *byte = *byte << 1;
    if (msb)
    {
        *byte = *byte ^ m;
    }
    return byte;
}

uint8_t* AESalgorithm::rcon(int n)
{
    uint8_t* result = new uint8_t[4];
    uint8_t c = 1;
    for (int i = 0; i < n - 1; i++)
    {
        c = *xtime(&c);
    }
    result[0] = c;
    result[1] = result[2] = result[3] = 0;
    return result;
}

uint8_t* AESalgorithm::keyExpansion(const uint8_t* key)
{
    int arr_size = this->NB * (this->NR + 1); // 256bit key: 4 * 16 = 60 words.
    uint8_t* words = new uint8_t[arr_size << 2]; //
    uint8_t* temp = new uint8_t[4];
    memset(words, '\0', arr_size << 2);
    memcpy((void*)words, (void*)key, 32);
    int i = this->NK;
    int lim = arr_size;
    while (i < lim)
    {
        memcpy(temp, words + ((i - 1) << 2), 4); // copy the (i-1)th word.
        if ((i % this->NK) == 0)
        {
            rotWord(temp);
            subWord(temp);
            uint8_t* rc = rcon(i / this->NK);
            uint8_t* result = xorWords(temp, rc);
            delete temp;
            temp = result;
        }
        else if ((this->NK > 6) && (i % this->NK == 4))
        {
            subWord(temp);
        }
        for (int j = 0; j < 4; ++j)
        {
            words[(i << 2) + j] = words[((i - this->NK) << 2) + j ] ^ temp[j];
        }
        i++;
    }
    return words;
}

/*************************************** DECRYPT **************************************/
uint8_t* AESalgorithm::invCipher(const uint8_t* input, const uint8_t* key)
{
    /* prepare state. */
    uint8_t** state = new uint8_t*[4];
    for (int i = 0; i < 4; ++i)
    {
        state[i] = new uint8_t[this->NB];
        memcpy(state[i], input + (i * this->NB), this->NB);
    }
    //printState(state);
    /* begin decryption. */
    addRoundKey(state, (uint8_t*)(key + ((this->NR * this->NB) << 2)));
    //printState(state);
    for (int round = this->NR-1; round > 0; --round)
    {
        invShiftRows(state);
        //printState(state);
        invSubBytes(state);
        //printState(state);
        addRoundKey(state, (uint8_t*)(key + ((round * this->NB) << 2)));
        //printState(state);
        invMixColumns(state);
        //printState(state);
    }
    invShiftRows(state);
    //printState(state);
    invSubBytes(state);
    //printState(state);
    addRoundKey(state, (uint8_t*)key);
    //printState(state);
    /* transpose state. */
    transposeState(state);
    /* prepare result. */
    uint8_t* result = new uint8_t[16];
    for (int i = 0; i < 4; ++i)
    {
        memcpy(result + (i << 2), state[i], this->NB);
    }
    return result;
}

void AESalgorithm::invSubBytes(uint8_t** state)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < this->NB; j++)
        {
            int x = state[i][j] >> 4;
            int y = state[i][j] & 0x0f;
            state[i][j] = inv_sbox[x][y];
        }
    }
}

void AESalgorithm::invShiftRows(uint8_t** state)
{
    /* shift row 1. */
    uint8_t aux1 = state[1][3];
    state[1][3] = state[1][2];
    state[1][2] = state[1][1];
    state[1][1] = state[1][0];
    state[1][0] = aux1;
    /* shift row 2. */
    aux1 = state[2][3];
    uint8_t aux2 = state[2][2];
    state[2][3] = state[2][1];
    state[2][2] = state[2][0];
    state[2][1] = aux1;
    state[2][0] = aux2;
    /* shift row 3. */
    aux1 = state[3][3];
    aux2 = state[3][2];
    uint8_t aux3 = state[3][1];
    state[3][3] = state[3][0];
    state[3][2] = aux1;
    state[3][1] = aux2;
    state[3][0] = aux3;
}

void AESalgorithm::invMixColumns(uint8_t** state)
{
    uint8_t temp[4], aux[4];

    for (int j = 0; j < this->NB; ++j)
    {
        for (int i = 0; i < 4; ++i)
        {
            temp[i] = state[i][j];
        }
        aux[0] = mulBytes(0x0e, temp[0]) ^ mulBytes(0x0b, temp[1]) ^ mulBytes(0x0d, temp[2]) ^ mulBytes(0x09, temp[3]);
        aux[1] = mulBytes(0x09, temp[0]) ^ mulBytes(0x0e, temp[1]) ^ mulBytes(0x0b, temp[2]) ^ mulBytes(0x0d, temp[3]);
        aux[2] = mulBytes(0x0d, temp[0]) ^ mulBytes(0x09, temp[1]) ^ mulBytes(0x0e, temp[2]) ^ mulBytes(0x0b, temp[3]);
        aux[3] = mulBytes(0x0b, temp[0]) ^ mulBytes(0x0d, temp[1]) ^ mulBytes(0x09, temp[2]) ^ mulBytes(0x0e, temp[3]);
        for (int i = 0; i < 4; ++i)
        {
            state[i][j] = aux[i];
        }
    }
}
