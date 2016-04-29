#ifndef JAVA_UNIT_H
#define JAVA_UNIT_H

#include <list>

enum JavaTypeKind
{
    TypeInteger,
    TypeVoid,
    TypeReference
};

enum JavaAccess
{
    AccessDefault,
    AccessPublic,
    AccessPrivate,
    AccessProtected
};

class JavaFile
{
private:
    std::list<JavaImport> imports;
    JavaClassDeclaration jclass;
};

class JavaImport
{
private:
    std::wstring importString;
};

class JavaClassDeclaration
{
private:
    JavaAccess accessMode;
    std::wstring name;
    std::list<JavaMethodDeclaration> methods;
};

class JavaMethodDeclaration
{
private:
    bool staticMethod;
    JavaAccess accessMode;

    JavaType returnType;
    JavaArgs arguments;

    JavaBlock body;
};

class JavaType
{
private:
    JavaTypeBase typeBase;

    /* TypeName[][][]... count*/
    uint8_t subCount;
};

class JavaTypeBase
{
    JavaTypeKind kind;
    std::string name;
};

class JavaArgs
{
private:
    std::list<JavaArg> args;
};

class JavaArg
{
    JavaType type;
    std::wstring name;
};

class JavaBlock
{
    std::list<JavaStatement> statements;
};

class JavaStatement
{
};

class JavaVarDeclaration // : JavaStatement
{
};

class JavaConditional // : JavaStatement
{
    JavaExpression condition;
    JavaBlock body;
};

class JavaIf // : JavaConditional
{
};

class JavaWhile // : JavaConditional
{
};

class JavaExpression
{
};

class JavaAssignment // : JavaStatement
{
};

class JavaBinaryOp
{
    JavaExpression left, right;
};

class JavaCmp // : JavaBinaryOp
{
    JavaTokenType cmpToken;
    
};

#endif /* JAVA_UNIT_H */
