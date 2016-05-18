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
    classFile(classFile)
{
    uint16_t superIndex = classFile->superClass;
    if (superIndex != 0) {
        std::string superPath = classFile->getIndexName(superIndex);
        super = ClassCache::getClass(superPath);
    } else {
        super = nullptr;
    }

    uint16_t totalFieldsCount = classFile->fields.size();
    uint16_t staticFieldsCount = 0, fieldsCount = 0;
    for (size_t i = 0; i < totalFieldsCount; i++) {
        MemberInfo *fieldInfo = classFile->fields[i];
        std::string fieldName = classFile->getUtf8(fieldInfo->nameIndex),
                fieldDescriptor = classFile->getUtf8(fieldInfo->descriptorIndex);
        if (fieldInfo->accessFlags & ACC_STATIC) {
            fieldOffset[fieldName] = staticFieldsLength;
            staticFieldsLength += fieldSize(fieldDescriptor);
            staticFieldsCount++;
        } else {
            fieldOffset[fieldName] = fieldsLength;
            fieldsLength += fieldSize(fieldDescriptor);
            fieldsCount++;
        }
    }

    /* Zero-initialization of fields */
    staticFields = new uint8_t[staticFieldsLength]();

    classInit = nullptr;

    for (MemberInfo* methodMember : classFile->methods) {
        uint16_t nameIndex = methodMember->nameIndex;
        uint16_t descriptorIndex = methodMember->descriptorIndex;

        std::string methodName = classFile->getUtf8(nameIndex);
        std::string methodDescriptor =
                classFile->getUtf8(descriptorIndex);
        std::string combined = methodName + ':' + methodDescriptor;
        Method *method = new Method(this, methodMember);

        methods[combined] = method;

        if (classInit == nullptr && combined == "<clinit>:()V") {
            method->isInit = true;
            classInit = method;
        }
    }
}

void Class::init()
{
    if (classInit != nullptr) {
        Thread initThread;
        initThread.invoke(classInit);
    }
}

Object *Class::newObject()
{
    return new Object(this);
}

Object::Object(Class *cls) :
    cls(cls)
{
    fields = new uint8_t[cls->fieldsLength]();
}

uint8_t Class::fieldSize(std::string descriptor)
{
    switch (descriptor[0]) {
        case 'B':
            return BYTE_SIZE;
        case 'C':
            return CHAR_SIZE;
        case 'D':
            return DOUBLE_SIZE;
        case 'F':
            return FLOAT_SIZE;
        case 'I':
            return INTEGER_SIZE;
        case 'J':
            return LONG_SIZE;
        case 'L':
            return OBJECT_SIZE;
        case 'S':
            return SHORT_SIZE;
        case 'Z':
            return BOOLEAN_SIZE;
        case '[':
            return ARRAY_SIZE;
        default:
            return 0;
    }
}

Method *Class::getMethod(std::string name, std::string descriptor)
{
    std::string combined = name + ':' + descriptor;

    auto findIterator = methods.find(combined);
    if (findIterator != methods.end())
        return (*findIterator).second;

    return nullptr;
}

std::map<std::string, Class*> ClassCache::classMap;

