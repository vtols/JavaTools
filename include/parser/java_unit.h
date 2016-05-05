#ifndef JAVA_UNIT_H
#define JAVA_UNIT_H

#include <list>
#include <string>

#include <parser/java_lexer.h>

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

enum NodeTag
{
    NodeEmpty,
    NodeClass,
    NodeMethod,
    NodeType,
    NodeTypeBase,
    NodeArgumentList,
    NodeArgument,
    NodeBlock,
    NodeIf,
    NodeWhile,
    NodeAssign,
    NodeOr,
    NodeAnd,
    NodeLess,
    NodeLeq,
    NodeAdd,
    NodeMul,
    NodeLogicalNot,
    NodePreIncrement,
    NodePostIncrement,
    NodeIntegerLiteral,
    NodeStringLiteral,
    NodeCallMethod,
    NodeCallArguments,
    NodeId,
    NodeSubscript
};

struct Node
{
    NodeTag tag;
    void *nodeData;

    Node();
    Node(NodeTag tag, void *nodeData);
    static Node binary(NodeTag tag, Node l, Node r);
    static Node unary(NodeTag tag, Node n);
};

struct SourceFile
{
    std::string package;
    std::list<std::string> imports;
    Node jclass;
};

struct ClassDeclaration
{
    JavaAccess accessMode;
    std::string name;
    std::list<Node> methods;
};

struct MethodDeclaration
{
    bool staticMethod;
    JavaAccess accessMode;
    std::string name;
    Node returnType;
    Node arguments;
    Node body;
};

struct Type
{
    Node typeBase;

    /* TypeName[][][]... count*/
    uint8_t subCount;
};

struct TypeBase
{
    JavaTypeKind kind;
    std::string name;
};

struct NodeList
{
    std::list<Node> nodes;
};

struct BinaryNode
{
    Node left, right;
};

struct UnaryNode
{
    Node next;
};

struct Argument
{
    Node type;
    std::string name;
};

struct AccessElement
{
    Node prev;
    Node attribute;
    std::string name;
};

struct Literal
{
    std::string lit;
};

#endif /* JAVA_UNIT_H */
