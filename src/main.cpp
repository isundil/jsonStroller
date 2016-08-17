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

void displayException(const std::string &filename, const Params &params, const std::string &type, const JsonException &e)
{
    std::string buffer = e.getHistory();
    std::cerr << params.getProgName() << ": " << filename << " [" << type << "] at line " << e.currentLine() << ", " << e.currentCol() << " ("  << e.what() << ") while reading" << std::endl;
    std::cerr << buffer << std::endl << std::string(buffer.size() -1, '~') << '^' << std::endl;
}

StreamConsumer *readFile(std::pair<std::string, std::basic_istream<char>*> input, const Params &params)
{
    StreamConsumer *stream = new StreamConsumer(*(input.second));
    stream->withConfig(&params);
    bool success = true;

    try
    {
        stream->read();
        if (!stream->getRoot())
            throw EofException();
    }
    catch (EofException &e)
    {
        std::cerr << params.getProgName() << ": " << input.first << " " << Warning::getType(e) << " ("  << e.what() << ") error while reading" << std::endl;
        delete stream;
        return nullptr;
    }
    catch (JsonException &e)
    {
        std::cerr << "Error: ";
        displayException(input.first, params, Warning::getType(e), e);
        delete stream;
        return nullptr;
    }
    for (Warning w : stream->getMessages())
    {
        std::cerr << "Warning: ";
        displayException(input.first, params, w.getType(), w());
    }
    return stream;
}

void runDiff(const Params &params)
{
    const std::map<std::string, std::basic_istream<char>*> inputs = params.getInputs();
    std::set<StreamConsumer *> streams;
    std::list<JSonElement *> roots;
    bool success = true;

    if (!params.isIgnoringUnicode())
        setlocale(LC_ALL, "");
    for (std::pair<std::string, std::basic_istream<char>*> input : inputs)
    {
        StreamConsumer *stream = readFile(input, params);
        if (!stream)
        {
            success = false;
            break;
        }
        roots.push_back(stream->getRoot());
        streams.insert(stream);
    }
    if (success)
    {
        CurseOutput out(params);
        out.run(roots);
    }
    for (StreamConsumer *stream: streams)
        delete stream;
}

void run(const Params &params)
{
    std::map<std::string, std::basic_istream<char>*> inputs = params.getInputs();
    CurseOutput *out = new CurseOutput(params);

    if (!params.isIgnoringUnicode())
        setlocale(LC_ALL, "");
    for (std::pair<std::string, std::basic_istream<char>*> input : inputs)
    {
        StreamConsumer *stream = readFile(input, params);
        if (stream)
        {
            out->run(stream->getRoot());
            delete stream;
        }
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
    {
        if (params->isDiff())
            runDiff(*params);
        else
            run(*params);
    }
    delete params;
    return 0;
}

