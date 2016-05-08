#include <gen/java_gen.h>

Environment::Environment(Environment *parent)
{
    parentEnv = parent;
}

Environment *Environment::open(Environment *parent)
{
    Environment *env = new Environment(parent);
    return env;
}

Environment *Environment::close(Environment *env)
{
    Environment *prev = env->parentEnv;
    delete env;
    return prev;
}

int Environment::localsCount()
{
    return maxLocals;
}

bool Environment::putClass(std::string name, std::string qualName)
{
    if (classIndex.find(name) != classIndex.end())
        return false;

    classIndex[name] = qualName;
    return true;
}

std::string Environment::getClass(std::string name)
{
    if (classIndex.find(name) == classIndex.end()) {
        if (parentEnv == nullptr)
            return "";
        return parentEnv->getClass(name);
    }

    return classIndex[name];
}

int Environment::putLocal(std::string name, Node type)
{
    if (localIndex.find(name) != localIndex.end())
        return -1;

    localType.push_back(type);
    localInit.push_back(false);

    return localIndex[name] = maxLocals++;
}

int Environment::getIndexLocal(std::string name)
{
    if (localIndex.find(name) == localIndex.end())
        return -1;

    return localIndex[name];
}

Node Environment::getTypeLocal(std::string name)
{
    int index = getIndexLocal(name);

    if (index == -1)
        return Node();

    return localType[index];
}

bool Environment::getInitLocal(std::string name)
{
    int index = getIndexLocal(name);

    if (index == -1)
        return false;

    return localInit[index];
}

bool Environment::setInitLocal(std::string name)
{
    int index = getIndexLocal(name);

    if (index == -1)
        return false;

    return localInit[index] = true;
}
