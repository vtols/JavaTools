#ifndef JAVA_PARSER_H
#define JAVA_PARSER_H

#include <java_lexer.h>

class JavaParser
{
    void parse();
    
private:
    JavaLexer l;
    JavaToken token, lookup;

    void parseFile();
    void parseImports();
    void parseClass();
    void parseMethod();
};

#endif /* JAVA_PARSER_H */
