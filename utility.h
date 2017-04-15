//
// Created by lag1924 on 24.03.17.
//

#ifndef PROJECT_UTILITY_H
#define PROJECT_UTILITY_H

#include <algorithm>

int VarIntRead(unsigned char *data, size_t &readed);

size_t VarIntWrite(unsigned int value, unsigned char *data);

long long int ReadVarLong(unsigned char *data, int &readed);

unsigned char *WriteVarLong(unsigned long long int value, int &len);

template<class T>
void endswap(T *objp) {
    unsigned char *memp = reinterpret_cast<unsigned char *>(objp);
    std::reverse(memp, memp + sizeof(T));
}

#endif //PROJECT_UTILITY_H