Class *ClassCache::getClass(std::string path)
{
    auto findIterator = classMap.find(path);
    if (findIterator != classMap.end())
        return (*findIterator).second;

    Class *loadedClass = ClassLoader::loadClass(path);
    classMap[path] = loadedClass;

    return loadedClass;
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

Frame::Frame(Method *m) :
    owner(m), pc(0), stackTop(0)
{
    pc = stackTop = 0;
    maxStack = m->codeAttr->maxStack;
    maxLocals = m->codeAttr->maxLocals;
    stack = new uint32_t[m->codeAttr->maxStack]();
    locals = new uint32_t[m->codeAttr->maxLocals]();
    code = m->code;

    /* Used to mark references and wide values (long, double) on stack
     *
    f->wide_operand = new uint64_t[(m->codeAttr->maxStack + 63) / 64]();
    f->ref_operand = new uint64_t[(m->codeAttr->maxStack + 63) / 64]();
    */
}

Frame::~Frame()
{
    delete stack;
    delete locals;
}

void Thread::invoke(Method *m)
{
    pushMethod(m);
    if (!initStack.empty())
        pushInit();
    runLoop();
}

void Thread::pushMethod(Method *m)
{
    Frame *startFrame = newFrame(m);
    pushFrame(startFrame);
}

Frame *Thread::newFrame(Method *m)
{
    return new Frame(m);
}

void Thread::popFrame()
{
    Frame *f = frameStack.top();
    delete f;
    frameStack.pop();
}

void Thread::pushFrame(Frame *f)
{
    frameStack.push(f);
}

void Thread::loadFrame()
{
    top = frameStack.top();
    pc = top->pc;
    code = top->code;
    locals = top->locals;
    stack = top->stack;
    stackTop = top->stackTop;

    frameClass = top->owner->owner;
}

void Thread::saveFrame()
{
    top->pc = pc;
    top->stackTop = stackTop;
}

void Thread::prepareInit(Class *c)
{
    c->initStarted = true;
    if (c->super != nullptr)
        prepareInit(c->super);

    if (c->classInit != nullptr)
        initStack.push(c->classInit);
}

void Thread::pushInit()
{
    Method *initMethod = initStack.top();
    initStack.pop();

    pushMethod(initMethod);
}

void Thread::runLoop()
{
    loadFrame();
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
        case opcodes::PUTSTATIC:
            if (prepareField()) {
                saveFrame();
                pushInit();
                loadFrame();
                break;
            }
            storeField();
            pc += 3;
            break;
        case opcodes::RETURN:
            if (top->owner->isInit)
                top->owner->owner->initDone = true;
            popFrame();
            if (!initStack.empty())
                pushInit();
            if (frameStack.empty())
                return;
            loadFrame();
            break;
        default:
            break;
        }
        debugFrame();
    }
}

bool Thread::prepareField()
{
    refIndex = (code[pc + 1] << 8) | code[pc + 2];
    ref = static_cast<RefInfo *>(frameClass->classFile->constantPool[refIndex - 1]);
    className = frameClass->classFile->getIndexName(ref->firstIndex);

    fieldClass = ClassCache::getClass(className);

    if (!fieldClass->initStarted) {
        prepareInit(fieldClass);
        return true;
    }
    /* In other case we must wait for initialization
     * only if initializing tread differs from current
     */

    nameTypeIndex = ref->secondIndex;
    nameType = static_cast<RefInfo *>(frameClass->classFile->constantPool[nameTypeIndex - 1]);
    memberName = frameClass->classFile->getUtf8(nameType->firstIndex);
    descriptor = frameClass->classFile->getUtf8(nameType->secondIndex);

    isRef = isWide = false;

    offset = fieldClass->fieldOffset[memberName];
    fieldPtr = &fieldClass->staticFields[offset];

    return false;
}

void Thread::storeField()
{
    switch (descriptor[0]) {
        case 'B':
        case 'Z':
            *fieldPtr = (int8_t) stack[--stackTop];
            break;
        case 'C':
        case 'S':
            *(int16_t *) fieldPtr = (int16_t) stack[--stackTop];
            break;
        case 'F':
        case 'I':
            *(int32_t *) fieldPtr = (int32_t) stack[--stackTop];
            break;
        case 'D':
        case 'J':
            *(int64_t *) fieldPtr = *(int64_t *) &stack[stackTop - 2];
            stackTop -= 2;
            break;
        case 'L':
        case '[':
            /* Fetch pointer with 32-bit index on stack
             * from sepcial frame-specific array
             */
            break;
        deafult:
            break;
    }
}

void Thread::debugFrame()
{
    if (frameStack.empty()) {
        std::cout << "No frames" << std::endl;
        return;
    }

    std::cout << "Locals:\t\n[";
    for (int i = 0; i < frameStack.top()->maxLocals; i++) {
        if (i > 0)
            std::cout << ", ";
        std::cout << frameStack.top()->locals[i];
    }
    std::cout << "]\n";
}
