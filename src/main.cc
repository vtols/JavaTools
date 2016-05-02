#include <iostream>
#include <clocale>
#include <cwchar>

#include <file_stream.h>
#include <java_lexer.h>
#include <java_parser.h>

int main()
{
    std::locale::global(std::locale(""));
    
    CharStream *f = new FileStream("Sample.java");
    JavaLexer *l = new JavaLexer(f);
    JavaParser p = JavaParser(l);

    JavaFile *j = p.parse();

    delete l;
    delete f;

    return 0;
}
