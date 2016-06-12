#include <iostream>
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

StreamConsumer *StreamConsumer::read(std::istream &stream)
{
    StreamConsumer *inst = new StreamConsumer(stream);
    inst->root = inst->readNext();
    return inst;
}

JSonElement *StreamConsumer::readNext()
{
    std::string buf;
    JSonElement *root = consumeToken(buf);

    if (root == nullptr)
    {
        if (buf == "{")
        {
            return readObject();
        }
        else if (buf == "[")
        {
            return readArray();
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

JSonObject *StreamConsumer::readObject()
{
    JSonElement *keyObj;
    JSonObject *result = nullptr;
    std::string buf;

    do
    {
        keyObj = consumeToken(buf);
        if (result == nullptr && keyObj == nullptr && buf == "}")
            return new JSonObject();
        JSonPrimitive<std::string> *key = dynamic_cast<JSonPrimitive<std::string> *>(keyObj);
        if (key == nullptr)
            throw new JsonException(stream.tellg());
        if (consumeToken(buf) != nullptr || buf != ":")
            throw new JsonException(stream.tellg());
        JSonElement *child = readNext();
        if (result == nullptr)
            result = new JSonObject();
        else if (result->contains(key->getValue()))
            throw new JsonException(stream.tellg()); //Double key
        result->push(key->getValue(), child);
        delete keyObj;
        keyObj = consumeToken(buf);
    } while (!keyObj && buf != "}");
    return result;
}

JSonArray *StreamConsumer::readArray()
{
    JSonArray *result = nullptr;
    JSonElement *child = readNext();
    std::string buf;

    if (child == nullptr && buf == "]")
        return new JSonArray(); //Empty object
    do
    {
        if (child == nullptr)
            throw new JsonException(stream.tellg());
        if (result == nullptr)
            result = new JSonArray();
        result->push_back(child);
        child = consumeToken(buf);
        if (child != nullptr)
            throw new JsonException(stream.tellg());
        else if (buf == "]")
            break;
        else if (buf != ",")
            throw new JsonException(stream.tellg());
        child = consumeToken(buf);
    } while (true);
    return result;
}

JSonElement *StreamConsumer::consumeToken(std::string &buf)
{
    bool escaped = false;
    bool inString = false;
    bool inBool = false;
    bool inNumber = false;
    bool numberIsFloat = false;

    while (stream.good())
    {
        char c = stream.get();

        if (inString)
        {
            if (!escaped)
            {
                if (c == '"')
                    return new JSonPrimitive<std::string>(buf);
                else if (c == '\\')
                    escaped = true;
                else
                    buf += c;
            }
            else
            {
                if (c == '\\' || c == '"')
                    buf += c;
                else
                    throw new JsonEscapedException(c, stream.tellg());
            }
        }
        else if (inBool)
        {
            if (c == 'a' || c == 'e' || c == 'l' || c == 'r' || c == 's' || c == 'u')
                buf += c;
            else if (buf == "true")
            {
                stream.unget();
                return new JSonPrimitive<bool>(true);
            }
            else if (buf == "false")
            {
                stream.unget();
                return new JSonPrimitive<bool>(false);
            }
            else if (ignoreChar(c))
                ;
            else
                throw new JsonFormatException(c, stream.tellg());
        }
        else if (inNumber)
        {
            if (c >= '0' && c <= '9')
                buf += c;
            else if (c == '.' && !numberIsFloat)
            {
                numberIsFloat = true;
                buf += c;
            }
            else
            {
                stream.unget();
                if (numberIsFloat)
                    return new JSonPrimitive<float>(std::stof(buf));
                try
                {
                    return new JSonPrimitive<int>(std::stoi(buf));
                }
                catch(std::out_of_range e)
                {
                    return new JSonPrimitive<long long>(std::stol(buf));
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
            else if ((c >= '0' && c <= '9') || c == '.')
            {
                buf = c;
                inNumber = true;
            }
            else if (!ignoreChar(c))
                throw new JsonFormatException(c, stream.tellg());
        }
    }
    buf = "";
    return nullptr;
}

bool StreamConsumer::ignoreChar(char c) const noexcept
{
    return (c <= 32 || c >= 127);
}

