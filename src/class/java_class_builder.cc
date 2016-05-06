#include <class/java_class_builder.h>

ClassBuilder::ClassBuilder(std::string name)
{
    this->name = name;
}

MethodBuilder *ClassBuilder::createMethod(std::string name)
{
    MethodBuilder *mb = new MethodBuilder(this);
    mb->setName(name);
    methodBuilders.push_back(mb);
    return mb;
}

uint16_t ClassBuilder::addNewItem(ConstantPoolInfo *ci)
{
    constantPool.push_back(ci);
    return constCounter++;
}

uint16_t ClassBuilder::addUtf8(std::string str)
{
    if (utf8Index.find(str) != utf8Index.end())
        return utf8Index[str];

    Utf8Info *utf8 = new Utf8Info;

    // TODO Set tag in constructor
    utf8->tag = CONSTANT_Utf8;
    utf8->str = str;
    uint16_t ref = addNewItem(utf8);
    utf8Index[str] = ref;
    return ref;
}

uint16_t ClassBuilder::addNameType(std::string name, std::string type)
{
    RefInfo *nameType = new RefInfo;

    uint16_t nameIndex = addUtf8(name);
    uint16_t typeIndex = addUtf8(type);

    nameType->tag = CONSTANT_NameAndType;
    nameType->firstIndex = nameIndex;
    nameType->secondIndex = typeIndex;

    return addNewItem(nameType);
}

uint16_t ClassBuilder::addClass(std::string name)
{
    IndexInfo *classInf = new IndexInfo;

    classInf->tag = CONSTANT_Class;
    classInf->index = addUtf8(name);
    return addNewItem(classInf);
}

uint16_t ClassBuilder::addString(std::string str)
{
    IndexInfo *strInf = new IndexInfo;

    strInf->tag = CONSTANT_String;
    strInf->index = addUtf8(str);
    return addNewItem(strInf);
}

// TODO Refactor code duplication

uint16_t ClassBuilder::addFieldRef(
        std::string className,
        std::string fieldName,
        std::string typeName)
{
    RefInfo *fieldRef = new RefInfo;

    uint16_t classIndex = addClass(className);
    uint16_t nameTypeIndex = addNameType(fieldName, typeName);

    fieldRef->tag = CONSTANT_Fieldref;
    fieldRef->firstIndex = classIndex;
    fieldRef->secondIndex = nameTypeIndex;
    return addNewItem(fieldRef);
}

uint16_t ClassBuilder::addMethodRef(
        std::string className,
        std::string methodName,
        std::string typeName)
{
    RefInfo *methodRef = new RefInfo;

    uint16_t classIndex = addClass(className);
    uint16_t nameTypeIndex = addNameType(methodName, typeName);

    methodRef->tag = CONSTANT_Methodref;
    methodRef->firstIndex = classIndex;
    methodRef->secondIndex = nameTypeIndex;
    return addNewItem(methodRef);
}

void ClassBuilder::build(ByteStreamWriter* w)
{
    classFile.magic = 0xCAFEBABE;

    classFile.minorVersion = 0;
    classFile.majorVersion = 52;

    classFile.interfacesCount = 0;
    classFile.fieldsCount = 0;
    classFile.attributesCount = 0;

    classFile.methodsCount = methodBuilders.size();
    for (uint16_t i = 0; i < methodBuilders.size(); i++)
        classFile.methods.push_back(methodBuilders[i]->build());
    classFile.thisClass = addClass(name);
    classFile.superClass = addClass("java/lang/Object");

    classFile.accessFlags = ACC_SUPER;

    /* Save pool when all constants created */
    classFile.constantPoolCount = constCounter;
    classFile.constantPool = constantPool;

    classFile.write(w);
}

void Label::addRef(uint32_t at)
{
    refPositions.push_back(at);
}

void Label::setJumps(uint32_t addr, ByteBuffer* codeBuilder)
{
    uint32_t saveWritePos = codeBuilder->written;
    for (uint16_t i = 0; i < refPositions.size(); i++) {
        // +1 offset of address where writing with jump instruction address
        int16_t offset = (int16_t) (addr - (int32_t) refPositions[i]) + 1;
        codeBuilder->setPosition(refPositions[i]);
        codeBuilder->ByteStreamWriter::write((uint16_t) offset);
    }
    codeBuilder->written = saveWritePos;
    codeBuilder->setPosition(saveWritePos);
}

MethodBuilder::MethodBuilder(ClassBuilder *cb)
{
    this->cb = cb;
    accessFlags = 0;
    attributesCount = 0;
    setDescriptor("()V");
    codeBuilder = new ByteBuffer(&code);
    codeWriter = codeBuilder;
}

void MethodBuilder::setAccessFlags(uint16_t flags)
{
    accessFlags = flags;
}

void MethodBuilder::setName(std::string name)
{
    this->name = name;
}

void MethodBuilder::setDescriptor(std::string descriptor)
{
    this->descriptor = descriptor;
}

void MethodBuilder::jump(uint8_t opCode, Label* label)
{
    codeWriter->write(opCode);
    label->addRef(codeBuilder->written);
    codeWriter->write((uint16_t) 0);
}

void MethodBuilder::insertLabel(Label* label)
{
    label->setJumps(codeBuilder->written, codeBuilder);
}

void MethodBuilder::instruction(uint8_t opCode)
{
    codeWriter->write(opCode);
}

void MethodBuilder::loadString(std::string str)
{
    uint16_t ref = cb->addString(str);
    if (ref < 256) {
        codeWriter->write(opcodes::LDC);
        codeWriter->write((uint8_t) ref);
    } else {
        codeWriter->write(opcodes::LDC_W);
        codeWriter->write(ref);
    }
}

void MethodBuilder::invoke(uint8_t opCode, std::string className,
        std::string method, std::string descriptor)
{
    uint16_t ref = cb->addMethodRef(className, method, descriptor);
    codeWriter->write(opCode);
    codeWriter->write(ref);
}

void MethodBuilder::field(uint8_t opCode, std::string className,
        std::string field, std::string descriptor)
{
    uint16_t ref = cb->addFieldRef(className, field, descriptor);
    codeWriter->write(opCode);
    codeWriter->write(ref);
}

void MethodBuilder::setMax(uint16_t maxStack, uint16_t maxLocals)
{
    this->maxStack = maxStack;
    this->maxLocals = maxLocals;
}

void MethodBuilder::frame(Frame *frame)
{
    frame.ref = codeBuilder->written;
    frames.push_back(frame);
}

MemberInfo *MethodBuilder::build()
{
    CodeAttribute *codeAttr = new CodeAttribute;
    codeAttr->length =
            codeAttr->maxStack = maxStack;
    codeAttr->maxLocals = maxLocals;

    codeAttr->codeLength = code.size();
    codeAttr->code = new uint8_t[code.size()];
    std::copy(code.begin(), code.end(), codeAttr->code);

    codeAttr->exceptionTableLength = 0;

    codeAttr->attributesCount = 0;
    /*
    SameFrame *s = new SameFrame;
    s->frameType = 0;
    cod*/

    codeAttr->nameIndex = cb->addUtf8("Code");
    codeAttr->length = 12 + code.size();

    MemberInfo *methodInfo = new MemberInfo;

    methodInfo->accessFlags = accessFlags;
    methodInfo->nameIndex = cb->addUtf8(name);
    methodInfo->descriptorIndex = cb->addUtf8(descriptor);

    methodInfo->attributesCount = 1;
    methodInfo->attributes.push_back(codeAttr);

    return methodInfo;
}
