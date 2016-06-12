#pragma once

#include <istream>
#include "jsonObject.hh"
#include "jsonArray.hh"
#include "jsonPrimitive.hh"

class StreamConsumer
{
    public:
        static StreamConsumer *read(std::istream &stream);
        JSonElement * const getRoot() const;

    private:
        StreamConsumer(std::istream &stream);
        /**
         * @return true on success
        **/
        JSonElement *consumeToken(std::string &buf);
        JSonElement *readNext();

        JSonObject *readObject();
        JSonArray *readArray();
        bool ignoreChar(char c) const noexcept;

        std::istream &stream;
        JSonElement *root;
};

