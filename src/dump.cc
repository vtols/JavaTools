#include <iostream>

#include <file_byte_stream.h>
#include <java_class.h>

int main()
{
    std::locale::global(std::locale(""));

    FileByteStream *f = new FileByteStream("Sample.class");
    ClassFile c = ClassFile::read(f);
    std::cout << std::hex << "0x" << std::uppercase << c.magic
              << std::dec << std::endl;
    std::cout << c.majorVersion << ' '
              << c.minorVersion << ' '
              << c.constantPoolCount - 1 << std::endl;
    std::cout << "Constant pool size "
              << c.constantPoolCount << std::endl;

    for (int i = 1; i < c.constantPoolCount; i++) {
        std::cout << "#" << i << std::endl << '\t';

        ConstantPoolInfo *inf = c.constantPool[i - 1];
        RefInfo *ref;
        IndexInfo *ind;
        Utf8Info *utf8;

        switch (inf->tag) {
            case CONSTANT_Methodref:
            case CONSTANT_Fieldref:
            case CONSTANT_InterfaceMethodref:
            case CONSTANT_NameAndType:
                ref = (RefInfo *) inf;
                std::cout << '#' << ref->classIndex << ':'
                          << '#' << ref->nameAndTypeIndex << std::endl;
                break;
            case CONSTANT_String:
            case CONSTANT_Class:
                ind = (IndexInfo *) inf;
                std::cout << '#' << ind->index << std::endl;
                break;
            case CONSTANT_Utf8:
                utf8 = (Utf8Info *) inf;
                std::cout << utf8->str << std::endl;
                break;
            default:
                break;
        }
    }
    return 0;
}
