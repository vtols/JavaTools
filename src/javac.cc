#include <iostream>
#include <clocale>
#include <cwchar>

#include <io/file_stream.h>
#include <io/file_byte_writer.h>
#include <parser/java_lexer.h>
#include <parser/java_parser.h>
#include <gen/java_gen.h>

int main(int argc, char *argv[])
{
    std::locale::global(std::locale(""));

    std::string sourcePath = argv[1];
    size_t found = sourcePath.find_last_of('.');
    
    FileStream f(sourcePath);
    JavaLexer l(&f);
    JavaParser p = JavaParser(&l);

    SourceFile *src = p.parse();

    ClassGenerator gen(src);
    ClassFile *classFile = gen.generate();

    FileByteWriter w(sourcePath.substr(0, found) + ".class");
    classFile->write(&w);

    return 0;
}
