#include <iostream>
#include <clocale>
#include <cwchar>

#include <io/file_stream.h>
#include <parser/java_lexer.h>
#include <parser/java_parser.h>

int main()
{
    std::locale::global(std::locale(""));
    
    CharStream *f = new FileStream("Sample.java");
    JavaLexer *l = new JavaLexer(f);
    JavaParser p = JavaParser(l);

    SourceFile *src = p.parse();

    delete l;
    delete f;

    return 0;
}
