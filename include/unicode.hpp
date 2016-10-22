#pragma once

unsigned char hexbyte(const char c);

template<typename T>
static T hexbyte(const char str[], unsigned int len)
{
    T result = 0;
    for (unsigned int i =0; i < len; ++i)
        result = (result << 4) + hexbyte(str[i]);
    return result;
}


