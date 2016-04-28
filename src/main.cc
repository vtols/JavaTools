#include <iostream>
#include <clocale>
#include <cwchar>

#include <file_stream.h>
#include <java_lexer.h>

int main()
{
    std::locale::global(std::locale(""));
    
    CharStream *f = new FileStream("Main.java");
    JavaLexer l = JavaLexer(f);

    while (true) {
        JavaToken jtok = l.next();
        if (jtok.type == TokenEof)
            break;
        std::wcout << jtok.buffer << L"\n";
    }

    delete f;

    return 0;
}
