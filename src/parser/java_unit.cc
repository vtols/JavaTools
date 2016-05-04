#include <parser/java_unit.h>

JavaAssignment::JavaAssignment(JavaExpression *to,
    JavaExpression * from)
{
    this->to = to;
    this->from = from;
}

bool JavaExpression::isVariable()
{
    return false;
}

JavaOr::JavaOr(JavaExpression *left, JavaExpression *right)
{
    this->left = left;
    this->right = right;
}

JavaAnd::JavaAnd(JavaExpression *left, JavaExpression *right)
{
    this->left = left;
    this->right = right;
}

JavaAdd::JavaAdd(JavaExpression *left, JavaExpression *right)
{
    this->left = left;
    this->right = right;
}

JavaMul::JavaMul(JavaExpression *left, JavaExpression *right)
{
    this->left = left;
    this->right = right;
}

JavaCmp::JavaCmp(JavaTokenType type, JavaExpression *left,
    JavaExpression *right)
{
    this->cmpToken = type;
    this->left = left;
    this->right = right;
}

JavaUnaryOp::JavaUnaryOp(JavaTokenType type, JavaExpression *e)
{
    this->opToken = type;
    this->expr = e;
}

JavaAccessSequence::JavaAccessSequence()
{
    this->base = nullptr;
}

JavaMethodCall::JavaMethodCall(std::wstring name) :
    JavaIdAccess(name)
{
}

JavaIdAccess::JavaIdAccess(std::wstring name)
{
    this->name = name;
}

JavaSubscript::JavaSubscript(JavaExpression *index)
{
    this->subscriptExpression = index;
}

JavaLiteral::JavaLiteral(JavaToken token)
{
    this->literalToken = token;
}
