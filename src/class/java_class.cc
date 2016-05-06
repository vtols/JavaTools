#include <iostream>

#include <io/byte_stream_writer.h>
#include <class/java_class.h>

ClassFile ClassFile::read(ByteStream *bs)
{
    ClassFile cf;

    cf.magic = bs->read32();

    cf.minorVersion = bs->read16();
    cf.majorVersion = bs->read16();

    cf.constantPoolCount = bs->read16();
    ConstantPoolInfo *ci;
    for (int i = 1; i < cf.constantPoolCount; i++) {
        std::cout << "Reading const #" << i << std::endl;
        ci = ConstantPoolInfo::read(bs);
        cf.constantPool.push_back(ci);
    }

    cf.accessFlags = bs->read16();
    cf.thisClass = bs->read16();
    cf.superClass = bs->read16();

    cf.interfacesCount = bs->read16();
    for (int i = 0; i < cf.interfacesCount; i++)
        cf.interfaces.push_back(bs->read16());

    cf.fieldsCount = bs->read16();
    for (int i = 0; i < cf.fieldsCount; i++)
        cf.fields.push_back(MemberInfo::read(bs, &cf));

    cf.methodsCount = bs->read16();
    for (int i = 0; i < cf.methodsCount; i++)
        cf.methods.push_back(MemberInfo::read(bs, &cf));

    cf.attributesCount = bs->read16();
    for (int i = 0; i < cf.attributesCount; i++)
        cf.attributes.push_back(AttributeInfo::read(bs, &cf));

    return cf;
}

void ClassFile::write(ByteStreamWriter* bs)
{
    bs->write(magic);

    bs->write(minorVersion);
    bs->write(majorVersion);

    bs->write(constantPoolCount);
    for (int i = 1; i < constantPoolCount; i++)
        constantPool[i - 1]->write(bs);

    bs->write(accessFlags);
    bs->write(thisClass);
    bs->write(superClass);

    bs->write(interfacesCount);
    for (int i = 0; i < interfacesCount; i++)
        bs->write(interfaces[i]);

    bs->write(fieldsCount);
    for (int i = 0; i < fieldsCount; i++)
        fields[i]->write(bs);

    bs->write(methodsCount);
    for (int i = 0; i < methodsCount; i++)
        methods[i]->write(bs);

    bs->write(attributesCount);
    for (int i = 0; i < attributesCount; i++)
        attributes[i]->write(bs);
}

std::string ClassFile::getUtf8(uint16_t index)
{
    ConstantPoolInfo *ci = constantPool[index - 1];
    if (ci->tag == CONSTANT_Utf8) {
        Utf8Info *utf8 = (Utf8Info *) ci;
        return utf8->str;
    }
    return "";
}

ConstantPoolInfo *ConstantPoolInfo::read(ByteStream *bs)
{
    ConstantPoolInfo *ci = nullptr;
    uint8_t tag = bs->read8();

    switch (tag) {
        case CONSTANT_Methodref:
        case CONSTANT_Fieldref:
        case CONSTANT_InterfaceMethodref:
        case CONSTANT_NameAndType:
            ci = RefInfo::read(bs);
            break;
        case CONSTANT_String:
        case CONSTANT_Class:
            ci = IndexInfo::read(bs);
            break;
        case CONSTANT_Utf8:
            ci = Utf8Info::read(bs);
            break;
        default:
            break;
    }

    ci->tag = tag;
    return ci;
}

void ConstantPoolInfo::write(ByteStreamWriter *bs)
{
    bs->write(tag);
}

RefInfo *RefInfo::read(ByteStream *bs)
{
    RefInfo *mi = new RefInfo;

    mi->firstIndex = bs->read16();
    mi->secondIndex = bs->read16();
    return mi;
}

void RefInfo::write(ByteStreamWriter *bs)
{
    ConstantPoolInfo::write(bs);
    bs->write(firstIndex);
    bs->write(secondIndex);
}

