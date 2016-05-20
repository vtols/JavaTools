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

    if (super != nullptr)
        fieldsLength = super->fieldsLength;

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

    size_t argIndex = 1;
    std::string descriptor =
            owner->classFile->getUtf8(info->descriptorIndex);
    while (descriptor[argIndex] != ')')
        if (descriptor[argIndex] == '[' ||
                descriptor[argIndex] == 'L') {
            size_t argEnd = descriptor.find(';', argIndex) + 1;
            argDescriptors.push_back(descriptor.substr(argIndex, argEnd - argIndex));
            argIndex = argEnd;
        } else {
            char argDescriptor = descriptor[argIndex++];
            argDescriptors.push_back(std::string(1, argDescriptor));
            if (argDescriptor == 'J' || argDescriptor == 'D') {
                argDescriptors.push_back(argDescriptor + "+");
            }
        }
    returnDescriptor = descriptor.substr(++argIndex, descriptor.length() - 1);

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
    prev = top;
}

void Thread::prepareInit(Class *c)
{
    if (c->initStarted || c->initDone)
        return;
    c->initStarted = true;

    if (c->classInit != nullptr)
        initStack.push(c->classInit);
    else
        c->initDone = true;

    if (c->super != nullptr)
        prepareInit(c->super);
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
        case opcodes::ALOAD_0:
        case opcodes::ALOAD_1:
        case opcodes::ALOAD_2:
        case opcodes::ALOAD_3:
            stack[stackTop++] =
                    locals[code[pc++] - opcodes::ALOAD_0];
            break;
        case opcodes::ISTORE_0:
        case opcodes::ISTORE_1:
        case opcodes::ISTORE_2:
        case opcodes::ISTORE_3:
            locals[code[pc++] - opcodes::ISTORE_0] =
                    stack[--stackTop];
            break;
        case opcodes::ASTORE_0:
        case opcodes::ASTORE_1:
        case opcodes::ASTORE_2:
        case opcodes::ASTORE_3:
            locals[code[pc++] - opcodes::ASTORE_0] =
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
        case opcodes::IINC:
            locals[code[pc + 1]] += code[pc + 2];
            pc += 3;
            break;
        case opcodes::DUP:
            stack[stackTop] = stack[stackTop - 1];
            stackTop++;
            pc++;
            break;
        case opcodes::POP:
            stackTop--;
            pc++;
            break;
        case opcodes::IF_ICMPGE:
            if (stack[stackTop - 2] >= stack[stackTop - 1])
                pc += (int16_t) ((code[pc + 1] << 8) | code[pc + 2]);
            else
                pc += 3;
            stackTop -= 2;
            break;
        case opcodes::GOTO:
            pc += (int16_t) ((code[pc + 1] << 8) | code[pc + 2]);
            break;
        case opcodes::GETFIELD:
        case opcodes::PUTFIELD:
            prepareField();
            if (code[pc] == opcodes::GETFIELD)
                loadField();
            else
                storeField();
            pc += 3;
            break;
        case opcodes::GETSTATIC:
        case opcodes::PUTSTATIC:
            if (prepareStaticField()) {
                saveFrame();
                pushInit();
                loadFrame();
                break;
            }
            if (code[pc] == opcodes::GETSTATIC)
                loadField();
            else
                storeField();
            pc += 3;
            break;
        case opcodes::INVOKESTATIC:
        case opcodes::INVOKESPECIAL:
            /* No valuable difference between them yet */
            if (prepareMethod()) {
                saveFrame();
                pushInit();
                loadFrame();
                break;
            }
            instanceMethod = code[pc] == opcodes::INVOKESPECIAL;
            pc += 3;
            saveFrame();
            pushMethod(resolvedMethod);
            loadFrame();
            loadArgs();
            break;
        case opcodes::INVOKEVIRTUAL:
            prepareMethod();
            selectOverriding();
            instanceMethod = true;
            pc += 3;
            saveFrame();
            pushMethod(resolvedMethod);
            loadFrame();
            loadArgs();
            break;
        case opcodes::NEW:
            if (prepareClass(false)) {
                saveFrame();
                pushInit();
                loadFrame();
                break;
            }
            tmpObject = memberClass->newObject();
            stack[stackTop++] = referenceObject(tmpObject);
            pc += 3;
            break;
        case opcodes::RETURN:
            if (top->owner->isInit)
                frameClass->initDone = true;
            popFrame();
            if (!initStack.empty())
                pushInit();
            if (frameStack.empty())
                return;
            loadFrame();
            break;
        default:
            std::cout << "Unimplemented instruction" << std::endl;
            return;
        }
        debugFrame();
    }
}

