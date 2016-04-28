#ifndef JAVA_LEXER_H
#define JAVA_LEXER_H

#include <string>

#include <char_stream.h>
#include <java_lexer.h>

enum JavaTokenType
{
    /* Keywords */
    TokenImport,
    TokenClass,
    TokenStatic,
    TokenVoid,
    TokenPublic,
    TokenPrivate,
    TokenInt,

    /* Identifier */
    TokenId,

    /* Literals */
    TokenInteger,
    TokenString,

    /* Special characters and operators*/
    TokenDot,
    TokenAsterisk,
    TokenSemicolon,
    TokenLeftBracket,
    TokenRightBracket,
    TokenLeftCurly,
    TokenRightCurly,
    TokenLeftSquare,
    TokenRightSquare,

    TokenAssign,
    TokenEqual,
    TokenLess,
    TokenLessOrEqual,
    TokenAdd,
    TokenAddAssign,
    TokenIncrement,

    /* End of file */
    TokenEof
};

class JavaToken
{
public:
    JavaToken(JavaTokenType type, std::wstring buffer);
    JavaTokenType type;
    std::wstring buffer;
};

class JavaLexer
{
public:
    JavaLexer(CharStream *stream);

    JavaToken next();

private:
    CharStream *s;
    std::wstring buffer;

    JavaTokenType getTokenType();
    void saveCharacter();
    void clearBuffer();
};

#endif /* JAVA_LEXER_H */