IndexInfo *IndexInfo::read(ByteStream *bs)
{
    IndexInfo *ri = new IndexInfo;

    ri->index = bs->read16();
    return ri;
}

void IndexInfo::write(ByteStreamWriter *bs)
{
    ConstantPoolInfo::write(bs);
    bs->write(index);
}

Utf8Info *Utf8Info::read(ByteStream *bs)
{
    Utf8Info *utf8 = new Utf8Info;

    uint16_t len = bs->read16();
    uint8_t *str = new uint8_t[len + 1];
    bs->read(str, len);
    str[len] = 0;
    utf8->str = std::string((char *) str);
    return utf8;
}

void Utf8Info::write(ByteStreamWriter *bs)
{
    ConstantPoolInfo::write(bs);
    bs->write((uint16_t) str.length());
    bs->write((uint8_t *) str.c_str(), str.length());
}

MemberInfo *MemberInfo::read(ByteStream *bs, ClassFile *cf)
{
    MemberInfo *mi = new MemberInfo;

    mi->accessFlags = bs->read16();
    mi->nameIndex = bs->read16();
    mi->descriptorIndex = bs->read16();

    mi->attributesCount = bs->read16();
    for (int i = 0; i < mi->attributesCount; i++)
        mi->attributes.push_back(AttributeInfo::read(bs, cf));
    return mi;
}

void MemberInfo::write(ByteStreamWriter *bs)
{
    bs->write(accessFlags);
    bs->write(nameIndex);
    bs->write(descriptorIndex);

    bs->write(attributesCount);
    for (int i = 0; i < attributesCount; i++)
        attributes[i]->write(bs);
}

AttributeInfo *AttributeInfo::read(ByteStream *bs, ClassFile *cf)
{
    AttributeInfo *attr = new AttributeInfo;

    uint16_t nameIndex = bs->read16();
    uint32_t length = bs->read32();

    std::string attrName = cf->getUtf8(nameIndex);
    if (attrName == "Code")
        attr = CodeAttribute::read(bs, cf);
    else if (attrName == "LineNumberTable")
        attr = LineNumberTableAttribute::read(bs);
    else if (attrName == "StackMapTable")
        attr = StackMapTableAttribute::read(bs);
    else if (attrName == "SourceFile")
        attr = SourceFileAttribute::read(bs);

    attr->nameIndex = nameIndex;
    attr->length = length;

    return attr;
}

void AttributeInfo::write(ByteStreamWriter *bs)
{
    bs->write(nameIndex);
    bs->write(length);
}

CodeAttribute *CodeAttribute::read(ByteStream *bs, ClassFile *cf)
{
    CodeAttribute *attr = new CodeAttribute;

    attr->maxStack = bs->read16();
    attr->maxLocals = bs->read16();

    attr->codeLength = bs->read32();

    attr->code = new uint8_t[attr->codeLength];
    bs->read(attr->code, attr->codeLength);

    attr->exceptionTableLength = bs->read16();
    for (int i = 0; i < attr->exceptionTableLength; i++)
        attr->exceptionTable.push_back(Exception::read(bs));

    attr->attributesCount = bs->read16();
    for (int i = 0; i < attr->attributesCount; i++)
        attr->attributes.push_back(AttributeInfo::read(bs, cf));

    return attr;
}

void CodeAttribute::write(ByteStreamWriter *bs)
{
    AttributeInfo::write(bs);

    bs->write(maxStack);
    bs->write(maxLocals);

    bs->write(codeLength);
    bs->write(code, codeLength);

    bs->write(exceptionTableLength);
    for (int i = 0; i < exceptionTableLength; i++)
        exceptionTable[i].write(bs);

    bs->write(attributesCount);
    for (int i = 0; i < attributesCount; i++)
        attributes[i]->write(bs);
}

Exception Exception::read(ByteStream *bs)
{
    Exception ex;

    ex.startPc = bs->read16();
    ex.endPc = bs->read16();
    ex.handlerPc = bs->read16();
    ex.catchType = bs->read16();
    return ex;
}

