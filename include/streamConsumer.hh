/**
 * streamConsumer.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include <istream>
#include "params.hh"
#include "jsonObject.hh"
#include "jsonArray.hh"
#include "jsonPrimitive.hh"
#include "linearHistory.hh"
#include "warning.hh"

class StreamConsumer
{
    public:
        /**
         * constructor
         * does not read yet
        **/
        StreamConsumer(std::istream &stream);
        virtual ~StreamConsumer();

        /**
         * read from stream and set root data
         * @return current instance
        **/
        StreamConsumer *read();

        /**
         * get root node
        **/
        const JSonElement * getRoot() const;
        JSonElement * getRoot();

        /**
         * return non-blocking error messages
        **/
        const std::list<Warning> &getMessages() const;

        /**
         * set builder's params
        **/
        StreamConsumer *withConfig(const AParams *);

        /**
         * find \uXXXX in buffer and replace them
        **/
        static std::string extractUnicode(const char *);
        static std::string extractUnicode(const std::string &);

    protected:
        /**
         * @return non-null on successfully read JSonElement, or null if token (',', '[', ...)
        **/
        JSonElement *consumeToken(JSonContainer *parent, std::stringstream &buf);
        JSonElement *consumeString(JSonContainer *parent, std::stringstream &buf);
        JSonElement *consumeBool(JSonContainer *parent, std::stringstream &buf, char c);
        JSonElement *consumeNumber(JSonContainer *parent, std::stringstream &buf, char c);
        JSonElement *consumeNull(JSonContainer *parent, std::stringstream &buf, char firstChar);
        bool consumeEscapedChar(char c, std::stringstream &buf);

        /**
         * read next item, fill object or array if found
        **/
        JSonElement *readNext(JSonContainer *parent);
        /**
         * fill object
        **/
        JSonObject *readObject(JSonContainer *parent);
        /**
         * fill array
        **/
        JSonArray *readArray(JSonContainer *parent);

        /**
         * out of token, should we ignore that char ?
        **/
        bool ignoreChar(char c) const noexcept;

        /**
         * compute unicode value and append it to buffer
        **/
        static void appendUnicode(const char [4], std::stringstream &);

        /**
         * input stream
        **/
        std::istream &stream;
        /**
         * once read, contains root element
        **/
        JSonElement *root;
        /**
         * builder params (program's one in fact)
        **/
        const AParams *params;

        /**
         * non-blocking errors
        **/
        std::list<Warning> warnings;
        /**
         * last read input
        **/
        LinearHistory history;
};

