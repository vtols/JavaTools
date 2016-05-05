#include <parser/java_lexer.h>
#include <parser/java_parser.h>

JavaParser::JavaParser(JavaLexer *lexer)
{
    l = lexer;
}

JavaFile *JavaParser::parse()
{
    token = l->next();
    lookup = l->next();
    return parseFile();
}

void JavaParser::match(JavaTokenType type)
{
    move();
}

void JavaParser::move()
{
    token = lookup;
    lookup = l->next();
}

JavaFile *JavaParser::parseFile()
{
    JavaFile *f = new JavaFile;
    f->imports = parseImports();
    f->jclass = parseClass();
    return f;
}

std::list<JavaImport*> JavaParser::parseImports()
{
    std::list<JavaImport*> imports;
    while (token.type == TokenImport) {
        JavaImport *import = new JavaImport;
        import->importString = "";

        match(TokenImport);
        while (token.type != TokenSemicolon) {
            import->importString += token.buffer;
            match(TokenId);
            if (token.type == TokenDot) {
                match(TokenDot);
                import->importString += '.';
            }
            else
                break;
        }
        match(TokenSemicolon);

        imports.push_back(import);
    }
    return imports;
}

JavaClassDeclaration *JavaParser::parseClass()
{
    JavaClassDeclaration *cls = new JavaClassDeclaration;
    if (token.type == TokenPrivate) {
        cls->accessMode = AccessPrivate;
        match(TokenPrivate);
    } else if (token.type == TokenPublic) {
        cls->accessMode = AccessPublic;
        match(TokenPublic);
    } else
        cls->accessMode = AccessDefault;

    /* class keyword*/
    match(TokenClass);

    cls->name = token.buffer;
    /* Class name */
    match(TokenId);
    
    /* { */
    match(TokenLeftCurly);
    
    while (token.type != TokenRightCurly)
        cls->methods.push_back(parseMethod());

    /* } */
    match(TokenRightCurly);

    return cls;
}

JavaMethodDeclaration *JavaParser::parseMethod()
{
    JavaMethodDeclaration *method = new JavaMethodDeclaration;
    
    method->accessMode = AccessDefault;
    method->staticMethod = false;

    bool mods = true;
    while (mods) {
        switch (token.type) {
            case TokenPublic:
                method->accessMode = AccessPublic;
                break;
            case TokenPrivate:
                method->accessMode = AccessPrivate;
                break;
            case TokenStatic:
                method->staticMethod = true;
                break;
            default:
                mods = false;
                break;
        }
        if (mods)
            move();
    }

    method->returnType = parseType();
    method->name = token.buffer;
    match(TokenId);

    method->arguments = parseMethodArguments();
    method->body = parseBlock();

    return method;
}