void Exception::write(ByteStreamWriter *bs)
{
    bs->write(startPc);
    bs->write(endPc);
    bs->write(handlerPc);
    bs->write(catchType);
}

LineNumberTableAttribute *LineNumberTableAttribute::read(ByteStream *bs)
{
    LineNumberTableAttribute *attr = new LineNumberTableAttribute;

    attr->lineNumberTableLength = bs->read16();
    for (int i = 0; i < attr->lineNumberTableLength; i++)
        attr->lineNumberTable.push_back(LineNumber::read(bs));

    return attr;
}

void LineNumberTableAttribute::write(ByteStreamWriter *bs)
{
    AttributeInfo::write(bs);

    bs->write(lineNumberTableLength);
    for (int i = 0; i < lineNumberTableLength; i++)
        lineNumberTable[i].write(bs);
}

LineNumber LineNumber::read(ByteStream *bs)
{
    LineNumber ln;

    ln.startPc = bs->read16();
    ln.lineNumber = bs->read16();
    return ln;
}

void LineNumber::write(ByteStreamWriter *bs)
{
    bs->write(startPc);
    bs->write(lineNumber);
}

StackMapTableAttribute *StackMapTableAttribute::read(ByteStream *bs)
{
    StackMapTableAttribute *attr = new StackMapTableAttribute;

    attr->numberOfEntries = bs->read16();
    for (int i = 0; i < attr->numberOfEntries; i++)
        attr->entries.push_back(StackMapFrame::read(bs));
    return attr;
};

void StackMapTableAttribute::write(ByteStreamWriter *bs)
{
    AttributeInfo::write(bs);

    bs->write(numberOfEntries);
    for (int i = 0; i < numberOfEntries; i++)
        entries[i]->write(bs);
}

StackMapFrame *StackMapFrame::read(ByteStream *bs)
{
    StackMapFrame *frame = new StackMapFrame;

    uint8_t type = bs->read8();
    frame->frameType = type;

    if (0 <= type && type <= 63) {
        /* same_frame */
    } else if (type == 251) {
        /* same_frame_extended */
        frame->frameDelta = bs->read16();
    } else if (252 <= type && type <= 254) {
        /* append_frame */
        frame->frameDelta = bs->read16();
        frame->numberLocals = type - 251;
        for (int i = 0; i < frame->numberLocals; i++)
            frame->locals.push_back(VerificationTypeInfo::read(bs));
    }

    return frame;
}

void StackMapFrame::write(ByteStreamWriter *bs)
{
    bs->write(frameType);

    if (0 <= frameType && frameType <= 63) {
        /* same_frame */
    } else if (frameType == 251) {
        /* same_frame_extended */
        bs->write(frameDelta);
    } else if (252 <= frameType && frameType <= 254) {
        /* append_frame */
        bs->write(frameDelta);
        for (int i = 0; i < numberLocals; i++)
            locals[i].write(bs);
    }
}

SourceFileAttribute *SourceFileAttribute::read(ByteStream *bs)
{
    SourceFileAttribute *src = new SourceFileAttribute;

    src->sourceFileIndex = bs->read16();
    return src;
}

void SourceFileAttribute::write(ByteStreamWriter *bs)
{
    AttributeInfo::write(bs);

    bs->write(sourceFileIndex);
}

VerificationTypeInfo VerificationTypeInfo::read(ByteStream *bs)
{
    VerificationTypeInfo ver;

    ver.tag = bs->read8();
    if (ver.tag == ITEM_Object ||
            ver.tag == ITEM_Uninitialized)
        ver.data = bs->read16();

    return ver;
}

void VerificationTypeInfo::write(ByteStreamWriter *bs)
{
    bs->write(tag);
    if (tag == ITEM_Object ||
            tag == ITEM_Uninitialized)
        bs->write(data);
}
