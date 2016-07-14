#include <string>
#include <sstream>
#include <cassert>
#include <climits>
#include <stdlib.h>
#include "jsonElement.hh"
#include "streamConsumer.hh"

const char testJson[] = "{\"widget\": {\"debug\": \"on\",\"window\": {\"title\": \"Sample Konfabulator Widget\",\"name\": \"main_window\",\"width\": 500,\"height\": 500},\"image\": { \"src\": \"Images/Sun.png\",\"name\": \"sun1\",\"hOffset\": 250,\"vOffset\": 250,\"alignment\": \"center\"},\"text\": {\"data\": \"Click Here\",\"size\": 36,\"style\": \"bold\",\"name\": \"text1\",\"hOffset\": 250,\"vOffset\": 100,\"alignment\": \"center\",\"onMouseUp\": \"sun1.opacity = (sun1.opacity / 100) * 90;\"}}}";

StreamConsumer *toJson(std::string str)
{
    std::stringstream input(str);
    return StreamConsumer::read(input);
}

void checkArray()
{
    //Check array
    StreamConsumer *root = toJson("[true, 42, \"coucou\", 12.34, false, -12, -12.42]");
    JSonArray *arr = dynamic_cast<JSonArray *>(root->getRoot());
    assert(arr != nullptr);
    JSonArray::const_iterator it = arr->cbegin();
    assert((dynamic_cast<JSonPrimitive<bool> *> (*it)) != nullptr);
    assert(((JSonPrimitive<bool> *)(*it))->getValue() == true);
    it++;
    assert((dynamic_cast<JSonPrimitive<int> *> (*it)) != nullptr);
    assert(((JSonPrimitive<int> *)(*it))->getValue() == 42);
    it++;
    assert((dynamic_cast<JSonPrimitive<std::string> *> (*it)) != nullptr);
    assert(((JSonPrimitive<std::string> *)(*it))->getValue() == "coucou");
    it++;
    assert((dynamic_cast<JSonPrimitive<double> *> (*it)) != nullptr);
    assert(((JSonPrimitive<double> *)(*it))->getValue() == 12.34);
    it++;
    assert((dynamic_cast<JSonPrimitive<bool> *> (*it)) != nullptr);
    assert(((JSonPrimitive<bool> *)(*it))->getValue() == false);
    it++;
    assert((dynamic_cast<JSonPrimitive<int> *> (*it)) != nullptr);
    assert(((JSonPrimitive<int> *)(*it))->getValue() == -12);
    it++;
    assert((dynamic_cast<JSonPrimitive<double> *> (*it)) != nullptr);
    double value = ((JSonPrimitive<double> *)(*it))->getValue();
    assert(value == -12.42);
    delete root;
}

void checkTypes()
{
    //Check basic types
    StreamConsumer *root = toJson("{\"test\":\"value\"}");
    assert(dynamic_cast<JSonObject*>(root->getRoot()) != nullptr);
    delete root;
    root = toJson("[\"test\",\"value\"]");
    assert(dynamic_cast<JSonArray*>(root->getRoot()) != nullptr);
    delete root;
    root = toJson("\"test\"");
    assert(dynamic_cast<JSonPrimitive<std::string> *>(root->getRoot()) != nullptr);
    delete root;
    root = toJson("42");
    assert(dynamic_cast<JSonPrimitive<int> *>(root->getRoot()) != nullptr);
    delete root;
    root = toJson("42.2");
    assert(dynamic_cast<JSonPrimitive<double> *>(root->getRoot()) != nullptr);
    assert((((JSonPrimitive<double> *)(root->getRoot()))->getValue()) == 42.2);
    delete root;
    root = toJson(std::to_string((long long)LLONG_MAX));
    assert(dynamic_cast<JSonPrimitive<long long> *>(root->getRoot()) != nullptr);
    delete root;
    root = toJson("true");
    assert(dynamic_cast<JSonPrimitive<bool> *>(root->getRoot()) != nullptr);
    delete root;
}

void checkObject()
{
    //Check Obj
    StreamConsumer *root = toJson("{\"bool\":true, \"int\":42, \"str\":\"coucou\", \"double\":12.34, \"arrayOfInt\":[1, 2, 3, 4.5]}");
    assert(dynamic_cast<JSonObject *>(root->getRoot()) != nullptr);
    assert(((JSonObject *)(root->getRoot()))->size() == 5);
    const JSonElement *tmp = ((JSonObject *)(root->getRoot()))->get("bool");
    assert((dynamic_cast<const JSonPrimitive<bool> *> (tmp)) != nullptr);
    assert((dynamic_cast<const JSonPrimitive<bool> *> (tmp))->getValue() == true);
    tmp = ((JSonObject *)(root->getRoot()))->get("int");
    assert((dynamic_cast<const JSonPrimitive<int> *> (tmp)) != nullptr);
    assert((dynamic_cast<const JSonPrimitive<int> *> (tmp))->getValue() == 42);
    tmp = ((JSonObject *)(root->getRoot()))->get("str");
    assert((dynamic_cast<const JSonPrimitive<std::string> *> (tmp)) != nullptr);
    assert((dynamic_cast<const JSonPrimitive<std::string> *> (tmp))->getValue() == "coucou");
    tmp = ((JSonObject *)(root->getRoot()))->get("double");
    assert((dynamic_cast<const JSonPrimitive<double> *> (tmp)) != nullptr);
    assert((dynamic_cast<const JSonPrimitive<double> *> (tmp))->getValue() == 12.34);
    tmp = ((JSonObject *)(root->getRoot()))->get("arrayOfInt");
    const JSonArray *arr2 = dynamic_cast<const JSonArray *> (tmp);
    assert(arr2 != nullptr);
    assert(arr2->size() == 4);
    JSonArray::const_iterator it = arr2->cbegin();
    assert((dynamic_cast<JSonPrimitive<int> *> (*it)) != nullptr);
    assert(((JSonPrimitive<int> *)(*it))->getValue() == 1);
    it++;
    assert((dynamic_cast<JSonPrimitive<int> *> (*it)) != nullptr);
    assert(((JSonPrimitive<int> *)(*it))->getValue() == 2);
    it++;
    assert((dynamic_cast<JSonPrimitive<int> *> (*it)) != nullptr);
    assert(((JSonPrimitive<int> *)(*it))->getValue() == 3);
    it++;
    assert((dynamic_cast<JSonPrimitive<double> *> (*it)) != nullptr);
    assert(((JSonPrimitive<double> *)(*it))->getValue() == 4.5);
    delete root;
}

void checkSample()
{
    StreamConsumer *root = toJson(testJson);
    root->getRoot();
}

int main(int ac, char **av)
{
    checkTypes();
    checkArray();
    checkObject();
    checkSample();
    exit(EXIT_SUCCESS);
}

