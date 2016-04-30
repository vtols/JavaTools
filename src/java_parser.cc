#include <java_lexer.h>
#include <java_parser.h>

JavaParser::JavaParser(JavaLexer *lexer)
{
    l = lexer;
}

JavaFile JavaParser::parse()
{
    token = l.next();
    lookup = l.next();
    return parseFile();
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

    if (token.type != TokenRightBracket)
        while (true) {
            JavaArg arg;
            arg.type = parseType();

            /* Argument name */
            arg.name = token.buffer;
            match(TokenId);

            args.args.push_back(arg);

            /* , */
            if (token.type == TokenComma)
                match(TokenComma);
            else
                break;
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
    JavaExpression e = parseBracketedOrUnary();
    while (token.type == TokenMul) {
        match(TokenMul);
        e = JavaAnd(e, parseBracketedOrUnary());
    }
    return e;
}

JavaExpression JavaParser::parseBracketedOrUnary()
{
    JavaExpression e;
    if (token.type == TokenLeftBracket) {
        match(TokenLeftBracket);
        e = parseExpression();
        match(TokenRightBracket);
    } else
        e = parseUnary();
    return e;
}

JavaExpression JavaParser::parseUnary()
{
    JavaExpression e;
    if (token.type == TokenLogicalNot) {
        match(TokenLogicalNot);
        /* It is possible that we need
         * separate class for logical binaries
         */
        e = JavaUnaryOp(TokenLogicalNot, parseBracketedOrUnary());
    } else if (token.type == TokenIncrement) {
        match(TokenIncrement)
        e = JavaUnaryOp(TokenIncrement, parseBracketedOrUnary());
    } else if (token.type == TokenLeftBracket)
        e = parseBracketedOrUnary();
    else {
        e = parseAccessSequence();
        /* Differentiate it with prefix increment */
        if (token.type == TokenIncrement) {
            match(TokenIncrement);
            e = JavaUnaryOp(TokenIncrement, e);
        }
    }
    return e;
}

JavaAccessSequence JavaParser::parseAccessSequence()
{
    JavaAccessSequence s = nullptr;

    bool tail = true;

    while (true) {
        bool dot_matched = false;
        if (tail && token.type == TokenDot) {
            match(TokenDot);
            dot_matched = true;
        }

        if (token.type == TokenId &&
            lookup.type == TokenLeftBracket) {
            JavaMethodCall mc = JavaMethodCall(token.buffer);
            mc.name = token.buffer;

            match(TokenId);
            match(TokenLeftBracket);

            if (token.type != TokenRightBracket)
                while (true) {
                    JavaExpression arg = parseExpression();

                    mc.argExpressions.push_back(arg);

                    /* , */
                    if (token.type == TokenComma)
                        match(TokenComma);
                    else
                        break;
                }

            /* ) */
            match(TokenRightBracket);

            mc.base = s;
            s = mc;
        }
        else if (token.type == TokenId) {
            JavaIdAccess id = JavaIdAccess(token.buffer);
            match(TokenId);

            id.base = s;
            s = id;
        }
        else if (tail && !dot_matched &&
                 token.type == TokenLeftSquare) {
            JavaExpression index;

            /* [ */
            match(TokenLeftSquare);

            index = parseExpression();

            /* ] */
            match(TokenRightSquare);

            JavaSubscript sub = JavaSubscript(index);
            sub.base = s;
        }

        if (token.type != TokenDot &&
            token.type != TokenLeftSquare)
            break;
        tail = true;
    }
}

