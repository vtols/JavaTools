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

uint16_t ClassBuilder::addInteger(int32_t integer)
{
    Const32Info *intInf = new Const32Info;

    intInf->tag = CONSTANT_Integer;
    intInf->value = integer;
    return addNewItem(intInf);
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

ClassFile* ClassBuilder::build()
{
    ClassFile *classFile = new ClassFile;

    classFile->magic = 0xCAFEBABE;

    classFile->minorVersion = 0;
    classFile->majorVersion = 52;

    classFile->interfacesCount = 0;
    classFile->fieldsCount = 0;
    classFile->attributesCount = 0;

    classFile->methodsCount = methodBuilders.size();
    for (uint16_t i = 0; i < methodBuilders.size(); i++)
        classFile->methods.push_back(methodBuilders[i]->build());
    classFile->thisClass = addClass(name);
    classFile->superClass = addClass("java/lang/Object");

    classFile->accessFlags = ACC_SUPER;

    /* Save pool when all constants created */
    classFile->constantPoolCount = constCounter;
    classFile->constantPool = constantPool;

    return classFile;
}

void Label::setPosition(uint32_t pos)
{
    labelPosition = pos;
}

void Label::addRef(uint32_t at)
{
    refPositions.push_back(at);
}

void Label::setJumps(ByteBuffer* codeBuilder)
{
    uint32_t saveWritePos = codeBuilder->written;
    for (uint16_t i = 0; i < refPositions.size(); i++) {
        // +1 offset of address where writing with jump instruction address
        int16_t offset = (int16_t) (labelPosition - (int32_t) refPositions[i]) + 1;
        codeBuilder->setPosition(refPositions[i]);
        codeBuilder->ByteWriter::write((uint16_t) offset);
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
    label->setPosition(codeBuilder->written);
    labels.push_back(label);
}

void MethodBuilder::instruction(uint8_t opCode)
{
    codeWriter->write(opCode);
}

void MethodBuilder::loadString(std::string str)
{
    loadRef(cb->addString(str));
}

void MethodBuilder::loadInteger(int32_t integer)
{
    if (integer == -1) {
        instruction(opcodes::ICONST_M1);
    } else if (0 <= integer && integer <= 5) {
        instruction(opcodes::ICONST_0 + integer);
    } else if (-128 <= integer && integer <= 127) {
        codeWriter->write(opcodes::BIPUSH);
        codeWriter->write((uint8_t) integer);
    } else if (-32768 <= integer && integer <= 32767) {
        codeWriter->write(opcodes::SIPUSH);
        codeWriter->write((uint16_t) integer);
    } else {
        loadRef(cb->addInteger(integer));
    }
}

void MethodBuilder::loadRef(uint16_t ref)
{
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
    frame->ref = codeBuilder->written;
    frames.push_back(frame);
}

void MethodBuilder::frameSame()
{
    Frame *f = new Frame;
    f->frameTag = same_frame;
    frame(f);
}

void MethodBuilder::frameAppend(std::vector<FrameType> &types)
{
    Frame *f = new Frame;
    f->frameTag = append_frame;
    /* types size must be in range 1..3 */
    f->locals = types;
    frame(f);
}

void MethodBuilder::local(uint8_t opCode, uint16_t index)
{
    if (index <= 3) {
        switch (opCode) {
            case opcodes::ILOAD:
                instruction(opcodes::ILOAD_0 + index);
                return;
            case opcodes::ISTORE:
                instruction(opcodes::ISTORE_0 + index);
                return;
            case opcodes::ALOAD:
                instruction(opcodes::ALOAD_0 + index);
                return;
            case opcodes::ASTORE:
                instruction(opcodes::ASTORE_0 + index);
                return;
            default:
                break;
        }
    }

    /* Now doesn't work with index > 255 */
    codeWriter->write(opCode);
    codeWriter->write((uint8_t) index);
}

MemberInfo *MethodBuilder::build()
{
    for (size_t i = 0; i < labels.size(); i++)
        labels[i]->setJumps(codeBuilder);

    CodeAttribute *codeAttr = new CodeAttribute;
    codeAttr->length =
            codeAttr->maxStack = maxStack;
    codeAttr->maxLocals = maxLocals;

    codeAttr->codeLength = code.size();
    codeAttr->code = new uint8_t[code.size()];
    std::copy(code.begin(), code.end(), codeAttr->code);

    codeAttr->exceptionTableLength = 0;

    codeAttr->nameIndex = cb->addUtf8("Code");
    codeAttr->length = 12 + code.size();

    if (frames.size() != 0) {
        codeAttr->attributesCount = 1;

        StackMapTableAttribute *tableAttr = buildStackMapTable();
        codeAttr->attributes.push_back(tableAttr);
        codeAttr->length += tableAttr->length + 6;
    } else {
        codeAttr->attributesCount = 0;
    }

    MemberInfo *methodInfo = new MemberInfo;

    methodInfo->accessFlags = accessFlags;
    methodInfo->nameIndex = cb->addUtf8(name);
    methodInfo->descriptorIndex = cb->addUtf8(descriptor);

    methodInfo->attributesCount = 1;
    methodInfo->attributes.push_back(codeAttr);

    return methodInfo;
}

StackMapTableAttribute *MethodBuilder::buildStackMapTable()
{
    StackMapTableAttribute *tableAttr = new StackMapTableAttribute;

    uint16_t current = 0;
    tableAttr->nameIndex = cb->addUtf8("StackMapTable");
    tableAttr->length = 2;
    tableAttr->numberOfEntries = frames.size();
    for (size_t i = 0; i < frames.size(); i++) {
        StackMapFrame *fr = new StackMapFrame;
        uint16_t delta = frames[i]->ref - current;
        switch (frames[i]->frameTag) {
            case same_frame:
                if (delta < 64) {
                    fr->frameType = delta;
                    tableAttr->length += 1;
                } else {
                    fr->frameType = 251;
                    fr->frameDelta = delta;
                    tableAttr->length += 3;
                }
                break;
            case append_frame:
                fr->numberLocals = frames[i]->locals.size();
                fr->frameType = 251 + fr->numberLocals;
                fr->frameDelta = delta;
                tableAttr->length += 3;
                for (FrameType ft : frames[i]->locals) {
                    VerificationTypeInfo ver;
                    ver.tag = ft.tag;
                    tableAttr->length += 1;
                    if (ft.tag == ITEM_Object) {
                        /* Code for object here */
                    } else if (ft.tag == ITEM_Uninitialized) {
                    }
                    fr->locals.push_back(ver);
                }
                break;
            default:
                break;
        }
        tableAttr->entries.push_back(fr);
        current = frames[i]->ref;
    }

    return tableAttr;
}

FrameType::FrameType(uint8_t tag) :
    tag(tag) {}

FrameType::FrameType(uint8_t tag, std::string name) :
    tag(tag), name(name) {}

