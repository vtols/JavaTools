#include <jvm/jvm.h>
#include <class/java_opcodes.h>
#include <io/file_byte_reader.h>
#include <iostream>

Class *ClassLoader::loadClass(std::string path)
{
    FileByteReader fr(path + ".class");

    return loadClass(&fr);
}

Class *ClassLoader::loadClass(ClassFile *cf)
{
    return new Class(cf);
}

Class *ClassLoader::loadClass(ByteReader *br)
{
    ClassFile *cf = new ClassFile;
    *cf = ClassFile::read(br);
    return ClassLoader::loadClass(cf);
}

Class::Class(ClassFile *classFile) :
    classFile(classFile) {}

Method *Class::getMethod(std::string name, std::string descriptor)
{
    for (MemberInfo* methodMember : classFile->methods) {
        uint16_t nameIndex = methodMember->nameIndex;
        uint16_t descriptorIndex = methodMember->descriptorIndex;

        std::string methodName = classFile->getUtf8(nameIndex);
        std::string methodDescriptor =
                classFile->getUtf8(descriptorIndex);


        if (name == methodName &&
                descriptor == methodDescriptor) {
            return new Method(this, methodMember);
        }
    }
    return nullptr;
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

void Thread::invoke(Method *m)
{
    Interpreter runInterpeter;
    Frame *startFrame = runInterpeter.frameStack.newFrame(m);
    runInterpeter.frameStack.pushFrame(startFrame);
    runInterpeter.run();
}

void Stack::pushMethod(Method *m)
{
    Frame *startFrame = newFrame(m);
    pushFrame(startFrame);
}

Frame *Stack::newFrame(Method *m)
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

void Stack::popFrame()
{
    Frame *f = top->prev;
    delete top->stack;
    delete top->locals;
    delete top;
    top = f;
}

void Stack::pushFrame(Frame *f)
{
    f->prev = top;
    top = f;
}

void Interpreter::run()
{
    Frame *top = frameStack.top;
    uint32_t pc = top->pc;
    uint8_t *code = top->code;
    uint32_t *locals = top->locals;
    uint32_t *stack = top->stack;
    uint16_t stackTop = top->stackTop;
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
            break;
        case opcodes::IMUL:
            stack[stackTop - 2] =
                    stack[stackTop - 2] * stack[stackTop - 1];
            stackTop--;
            pc++;
            break;
        case opcodes::RETURN:
            frameStack.popFrame();
            return;
        default:
            break;
        }
        frameStack.top->debug();
    }
}

void Frame::debug()
{
    std::cout << "Locals:\t\n[";
    for (int i = 0; i < maxLocals; i++) {
        if (i > 0)
            std::cout << ", ";
        std::cout << locals[i];
    }
    std::cout << "]\n";
}
