#include <sstream>
#include <iostream>
#include "unicode.hpp"
#include "streamConsumer.hh"

#define FAILED(got, op, expt) {std::cout << __FILE__ << ":" << __LINE__ << ": failed asserting " << got << " " << op << " expected " << expt << std::endl; return false; }

class StreamConsumerTester: public StreamConsumer
{
    public:
        static const std::string getStringFromUnicode(const char unicode[4])
        {
            std::stringstream ss;
            appendUnicode(unicode, ss);
            return ss.str();
        };

        static bool test()
        {
            std::string s = getStringFromUnicode("00e8");
            if (s != "è")
                FAILED((int)(s.c_str()[0]), "!=", (int)L'è');
            return true;
        };
};

bool simpleTest()
{
    if (hexbyte<unsigned short>("0020", 4) != 32)
        FAILED(hexbyte<unsigned short>("0020", 4), "!=", 32);

    if (hexbyte<unsigned short>("20", 2) != 32)
        FAILED(hexbyte<unsigned short>("2020", 4), "!=", 32);

    if (hexbyte<unsigned short>("2020", 4) != 8224)
        FAILED(hexbyte<unsigned short>("2020", 4), "!=", 8224);

    if (hexbyte<unsigned short>("FFFF", 4) != 65535)
        FAILED(hexbyte<unsigned short>("FFFF", 4), "!=", 65535);

    if (hexbyte<unsigned short>("0000", 4) != 0)
        FAILED(hexbyte<unsigned short>("0000", 4), "!=", 0);

    if (hexbyte<unsigned short>("", 0) != 0)
        FAILED(hexbyte<unsigned short>("", 0), "!=", 0);

    if (hexbyte<unsigned short>("002020", 6) != 8224)
        FAILED(hexbyte<unsigned short>("2020", 6), "!=", 8224);

    return true;
}

int main()
{
    if (!simpleTest())
        exit(EXIT_FAILURE);
    if (!StreamConsumerTester::test())
        exit(EXIT_FAILURE);
    exit(EXIT_SUCCESS);
}

