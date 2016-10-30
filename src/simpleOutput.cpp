#include "jsonObjectEntry.hh"
#include "jsonContainer.hh"
#include "jsonPrimitive.hh"
#include "jsonElement.hh"

#include "simpleOutput.hh"
#include "config.h"
#include "params.hh"

SimpleOutput::SimpleOutput(std::ostream &output, const Params &p): out(output), params(p), indent(0)
{ }

void SimpleOutput::writeObjectEntry(const JSonObjectEntry *item, bool prependComma)
{
    out << getIndent() << (prependComma ? ",\"" : "\"") << item->stringify() << "\":";

    if (dynamic_cast<const JSonContainer *> (**item))
    {
        if (!params.compressed())
            out << std::endl;
        writeContainer((const JSonContainer*) **item, false);
    }
    else
    {
        out << (**item)->stringify();
        if (!params.compressed())
            out << std::endl;
    }
}

void SimpleOutput::writePrimitive(const AJSonPrimitive *item, bool prependComma)
{
    out << getIndent() << (prependComma ? "," : "");

    if (dynamic_cast<const JSonPrimitive<std::string>*> (item))
        out << "\"" << item->stringify() << "\"";
    else
        out << item->stringify();
    if (!params.compressed())
        out << std::endl;
}

void SimpleOutput::writeContainer(const JSonContainer *item, bool prependComma)
{
    std::string _indent = getIndent();
    const char *brackets = item->stringify().c_str();
    bool first = true;

    out << _indent << (prependComma ? "," : "") << std::string(1, brackets[0]);
    if (!params.compressed())
        out << std::endl;

    indent++;
    for (JSonElement *i : *item)
    {
        write(i, !first);
        if (first)
            first = false;
    }
    indent--;

    out << _indent << std::string(1, brackets[2]);
    if (!params.compressed())
        out << std::endl;
}

void SimpleOutput::write(const JSonElement *item, bool prependComma)
{
    if (dynamic_cast<const JSonContainer *>(item))
        writeContainer((const JSonContainer *) item, prependComma);
    else if (dynamic_cast<const JSonObjectEntry*> (item))
        writeObjectEntry((const JSonObjectEntry *) item, prependComma);
    else
        writePrimitive((const AJSonPrimitive *) item, prependComma);
}

std::string SimpleOutput::getIndent() const
{
    if (params.compressed())
        return "";
    return std::string(indent * INDENT_LEVEL, ' ');
}

void SimpleOutput::display(std::ostream &out, const JSonElement *root, const Params &p)
{
    SimpleOutput writer(out, p);
    writer.write(root, false);
}

