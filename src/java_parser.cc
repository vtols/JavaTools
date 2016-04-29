#include <java_lexer.h>
#include <java_parser.h>

JavaParser::JavaParser(JavaLexer *lexer)
{
    l = lexer;
}

void JavaParser::parse()
{
    token = l.next();
    lookup = l.next();
    parseFile();
}

void JavaParser::match(JavaTokenType type)
{
    move();
}

void JavaParser::move()
{
    token = lookup;
    lookup = l.next();
}

JavaFile JavaParser::parseFile()
{
    JavaFile f;
    f.imports = parseImports();
    f.jclass = parseClass();

    return f;
}

std::list<JavaImport> JavaParser::parseImports()
{
    std::list<JavaImport> imports;
    while (token.type == TokenImport) {
        JavaImport import;
        import.importString = L"";

        match(TokenImport);
        while (token.type != TokenSemicolon) {
            import.importString += token.buffer;
            match(TokenId);
            match(TokenDot);
        }
        match(TokenSemicolon);

        imports.push_back(import);
    }
    return imports;
}

JavaClassDeclaration JavaParser::parseClass()
{
    JavaClassDeclaration cls;
    if (token.type == TokenPrivate) {
        cls.accessMode = AccessPrivate;
        match(TokenPrivate);
    } else if (toke.type == TokenPublic) {
        cls.accessMode = AccessPublic;
        match(TokenPublic);
    } else
        cls.accessMode = AccessDefault;

    /* class keyword*/
    match(TokenClass);

    cls.name = token.buffer;
    /* Class name */
    match(TokenId);
    
    /* { */
    match(TokenLeftCurly);
    
    while (token.type != TokenRightCurly)
        cls.methods.push_back(parseMethod());

    /* } */
    match(TokenRightCurly);

    return cls;
}

void JavaParser::parseMethod()
{
    JavaMethodDeclaration method;
    
    method.accessMode = AccessDefault;
    method.staticMethod = false;

    bool mods = true;
    while (mods) {
        switch (token.type) {
            case TokenPublic:
                method.accessMode = AccessPublic;
                break;
            case TokenPrivate:
                method.accessMode = AccessPrivate;
                break;
            case TokenStatic:
                method.staticMethod = true;
                break;
            default:
                mods = false;
                break;
        }
        if (mods)
            token = l.next();
    }

    method.returnType = parseType();
    
    parseBlock();
}

JavaArgs JavaParser::parseMethodArguments()
{
    JavaArgs args;
    /* ( */
    match(TokenLeftBracket);

    while (tok.type != TokenRightBracket) {
        JavaArg arg;
        arg.type = parseType();

        /* Argument name */
        arg.name = token.buffer;
        match(TokenId);

        args.args.push_back(arg);

        /* , */
        match(TokenComma);
    }
    
    /* ) */
    match(TokenRightBracket);

    return args;
}

JavaType JavaParser::parseType()
{
    JavaType jtype;

    switch (token.type) {
        case TokenInt:
            jtype.typeBase = TypeInteger;
            break;
        case TokenVoid:
            jtype.typeBase = TypeVoid;
            break;
        case TokenId:
            jtype.typeBase = TypeReference;
            break;
    }
    jtype.name = token.buffer;
    /* Type name */
    /* Check carefully */
    move();

    jtype.subCount = 0;

    while (token.type == TokenLeftSquare) {
        /* [ */
        match(TokenLeftBracket);
        /* ] */
        match(TokenRightBracket);
        jtype.subCount++;
    }

    return jtype;
}

JavaBlock JavaParser::parseBlock()
{
    JavaBlock block;
    
    /* { */
    match(TokenLeftCurly);
    
    while (token.type != TokenRightCurly) {
        block.statements.push_back(parseStatement());
    }
    
    /* } */
    match(TokenRightCurly);
    
    return block;
}

JavaStatement JavaParser::parseStatement()
{
    if (token.type == TokenIf)
        return parseIf();
    if (token.type == TokenWhile)
        return parseWhile();
    if (lookup.type = TokenId)
        return parseVarDeclaration();
    return parseExpression();
}

JavaIf JavaParser::parseIf()
{
    JavaIf jif;

    match(TokenIf);
    match(TokenLeftBracket);
    jif.condition = parseExpression()
    match(TokenRightBracket);
    jif.body = parseBlock();
}

JavaWhile JavaParser::parseWhile()
{
    JavaWhile jwhile;

    match(TokenWhile);
    match(TokenLeftBracket);
    jif.condition = parseExpression()
    match(TokenRightBracket);
    jif.body = parseBlock();
}

JavaExpression JavaParser::parseExpression()
{
    return parseAssignment();
}

JavaExpression JavaParser::parseAssignment()
{
    JavaExpression e = parseOr();
    if (token.type == TokenAssign) {
        match(TokenAssign);
        e = JavaAssignment(e, parseAssignment())
    }
    return e;
}

JavaExpression JavaParser::parseOr()
{
    JavaExpression e = parseAnd();
    while (token.type == TokenOr) {
        match(TokenOr);
        e = JavaOr(e, parseAnd());
    }
    return e;
}

JavaExpression JavaParser::parseAnd()
{
    JavaExpression e = parseComparison();
    while (token.type == TokenAnd) {
        match(TokenAnd);
        e = JavaAnd(e, parseComparison());
    }
    return e;
}

JavaExpression JavaParser::parseComparison()
{
    JavaExpression e = parseSum();
    switch (token.type) {
        case TokenLess:
        case TokenLessOrEqual:
            JavaToken save = token;
            move();
            e = JavaCmp(token.type, e, parseSum());
            break;
    }
}

JavaExpression JavaParser::parseSum()
{
    JavaExpression e = parseProduct();
    while (token.type == TokenAdd) {
        match(TokenAdd);
        e = JavaAnd(e, parseProduct());
    }
    return e;
}

JavaExpression JavaParser::parseProduct()
{
    JavaExpression e = parseUnary();
    while (token.type == TokenMul) {
        match(TokenMul);
        e = JavaAnd(e, parseUnary());
    }
    return e;
}

JavaExpression JavaParser::parseUnary()
{
    JavaExpression e;
    if (token.type == TokenLeftBracket) {
        match(TokenLeftBracket);
        e = parseExpression();
        match(TokenRightBracket);
    }
    else if (
    e = parseAccessSequence();
}



