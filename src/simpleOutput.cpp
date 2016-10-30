#include "jsonObjectEntry.hh"
#include "jsonContainer.hh"
#include "jsonPrimitive.hh"
#include "jsonElement.hh"

#include "simpleOutput.hh"
#include "config.h"
#include "params.hh"

SimpleOutput::SimpleOutput(std::ostream &output, const Params &p): out(output), params(p), indent(0)
{ }

void SimpleOutput::writeObjectEntry(const JSonObjectEntry *item)
{
    out << getIndent() << item->stringify() << ": ";

    if (dynamic_cast<const JSonContainer *> (**item))
    {
        out << std::endl;
        writeContainer((const JSonContainer*) **item);
    }
    else
        out << (**item)->stringify() << std::endl;
}

void SimpleOutput::writePrimitive(const AJSonPrimitive *item)
{
    if (indent)
        out << getIndent() << item->stringify() << std::endl;
    else
        out << item->stringify() << std::endl;
}

void SimpleOutput::writeContainer(const JSonContainer *item)
{
    std::string _indent = getIndent();
    const char *brackets = item->stringify().c_str();

    out << _indent << std::string(1, brackets[0]) << std::endl;

    indent_inc();
    for (JSonElement *i : *item)
        write(i);
    indent_inc(-1);

    out << _indent << std::string(1, brackets[2]) << std::endl;
}

void SimpleOutput::write(const JSonElement *item)
{
    if (dynamic_cast<const JSonContainer *>(item))
        writeContainer((const JSonContainer *) item);
    else if (dynamic_cast<const JSonObjectEntry*> (item))
        writeObjectEntry((const JSonObjectEntry *) item);
    else
        writePrimitive((const AJSonPrimitive *) item);
}

void SimpleOutput::indent_inc(int i)
{ indent += i; }

std::string SimpleOutput::getIndent() const
{ return std::string(indent * INDENT_LEVEL, ' '); }

void SimpleOutput::display(std::ostream &out, const JSonElement *root, const Params &p)
{
    SimpleOutput writer(out, p);
    writer.write(root);
}

