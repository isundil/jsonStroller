/**
 * main.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include <iostream>
#include <typeinfo>
#include <locale.h>
#include "streamConsumer.hh"
#include "curseOutput.hh"
#include "params.hh"
#include "jsonException.hh"

void displayException(const Params *params, const JsonException &e)
{
    std::string buffer = e.getHistory();
    std::cerr << params->getProgName() << ": [" << typeid(e).name() << "] at line " << e.currentLine() << " ("  << e.what() << ") while reading" << std::endl;
    std::cerr << buffer << std::endl << std::string(buffer.size() -1, '~') << '^' << std::endl;
}

void run(Params *params)
{

    StreamConsumer stream(StreamConsumer(params->getInput()));
    stream.withConfig(params);
    CurseOutput *out;
    JSonElement *root;

    if (!params->isIgnoringUnicode())
        setlocale(LC_ALL, "");
    try
    {
        root = stream.read()->getRoot();
        if (!root)
            throw EofException();
    }
    catch (EofException &e)
    {
        std::cerr << params->getProgName() << ": " << typeid(e).name() << " ("  << e.what() << ") error while reading" << std::endl;
        return;
    }
    catch (JsonException &e)
    {
        std::cerr << "Error: ";
        displayException(params, e);
        return;
    }
    for (Warning w : stream.getMessages())
    {
        std::cerr << "Warning: ";
        displayException(params, w());
    }
    out = new CurseOutput(root, *params);
    out->run();
    delete out;
}

int main(int ac, char **av)
{
    Params *params;

    try {
        params = new Params(ac, av);
    }
    catch (std::runtime_error &e)
    {
        std::cerr << *av << ": " << e.what() << std::endl;
        Params::usage(*av);
        exit (EXIT_FAILURE);
    }

    if (params->isValid())
        run(params);
    delete params;
    return 0;
}

