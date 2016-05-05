#include <parser/java_lexer.h>
#include <parser/java_parser.h>

JavaParser::JavaParser(JavaLexer *lexer)
{
    l = lexer;
}

SourceFile *JavaParser::parse()
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

SourceFile *JavaParser::parseFile()
{
    SourceFile *f = new SourceFile;
    parseImports(f->imports);
    f->jclass = parseClass();
    return f;
}

void JavaParser::parseImports(
        std::list<std::string>& imports)
{
    while (token.type == TokenImport) {
        std::string import;

        match(TokenImport);
        while (token.type != TokenSemicolon) {
            import += token.buffer;
            match(TokenId);
            if (token.type == TokenDot) {
                match(TokenDot);
                import += '.';
            }
            else
                break;
        }
        match(TokenSemicolon);

        imports.push_back(import);
    }
}

Node JavaParser::parseClass()
{
    ClassDeclaration *cls = new ClassDeclaration;
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

    return Node(NodeClass, cls);
}

Node JavaParser::parseMethod()
{
    MethodDeclaration *method = new MethodDeclaration;
    
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

    return Node(NodeMethod, method);
}

Node JavaParser::parseMethodArguments()
{
    NodeList *args = new NodeList;
    /* ( */
    match(TokenLeftBracket);

    if (token.type != TokenRightBracket)
        while (true) {
            Argument *arg = new Argument;
            arg->type = parseType();

            /* Argument name */
            arg->name = token.buffer;
            match(TokenId);

            args->nodes.push_back(Node(NodeArgument, arg));

            /* , */
            if (token.type == TokenComma)
                match(TokenComma);
            else
                break;
        }
    
    /* ) */
    match(TokenRightBracket);

    return Node(NodeArgumentList, args);
}

Node JavaParser::parseType()
{
    Type *jtype = new Type;
    TypeBase *base = new TypeBase;
    jtype->typeBase = Node(NodeTypeBase, base);

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

    return Node(NodeType, jtype);
}

Node JavaParser::parseBlock()
{
    NodeList *block = new NodeList;
    
    /* { */
    match(TokenLeftCurly);
    
    while (token.type != TokenRightCurly) {
        block->nodes.push_back(parseStatement());
    }
    
    /* } */
    match(TokenRightCurly);
    
    return Node(NodeBlock, block);
}

Node JavaParser::parseStatement()
{
    if (token.type == TokenIf ||
            token.type == TokenWhile)
        return parseConditional();

    Node st;

    if (lookup.type == TokenId)
        st = parseVarDeclaration();
    else
        st = parseExpression();
    match(TokenSemicolon);
    return st;
}

Node JavaParser::parseConditional()
{
    BinaryNode *cond = new BinaryNode;
    NodeTag tag;

    if (token.type == TokenIf)
        tag = NodeIf;
    else
        tag = NodeWhile;

    move();
    match(TokenLeftBracket);
    cond->left = parseExpression();
    match(TokenRightBracket);
    cond->right = parseBlock();

    return Node(tag, cond);
}

Node JavaParser::parseVarDeclaration()
{
    /* TODO */
    return Node();
}

Node JavaParser::parseExpression()
{
    return parseAssignment();
}

Node JavaParser::parseAssignment()
{
    Node e = parseOr();
    if (token.type == TokenAssign) {
        match(TokenAssign);
        e = Node::binary(NodeAssign, e, parseAssignment());
    }
    return e;
}

Node JavaParser::parseOr()
{
    Node e = parseAnd();
    while (token.type == TokenOr) {
        match(TokenOr);
        e = Node::binary(NodeOr, e, parseAnd());
    }
    return e;
}

Node JavaParser::parseAnd()
{
    Node e = parseComparison();
    while (token.type == TokenAnd) {
        match(TokenAnd);
        e = Node::binary(NodeAnd, e, parseComparison());
    }
    return e;
}

