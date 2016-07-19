#pragma once

#include <istream>
#include "config.h"
#include "params.hh"
#include "jsonObject.hh"
#include "jsonArray.hh"
#include "jsonPrimitive.hh"
#include "wrappedBuffer.hpp"

class StreamConsumer
{
    public:
        virtual ~StreamConsumer();

        static StreamConsumer *read(std::istream &stream, const Params *params=nullptr);
        JSonElement * const getRoot() const;

        StreamConsumer *withConfig(const Params *);

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
        void appendUnicode(const char [4], std::string &);

        std::istream &stream;
        JSonElement *root;
        const Params *params;

        WrappedBuffer<char, ERROR_HISTORY_LEN> history;

    private:
        static float _stof(const std::string &);
        static float _stof(const char *);
};

