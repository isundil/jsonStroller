#include <iostream>
#include "wrappedBuffer.hpp"

#define FAILED(got, op, expt) {std::cout << __FILE__ << ":" << __LINE__ << ": failed asserting " << got << " " << op << " expected " << expt << std::endl; return false; }

bool simpleTest()
{
    WrappedBuffer<char, 5> test;
    if (test.toString().size() != 0)
        FAILED(test.toString().size(), "!=", 0);
    if (test.size() != 0)
        FAILED(test.size(), "!=", 0);
    test.put('a');
    if (test.size() != 1)
        FAILED(test.size(), "!=", 1);
    if (test.toString().size() != 1)
        FAILED(test.toString().size(), "!=", 1);
    if (test.toString() != "a")
        FAILED(test.toString(), "!=", "a");

    test.put('b');
    if (test.size() != 2)
        FAILED(test.size(), "!=", 2);
    if (test.toString().size() != 2)
        FAILED(test.toString().size(), "!=", 2);
    if (test.toString() != "ab")
        FAILED(test.toString(), "!=", "ab");

    test.put('c');
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString().size() != 3)
        FAILED(test.toString().size(), "!=", 3);
    if (test.toString() != "abc")
        FAILED(test.toString(), "!=", "abc");

    test.pop_back();
    if (test.size() != 2)
        FAILED(test.size(), "!=", 2);
    if (test.toString().size() != 2)
        FAILED(test.toString().size(), "!=", 2);
    if (test.toString() != "ab")
        FAILED(test.toString(), "!=", "ab");
    return true;
}

bool _testBorder(WrappedBuffer<char, 3> &test)
{
    test.put('a');
    test.put('b');
    test.put('c');

    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "abc")
        FAILED(test.toString(), "!=", "abc");

    test.put('d');
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "bcd")
        FAILED(test.toString(), "!=", "bcd");

    test.put('e');
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "cde")
        FAILED(test.toString(), "!=", "cde");

    test.put('f');
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "def")
        FAILED(test.toString(), "!=", "def");

    test.put('g');
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "efg")
        FAILED(test.toString(), "!=", "efg");

    test.pop_back();
    if (test.size() != 2)
        FAILED(test.size(), "!=", 2);
    if (test.toString() != "ef")
        FAILED(test.toString(), "!=", "ef");

    test.put('g');
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "efg")
        FAILED(test.toString(), "!=", "efg");
    test.put('h');
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "fgh")
        FAILED(test.toString(), "!=", "fgh");

    test.pop_back();
    if (test.size() != 2)
        FAILED(test.size(), "!=", 2);
    if (test.toString() != "fg")
        FAILED(test.toString(), "!=", "fg");

    test.put('h');
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "fgh")
        FAILED(test.toString(), "!=", "fgh");
    test.put('i');
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "ghi")
        FAILED(test.toString(), "!=", "ghi");

    test.pop_back();
    if (test.size() != 2)
        FAILED(test.size(), "!=", 2);
    if (test.toString() != "gh")
        FAILED(test.toString(), "!=", "gh");

    test.pop_back();
    if (test.size() != 1)
        FAILED(test.size(), "!=", 1);
    if (test.toString() != "g")
        FAILED(test.toString(), "!=", "g");

    test.pop_back();
    if (test.size() != 0)
        FAILED(test.size(), "!=", 0);
    test.pop_back();
    if (test.size() != 0)
        FAILED(test.size(), "!=", 0);

    return true;
}

bool testBorder()
{
    WrappedBuffer<char, 3> test;

    if (!_testBorder(test))
        return false;
    return _testBorder(test);
}

bool _testBatch(WrappedBuffer<char, 3> &test)
{
    char buf[] = {'a', 'b', 'c', 'd', 'e'};

    test.put(buf, 2);
    if (test.size() != 2)
        FAILED(test.size(), "!=", 2);
    if (test.toString() != "ab")
        FAILED(test.toString(), "!=", "ab");
    test.put(&buf[2], 2);
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "bcd")
        FAILED(test.toString(), "!=", "bcd");
    test.put(&buf[2], 2);
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "dcd")
        FAILED(test.toString(), "!=", "dcd");
    test.put(&buf[3], 2);
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "dde")
        FAILED(test.toString(), "!=", "dde");
    test.put(&buf[2], 2);
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "ecd")
        FAILED(test.toString(), "!=", "ecd");
    test.put(buf, 3);
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "abc")
        FAILED(test.toString(), "!=", "abc");
    test.put(buf, 5);
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "cde")
        FAILED(test.toString(), "!=", "cde");

    test.pop_back();
    if (test.size() != 2)
        FAILED(test.size(), "!=", 2);
    if (test.toString() != "cd")
        FAILED(test.toString(), "!=", "cd");
    test.put(buf, 2);
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "dab")
        FAILED(test.toString(), "!=", "dab");

    test.pop_back();
    if (test.size() != 2)
        FAILED(test.size(), "!=", 2);
    if (test.toString() != "da")
        FAILED(test.toString(), "!=", "da");
    test.put(buf, 2);
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "aab")
        FAILED(test.toString(), "!=", "aab");

    test.pop_back();
    if (test.size() != 2)
        FAILED(test.size(), "!=", 2);
    if (test.toString() != "aa")
        FAILED(test.toString(), "!=", "aa");
    test.put(&buf[1], 2);
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "abc")
        FAILED(test.toString(), "!=", "abc");

    test.pop_back();
    if (test.size() != 2)
        FAILED(test.size(), "!=", 2);
    if (test.toString() != "ab")
        FAILED(test.toString(), "!=", "ab");
    test.put(buf, 2);
    if (test.size() != 3)
        FAILED(test.size(), "!=", 3);
    if (test.toString() != "bab")
        FAILED(test.toString(), "!=", "bab");
    return true;
}

bool testBatch()
{
    WrappedBuffer<char, 3> test;

    return _testBatch(test);
}

int main()
{
    if (!simpleTest())
        exit(EXIT_FAILURE);
    if (!testBorder())
        exit(EXIT_FAILURE);
    if (!testBatch())
        exit(EXIT_FAILURE);
    exit(EXIT_SUCCESS);
}

