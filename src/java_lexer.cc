#include <cwctype>

#include <java_lexer.h>

JavaToken::JavaToken()
{
    this->type = TokenEof;
    this->buffer = L"";
}

JavaToken::JavaToken(JavaTokenType type, std::wstring buffer)
{
    this->type = type;
    this->buffer = buffer;
}

JavaLexer::JavaLexer(CharStream *stream)
{
    s = stream;
    buffer = L"";
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
    buffer = L"";
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

        if (buffer == L"import")
            return TokenImport;
        else if (buffer == L"class")
            return TokenClass;
        else if (buffer == L"public")
            return TokenPublic;
        else if (buffer == L"static")
            return TokenStatic;
        else if (buffer == L"void")
            return TokenVoid;
        else if (buffer == L"int")
            return TokenInt;
        else if (buffer == L"if")
            return TokenInt;
        else if (buffer == L"while")
            return TokenInt;
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
        case L'.':
            tok = TokenDot;
            break;
        case L',':
            tok = TokenComma;
            break;
        case L'*':
            tok = TokenAsterisk;
            break;
        case L';':
            tok = TokenSemicolon;
            break;
        case L'(':
            tok = TokenLeftBracket;
            break;
        case L')':
            tok = TokenRightBracket;
            break;
        case L'{':
            tok = TokenLeftCurly;
            break;
        case L'}':
            tok = TokenRightCurly;
            break;
        case L'[':
            tok = TokenLeftSquare;
            break;
        case L']':
            tok = TokenRightSquare;
            break;
        case L'!':
            tok = TokenLogicalNot;
            break;
        case L'<':
            saveCharacter();
            if (s->peek() == L'=')
                tok = TokenLessOrEqual;
            else
                return TokenLess;
            break;
        case L'=':
            saveCharacter();
            if (s->peek() == L'=')
                tok = TokenEqual;
            else
                return TokenAssign;
            break;
        case L'+':
            saveCharacter();
            if (s->peek() == L'=')
                tok = TokenAddAssign;
            if (s->peek() == L'+')
                tok = TokenIncrement;
            else
                return TokenAdd;
            break;
        case '&':
            saveCharacter();
            if (s->peek() == L'&')
                tok = TokenLogicalAnd;
            else
                return TokenAnd;
            break;
        case '|':
            saveCharacter();
            if (s->peek() == L'|')
                tok = TokenLogicalOr;
            else
                return TokenOr;
            break;
    }
    saveCharacter();
    return tok;
}
