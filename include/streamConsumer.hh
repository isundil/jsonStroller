#pragma once

#include <istream>
#include "params.hh"
#include "jsonObject.hh"
#include "jsonArray.hh"
#include "jsonPrimitive.hh"
#include "linearHistory.hh"

class StreamConsumer
{
    public:
        StreamConsumer(std::istream &stream);
        virtual ~StreamConsumer();

        StreamConsumer *read();
        JSonElement * const getRoot() const;

        StreamConsumer *withConfig(const AParams *);

    private:
        /**
         * @return true on success
        **/
        JSonElement *consumeToken(JSonContainer *parent, std::string &buf);
        JSonElement *readNext(JSonContainer *parent);

        JSonObject *readObject(JSonContainer *parent);
        JSonArray *readArray(JSonContainer *parent);
        bool ignoreChar(char c) const noexcept;
        static void appendUnicode(const char [4], std::string &);

        std::istream &stream;
        JSonElement *root;
        const AParams *params;

        LinearHistory history;

    private:
        static float _stof(const std::string &);
        static float _stof(const char *);
};

