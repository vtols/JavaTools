#ifndef JAVA_PARSER_H
#define JAVA_PARSER_H

#include <parser/java_lexer.h>
#include <parser/java_unit.h>

class JavaParser
{
public:
    JavaParser(JavaLexer *lexer);
    SourceFile *parse();
    
private:
    JavaLexer *l;
    JavaToken token, lookup;
    void match(JavaTokenType type);
    void move();
    SourceFile *parseFile();
    void parseImports(std::list<std::string>& imports);
    Node parseClass();
    Node parseMethod();
    Node parseMethodArguments();
    Node parseType();
    Node parseBlock();
    Node parseStatement();
    Node parseConditional();
    Node parseVarDeclaration();
    Node parseExpression();
    Node parseAssignment();
    Node parseOr();
    Node parseAnd();
    Node parseComparison();
    Node parseSum();
    Node parseProduct();
    Node parseBracketedOrUnary();
    Node parseUnary();
    Node parseAccessSequence();
};

#endif /* JAVA_PARSER_H */
