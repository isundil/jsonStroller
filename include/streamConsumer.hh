#pragma once

#include <istream>
#include "config.h"
#include "jsonObject.hh"
#include "jsonArray.hh"
#include "jsonPrimitive.hh"
#include "wrappedBuffer.hpp"

class StreamConsumer
{
    public:
        virtual ~StreamConsumer();

        static StreamConsumer *read(std::istream &stream);
        JSonElement * const getRoot() const;

    private:
        StreamConsumer(std::istream &stream);
        /**
         * @return true on success
        **/
        JSonElement *consumeToken(JSonContainer *parent, std::string &buf);
        JSonElement *readNext(JSonContainer *parent);

        JSonObject *readObject(JSonContainer *parent);
        JSonArray *readArray(JSonContainer *parent);
        bool ignoreChar(char c) const noexcept;

        std::istream &stream;
        JSonElement *root;

        WrappedBuffer<char, ERROR_HISTORY_LEN> history;

    private:
        static float _stof(const std::string &);
        static float _stof(const char *);
};

