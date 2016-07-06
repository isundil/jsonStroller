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
    Params *params = new Params(ac, av);

    if (params->isValid())
        run(params);
    delete params;
    return 0;
}

