/**
 * main.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include <iostream>
#include <locale.h>
#include "streamConsumer.hh"
#include "curseOutput.hh"
#include "params.hh"
#include "jsonException.hh"

void displayException(const Params *params, const std::string &type, const JsonException &e)
{
    std::string buffer = e.getHistory();
    std::cerr << params->getProgName() << ": [" << type << "] at line " << e.currentLine() << ", " << e.currentCol() << " ("  << e.what() << ") while reading" << std::endl;
    std::cerr << buffer << std::endl << std::string(buffer.size() -1, '~') << '^' << std::endl;
}

void run(Params *params)
{
    std::list<std::basic_istream<char>*> inputs = params->getInputs();
    CurseOutput *out = new CurseOutput(*params);

    for (std::basic_istream<char>* input : inputs)
    {
        StreamConsumer stream(*input);
        stream.withConfig(params);
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
            std::cerr << params->getProgName() << ": " << Warning::getType(e) << " ("  << e.what() << ") error while reading" << std::endl;
            return;
        }
        catch (JsonException &e)
        {
            std::cerr << "Error: ";
            displayException(params, Warning::getType(e), e);
            return;
        }
        for (Warning w : stream.getMessages())
        {
            std::cerr << "Warning: ";
            displayException(params, w.getType(), w());
        }
        out->run(root);
    }
    delete out;
}

int main(int ac, char **av)
{
    (void) ac;
    Params *params = new Params(av);
    bool _run = false;

    try {
        _run = params->read();
    }
    catch (std::runtime_error &e)
    {
        std::cerr << *av << ": " << e.what() << std::endl;
        params->usage();
        delete params;
        exit (EXIT_FAILURE);
    }

    if (_run)
        run(params);
    delete params;
    return 0;
}

