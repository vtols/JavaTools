#include <iostream>

#include <io/file_byte_stream.h>
#include <io/file_byte_stream_writer.h>

#include <class/java_class.h>
#include <class/java_class_builder.h>

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
                std::cout << '#' << ref->firstIndex << ':'
                        << '#' << ref->secondIndex << std::endl;
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

    for (int i = 1; i < c.constantPoolCount; i++)
        if (c.constantPool[i - 1]->tag == CONSTANT_Utf8) {
            Utf8Info *utf8 = (Utf8Info *) c.constantPool[i - 1];
            if (utf8->str == "Writing from class Sample") {
                utf8->str = "Hello world";
                break;
            }
        }

    delete f;

    FileByteStreamWriter *w;

    w = new FileByteStreamWriter("Sample.class");
    c.write(w);
    delete w;

    w = new FileByteStreamWriter("Gen.class");
    ClassBuilder cb("Gen");

    MethodBuilder *mb = cb.createMethod("main");
    mb->setDescriptor("([Ljava/lang/String;)V");
    mb->setAccessFlags(ACC_PUBLIC | ACC_STATIC);
    mb->setMax(2, 1);

    Label l;
    mb->frameSame();
    mb->insertLabel(&l);
    mb->field(opcodes::GETSTATIC,
            "java/lang/System", "out", "Ljava/io/PrintStream;");
    mb->loadString("Hello World!");
    mb->invoke(opcodes::INVOKEVIRTUAL,
            "java/io/PrintStream", "println", "(Ljava/lang/String;)V");
    mb->jump(opcodes::GOTO, &l);

    ClassFile *cf = cb.build();
    cf->write(w);

    return 0;
}