JavaArgs *JavaParser::parseMethodArguments()
{
    JavaArgs *args = new JavaArgs;
    /* ( */
    match(TokenLeftBracket);

    if (token.type != TokenRightBracket)
        while (true) {
            JavaArg *arg = new JavaArg;
            arg->type = parseType();

            /* Argument name */
            arg->name = token.buffer;
            match(TokenId);

            args->args.push_back(arg);

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

JavaType *JavaParser::parseType()
{
    JavaType *jtype = new JavaType;
    JavaTypeBase *base = new JavaTypeBase;
    jtype->typeBase = base;

    switch (token.type) {
        case TokenInt:
            base->kind = TypeInteger;
            break;
        case TokenVoid:
            base->kind = TypeVoid;
            break;
        case TokenId:
            base->kind = TypeReference;
            break;
        default:
            break;
    }
    base->name = token.buffer;
    /* Type name */
    /* Check carefully */
    move();

    jtype->subCount = 0;

    while (token.type == TokenLeftSquare) {
        /* [ */
        match(TokenLeftBracket);
        /* ] */
        match(TokenRightBracket);
        jtype->subCount++;
    }

    return jtype;
}

JavaBlock *JavaParser::parseBlock()
{
    JavaBlock *block = new JavaBlock;
    
    /* { */
    match(TokenLeftCurly);
    
    while (token.type != TokenRightCurly) {
        block->statements.push_back(parseStatement());
    }
    
    /* } */
    match(TokenRightCurly);
    
    return block;
}

JavaStatement *JavaParser::parseStatement()
{
    if (token.type == TokenIf)
        return parseIf();
    if (token.type == TokenWhile)
        return parseWhile();

    JavaStatement *st;

    if (lookup.type == TokenId)
        st = parseVarDeclaration();
    else
        st = parseExpression();
    match(TokenSemicolon);
    return st;
}

JavaIf *JavaParser::parseIf()
{
    JavaIf *jif = new JavaIf;

    match(TokenIf);
    match(TokenLeftBracket);
    jif->condition = parseExpression();
    match(TokenRightBracket);
    jif->body = parseBlock();

    return jif;
}

JavaWhile *JavaParser::parseWhile()
{
    JavaWhile *jwhile = new JavaWhile;

    match(TokenWhile);
    match(TokenLeftBracket);
    jwhile->condition = parseExpression();
    match(TokenRightBracket);
    jwhile->body = parseBlock();

    return jwhile;
}

JavaVarDeclaration *JavaParser::parseVarDeclaration()
{
    /* TODO */
    return nullptr;
}

JavaExpression *JavaParser::parseExpression()
{
    return parseAssignment();
}

JavaExpression *JavaParser::parseAssignment()
{
    JavaExpression *e = parseOr();
    if (token.type == TokenAssign) {
        match(TokenAssign);
        e = new JavaAssignment(e, parseAssignment());
    }
    return e;
}

JavaExpression *JavaParser::parseOr()
{
    JavaExpression *e = parseAnd();
    while (token.type == TokenOr) {
        match(TokenOr);
        e = new JavaOr(e, parseAnd());
    }
    return e;
}

JavaExpression *JavaParser::parseAnd()
{
    JavaExpression *e = parseComparison();
    while (token.type == TokenAnd) {
        match(TokenAnd);
        e = new JavaAnd(e, parseComparison());
    }
    return e;
}

JavaExpression *JavaParser::parseComparison()
{
    JavaExpression *e = parseSum();
    JavaToken save = token;
    switch (token.type) {
        case TokenLess:
        case TokenLessOrEqual:
            move();
            e = new JavaCmp(save.type, e, parseSum());
            break;
        default:
            break;
    }
    return e;
}

JavaExpression *JavaParser::parseSum()
{
    JavaExpression *e = parseProduct();
    while (token.type == TokenAdd) {
        match(TokenAdd);
        e = new JavaAdd(e, parseProduct());
    }
    return e;
}

JavaExpression *JavaParser::parseProduct()
{
    JavaExpression *e = parseBracketedOrUnary();
    while (token.type == TokenMul) {
        match(TokenMul);
        e = new JavaMul(e, parseBracketedOrUnary());
    }
    return e;
}

JavaExpression *JavaParser::parseBracketedOrUnary()
{
    JavaExpression *e;
    if (token.type == TokenLeftBracket) {
        match(TokenLeftBracket);
        e = parseExpression();
        match(TokenRightBracket);
    } else
        e = parseUnary();
    return e;
}

JavaExpression *JavaParser::parseUnary()
{
    JavaExpression *e = new JavaExpression;
    if (token.type == TokenLogicalNot) {
        match(TokenLogicalNot);
        /* It is possible that we need
         * separate class for logical binaries
         */
        e = new JavaUnaryOp(TokenLogicalNot, parseBracketedOrUnary());
    } else if (token.type == TokenIncrement) {
        match(TokenIncrement);
        e = new JavaUnaryOp(TokenIncrement, parseBracketedOrUnary());
    } else if (token.type == TokenLeftBracket)
        e = parseBracketedOrUnary();
    else {
        e = parseAccessSequence();
        /* Differentiate it with prefix increment */
        if (token.type == TokenIncrement) {
            match(TokenIncrement);
            e = new JavaUnaryOp(TokenIncrement, e);
        }
    }
    return e;
}

JavaAccessSequence *JavaParser::parseAccessSequence()
{
    JavaAccessSequence *s = nullptr;

    bool tail = false;
    while (true) {
        bool dot_matched = false;
        if (tail && token.type == TokenDot) {
            match(TokenDot);
            dot_matched = true;
        }

        if (!tail && (token.type == TokenString ||
                token.type == TokenInteger)) {
            JavaLiteral *literal = new JavaLiteral(token);
            match(token.type);
            return literal;
        }
        if (token.type == TokenId &&
            lookup.type == TokenLeftBracket) {
            JavaMethodCall *mc = new JavaMethodCall(token.buffer);
            mc->name = token.buffer;

            match(TokenId);
            match(TokenLeftBracket);

            if (token.type != TokenRightBracket)
                while (true) {
                    JavaExpression *arg = parseExpression();

                    mc->argExpressions.push_back(arg);

                    /* , */
                    if (token.type == TokenComma)
                        match(TokenComma);
                    else
                        break;
                }

            /* ) */
            match(TokenRightBracket);

            mc->base = s;
            s = mc;
        }
        else if (token.type == TokenId) {
            JavaIdAccess *id = new JavaIdAccess(token.buffer);
            match(TokenId);

            id->base = s;
            s = id;
        }
        else if (tail && !dot_matched &&
                 token.type == TokenLeftSquare) {
            JavaExpression *index = new JavaExpression;

            /* [ */
            match(TokenLeftSquare);

            index = parseExpression();

            /* ] */
            match(TokenRightSquare);

            JavaSubscript *sub = new JavaSubscript(index);
            sub->base = s;
        }

        if (token.type != TokenDot &&
            token.type != TokenLeftSquare)
            break;
        tail = true;
    }
    return s;
}
