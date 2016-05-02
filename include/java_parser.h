#ifndef JAVA_PARSER_H
#define JAVA_PARSER_H

#include <java_lexer.h>
#include <java_unit.h>

class JavaParser
{
public:
    JavaParser(JavaLexer *lexer);
    JavaFile *parse();
    
private:
    JavaLexer *l;
    JavaToken token, lookup;
    void match(JavaTokenType type);
    void move();
    JavaFile *parseFile();
    std::list<JavaImport*> parseImports();
    JavaClassDeclaration *parseClass();
    JavaMethodDeclaration *parseMethod();
    JavaArgs *parseMethodArguments();
    JavaType *parseType();
    JavaBlock *parseBlock();
    JavaStatement *parseStatement();
    JavaIf *parseIf();
    JavaWhile *parseWhile();
    JavaVarDeclaration *parseVarDeclaration();
    JavaExpression *parseExpression();
    JavaExpression *parseAssignment();
    JavaExpression *parseOr();
    JavaExpression *parseAnd();
    JavaExpression *parseComparison();
    JavaExpression *parseSum();
    JavaExpression *parseProduct();
    JavaExpression *parseBracketedOrUnary();
    JavaExpression *parseUnary();
    JavaAccessSequence *parseAccessSequence();
};

#endif /* JAVA_PARSER_H */
