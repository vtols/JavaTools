#include <iostream>
#include <clocale>
#include <cwchar>

#include <io/file_stream.h>
#include <io/file_byte_stream_writer.h>
#include <parser/java_lexer.h>
#include <parser/java_parser.h>
#include <gen/java_gen.h>

int main()
{
    std::locale::global(std::locale(""));
    
    FileStream f("Sample.java");
    JavaLexer l(&f);
    JavaParser p = JavaParser(&l);

    SourceFile *src = p.parse();

    ClassGenerator gen(src);
    ClassFile *classFile = gen.generate();

    FileByteStreamWriter w("Sample.class");
    classFile->write(&w);

    return 0;
}
