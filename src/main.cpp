#include <iostream>
#include "streamConsumer.hh"
#include "curseOutput.hh"
#include "params.hh"

void run(Params *params)
{
    StreamConsumer *stream;
    CurseOutput *out;
    JSonElement *root;

    stream = StreamConsumer::read(params->getInput());
    root = stream->getRoot();
    if (root)
    {
        out = new CurseOutput(root);
        out->run();
        delete out;
    }
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

