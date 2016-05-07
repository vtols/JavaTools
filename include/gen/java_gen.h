#ifndef JAVA_GEN_H
#define JAVA_GEN_H

#include <map>

#include <parser/java_unit.h>

class ClassGenerator
{
public:
    ClassGenerator(SourceFile *source);
    ClassFile *generate();

private:
    SourceFile *src;
    Environment *env;

    std::string className;
    ClassDeclaration *classContext;
    ClassBuilder *cb;
    std::string methodName;
    MethodDeclaration *methodContext;
    MethodBuilder *mb;
};

class Environment
{
public:
    Environment();

    bool putClass(std::string name, std::string qualName);
    std::string getClass(std::string name);

    int putLocal(std::string name, Node type);
    Node getLocal(std::string name);

    static Environment *open(Environment *parent);
    static Environment *close(Environment *env);

private:
    Environment *parent;

    // Map class name to qualified name
    std::map<std::string, std::string> classIndex;

    // Map local variable name to its index
    std::map<std::string, uint8_t> localIndex;
    // Map local variable name to its type
    std::vector<Node> localType;
    // Whether local variable is initialized
    std::vector<bool> localInit;
    // First free index
    uint8_t maxLocals = 0;
};

#endif /* JAVA_GEN_H */
