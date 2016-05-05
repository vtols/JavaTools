#include <cwctype>

#include <parser/java_lexer.h>

JavaToken::JavaToken()
{
    this->type = TokenEof;
    this->buffer = "";
}

JavaToken::JavaToken(JavaTokenType type, std::string buffer)
{
    this->type = type;
    this->buffer = buffer;
}

JavaLexer::JavaLexer(CharStream *stream)
{
    s = stream;
    buffer = "";
}

JavaToken JavaLexer::next()
{
    clearBuffer();
    JavaTokenType type = getTokenType();
    return JavaToken(type, buffer);
}

void JavaLexer::saveCharacter()
{
    if (!s->eof())
        buffer += s->read();
}

void JavaLexer::clearBuffer()
{
    buffer = "";
}

JavaTokenType JavaLexer::getTokenType()
{
    while (!s->eof() &&
            (iswblank(s->peek()) || s->peek() == '\n'))
        s->read();
        
    if (s->eof())
        return TokenEof;

    if (iswalpha(s->peek())) {
        while (isalpha(s->peek()))
            saveCharacter();

        if (buffer == "import")
            return TokenImport;
        else if (buffer == "class")
            return TokenClass;
        else if (buffer == "public")
            return TokenPublic;
        else if (buffer == "static")
            return TokenStatic;
        else if (buffer == "void")
            return TokenVoid;
        else if (buffer == "int")
            return TokenInt;
        else if (buffer == "if")
            return TokenInt;
        else if (buffer == "while")
            return TokenWhile;
        else
            return TokenId;
    }

    if (iswdigit(s->peek())) {
        while (iswdigit(s->peek()))
            saveCharacter();

        return TokenInteger;
    }

    if (s->peek() == '"') {
        saveCharacter();
        while (s->peek() != '"')
            saveCharacter();
        saveCharacter();

        return TokenString;
    }

    JavaTokenType tok;
    switch (s->peek()) {
        case '.':
            tok = TokenDot;
            break;
        case ',':
            tok = TokenComma;
            break;
        case '*':
            tok = TokenAsterisk;
            break;
        case ';':
            tok = TokenSemicolon;
            break;
        case '(':
            tok = TokenLeftBracket;
            break;
        case ')':
            tok = TokenRightBracket;
            break;
        case '{':
            tok = TokenLeftCurly;
            break;
        case '}':
            tok = TokenRightCurly;
            break;
        case '[':
            tok = TokenLeftSquare;
            break;
        case ']':
            tok = TokenRightSquare;
            break;
        case '!':
            tok = TokenLogicalNot;
            break;
        case '<':
            saveCharacter();
            if (s->peek() == '=')
                tok = TokenLessOrEqual;
            else
                return TokenLess;
            break;
        case '=':
            saveCharacter();
            if (s->peek() == '=')
                tok = TokenEqual;
            else
                return TokenAssign;
            break;
        case '+':
            saveCharacter();
            if (s->peek() == '=')
                tok = TokenAddAssign;
            if (s->peek() == '+')
                tok = TokenIncrement;
            else
                return TokenAdd;
            break;
        case '&':
            saveCharacter();
            if (s->peek() == '&')
                tok = TokenLogicalAnd;
            else
                return TokenAnd;
            break;
        case '|':
            saveCharacter();
            if (s->peek() == '|')
                tok = TokenLogicalOr;
            else
                return TokenOr;
            break;
    }
    saveCharacter();
    return tok;
}
