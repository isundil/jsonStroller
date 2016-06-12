
#include <iostream>
#include "streamConsumer.hh"
#include "params.hh"

int main(int ac, char **av)
{
    Params *params = new Params(ac, av);
    JSonElement *rootNode;

    if (!params->isValid())
        return 0;
    rootNode = StreamConsumer::read(params->getInput())->getRoot();
}