Node JavaParser::parseComparison()
{
    NodeTag tag = NodeEmpty;
    Node e = parseSum();
    JavaToken save = token;
    switch (token.type) {
        case TokenLess:
            tag = NodeLess;
            break;
        case TokenLessOrEqual:
            tag = NodeLeq;
            break;
        default:
            break;
    }
    if (tag != NodeEmpty) {
        move();
        e = Node::binary(tag, e, parseSum());
    }
    return e;
}

Node JavaParser::parseSum()
{
    Node e = parseProduct();
    while (token.type == TokenAdd) {
        match(TokenAdd);
        e = Node::binary(NodeAdd, e, parseProduct());
    }
    return e;
}

Node JavaParser::parseProduct()
{
    Node e = parseBracketedOrUnary();
    while (token.type == TokenMul) {
        match(TokenMul);
        e = Node::binary(NodeMul, e, parseBracketedOrUnary());
    }
    return e;
}

Node JavaParser::parseBracketedOrUnary()
{
    Node e;
    if (token.type == TokenLeftBracket) {
        match(TokenLeftBracket);
        e = parseExpression();
        match(TokenRightBracket);
    } else
        e = parseUnary();
    return e;
}

Node JavaParser::parseUnary()
{
    Node e;
    if (token.type == TokenLogicalNot) {
        match(TokenLogicalNot);
        e = Node::unary(NodeLogicalNot, parseBracketedOrUnary());
    } else if (token.type == TokenIncrement) {
        match(TokenIncrement);
        e = Node::unary(NodePreIncrement, parseBracketedOrUnary());
    } else if (token.type == TokenLeftBracket)
        e = parseBracketedOrUnary();
    else {
        e = parseAccessSequence();
        if (token.type == TokenIncrement) {
            match(TokenIncrement);
            e = Node::unary(NodePostIncrement, e);
        }
    }
    return e;
}

Node JavaParser::parseAccessSequence()
{
    Node s;

    bool tail = false;
    while (true) {
        bool dot_matched = false;
        if (tail && token.type == TokenDot) {
            match(TokenDot);
            dot_matched = true;
        }

        if (!tail && (token.type == TokenString ||
                token.type == TokenInteger)) {
            NodeTag tag;
            Literal *literal = new Literal;
            literal->lit = token.buffer;
            if (token.type == TokenString)
                tag = NodeStringLiteral;
            else
                tag = NodeIntegerLiteral;
            match(token.type);
            return Node(tag, literal);
        }
        if (token.type == TokenId &&
            lookup.type == TokenLeftBracket) {
            AccessElement *methodCall = new AccessElement;
            NodeList *arguments = new NodeList;
            methodCall->name = token.buffer;
            methodCall->attribute =
                    Node(NodeCallArguments, arguments);

            match(TokenId);
            match(TokenLeftBracket);

            if (token.type != TokenRightBracket)
                while (true) {
                    Node arg = parseExpression();

                    arguments->nodes.push_back(arg);

                    /* , */
                    if (token.type == TokenComma)
                        match(TokenComma);
                    else
                        break;
                }

            /* ) */
            match(TokenRightBracket);

            methodCall->prev = s;
            s = Node(NodeCallMethod, methodCall);
        }
        else if (token.type == TokenId) {
            AccessElement *id = new AccessElement;
            id->name = token.buffer;
            match(TokenId);

            id->prev = s;
            s = Node(NodeId, id);
        }
        else if (tail && !dot_matched &&
                 token.type == TokenLeftSquare) {
            /* [ */
            match(TokenLeftSquare);

            Node index = parseExpression();

            /* ] */
            match(TokenRightSquare);

            AccessElement *sub = new AccessElement;
            sub->attribute = index;
            sub->prev = s;
            s = Node(NodeSubscript, sub);
        }

        if (token.type != TokenDot &&
            token.type != TokenLeftSquare)
            break;
        tail = true;
    }
    return s;
}
