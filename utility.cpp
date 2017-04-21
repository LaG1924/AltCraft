#include "utility.h"

int VarIntRead(unsigned char *data, size_t &readed) {
    readed = 0;
    int result = 0;
    char read;
    do {
        read = data[readed];
        int value = (read & 0b01111111);
        result |= (value << (7 * readed));

        readed++;
        if (readed > 5) {
            throw "VarInt is too big";
        }
    } while ((read & 0b10000000) != 0);

    return result;
}

size_t VarIntWrite(unsigned int value, unsigned char *data) {
    size_t len = 0;
    do {
        unsigned char temp = (unsigned char) (value & 0b01111111);
        value >>= 7;
        if (value != 0) {
            temp |= 0b10000000;
        }
        data[len] = temp;
        len++;
    } while (value != 0);
    return len;
}

long long int ReadVarLong(unsigned char *data, int &readed) {
    readed = 0;
    long long result = 0;
    unsigned char read;
    do {
        read = data[readed];
        long long value = (read & 0b01111111);
        result |= (value << (7 * readed));

        readed++;
        if (readed > 10) {
            throw "VarLong is too big";
        }
    } while ((read & 0b10000000) != 0);
    return result;
}

unsigned char *WriteVarLong(unsigned long long int value, int &len) {
    unsigned char *data = new unsigned char[10];
    len = 0;
    do {
        unsigned char temp = (unsigned char) (value & 0b01111111);
        value >>= 7;
        if (value != 0) {
            temp |= 0b10000000;
        }
        data[len] = temp;
        len++;
    } while (value != 0);

    return data;
}