bool Thread::prepareClass(bool ofMember=true)
{
    uint16_t refIndex = (code[pc + 1] << 8) | code[pc + 2];
    if (ofMember) {
        ref = static_cast<RefInfo *>(frameClass->classFile->constantPool[refIndex - 1]);
        refIndex = ref->firstIndex;
    }
    std::string className = frameClass->classFile->getIndexName(refIndex);

    memberClass = ClassCache::getClass(className);

    if (!memberClass->initStarted && !memberClass->initDone) {
        prepareInit(memberClass);
        return !initStack.empty();
    }
    /* In other case we must wait for initialization
     * only if initializing tread differs from current
     */
    return false;
}

void Thread::prepareMember()
{
    uint16_t nameTypeIndex = ref->secondIndex;
    RefInfo *nameType = static_cast<RefInfo *>(frameClass->classFile->constantPool[nameTypeIndex - 1]);
    memberName = frameClass->classFile->getUtf8(nameType->firstIndex);
    descriptor = frameClass->classFile->getUtf8(nameType->secondIndex);
}

bool Thread::prepareStaticField()
{
    if (prepareClass())
        return true;

    prepareMember();

    isRef = isWide = false;

    while (memberClass != nullptr) {
        auto findIterator = memberClass->fieldOffset.find(memberName);
        if (findIterator == memberClass->fieldOffset.end()) {
            memberClass = memberClass->super;
        } else {
            offset = memberClass->fieldOffset[memberName];
            fieldPtr = &memberClass->staticFields[offset];
            break;
        }
    }

    return false;
}

void Thread::prepareField()
{
    prepareClass();
    prepareMember();

    while (memberClass != nullptr) {
        auto findIterator = memberClass->fieldOffset.find(memberName);
        if (findIterator == memberClass->fieldOffset.end()) {
            memberClass = memberClass->super;
        } else {
            offset = memberClass->fieldOffset[memberName];
            fieldPtr = &tmpObject->fields[offset];
            break;
        }
    }
}

bool Thread::prepareMethod()
{
    if (prepareClass())
        return true;

    prepareMember();

    while (memberClass != nullptr) {
        resolvedMethod = memberClass->getMethod(memberName, descriptor);
        if (resolvedMethod != nullptr)
            return false;
        memberClass = memberClass->super;
    }

    return false;
}

void Thread::selectOverriding()
{
    Class *objClass = tmpObject->cls;
    Method *overriding = nullptr;

    while (objClass != memberClass) {
        overriding = objClass->getMethod(memberName, descriptor);
        if (overriding != nullptr) {
            resolvedMethod = overriding;
            return;
        }
        objClass = objClass->super;
    }
}

void Thread::loadField()
{
    switch (descriptor[0]) {
        case 'B':
        case 'Z':
            stack[stackTop++] = *fieldPtr;
            break;
        case 'C':
        case 'S':
            stack[stackTop++] = *(int16_t *) fieldPtr;
            break;
        case 'F':
        case 'I':
            stack[stackTop++] = *(int32_t *) fieldPtr;
            break;
        case 'D':
        case 'J':
             *(int64_t *) &stack[stackTop] = *(int64_t *) fieldPtr;
            stackTop += 2;
            break;
        case 'L':
        case '[':
            stack[stackTop++] = referenceObject(*(Object **) fieldPtr);
            break;
        default:
            break;
    }
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
             *(Object **) fieldPtr = top->objectIndex[stack[--stackTop]];
            break;
        default:
            break;
    }
}

void Thread::loadArgs()
{
    /* Now without special case for references */
    uint16_t argsLength = top->owner->argDescriptors.size();
    uint16_t argsStart = prev->stackTop - argsLength;
    int32_t local = 0, arg = 0;
    if (instanceMethod)
        arg--, prev->stackTop--;
    prev->stackTop -= argsLength;
    std::string argDescriptor;
    for (; arg < argsLength; arg++, local++) {
        if (arg >= 0)
            argDescriptor = top->owner->argDescriptors[arg];
        if (arg < 0 || argDescriptor[0] == 'L' || argDescriptor[0] == '[') {
            tmpObject = prev->objectIndex[prev->stack[argsStart + arg]];
            top->locals[local] = referenceObject(tmpObject);
        }
        else {
            top->locals[local] = prev->stack[argsStart + arg];
        }
    }
}

uint32_t Thread::referenceObject(Object *obj)
{
    /* Object referencing should be more clever */
    uint32_t objCount = top->objectIndex.size();
    top->objectIndex.push_back(obj);
    return objCount;
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
