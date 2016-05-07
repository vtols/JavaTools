#include <gen/java_gen.h>

ClassGenerator::ClassGenerator(SourceFile *source)
{
    src = source;
    classContext = (ClassDeclaration *) src.jclass->nodeData;
    className = classDecl->name;
    cb = new ClassBuilder(className);
    generate();
    delete cb;
}

ClassFile *ClassGenerator::generate()
{
    for (Node methodNode : classContext->methods) {
        methodContext = (MethodDeclaration *) methodNode->nodeData;
        generateMethod();
        env = Environment::open(env);
    }
}

void ClassGenerator::generateMethod()
{
    methodName = methodContext->name;
    mb = cb.createMethod(methodName);
}


void ClassGenerator::generateBlock()
{
    
}

void ClassGenerator::generateStatement()
{
    
}

void ClassGenerator::generateIf()
{
}
