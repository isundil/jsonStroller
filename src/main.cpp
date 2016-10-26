/**
 * main.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include <iostream>
#include <locale.h>
#include "streamConsumer.hh"
#include "curseSplitOutput.hh"
#include "curseSimpleOutput.hh"
#include "params.hh"
#include "jsonException.hh"

void displayException(const std::string &filename, const Params &params, const std::string &type, const JsonException &e)
{
    const std::string buffer = e.getHistory();

    std::cerr << params.getProgName() << ": " << filename << " [" << type << "] at line " << e.currentLine() << ", col " << e.currentCol() << " ("  << e.what() << ") while reading" << std::endl;
    std::cerr << buffer << std::endl << std::string(buffer.size() -1, '~') << '^' << std::endl;
}

StreamConsumer *readFile(std::pair<std::string, std::basic_istream<char>*> input, const Params &params)
{
    StreamConsumer *stream = new StreamConsumer(*(input.second));
    stream->withConfig(&params);

    stream->read();
    if (!stream->getRoot())
        throw EofException();
    return stream;
}

void runDiff(const Params &params)
{
    const IndexedDeque inputs = params.getInputs();
    const size_t nbInputs = inputs.size();
    std::set<StreamConsumer *> streams;
    std::deque<JSonElement *> roots;
    std::deque<Warning> warns;
    std::deque<std::string> inputNames;

    for (std::pair<std::string, std::basic_istream<char>*> input : inputs)
    {
        StreamConsumer *stream;

        inputNames.push_back(input.first);
        try
        {
            stream = readFile(input, params);
        }
        catch (EofException &e)
        {
            std::cerr << params.getProgName() << ": " << input.first << " " << Warning::getType(e) << " ("  << e.what() << ") error while reading" << std::endl;
            delete stream;
            stream = nullptr;
        }
        catch (JsonException &e)
        {
            std::cerr << "Error: ";
            displayException(input.first, params, Warning::getType(e), e);
            delete stream;
            stream = nullptr;
        }
        for (Warning w : stream->getMessages())
        {
            w.filename(input.first);
            warns.push_back(Warning(w));
        }
        roots.push_back(stream->getRoot());
        streams.insert(stream);
    }
    if (streams.size() == nbInputs)
    {
        CurseSplitOutput out(params);
        out.run(inputNames, roots);
    }
    for (StreamConsumer *stream: streams)
        delete stream;
    for (Warning w : warns)
    {
        std::cerr << "Warning: ";
        displayException(w.filename(), params, w.getType(), w());
    }
}

void run(const Params &params)
{
    IndexedDeque inputs = params.getInputs();
    CurseSimpleOutput *out = new CurseSimpleOutput(params);
    std::list<Warning> warns;

    for (std::pair<std::string, std::basic_istream<char>*> input : inputs)
    {
        StreamConsumer *stream;
        try
        {
            stream = readFile(input, params);
        }
        catch (EofException &e)
        {
            delete out;
            out = nullptr;
            std::cerr << params.getProgName() << ": " << input.first << " " << Warning::getType(e) << " ("  << e.what() << ") error while reading" << std::endl;
            break;
        }
        catch (JsonException &e)
        {
            delete out;
            out = nullptr;
            std::cerr << "Error: ";
            displayException(input.first, params, Warning::getType(e), e);
            break;;
        }
        for (Warning w : stream->getMessages())
        {
            w.filename(input.first);
            warns.push_back(Warning(w));
        }
        out->run(stream->getRoot(), input.first);
        delete stream;
    }
    if (out)
        delete out;
    for (Warning w : warns)
    {
        std::cerr << "Warning: ";
        displayException(w.filename(), params, w.getType(), w());
    }
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
        if (!params->isIgnoringUnicode())
            setlocale(LC_ALL, "");
        if (params->isDiff())
            runDiff(*params);
        else
            run(*params);
    }
    delete params;
    return 0;
}

