/**
 * streamConsumer.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include <iostream>
#include <sstream>
#include <codecvt>
#include <locale>
#include "jsonElement.hh"
#include "streamConsumer.hh"
#include "unicode.hpp"

StreamConsumer::StreamConsumer(std::istream &s): stream(s), root(nullptr)
{ }

StreamConsumer::~StreamConsumer()
{
    if (root)
        delete root;
}

StreamConsumer *StreamConsumer::withConfig(const AParams *p)
{
    params = p;
    return this;
}

StreamConsumer *StreamConsumer::read()
{
    if (root)
        return this;
    root = readNext(nullptr);
    return this;
}

JSonElement *StreamConsumer::readNext(JSonContainer *parent)
{
    std::stringstream sbuf;
    JSonElement *root = consumeToken(parent, sbuf);
    const std::string buf = sbuf.str();

    if (root == nullptr)
    {
        if (buf == "{")
        {
            return readObject(parent);
        }
        else if (buf == "[")
        {
            return readArray(parent);
        }
        else
            return nullptr;
    }
    return root;
}

const JSonElement * StreamConsumer::getRoot() const
{ return root; }

JSonElement * StreamConsumer::getRoot()
{ return root; }

JSonObject *StreamConsumer::readObject(JSonContainer *parent)
{
    JSonElement *keyObj;
    JSonObject *result = nullptr;
    std::stringstream buf;

    do
    {
        keyObj = consumeToken(result, buf);
        if (result == nullptr && keyObj == nullptr && buf.str() == "}")
            return new JSonObject(parent);
        JSonPrimitive<std::string> *key = dynamic_cast<JSonPrimitive<std::string> *>(keyObj);
        if (key == nullptr)
            throw JSonObject::NotAKeyException(stream.tellg(), history);
        if (consumeToken(parent, buf) != nullptr || buf.str() != ":")
            throw JsonUnexpectedException(':', stream.tellg(), history);
        if (result == nullptr)
            result = (!params || params->sortObjects()) ? new JSonSortedObject(parent) : new JSonObject(parent);
        else if (result->contains(key->getValue()))
        {
            if (!params || params->isStrict())
                throw JSonObject::DoubleKeyException(stream.tellg(), key->getValue(), history);
            else // add Warning, new key erase previous one
            {
                result->erase(key->getValue());
                warnings.push_back(Warning(JSonObject::DoubleKeyException(stream.tellg(), key->getValue(), history)));
            }
        }
        JSonElement *child = readNext(result);
        result->push(key->getValue(), child);
        delete keyObj;
        keyObj = consumeToken(result, buf);
    } while (!keyObj && buf.str() != "}");
    return result;
}

JSonArray *StreamConsumer::readArray(JSonContainer *parent)
{
    JSonArray *result = nullptr;
    std::stringstream sbuf;
    JSonElement *child = consumeToken(parent, sbuf);
    std::string buf = sbuf.str();

    if (child == nullptr && buf == "]")
        return new JSonArray(parent); //Empty object
    do
    {
        if (child == nullptr && buf == "[")
            child = readArray(nullptr);
        if (child == nullptr && buf == "{")
            child = readObject(nullptr);
        if (child == nullptr)
            throw JsonNotJsonException(stream.tellg(), history);
        if (result == nullptr)
            result = new JSonArray(parent);
        child->setParent(result);
        result->push_back(child);
        child = consumeToken(result, sbuf);
        buf = sbuf.str();
        if (child != nullptr)
            throw JsonUnexpectedException(']', stream.tellg(), history);
        else if (buf == "]")
            break;
        else if (buf != ",")
            throw JsonUnexpectedException(']', stream.tellg(), history);
        child = consumeToken(result, sbuf);
        buf = sbuf.str();
    } while (true);
    return result;
}

JSonElement *StreamConsumer::consumeString(JSonContainer *parent, std::stringstream &buf)
{
    bool escaped = false;

    buf.str("");
    buf.clear();

    while (stream.good())
    {
        char c = stream.get();
        history.put(c);

        if (!escaped)
        {
            if (c == '"')
                return new JSonPrimitive<std::string>(parent, buf.str());
            else if (c == '\\')
                escaped = true;
            else
                buf.write(&c, 1);
        }
        else
        {
            if (consumeEscapedChar(c, buf))
                escaped = false;
            else
                break;
        }
    }
    buf.str("");
    buf.clear();
    return nullptr;
}

bool StreamConsumer::consumeEscapedChar(char c, std::stringstream &buf)
{
    if (c == '\\' || c == '"' || c == '/')
        buf.write(&c, 1);
    else if (c == 'u')
    {
        if (params && params->isIgnoringUnicode())
            buf.write("\\u", 2);
        else
        {
            char unicodeBuf[4];
            stream.read(unicodeBuf, 4);
            std::streamsize gcount = stream.gcount();
            history.put(unicodeBuf, gcount);
            if (gcount != 4)
                return false;
            try {
                appendUnicode(unicodeBuf, buf);
            }
            catch (std::invalid_argument &e)
            {
                throw JsonHexvalueException(e.what(), stream.tellg(), history);
            }
        }
    }
    else if (c == 'b' || c == 'f' || c == 'r' || c == 'n' || c == 't')
        buf.write("\\", 1).write(&c, 1);
    else if (params && params->isStrict())
        throw JsonEscapedException(c, stream.tellg(), history);
    else
    {
        buf.write("\\", 1).write(&c, 1);
        warnings.push_back(Warning(JsonEscapedException(c, stream.tellg(), history)));
    }
    return true;
}

JSonElement *StreamConsumer::consumeBool(JSonContainer *parent, std::stringstream &buf, char firstChar)
{
    size_t read =1;

    buf.str("");
    buf.clear();
    buf.write(&firstChar, 1);

    //TODO batch-get 3 char, then do that
    while (stream.good())
    {
        char c = stream.get();
        history.put(c);

        if (c == 'a' || c == 'e' || c == 'l' || c == 'r' || c == 's' || c == 'u')
        {
            if ((read >= 5 && firstChar == 'f') || (read >= 4 && firstChar == 't'))
                throw JsonFormatException(stream.tellg(), history);
            buf.write(&c, 1);
            read++;
        }
        else if (buf.str() == "true")
        {
            history.pop_back();
            stream.unget();
            return new JSonPrimitive<bool>(parent, true);
        }
        else if (buf.str() == "false")
        {
            history.pop_back();
            stream.unget();
            return new JSonPrimitive<bool>(parent, false);
        }
        else if (ignoreChar(c))
            ;
        else
            throw JsonFormatException(stream.tellg(), history);
    }
    buf.str("");
    buf.clear();
    return nullptr;
}

JSonElement *StreamConsumer::consumeNumber(JSonContainer *parent, std::stringstream &buf, char firstChar)
{
    bool numberIsDouble = false;

    buf.str("");
    buf.clear();
    buf.write(&firstChar, 1);

    while (stream.good())
    {
        char c = stream.get();
        history.put(c);

        if (c >= '0' && c <= '9')
            buf.write(&c, 1);
        else if (c == '.' && !numberIsDouble)
        {
            numberIsDouble = true;
            buf.write(&c, 1);
        }
        else
        {
            history.pop_back();
            stream.unget();
            if (numberIsDouble)
            {
                try {
                    return new JSonPrimitive<double>(parent, atof(buf.str().c_str()));
                } catch (std::runtime_error &e)
                {
                    throw JsonFormatException(stream.tellg(), history);
                }
            }
            try
            {
                return new JSonPrimitive<int>(parent, std::stoi(buf.str()));
            }
            catch(std::out_of_range e)
            {
                return new JSonPrimitive<long long>(parent, std::stol(buf.str()));
            }
        }
    }
    buf.str("");
    buf.clear();
    return nullptr;
}

JSonElement *StreamConsumer::consumeNull(JSonContainer *parent, std::stringstream &buf)
{
    char _buf[5] = { 'n', '\0', '\0', '\0', '\0' };

    buf.str("");
    buf.clear();

    stream.read(&_buf[1], 3);
    buf.write(_buf, 4);
    history.put(&_buf[1], 3);
    if (!stream.good())
    {
        buf.str("");
        buf.clear();
        return nullptr;
    }
    if (std::string("null") == _buf)
        return new JSonPrimitive<Null>(parent, Null());
    throw JsonFormatException(stream.tellg(), history);
}

JSonElement *StreamConsumer::consumeToken(JSonContainer *parent, std::stringstream &buf)
{
    while (stream.good())
    {
        char c = stream.get();
        history.put(c);

        //!InString, !inbool
        if (c == '"')
            return consumeString(parent, buf);
        else if (c == 't' || c == 'f')
            return consumeBool(parent, buf, c);
        else if (c == 'n')
            return consumeNull(parent, buf);
        else if ((c >= '0' && c <= '9') || c == '.' || c == '-')
            return consumeNumber(parent, buf, c);
        else if (c == '{' || c == '[' || c == '}' || c == ']' || c == ':' || c == ',')
        {
            buf.str("");
            buf.clear();
            buf.write(&c, 1);
            return nullptr;
        }
        else if (!ignoreChar(c))
            throw JsonFormatException(stream.tellg(), history);
    }
    buf.str("");
    buf.clear();
    return nullptr;
}

void StreamConsumer::appendUnicode(const char unicode[4], std::stringstream &buf)
{
    unsigned short uni = hexbyte<unsigned short>(unicode, 4);
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::string unichar = converter.to_bytes(uni);
    buf.write(unichar.c_str(), unichar.size());
}

std::string StreamConsumer::extractUnicode(const char *buf)
{
    std::stringstream result;

    for (; *buf; buf++)
    {
        if (*buf == '\\' && buf[1] == 'u' && buf[2] && buf[3] && buf[4] && buf[5])
        {
            appendUnicode(buf +2, result);
            buf += 6;
        }
        else
            result.write(buf, 1);
    }
    return result.str();
}

std::string StreamConsumer::extractUnicode(const std::string &buf)
{
    return extractUnicode(buf.c_str());
}

bool StreamConsumer::ignoreChar(char c) const noexcept
{
    return (c <= 32 || c >= 127 || c == '\n');
}

const std::list<Warning> &StreamConsumer::getMessages() const
{ return warnings; }

