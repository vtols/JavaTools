#include <gen/java_gen.h>

ClassGenerator::ClassGenerator(SourceFile *source)
{
    src = source;
}

ClassFile *ClassGenerator::generate()
{
    env = Environment::open(nullptr);

    classContext = (ClassDeclaration *) src->jclass.nodeData;
    className = classContext->name;

    cb = new ClassBuilder(className);
    generateMethods();
    ClassFile *cf = cb->build();
    delete cb;

    env = Environment::close(env);

    return cf;
}

void ClassGenerator::generateMethods()
{
    for (Node methodNode : classContext->methods) {
        methodContext = (MethodDeclaration *) methodNode.nodeData;
        generateMethod();
    }
}

void ClassGenerator::generateMethod()
{
    methodName = methodContext->name;
    mb = cb->createMethod(methodName);

    /* Cheating, must generate access and descriptor */
    if (methodName == "main") {
        mb->setDescriptor("([Ljava/lang/String;)V");
        mb->setAccessFlags(ACC_PUBLIC | ACC_STATIC);
    }

    env = Environment::open(env);

    Node argNodeList = methodContext->arguments;
    std::list<Node> args = ((NodeList *) argNodeList.nodeData)->nodes;

    for (Node argNode : args) {
        Argument *arg = (Argument *) argNode.nodeData;
        env->putLocal(arg->name, arg->type);
    }
    generateBlock(methodContext->body);

    /*
    mb->field(opcodes::GETSTATIC,
            "java/lang/System", "out", "Ljava/io/PrintStream;");
    mb->local(opcodes::ILOAD, 1);
    mb->invoke(opcodes::INVOKEVIRTUAL,
            "java/io/PrintStream", "println", "(I)V");
    */
    mb->instruction(opcodes::RETURN);

    /* Must generate maxLocals */
    mb->setMax(100, env->localsCount());

    env = Environment::close(env);
}

void ClassGenerator::generateBlock(Node block)
{
    std::list<Node> statementNodes = ((NodeList *) block.nodeData)->nodes;

    for (Node stNode : statementNodes) {
        generateNode(stNode, true);
    }
}

void ClassGenerator::generateNode(Node st, bool dropResult = false)
{
    switch (st.tag) {
    /*
        case NodeIf:
            break;
        case NodeWhile:
            break;
     */
        case NodeAssign:
            generateAssign(st, dropResult);
            dropResult = false;
            break;
        case NodeAdd:
            generateAdd(st);
            break;
        case NodeMul:
            generateMul(st);
            break;
        case NodeVarDecl:
            generateVarDecl(st);
            dropResult = false;
            break;
        case NodeId:
            generateId(st);
            break;
        case NodeStringLiteral:
            generateStringLiteral(st);
            break;
        case NodeIntegerLiteral:
            generateIntegerLiteral(st);
            break;
        default:
            break;
    }

    if (dropResult)
        mb->instruction(opcodes::POP);
}

void ClassGenerator::generateVarDecl(Node varDecl)
{
    BinaryNode *typeAndAssign = (BinaryNode *) varDecl.nodeData;
    Node type = typeAndAssign->left;

    std::list<Node> assignList = ((NodeList *) typeAndAssign->right.nodeData)->nodes;

    for (Node assign : assignList) {
        if (assign.tag == NodeAssign) {
            BinaryNode *varAndExpr = (BinaryNode *) assign.nodeData;
            AccessElement *var = (AccessElement *) varAndExpr->left.nodeData;
            env->putLocal(var->name, type);
            generateAssign(assign, true);
        } else {
            AccessElement *var = (AccessElement *) assign.nodeData;
            env->putLocal(var->name, type);
        }
    }
}

void ClassGenerator::generateAssign(Node assign, bool dropResult)
{
    BinaryNode *varAndExpr = (BinaryNode *) assign.nodeData;
    AccessElement *var = (AccessElement *) varAndExpr->left.nodeData;

    generateNode(varAndExpr->right);

    Type *type = (Type *) env->getTypeLocal(var->name).nodeData;
    JavaTypeKind kind = ((TypeBase *) type->typeBase.nodeData)->kind;

    uint8_t store_instruction, load_instruction;

    switch (kind) {
        case TypeInteger:
            store_instruction = opcodes::ISTORE;
            load_instruction = opcodes::ILOAD;
            break;
        case TypeReference:
            store_instruction = opcodes::ASTORE;
            load_instruction = opcodes::ALOAD;
            break;
        default:
            break;
    }

    mb->local(store_instruction, env->getIndexLocal(var->name));
    env->setInitLocal(var->name);

    if (!dropResult)
        mb->local(load_instruction, env->getIndexLocal(var->name));
}

/* Now works only with integers*/
void ClassGenerator::generateAdd(Node add)
{
    BinaryNode *lr = (BinaryNode *) add.nodeData;
    generateNode(lr->left);
    generateNode(lr->right);

    mb->instruction(opcodes::IADD);
}

/* Now works only with integers*/
void ClassGenerator::generateMul(Node mul)
{
    BinaryNode *lr = (BinaryNode *) mul.nodeData;
    generateNode(lr->left);
    generateNode(lr->right);

    mb->instruction(opcodes::IMUL);
}

/* Now works only with local variables */
void ClassGenerator::generateId(Node id)
{
    AccessElement *var = (AccessElement *) id.nodeData;

    Type *type = (Type *) env->getTypeLocal(var->name).nodeData;
    JavaTypeKind kind = ((TypeBase *) type->typeBase.nodeData)->kind;

    uint8_t load_instruction;

    switch (kind) {
        case TypeInteger:
            load_instruction = opcodes::ILOAD;
            break;
        case TypeReference:
            load_instruction = opcodes::ALOAD;
            break;
        default:
            break;
    }

    mb->local(load_instruction, env->getIndexLocal(var->name));
}

void ClassGenerator::generateStringLiteral(Node lit)
{
    Literal *strLit = (Literal *) lit.nodeData;
    mb->loadString(strLit->lit);
}

void ClassGenerator::generateIntegerLiteral(Node lit)
{
    Literal *intLit = (Literal *) lit.nodeData;
    mb->loadInteger(std::stoi(intLit->lit));
}
