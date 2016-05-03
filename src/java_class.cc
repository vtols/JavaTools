#include <java_class.h>

ClassFile ClassFile::read(ByteStream *bs)
{
    ClassFile cf;

    cf.magic = bs->read32();
    
    cf.minorVersion = bs->read16();
    cf.majorVersion = bs->read16();

    cf.constantPoolCount = bs->read16();
    ConstantPoolInfo *ci;
    for (int i = 1; i < cf.constantPoolCount; i++) {
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

RefInfo *RefInfo::read(ByteStream *bs)
{
    RefInfo *mi = new RefInfo;
    
    mi->classIndex = bs->read16();
    mi->nameAndTypeIndex = bs->read16();
    return mi;
}

IndexInfo *IndexInfo::read(ByteStream *bs)
{
    IndexInfo *ri = new IndexInfo;

    ri->index = bs->read16();
    return ri;
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

Exception Exception::read(ByteStream *bs)
{
    Exception ex;

    ex.startPc = bs->read16();
    ex.endPc = bs->read16();
    ex.handlerPc = bs->read16();
    ex.catchType = bs->read16();
    return ex;
}

LineNumberTableAttribute *LineNumberTableAttribute::read(ByteStream *bs)
{
    LineNumberTableAttribute *attr = new LineNumberTableAttribute;

    attr->lineNumberTableLength = bs->read16();
    for (int i = 0; i < attr->lineNumberTableLength; i++)
        attr->lineNumberTable.push_back(LineNumber::read(bs));

    return attr;
}

LineNumber LineNumber::read(ByteStream *bs)
{
    LineNumber ln;

    ln.startPc = bs->read16();
    ln.lineNumber = bs->read16();
    return ln;
}

StackMapTableAttribute *StackMapTableAttribute::read(ByteStream *bs)
{
    StackMapTableAttribute *attr = new StackMapTableAttribute;

    attr->numberOfEntries = bs->read16();
    for (int i = 0; i < attr->numberOfEntries; i++)
        attr->entries.push_back(StackMapFrame::read(bs));
    return attr;
};

StackMapFrame *StackMapFrame::read(ByteStream *bs)
{
    StackMapFrame *frame;

    uint8_t type = bs->read8();

    if (type < 64)
        frame = SameFrame::read(bs);

    frame->frameType = type;

    return frame;
}

SameFrame *SameFrame::read(ByteStream *bs)
{
    return new SameFrame;
}

SourceFileAttribute *SourceFileAttribute::read(ByteStream *bs)
{
    SourceFileAttribute *src = new SourceFileAttribute;

    src->sourceFileIndex = bs->read16();
    return src;
}
