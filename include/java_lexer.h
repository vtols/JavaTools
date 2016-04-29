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
    TokenIf,
    TokenWhile,

    /* Identifier */
    TokenId,

    /* Literals */
    TokenInteger,
    TokenString,

    /* Special characters and operators*/
    TokenDot,
    TokenComma,
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
    TokenMul,
    TokenAddAssign,
    TokenIncrement,
    TokenNot,
    TokenOr,
    TokenAnd,
    TokenLogicalNot,
    TokenLogicalOr,
    TokenLogiaclAnd,

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
