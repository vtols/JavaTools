#ifndef JAVA_GEN_H
#define JAVA_GEN_H

#include <map>

#include <class/java_class_builder.h>
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

    void generateMethods();
    void generateMethod();
    void generateBlock(Node block);
    void generateNode(Node st);
    void generateVarDecl(Node varDecl);
    void generateAssign(Node assign);
    void generateAdd(Node add);
    void generateMul(Node mul);
    void generateId(Node id);
    void generateStringLiteral(Node lit);
    void generateIntegerLiteral(Node lit);
};

class Environment
{
public:
    Environment(Environment *parent);

    bool putClass(std::string name, std::string qualName);
    std::string getClass(std::string name);

    int putLocal(std::string name, Node type);
    Node getTypeLocal(std::string name);
    int getIndexLocal(std::string name);
    bool setInitLocal(std::string name);
    bool getInitLocal(std::string name);

    static Environment *open(Environment *parent);
    static Environment *close(Environment *env);

private:
    Environment *parentEnv;

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
