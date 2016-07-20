#include <iostream>
#include <sstream>
#include "jsonException.hh"
#include "jsonElement.hh"
#include "streamConsumer.hh"

StreamConsumer::StreamConsumer(std::istream &s): stream(s)
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

StreamConsumer *StreamConsumer::read(std::istream &stream, const AParams *config)
{
    StreamConsumer *inst = (new StreamConsumer(stream))->withConfig(config);
    inst->root = inst->readNext(nullptr);
    return inst;
}

JSonElement *StreamConsumer::readNext(JSonContainer *parent)
{
    std::string buf;
    JSonElement *root = consumeToken(parent, buf);

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

JSonElement * const StreamConsumer::getRoot() const
{
    return root;
}

JSonObject *StreamConsumer::readObject(JSonContainer *parent)
{
    JSonElement *keyObj;
    JSonObject *result = nullptr;
    std::string buf;

    do
    {
        keyObj = consumeToken(result, buf);
        if (result == nullptr && keyObj == nullptr && buf == "}")
            return new JSonObject(parent);
        JSonPrimitive<std::string> *key = dynamic_cast<JSonPrimitive<std::string> *>(keyObj);
        if (key == nullptr)
            throw JSonObject::NotAKeyException(stream.tellg(), history);
        if (consumeToken(parent, buf) != nullptr || buf != ":")
            throw JsonUnexpectedException(':', stream.tellg(), history);
        if (result == nullptr)
            result = new JSonObject(parent);
        else if (result->contains(key->getValue()))
            throw JSonObject::DoubleKeyException(stream.tellg(), key->getValue(), history); //Double key
        JSonElement *child = readNext(result);
        result->push(key->getValue(), child);
        delete keyObj;
        keyObj = consumeToken(result, buf);
    } while (!keyObj && buf != "}");
    return result;
}

JSonArray *StreamConsumer::readArray(JSonContainer *parent)
{
    JSonArray *result = nullptr;
    std::string buf;
    JSonElement *child = consumeToken(parent, buf);

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
        child = consumeToken(result, buf);
        if (child != nullptr)
            throw JsonUnexpectedException(']', stream.tellg(), history);
        else if (buf == "]")
            break;
        else if (buf != ",")
            throw JsonUnexpectedException(']', stream.tellg(), history);
        child = consumeToken(result, buf);
    } while (true);
    return result;
}

JSonElement *StreamConsumer::consumeToken(JSonContainer *parent, std::string &buf)
{
    bool escaped = false;
    bool inString = false;
    bool inBool = false;
    bool inNumber = false;
    bool numberIsDouble = false;

    while (stream.good())
    {
        char c = stream.get();
        history.put(c);

        if (inString)
        {
            if (!escaped)
            {
                if (c == '"')
                    return new JSonPrimitive<std::string>(parent, buf);
                else if (c == '\\')
                    escaped = true;
                else
                    buf += c;
            }
            else
            {
                if (c == '\\' || c == '"')
                {
                    buf += c;
                    escaped = false;
                }
                else if (c == 'u')
                {
                    if (params && params->isIgnoringUnicode())
                    {
                        buf += "\\u";
                        escaped = false;
                    }
                    else
                    {
                        char unicodeBuf[4];
                        stream.read(unicodeBuf, 4);
                        std::streamsize gcount = stream.gcount();
                        history.put(unicodeBuf, gcount);
                        if (gcount != 4)
                            break;
                        appendUnicode(unicodeBuf, buf);
                        escaped = false;
                    }
                }
                else
                    throw JsonEscapedException(c, stream.tellg(), history);
            }
        }
        else if (inBool)
        {
            if (c == 'a' || c == 'e' || c == 'l' || c == 'r' || c == 's' || c == 'u')
                buf += c;
            else if (buf == "true")
            {
                history.pop_back();
                stream.unget();
                return new JSonPrimitive<bool>(parent, true);
            }
            else if (buf == "false")
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
        else if (inNumber)
        {
            if (c >= '0' && c <= '9')
                buf += c;
            else if (c == '.' && !numberIsDouble)
            {
                numberIsDouble = true;
                buf += c;
            }
            else
            {
                history.pop_back();
                stream.unget();
                if (numberIsDouble)
                {
                    try {
                        return new JSonPrimitive<double>(parent, atof(buf.c_str()));
                    } catch (std::runtime_error &e)
                    {
                        throw JsonFormatException(stream.tellg(), history);
                    }
                }
                try
                {
                    return new JSonPrimitive<int>(parent, std::stoi(buf));
                }
                catch(std::out_of_range e)
                {
                    return new JSonPrimitive<long long>(parent, std::stol(buf));
                }
            }
        }
        else
        {
            //!InString, !inbool
            if (c == '"')
            {
                buf = "";
                inString = true;
            }
            else if (c == 't' || c == 'f')
            {
                buf = c;
                inBool = true;
            }
            else if (c == '{' || c == '[' || c == '}' || c == ']' || c == ':' || c == ',')
            {
                buf = c;
                return nullptr;
            }
            else if ((c >= '0' && c <= '9') || c == '.' || c == '-')
            {
                buf = c;
                inNumber = true;
            }
            else if (!ignoreChar(c))
                throw JsonFormatException(stream.tellg(), history);
        }
    }
    buf = "";
    return nullptr;
}

void StreamConsumer::appendUnicode(const char unicode[4], std::string &buf)
{
    std::string rawHex = { '0', 'x', unicode[0], unicode[1], unicode[2], unicode[3], '\0' };
    wchar_t unichar = std::stoul(rawHex, nullptr, 16);
    buf += unichar;
}

bool StreamConsumer::ignoreChar(char c) const noexcept
{
    return (c <= 32 || c >= 127 || c == '\n');
}

