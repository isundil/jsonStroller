#include <iostream>
#include <typeinfo>
#include "streamConsumer.hh"
#include "curseOutput.hh"
#include "params.hh"
#include "jsonException.hh"

void run(Params *params)
{
    StreamConsumer *stream;
    CurseOutput *out;
    JSonElement *root;

    try
    {
        stream = StreamConsumer::read(params->getInput());
        root = stream->getRoot();
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
        std::cerr << params->getProgName() << ": [" << typeid(e).name() << "] ("  << e.what() << ") error while reading" << std::endl;
        std::string buffer = e.getHistory();
        std::cerr << buffer << std::endl << std::string(buffer.size() -1, '~') << '^' << std::endl;
        return;
    }
    out = new CurseOutput(root);
    out->run();
    delete out;
    delete stream;
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

