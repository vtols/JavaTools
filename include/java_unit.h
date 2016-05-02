#ifndef JAVA_UNIT_H
#define JAVA_UNIT_H

#include <list>
#include <string>

#include <java_lexer.h>

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

struct JavaFile;
struct JavaImport;
struct JavaClassDeclaration;
struct JavaMethodDeclaration;
struct JavaType;
struct JavaTypeBase;
struct JavaArgs;
struct JavaArg;
struct JavaBlock;
struct JavaStatement;
struct JavaVarDeclaration;
struct JavaAssignment;
struct JavaConditional;
struct JavaIf;
struct JavaWhile;
struct JavaExpression;
struct JavaBinaryOp;
struct JavaAnd;
struct JavaOr;
struct JavaAdd;
struct JavaMul;
struct JavaUnaryOp;
struct JavaCmp;
struct JavaAccessSequence;
struct JavaIdAccess;
struct JavaMethodCall;
struct JavaSubscript;

struct JavaFile
{
    std::list<JavaImport*> imports;
    JavaClassDeclaration *jclass;
};

struct JavaImport
{
    std::wstring importString;
};

struct JavaClassDeclaration
{
    JavaAccess accessMode;
    std::wstring name;
    std::list<JavaMethodDeclaration*> methods;
};

struct JavaMethodDeclaration
{
    bool staticMethod;
    JavaAccess accessMode;
    std::wstring name;
    JavaType *returnType;
    JavaArgs *arguments;
    JavaBlock *body;
};

struct JavaType
{
    JavaTypeBase *typeBase;

    /* TypeName[][][]... count*/
    uint8_t subCount;
};

struct JavaTypeBase
{
    JavaTypeKind kind;
    std::wstring name;
};

struct JavaArgs
{
    std::list<JavaArg*> args;
};

struct JavaArg
{
    JavaType *type;
    std::wstring name;
};

struct JavaBlock
{
    std::list<JavaStatement*> statements;
};

struct JavaStatement
{
};

struct JavaVarDeclaration : JavaStatement
{
};

struct JavaConditional : JavaStatement
{
    JavaExpression *condition;
    JavaBlock *body;
};

struct JavaIf : JavaConditional
{
};

struct JavaWhile : JavaConditional
{
};

struct JavaExpression : JavaStatement
{
    // return false; by default
    // need to check if we use ++ and --
    virtual bool isVariable();
};

struct JavaAssignment : JavaExpression
{
    JavaExpression *to, *from;
    JavaAssignment(JavaExpression *to, JavaExpression *from);
};

struct JavaBinaryOp : JavaExpression
{
    JavaExpression *left, *right;
};

struct JavaAnd : JavaBinaryOp
{
    JavaAnd(JavaExpression *left, JavaExpression *right);
};

struct JavaOr : JavaBinaryOp
{
    JavaOr(JavaExpression *left, JavaExpression *right);
};

struct JavaAdd : JavaBinaryOp
{
    JavaAdd(JavaExpression *left, JavaExpression *right);
};

struct JavaMul : JavaBinaryOp
{
    JavaMul(JavaExpression *left, JavaExpression *right);
};

struct JavaUnaryOp : JavaExpression
{
    JavaTokenType opToken;
    JavaExpression *expr;

    JavaUnaryOp(JavaTokenType type, JavaExpression *e);
};

struct JavaCmp : JavaBinaryOp
{
    JavaTokenType cmpToken;

    JavaCmp(JavaTokenType type, JavaExpression *left,
        JavaExpression *right);
};

struct JavaAccessSequence : JavaExpression
{
    JavaAccessSequence *base;

    JavaAccessSequence();
};

struct JavaIdAccess : JavaAccessSequence
{
    std::wstring name;

    JavaIdAccess(std::wstring name);
};

struct JavaMethodCall : JavaIdAccess
{
    std::list<JavaExpression*> argExpressions;

    JavaMethodCall(std::wstring name);
};

struct JavaSubscript : JavaAccessSequence
{
    JavaExpression *subscriptExpression;

    JavaSubscript(JavaExpression *index);
};

#endif /* JAVA_UNIT_H */
