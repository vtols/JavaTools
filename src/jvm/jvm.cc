#include <jvm/jvm.h>
#include <class/java_opcodes.h>
#include <iostream>

Class::Class(ClassFile *classFile) :
    classFile(classFile) {}

Method *Class::getMethod(std::string name)
{
    for (MemberInfo* methodMember : classFile->methods) {
        uint16_t nameIndex = methodMember->nameIndex;
        std::string methodName = classFile->getUtf8(nameIndex);
        if (name == methodName) {
            return new Method(this, methodMember);
        }
    }
    return nullptr;
}

Class *Class::loadClass(ClassFile *cf)
{
    return new Class(cf);
}

Class *Class::loadClass(ByteReader *br)
{
    ClassFile *cf = new ClassFile;
    *cf = ClassFile::read(br);
    return Class::loadClass(cf);
}

Method::Method(Class *owner, MemberInfo *info) :
    owner(owner), methodInfo(info)
{
    for (AttributeInfo *attr : info->attributes) {
        uint16_t nameIndex = attr->nameIndex;

        /* Indirect access to constant pool is not good */
        std::string attrName =
                owner->classFile->getUtf8(nameIndex);

        if (attrName == "Code") {
            codeAttr = static_cast<CodeAttribute *>(attr);
            break;
        }
    }

    codeLength = codeAttr->codeLength;
    code = codeAttr->code;
}

void Thread::runThread(Method *m)
{
    Interpreter runInterpeter;
    Frame *startFrame = runInterpeter.newFrame(m);
    runInterpeter.pushFrame(startFrame);
    runInterpeter.run();
}

Frame *Interpreter::newFrame(Method *m)
{
    Frame *f = new Frame;
    f->owner = m;
    f->pc = 0;
    f->stackTop = 0;
    f->maxLocals = m->codeAttr->maxLocals;
    f->stack = new uint32_t[m->codeAttr->maxStack];
    f->locals = new uint32_t[m->codeAttr->maxLocals];
    f->code = m->code;

    return f;
}

void Interpreter::popFrame()
{
    Frame *f = current->prev;
    delete current->stack;
    delete current->locals;
    delete current;
    current = f;
}

void Interpreter::pushFrame(Frame *f)
{
    current = f;
}

void Interpreter::run()
{
    uint32_t pc = current->pc;
    uint8_t *code = current->code;
    uint32_t *locals = current->locals;
    uint32_t *stack = current->stack;
    uint16_t stackTop = current->stackTop;
    while (true) {
        switch (code[pc]) {
        case opcodes::BIPUSH:
            stack[stackTop++] = code[pc + 1];
            pc += 2;
            break;
        case opcodes::SIPUSH:
            stack[stackTop++] =
                    (code[pc + 1] << 8) | code[pc + 2];
            pc += 3;
            break;
        case opcodes::ICONST_M1:
        case opcodes::ICONST_0:
        case opcodes::ICONST_1:
        case opcodes::ICONST_2:
        case opcodes::ICONST_3:
        case opcodes::ICONST_4:
        case opcodes::ICONST_5:
            stack[stackTop++] =
                    code[pc++] - opcodes::ICONST_0;
            break;
        case opcodes::ILOAD_0:
        case opcodes::ILOAD_1:
        case opcodes::ILOAD_2:
        case opcodes::ILOAD_3:
            stack[stackTop++] =
                    locals[code[pc++] - opcodes::ILOAD_0];
            break;
        case opcodes::ISTORE_0:
        case opcodes::ISTORE_1:
        case opcodes::ISTORE_2:
        case opcodes::ISTORE_3:
            locals[code[pc++] - opcodes::ISTORE_0] =
                    stack[--stackTop];
            break;
        case opcodes::IADD:
            stack[stackTop - 2] =
                    stack[stackTop - 2] + stack[stackTop - 1];
            stackTop--;
            pc++;
        case opcodes::IMUL:
            stack[stackTop - 2] =
                    stack[stackTop - 2] * stack[stackTop - 1];
            stackTop--;
            pc++;
        case opcodes::RETURN:
            debugFrame();
            popFrame();
            return;
        default:
            break;
        }
        current->pc = pc;
    }
}

void Interpreter::debugFrame()
{
    std::cout << "Locals:\t\n[";
    for (int i = 0; i < current->maxLocals; i++) {
        if (i > 0)
            std::cout << ", ";
        std::cout << current->locals[i];
    }
    std::cout << "]\n";
}


